/*
rkpluginbrowser - This file is part of RKWard (https://rkward.kde.org). Created: Sat Mar 10 2005
SPDX-FileCopyrightText: 2005-2024 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "rkpluginbrowser.h"

#include <QVBoxLayout>
#include <QUrl>
#include <QDir>
#include <QCheckBox>

#include <KLocalizedString>

#include "../misc/xmlhelper.h"
#include "../misc/getfilenamewidget.h"

#include "../debug.h"

RKPluginBrowser::RKPluginBrowser (const QDomElement &element, RKComponent *parent_component, QWidget *parent_widget) : RKComponent (parent_component, parent_widget) {
	RK_TRACE (PLUGIN);

	// get xml-helper
	XMLHelper *xml = parent_component->xmlHelper ();

	// create and add property
	addChild ("selection", selection = new RKComponentPropertyBase (this, true));
	connect (selection, &RKComponentPropertyBase::valueChanged, this, &RKPluginBrowser::textChanged);

	setRequired (xml->getBoolAttribute (element, "required", true, DL_INFO));
	connect(requirednessProperty(), &RKComponentPropertyBase::valueChanged, this, &RKPluginBrowser::validateInput);
	connect(enablednessProperty(), &RKComponentPropertyBase::valueChanged, this, &RKPluginBrowser::validateInput);

	QVBoxLayout *vbox = new QVBoxLayout (this);
	vbox->setContentsMargins (0, 0, 0, 0);

	int intmode = xml->getMultiChoiceAttribute (element, "type", "file;dir;savefile", 0, DL_INFO);
	GetFileNameWidget::FileType mode;
	if (intmode == 0) mode = GetFileNameWidget::ExistingFile;
	else if (intmode == 1) mode = GetFileNameWidget::ExistingDirectory;
	else mode = GetFileNameWidget::SaveFile;

	only_local = !xml->getBoolAttribute (element, "allow_urls", false, DL_INFO);

	label_string = xml->i18nStringAttribute (element, "label", i18n ("Enter filename"), DL_INFO);
	selector = new GetFileNameWidget (this, mode, only_local, label_string, i18n ("Select"), xml->getStringAttribute (element, "initial", QString (), DL_INFO));
	QString filter = xml->getStringAttribute (element, "filter", QString (), DL_INFO);
	if (!filter.isEmpty ()) {
		filter.append ("\n*|All files (*)");
		selector->setFilter (filter);
	}
	connect (selector, &GetFileNameWidget::locationChanged, this, &RKPluginBrowser::textChangedFromUi);

	vbox->addWidget (selector);

	overwrite_confirm = new QCheckBox(i18n("Overwrite?"));
	connect (overwrite_confirm, &QCheckBox::toggled, this, &RKPluginBrowser::validateInput);
	vbox->addWidget (overwrite_confirm);
	overwrite_confirm->setVisible (mode == GetFileNameWidget::SaveFile);
	auto overwrite_prop = new RKComponentPropertyBool(this, false, false);
	addChild("overwrite", overwrite_prop);
	connect(overwrite_confirm, &QCheckBox::toggled, overwrite_prop, [overwrite_prop](bool checked) { if (checked != overwrite_prop->boolValue()) overwrite_prop->setBoolValue(checked); });

	validation_timer.setSingleShot (true);
	connect (&validation_timer, &QTimer::timeout, this, &RKPluginBrowser::validateInput);

	// initialize
	updating = false;
	textChangedFromUi ();
}

RKPluginBrowser::~RKPluginBrowser () {
	RK_TRACE (PLUGIN);
}

RKComponentBase::ComponentStatus RKPluginBrowser::recursiveStatus () {
	if (status == RKComponentBase::Processing) return status;
	if (isInactive () || !required) return RKComponentBase::Satisfied;
	return status;
}

void RKPluginBrowser::textChanged (RKComponentPropertyBase *) {
	RK_TRACE (PLUGIN);

	if (updating) return;
	updating = true;

	if (status != RKComponentBase::Processing) {
		status = RKComponentBase::Processing;
		changed ();
	}
	validation_timer.start (300);

	QUrl url = QUrl::fromUserInput (selection->value ().toString (), QDir::currentPath (), QUrl::AssumeLocalFile);
	if (!url.isValid ()) url = QUrl (selector->getLocation ());
	QString urlstring = only_local ? url.url (QUrl::PreferLocalFile) : url.url ();
	if (urlstring != selection->value ().toString ()) {
		// NOTE: We refuse to accept relative urls
		selection->setValue (urlstring);
	}
	selector->setLocation (url.url ());
	updateColor ();

	updating = false;
}

void RKPluginBrowser::textChangedFromUi () {
	RK_TRACE (PLUGIN);

	selection->setValue (selector->getLocation ());
}

void RKPluginBrowser::validateInput () {
	RK_TRACE (PLUGIN);

	QString tip;
	QUrl url = QUrl::fromUserInput (selection->value ().toString (), QDir::currentPath (), QUrl::AssumeLocalFile);
	RK_ASSERT (!url.isRelative ());
	if (url.isValid ()) {
		if (url.isLocalFile ()) {
			GetFileNameWidget::FileType mode = selector->getMode ();
			QFileInfo fi (url.toLocalFile ());
			if (mode == GetFileNameWidget::ExistingFile || mode == GetFileNameWidget::ExistingDirectory) {
				if (!fi.exists ()) {
					tip = i18n ("The file or directory does not exist.");
					status = RKComponentBase::Unsatisfied;
				} else if (mode == GetFileNameWidget::ExistingFile && !fi.isFile ()) {
					tip = i18n ("Only files (not directories) are acceptable, here.");
					status = RKComponentBase::Unsatisfied;
				} else if (mode == GetFileNameWidget::ExistingDirectory && !fi.isDir ()) {
					tip = i18n ("Only directories (not files) are acceptable, here.");
					status = RKComponentBase::Unsatisfied;
				} else {
					status = RKComponentBase::Satisfied;
				}
			} else {
				RK_ASSERT (mode == GetFileNameWidget::SaveFile);
				overwrite_confirm->setText (i18n ("Overwrite?"));
				overwrite_confirm->setEnabled (false);
				if (!(fi.isWritable () || (!fi.exists () && QFileInfo (fi.dir ().absolutePath ()).isWritable ()))) {
					tip = i18n ("The specified file is not writable.");
					status = RKComponentBase::Unsatisfied;
				} else if (fi.isDir ()) {
					tip = i18n ("You have to specify a filename (not directory) to write to.");
					status = RKComponentBase::Unsatisfied;
				} else if (fi.exists ()) {
					overwrite_confirm->setText ("Overwrite? (The given file already exists)");
					overwrite_confirm->setEnabled (true);
					// TODO: soft warning (icon)
					tip = i18n ("<b>Note:</b> The given file already exists, and will be modified / overwritten.");
					status = overwrite_confirm->isChecked () ? RKComponentBase::Satisfied : RKComponentBase::Unsatisfied;
				} else {
					status = RKComponentBase::Satisfied;
				}
			}
		} else {
			if (only_local) {
				tip = i18n ("Only local files are allowed, here.");
				status = RKComponentBase::Unsatisfied;
			} else {
				tip = i18n ("This url looks valid.");
				status = RKComponentBase::Satisfied;
			}
		}
	} else {
		tip = i18n ("The given filename / url is not valid.");
		status = RKComponentBase::Unsatisfied;
	}
	setToolTip (tip);
	updateColor ();
	changed ();
}

void RKPluginBrowser::updateColor () {
	RK_TRACE (PLUGIN);

	if (isEnabled ()) {
		if (status == RKComponentBase::Satisfied) {
			selector->setStyleSheet (QString (""));
		} else if (status == RKComponentBase::Processing) {
			selector->setStyleSheet (QString ("background: yellow; color: black"));
		} else {
			selector->setStyleSheet (QString ("background: red; color: black"));
		}
	} else {
		selector->setStyleSheet (QString ("background: rgb(200, 200, 200); color: black"));
	}
}

QStringList RKPluginBrowser::getUiLabelPair () const {
	RK_TRACE (PLUGIN);

	QStringList ret (label_string);
	ret.append (selection->value ().toString ());
	return ret;
}

