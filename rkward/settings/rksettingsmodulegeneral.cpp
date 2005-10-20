/***************************************************************************
                          rksettingsmodulegeneral  -  description
                             -------------------
    begin                : Fri Jul 30 2004
    copyright            : (C) 2004 by Thomas Friedrichsmeier
    email                : tfry@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "rksettingsmodulegeneral.h"

#include <klocale.h>
#include <kconfig.h>
#include <kglobal.h>
#include <kstandarddirs.h>

#include <qlayout.h>
#include <qlabel.h>
#include <qdir.h>
#include <qcombobox.h>

#include "../misc/getfilenamewidget.h"
#include "../rkglobals.h"
#include "../debug.h"

// static members
QString RKSettingsModuleGeneral::files_path;
QString RKSettingsModuleGeneral::new_files_path;
StartupDialog::Result RKSettingsModuleGeneral::startup_action;

RKSettingsModuleGeneral::RKSettingsModuleGeneral (RKSettings *gui, QWidget *parent) : RKSettingsModule (gui, parent) {
	RK_TRACE (SETTINGS);

	QVBoxLayout *main_vbox = new QVBoxLayout (this, RKGlobals::marginHint ());
	QLabel *label = new QLabel (i18n ("Settings marked with (*) do not take effect until you restart RKWard"), this);
	label->setAlignment (Qt::AlignAuto | Qt::AlignVCenter | Qt::ExpandTabs | Qt::WordBreak);
	main_vbox->addWidget (label);

	files_choser = new GetFileNameWidget (this, GetFileNameWidget::ExistingDirectory, i18n ("Directory where the logfiles should be kept (*)"), QString::null, new_files_path);
	connect (files_choser, SIGNAL (locationChanged ()), this, SLOT (pathChanged ()));
	main_vbox->addWidget (files_choser);

	main_vbox->addWidget (new QLabel (i18n ("Startup Action (*)"), this));
	startup_action_choser = new QComboBox (false, this);
	startup_action_choser->insertItem (i18n ("Start with an empty workspace"), StartupDialog::EmptyWorkspace);
	startup_action_choser->insertItem (i18n ("Start with an empty table"), StartupDialog::EmptyTable);
	startup_action_choser->insertItem (i18n ("Ask for a file to open"), StartupDialog::ChoseFile);
	startup_action_choser->insertItem (i18n ("Show selection dialog (default)"), StartupDialog::NoSavedSetting);
	startup_action_choser->setCurrentItem (startup_action);
	connect (startup_action_choser, SIGNAL (activated (int)), this, SLOT (boxChanged (int)));
	main_vbox->addWidget (startup_action_choser);

	main_vbox->addStretch ();
}

RKSettingsModuleGeneral::~RKSettingsModuleGeneral() {
	RK_TRACE (SETTINGS);
}

void RKSettingsModuleGeneral::pathChanged () {
	RK_TRACE (SETTINGS);
	change ();
}

void RKSettingsModuleGeneral::boxChanged (int) {
	RK_TRACE (SETTINGS);
	change ();
}

QString RKSettingsModuleGeneral::caption () {
	RK_TRACE (SETTINGS);
	return (i18n ("General"));
}

bool RKSettingsModuleGeneral::hasChanges () {
	RK_TRACE (SETTINGS);
	return changed;
}

void RKSettingsModuleGeneral::applyChanges () {
	RK_TRACE (SETTINGS);
	new_files_path = files_choser->getLocation ();
	startup_action = (StartupDialog::Result) startup_action_choser->currentItem ();
}

void RKSettingsModuleGeneral::save (KConfig *config) {
	RK_TRACE (SETTINGS);
	saveSettings (config);
}

void RKSettingsModuleGeneral::saveSettings (KConfig *config) {
	RK_TRACE (SETTINGS);

	config->setGroup ("Logfiles");
	config->writeEntry ("logfile dir", new_files_path);

	config->setGroup ("General");
	config->writeEntry ("startup action", (int) startup_action);
}

void RKSettingsModuleGeneral::loadSettings (KConfig *config) {
	RK_TRACE (SETTINGS);

	config->setGroup ("Logfiles");
	files_path = new_files_path = config->readEntry ("logfile dir", QDir ().homeDirPath () + "/.rkward/");

	config->setGroup ("General");
	startup_action = (StartupDialog::Result) config->readNumEntry ("startup action", StartupDialog::NoSavedSetting);
}

#include "rksettingsmodulegeneral.moc"
