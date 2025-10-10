/*
rkpreviewbox - This file is part of RKWard (https://rkward.kde.org). Created: Wed Jan 24 2007
SPDX-FileCopyrightText: 2007-2020 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "rkpreviewbox.h"

#include <QGroupBox>
#include <QTextDocument>
#include <qlabel.h>
#include <qlayout.h>
#include <qtimer.h>

#include <KLocalizedString>

#include "../debug.h"
#include "../misc/rkcommonfunctions.h"
#include "../misc/rkxmlguipreviewarea.h"
#include "../misc/xmlhelper.h"
#include "../rbackend/rkrinterface.h"
#include "../windows/rkwindowcatcher.h"
#include "../windows/rkworkplace.h"
#include "rkstandardcomponent.h"

#define DO_PREVIEW 102

RKPreviewBox::RKPreviewBox(const QDomElement &element, RKComponent *parent_component, QWidget *parent_widget) : RKComponent(parent_component, parent_widget) {
	RK_TRACE(PLUGIN);

	manager = new RKPreviewManager(this);
	connect(manager, &RKPreviewManager::statusChanged, this, &RKPreviewBox::tryPreview);

	// get xml-helper
	XMLHelper *xml = parent_component->xmlHelper();

	preview_mode = (PreviewMode)xml->getMultiChoiceAttribute(element, QStringLiteral("mode"), QStringLiteral("plot;data;output;custom"), 0, DL_INFO);
	placement = (PreviewPlacement)xml->getMultiChoiceAttribute(element, QStringLiteral("placement"), QStringLiteral("default;attached;detached;docked"), 0, DL_INFO);
	if (placement == DefaultPreview) placement = DockedPreview;
	preview_active = xml->getBoolAttribute(element, QStringLiteral("active"), false, DL_INFO);
	idprop = RObject::rQuote(manager->previewId());

	// create and add property
	addChild(QStringLiteral("state"), state = new RKComponentPropertyBool(this, true, preview_active, QStringLiteral("active"), QStringLiteral("inactive")));
	state->setInternal(true); // restoring this does not make sense.

	// create checkbox
	QVBoxLayout *vbox = new QVBoxLayout(this);
	vbox->setContentsMargins(0, 0, 0, 0);
	toggle_preview_box = new QGroupBox(xml->i18nStringAttribute(element, QStringLiteral("label"), i18n("Preview"), DL_INFO), this);
	toggle_preview_box->setCheckable(true);
	toggle_preview_box->setAlignment(Qt::AlignLeft);
	vbox->addWidget(toggle_preview_box);
	toggle_preview_box->setChecked(preview_active);
	connect(toggle_preview_box, &QGroupBox::toggled, this, &RKPreviewBox::changedStateFromUi);

	// status label
	auto box_layout = new QVBoxLayout(toggle_preview_box);
	box_layout->addWidget(manager->inlineStatusWidget());

	// prepare placement
	placement_command = u".rk.with.window.hints ({"_s;
	placement_end = u"\n}, "_s;
	if (placement == AttachedPreview) placement_end.append(u"\"attached\""_s);
	else if (placement == DetachedPreview) placement_end.append(u"\"detached\""_s);
	else placement_end.append(u"\"\""_s);
	placement_end.append(u", "_s + idprop + u", style=\"preview\")"_s);
	if (placement == DockedPreview) {
		RKStandardComponent *uicomp = topmostStandardComponent();
		if (uicomp) {
			uicomp->addDockedPreview(state, toggle_preview_box->title(), manager);

			if (preview_mode == OutputPreview) {
				RInterface::issueCommand(QStringLiteral("local ({\n"
				                                        "outfile <- tempfile (fileext='.html')\n"
				                                        "rk.assign.preview.data(") +
				                             idprop + QStringLiteral(", list (filename=outfile, on.delete=function (id) {\n"
				                                                     "	oldfile <- rk.get.output.html.file()\n"
				                                                     "	rk.flush.output(outfile, ask=FALSE)\n"
				                                                     "	unlink(outfile)\n"
				                                                     "	rk.set.output.html.file(oldfile)\n"
				                                                     "}))\n"
				                                                     "oldfile <- rk.set.output.html.file (outfile, style='preview')  # for initialization\n"
				                                                     "rk.set.output.html.file (oldfile)\n"
				                                                     "})\n") +
				                             placement_command + u"rk.show.html(rk.get.preview.data ("_s + idprop + u")$filename)"_s + placement_end,
				                         RCommand::Plugin | RCommand::Sync);
			} else {
				// For all others, create an empty data.frame as dummy. Even for custom docked previews it has the effect of initializing the preview area with _something_.
				RInterface::issueCommand(u"local ({\nrk.assign.preview.data("_s + idprop + u", data.frame ())\n})\n"_s + placement_command + u"rk.edit(rkward::.rk.variables$.rk.preview.data[["_s + idprop + u"]])"_s + placement_end, RCommand::Plugin | RCommand::Sync);
			}

			// A bit of a hack: For now, in wizards, docked previews are always active, and control boxes are meaningless.
			if (uicomp->isWizardish()) {
				hide();
				state->setBoolValue(true);
			}
		}
	}
	connect(state, &RKComponentPropertyBase::valueChanged, this, &RKPreviewBox::changedState); // AFTER state->setBoolValue(), above!

	// find and connect to code property of the parent
	QString dummy;
	RKComponentBase *cp = parentComponent()->lookupComponent(QStringLiteral("code"), &dummy);
	if (cp && dummy.isNull() && (cp->type() == PropertyCode)) {
		code_property = static_cast<RKComponentPropertyCode *>(cp);
		connect(code_property, &RKComponentPropertyBase::valueChanged, this, &RKPreviewBox::changedCode);
	} else {
		RK_DEBUG(PLUGIN, DL_WARNING, "Could not find code property in preview box (remainder: %s)", dummy.toLatin1().data());
		code_property = nullptr;
	}

	// initialize
	update_timer = new QTimer(this);
	update_timer->setSingleShot(true);
	connect(update_timer, &QTimer::timeout, this, &RKPreviewBox::tryPreviewNow);
	updating = false;
	changedState(nullptr);
}

RKPreviewBox::~RKPreviewBox() {
	RK_TRACE(PLUGIN);

	killPreview(true);
}

QVariant RKPreviewBox::value(const QString &modifier) {
	if (modifier == QLatin1String("id")) {
		return idprop;
	}
	return (state->value(modifier));
}

void RKPreviewBox::changedState(RKComponentPropertyBase *) {
	RK_TRACE(PLUGIN);

	if (updating) return;
	updating = true;
	toggle_preview_box->setChecked(state->boolValue());
	updating = false;

	if (toggle_preview_box->isChecked()) manager->setUpdatePending();
	else manager->setPreviewDisabled();

	tryPreview();
	changed();
}

void RKPreviewBox::changedCode(RKComponentPropertyBase *) {
	RK_TRACE(PLUGIN);

	if (toggle_preview_box->isChecked()) manager->setUpdatePending();
	tryPreview();
}

void RKPreviewBox::changedStateFromUi() {
	RK_TRACE(PLUGIN);

	state->setBoolValue(toggle_preview_box->isChecked());
}

void RKPreviewBox::tryPreview() {
	RK_TRACE(PLUGIN);

	if (isEnabled() && toggle_preview_box->isChecked()) {
		update_timer->start(10);
	} else {
		killPreview();
		manager->setPreviewDisabled();
	}
}

void RKPreviewBox::tryPreviewNow() {
	RK_TRACE(PLUGIN);

	if (!code_property) return;

	ComponentStatus s = parentComponent()->recursiveStatus();
	if (s != Satisfied) {
		manager->setNoPreviewAvailable();
		if (s == Processing) tryPreview();
		return;
	}

	if (!manager->needsCommand()) return; // if the last plot is not done, yet, wait before starting the next.

	preview_active = true;

	RCommand *command;
	if (preview_mode == PlotPreview) {
		RInterface::issueCommand(placement_command + u".rk.startPreviewDevice ("_s + idprop + u')' + placement_end, RCommand::Plugin | RCommand::Sync, QString());
		// creating window generates warnings, sometimes. Don't make those part of the warnings shown for the preview -> separate command for the actual plot.
		command = new RCommand(u"local({\n"_s + code_property->preview() + u"})\n"_s, RCommand::Plugin | RCommand::Sync);
	} else if (preview_mode == DataPreview) {
		command =
		    new RCommand(u"local({try({\n"_s + code_property->preview() +
		                     u"\n})\nif(!exists(\"preview_data\",inherits=FALSE)) preview_data <- data.frame ('ERROR')\nrk.assign.preview.data("_s + idprop +
		                     u", preview_data)\n})\n"_s + placement_command + u"rk.edit(rkward::.rk.variables$.rk.preview.data[["_s + idprop + u"]])"_s + placement_end,
		                 RCommand::Plugin | RCommand::Sync);
	} else if (preview_mode == OutputPreview) {
		command = new RCommand(placement_command +
		                           u"local({\n"_s
		                           u"	oldfile <- rk.set.output.html.file(rk.get.preview.data ("_s +
		                           idprop +
		                           u")$filename, style='preview')\n"_s
		                           u"	rk.flush.output(ask=FALSE, style='preview')\n"_s
		                           u"	local({try({\n"_s +
		                           code_property->preview() +
		                           u"\n})})\n"_s // nested local to make sure "oldfile" is not overwritten.
		                           u"	rk.set.output.html.file(oldfile)\n})\n"_s
		                           u"rk.show.html(rk.get.preview.data ("_s +
		                           idprop + u")$filename)"_s + placement_end,
		                       RCommand::Plugin | RCommand::Sync);
	} else {
		command = new RCommand(u"local({\n"_s + placement_command + code_property->preview() + placement_end + u"})\n"_s, RCommand::Plugin | RCommand::Sync);
	}
	manager->setCommand(command);
}

void RKPreviewBox::killPreview(bool cleanup) {
	RK_TRACE(PLUGIN);

	if (!(preview_active || cleanup)) return;
	preview_active = false;

	if (cleanup) {
		QString command;
		if (preview_mode == PlotPreview) command = u".rk.killPreviewDevice ("_s + idprop + u")\n"_s;
		command += u"rk.discard.preview.data ("_s + idprop + u')';
		RInterface::issueCommand(command, RCommand::Plugin | RCommand::Sync);
	}
	if (placement != DockedPreview) {
		RKMDIWindow *window = RKWorkplace::mainWorkplace()->getNamedWindow(manager->previewId());
		if (window) window->deleteLater();
	}
}
