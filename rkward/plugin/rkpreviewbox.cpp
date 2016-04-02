/***************************************************************************
                          rkpreviewbox  -  description
                             -------------------
    begin                : Wed Jan 24 2007
    copyright            : (C) 2007-2016 by Thomas Friedrichsmeier
    email                : thomas.friedrichsmeier@kdemail.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "rkpreviewbox.h"

#include <qlayout.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qtimer.h>
#include <QTextDocument>

#include <klocale.h>
#include <kvbox.h>

#include "../rkglobals.h"
#include "../rbackend/rinterface.h"
#include "../misc/xmlhelper.h"
#include "../misc/rkxmlguipreviewarea.h"
#include "../windows/rkwindowcatcher.h"
#include "../windows/rkworkplace.h"
#include "rkstandardcomponent.h"
#include "../debug.h"

#define DO_PREVIEW 102

RKPreviewBox::RKPreviewBox (const QDomElement &element, RKComponent *parent_component, QWidget *parent_widget) : RKComponent (parent_component, parent_widget) {
	RK_TRACE (PLUGIN);

	prior_preview_done = true;
	new_preview_pending = false;

	// get xml-helper
	XMLHelper *xml = parent_component->xmlHelper ();

	preview_mode = (PreviewMode) xml->getMultiChoiceAttribute (element, "mode", "plot;data;output;custom", 0, DL_INFO);
	placement = (PreviewPlacement) xml->getMultiChoiceAttribute (element, "placement", "default;attached;detached;docked", 0, DL_INFO);
	if (placement == DefaultPreview) placement = DockedPreview;
	preview_active = xml->getBoolAttribute (element, "active", false, DL_INFO);
	idprop = RObject::rQuote (QString ().sprintf ("%p", this));

	// create and add property
	addChild ("state", state = new RKComponentPropertyBool (this, true, preview_active, "active", "inactive"));
	state->setInternal (true);	// restoring this does not make sense.
	connect (state, SIGNAL (valueChanged(RKComponentPropertyBase*)), this, SLOT (changedState(RKComponentPropertyBase*)));

	// create checkbox
	QVBoxLayout *vbox = new QVBoxLayout (this);
	vbox->setContentsMargins (0, 0, 0, 0);
	toggle_preview_box = new QCheckBox (xml->i18nStringAttribute (element, "label", i18n ("Preview"), DL_INFO), this);
	vbox->addWidget (toggle_preview_box);
	toggle_preview_box->setChecked (preview_active);
	connect (toggle_preview_box, SIGNAL (stateChanged(int)), this, SLOT (changedState(int)));

	// status label
	status_label = new QLabel (QString (), this);
	status_label->setWordWrap (true);
	vbox->addWidget (status_label);

	// prepare placement
	placement_command = ".rk.with.window.hints ({";
	placement_end = "\n}, ";
	if (placement == AttachedPreview) placement_end.append ("\"attached\"");
	else if (placement == DetachedPreview) placement_end.append ("\"detached\"");
	else placement_end.append ("\"\"");
	placement_end.append (", " + RObject::rQuote (idprop) + ", style=\"preview\")");
	if (placement == DockedPreview) {
		RKStandardComponent *uicomp = topmostStandardComponent ();
		if (uicomp) {
			uicomp->addDockedPreview (state, toggle_preview_box->text (), idprop);

			if (preview_mode == OutputPreview) {
				RKGlobals::rInterface ()->issueCommand ("local ({\n"
				    "outfile <- tempfile (fileext='html')\n"
				    "rk.assign.preview.data(" + idprop + ", list (filename=outfile, on.delete=function (id) {\n"
				    "	rk.flush.output (outfile, ask=FALSE)\n"
				    "	unlink (outfile)\n"
				    "}))\n"
				    "oldfile <- rk.set.output.html.file (outfile, style='preview')  # for initialization\n"
				    "rk.set.output.html.file (oldfile)\n"
				    "})\n" + placement_command + "rk.show.html(rk.get.preview.data (" + idprop + ")$filename)" + placement_end, RCommand::Plugin | RCommand::Sync);
			} else {
				// For all others, create an empty data.frame as dummy. Even for custom docked previews it has the effect of initializing the preview area with _something_.
				RKGlobals::rInterface ()->issueCommand ("local ({\nrk.assign.preview.data(" + idprop + ", data.frame ())\n})\n" + placement_command + "rk.edit(rkward::.rk.variables$.rk.preview.data[[" + idprop + "]])" + placement_end, RCommand::Plugin | RCommand::Sync);
			}

			// A bit of a hack: For now, in wizards, docked previews are always active, and control boxes are meaningless.
			if (uicomp->isWizardish ()) {
				hide ();
				toggle_preview_box->setChecked (true);
			}
		}
	}

	// find and connect to code property of the parent
	QString dummy;
	RKComponentBase *cp = parentComponent ()->lookupComponent ("code", &dummy);
	if (cp && dummy.isNull () && (cp->type () == PropertyCode)) {
		code_property = static_cast<RKComponentPropertyCode *> (cp);
		connect (code_property, SIGNAL (valueChanged(RKComponentPropertyBase*)), this, SLOT (changedCode(RKComponentPropertyBase*)));
	} else {
		RK_DEBUG (PLUGIN, DL_WARNING, "Could not find code property in preview box (remainder: %s)", dummy.toLatin1().data ());
		code_property = 0;
	}

	// initialize
	update_timer = new QTimer (this);
	update_timer->setSingleShot (true);
	connect (update_timer, SIGNAL (timeout()), this, SLOT (tryPreviewNow()));
	updating = false;
	changedState (0);
}

RKPreviewBox::~RKPreviewBox () {
	RK_TRACE (PLUGIN);

	killPreview (true);
}

QVariant RKPreviewBox::value(const QString& modifier) {
	if (modifier == "id") {
		return idprop;
	}
	return (state->value (modifier));
}


void RKPreviewBox::changedState (RKComponentPropertyBase *) {
	RK_TRACE (PLUGIN);

	if (updating) return;
	updating = true;
	toggle_preview_box->setChecked (state->boolValue ());
	updating = false;

	tryPreview ();
	changed ();
}

void RKPreviewBox::changedCode (RKComponentPropertyBase *) {
	RK_TRACE (PLUGIN);

	tryPreview ();
}

void RKPreviewBox::changedState (int) {
	RK_TRACE (PLUGIN);

	state->setBoolValue (toggle_preview_box->isChecked ());
}

void RKPreviewBox::tryPreview () {
	RK_TRACE (PLUGIN);

	if (isEnabled () && toggle_preview_box->isChecked ()) update_timer->start (10);
	else killPreview ();

	updateStatusLabel ();
}

void RKPreviewBox::tryPreviewNow () {
	RK_TRACE (PLUGIN);

	if (!code_property) return;
	ComponentStatus s = parentComponent ()->recursiveStatus ();
	if (s != Satisfied) {
		if (s == Processing) tryPreview ();
		else {
			setStatusMessage (i18n ("Preview not (currently) possible"));
		}
		return;
	}

	if (!prior_preview_done) {		// if the last plot is not done, yet, wait before starting the next.
		new_preview_pending = true;
		updateStatusLabel ();
		return;
	}

	preview_active = true;

	setStatusMessage (i18n ("Preview updating"));
	if (preview_mode == PlotPreview) {
		RKGlobals::rInterface ()->issueCommand (placement_command + ".rk.startPreviewDevice (" + idprop + ')' + placement_end, RCommand::Plugin | RCommand::Sync, QString ());
		// creating window generates warnings, sometimes. Don't make those part of the warnings shown for the preview -> separate command for the actual plot.
		RKGlobals::rInterface ()->issueCommand ("local({\n" + code_property->preview () + "})\n", RCommand::Plugin | RCommand::Sync, QString (), this, DO_PREVIEW);
	} else if (preview_mode == DataPreview) {
		RKGlobals::rInterface ()->issueCommand ("local({try({\n" + code_property->preview () + "\n})\nif(!exists(\"preview_data\",inherits=FALSE)) preview_data <- data.frame ('ERROR')\nrk.assign.preview.data(" + idprop + ", preview_data)\n})\n" + placement_command + "rk.edit(rkward::.rk.variables$.rk.preview.data[[" + idprop + "]])" + placement_end, RCommand::Plugin | RCommand::Sync, QString (), this, DO_PREVIEW);
	} else if (preview_mode == OutputPreview) {
		RKGlobals::rInterface ()->issueCommand (placement_command + "local({\n"
		    "	oldfile <- rk.set.output.html.file(rk.get.preview.data (" + idprop + ")$filename, style='preview')\n"
		    "	rk.flush.output(ask=FALSE, style='preview')\n"
		    "	local({try({\n" + code_property->preview () + "\n})})\n"  // nested local to make sure "oldfile" is not overwritten.
		    "	rk.set.output.html.file(oldfile)\n})\n"
		    "rk.show.html(rk.get.preview.data (" + idprop + ")$filename)" + placement_end, RCommand::Plugin | RCommand::Sync, QString (), this, DO_PREVIEW);
	} else {
		RKGlobals::rInterface ()->issueCommand ("local({\n" + placement_command + code_property->preview () + placement_end + "})\n", RCommand::Plugin | RCommand::Sync, QString (), this, DO_PREVIEW);
	}

	prior_preview_done = false;
	new_preview_pending = false;

	updateStatusLabel ();
}

void RKPreviewBox::setStatusMessage(const QString& status) {
	RK_TRACE (PLUGIN);

	RKMDIWindow *window = RKWorkplace::mainWorkplace ()->getNamedWindow (idprop);
	if (!window) return;
	window->setStatusMessage (status);
}

void RKPreviewBox::killPreview (bool cleanup) {
	RK_TRACE (PLUGIN);

	if (!preview_active) return;
	preview_active = false;

	if (cleanup) {
		QString command;
		if (preview_mode == PlotPreview) command = ".rk.killPreviewDevice (" + idprop + ')';
		else command = "rk.discard.preview.data (" + idprop + ')';
		RKGlobals::rInterface ()->issueCommand (command, RCommand::Plugin | RCommand::Sync);
	}
	if (placement != DockedPreview) {
		RKMDIWindow *window =  RKWorkplace::mainWorkplace ()->getNamedWindow (idprop);
		if (window) window->deleteLater ();
	}

	prior_preview_done = true;
	new_preview_pending = false;
}

void RKPreviewBox::rCommandDone (RCommand *command) {
	RK_TRACE (PLUGIN);

	prior_preview_done = true;
	if (new_preview_pending) tryPreview ();

	QString warnings = command->warnings () + command->error ();
	if (!warnings.isEmpty ()) warnings = QString ("<b>%1</b>\n<pre>%2</pre>").arg (i18n ("Warnings or Errors:")).arg (Qt::escape (warnings));
	setStatusMessage (warnings);

	updateStatusLabel ();
}

void RKPreviewBox::updateStatusLabel () {
	RK_TRACE (PLUGIN);

	if (!toggle_preview_box->isChecked ()) {
		status_label->setText (i18n ("Preview disabled"));
	} else {
		if (parentComponent ()->isSatisfied ()) {
			if (prior_preview_done && (!new_preview_pending)) {
				status_label->setText (i18n ("Preview up to date"));
			} else {
				status_label->setText (i18n ("Preview updating"));
			}
		} else {
			status_label->setText (i18n ("Preview not (yet) possible"));
		}
	}
}

#include "rkpreviewbox.moc"
