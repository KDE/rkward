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
#include <qbuttongroup.h>
#include <qradiobutton.h>

#include "../misc/getfilenamewidget.h"
#include "../rkglobals.h"
#include "../debug.h"

// static members
QString RKSettingsModuleGeneral::files_path;
QString RKSettingsModuleGeneral::new_files_path;
StartupDialog::Result RKSettingsModuleGeneral::startup_action;
RKSettingsModuleGeneral::WorkplaceSaveMode RKSettingsModuleGeneral::workplace_save_mode;

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

	main_vbox->addSpacing (2*RKGlobals::spacingHint ());

	label = new QLabel (i18n ("The workplace layout (i.e. which script-, data-, help-windows are open) may be saved (and loaded) per R workspace, or independent of the R workspace. Which do you prefer?"), this);
	label->setAlignment (Qt::AlignAuto | Qt::AlignVCenter | Qt::ExpandTabs | Qt::WordBreak);
	main_vbox->addWidget (label);

	workplace_save_chooser = new QButtonGroup (this);
	workplace_save_chooser->setColumnLayout (0, Qt::Vertical);
	workplace_save_chooser->layout()->setSpacing (6);
	workplace_save_chooser->layout()->setMargin (11);
	QVBoxLayout *group_layout = new QVBoxLayout(workplace_save_chooser->layout());
	group_layout->addWidget (new QRadioButton (i18n ("Save/restore with R workspace, when saving/loading R workspace"), workplace_save_chooser));
	group_layout->addWidget (new QRadioButton (i18n ("Save/restore independent of R workspace (save at end of RKWard session, restore at next start)"), workplace_save_chooser));
	group_layout->addWidget (new QRadioButton (i18n ("Do not save/restore workplace layout"), workplace_save_chooser));
	workplace_save_chooser->setButton (static_cast<int> (workplace_save_mode));
	connect (workplace_save_chooser, SIGNAL (clicked (int)), this, SLOT (boxChanged (int)));
	main_vbox->addWidget (workplace_save_chooser);
	#warning option unfinished!

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

	config->setGroup ("Workplace");
	config->writeEntry ("save mode", (int) workplace_save_mode);
}

void RKSettingsModuleGeneral::loadSettings (KConfig *config) {
	RK_TRACE (SETTINGS);

	config->setGroup ("Logfiles");
	files_path = new_files_path = config->readEntry ("logfile dir", QDir ().homeDirPath () + "/.rkward/");

	config->setGroup ("General");
	startup_action = (StartupDialog::Result) config->readNumEntry ("startup action", StartupDialog::NoSavedSetting);

	config->setGroup ("Workplace");
	workplace_save_mode = (WorkplaceSaveMode) config->readNumEntry ("save mode", SaveWorkplaceWithWorkspace);
}

#include "rksettingsmodulegeneral.moc"
