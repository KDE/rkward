/***************************************************************************
                          rksettingsmodulegeneral  -  description
                             -------------------
    begin                : Fri Jul 30 2004
    copyright            : (C) 2004, 2007, 2008, 2010 by Thomas Friedrichsmeier
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
#include <qcheckbox.h>
#include <qbuttongroup.h>
#include <qgroupbox.h>
#include <qradiobutton.h>
#include <QVBoxLayout>

#include "../misc/getfilenamewidget.h"
#include "../misc/rkspinbox.h"
#include "../rkglobals.h"
#include "../debug.h"

// static members
QString RKSettingsModuleGeneral::files_path;
QString RKSettingsModuleGeneral::new_files_path;
StartupDialog::Result RKSettingsModuleGeneral::startup_action;
RKSettingsModuleGeneral::WorkplaceSaveMode RKSettingsModuleGeneral::workplace_save_mode;
bool RKSettingsModuleGeneral::show_help_on_startup;
int RKSettingsModuleGeneral::warn_size_object_edit;
RKSettingsModuleGeneral::RKMDIFocusPolicy RKSettingsModuleGeneral::mdi_focus_policy;

RKSettingsModuleGeneral::RKSettingsModuleGeneral (RKSettings *gui, QWidget *parent) : RKSettingsModule (gui, parent) {
	RK_TRACE (SETTINGS);

	QVBoxLayout *main_vbox = new QVBoxLayout (this);
	QLabel *label = new QLabel (i18n ("Settings marked with (*) do not take effect until you restart RKWard"), this);
	label->setWordWrap (true);
	main_vbox->addWidget (label);

	main_vbox->addSpacing (2*RKGlobals::spacingHint ());

	files_choser = new GetFileNameWidget (this, GetFileNameWidget::ExistingDirectory, true, i18n ("Directory where rkward may store files (*)"), QString::null, new_files_path);
	connect (files_choser, SIGNAL (locationChanged ()), this, SLOT (pathChanged ()));
	main_vbox->addWidget (files_choser);

	main_vbox->addSpacing (2*RKGlobals::spacingHint ());

	main_vbox->addWidget (new QLabel (i18n ("Startup Action (*)"), this));
	startup_action_choser = new QComboBox (this);
	startup_action_choser->setEditable (false);
	startup_action_choser->insertItem (StartupDialog::EmptyWorkspace, i18n ("Start with an empty workspace"));
	startup_action_choser->insertItem (StartupDialog::EmptyTable, i18n ("Start with an empty table"));
	startup_action_choser->insertItem (StartupDialog::ChoseFile, i18n ("Ask for a file to open"));
	startup_action_choser->insertItem (StartupDialog::NoSavedSetting, i18n ("Show selection dialog (default)"));
	startup_action_choser->setCurrentIndex (startup_action);
	connect (startup_action_choser, SIGNAL (activated (int)), this, SLOT (boxChanged (int)));
	main_vbox->addWidget (startup_action_choser);

	show_help_on_startup_box = new QCheckBox (i18n ("Show RKWard Help on Startup"), this);
	show_help_on_startup_box->setChecked (show_help_on_startup);
	connect (show_help_on_startup_box, SIGNAL (stateChanged (int)), this, SLOT (boxChanged (int)));
	main_vbox->addWidget (show_help_on_startup_box);

	main_vbox->addSpacing (2*RKGlobals::spacingHint ());

	label = new QLabel (i18n ("The workplace layout (i.e. which script-, data-, help-windows are open) may be saved (and loaded) per R workspace, or independent of the R workspace. Which do you prefer?"), this);
	label->setWordWrap (true);
	main_vbox->addWidget (label);

	workplace_save_chooser = new QButtonGroup (this);
	QGroupBox* group_box = new QGroupBox (this);
	QVBoxLayout *group_layout = new QVBoxLayout(group_box);

	QAbstractButton* button;
	button = new QRadioButton (i18n ("Save/restore with R workspace, when saving/loading R workspace"), group_box);
	group_layout->addWidget (button);
	workplace_save_chooser->addButton (button, SaveWorkplaceWithWorkspace);
	button = new QRadioButton (i18n ("Save/restore independent of R workspace (save at end of RKWard session, restore at next start)"), group_box);
	group_layout->addWidget (button);
	workplace_save_chooser->addButton (button, SaveWorkplaceWithSession);
	button = new QRadioButton (i18n ("Do not save/restore workplace layout"), group_box);
	group_layout->addWidget (button);
	workplace_save_chooser->addButton (button, DontSaveWorkplace);	
	if ((button = workplace_save_chooser->button (workplace_save_mode))) button->setChecked (true);
	connect (workplace_save_chooser, SIGNAL (buttonClicked (int)), this, SLOT (boxChanged (int)));
	main_vbox->addWidget (group_box);

	main_vbox->addSpacing (2*RKGlobals::spacingHint ());

	label = new QLabel (i18n ("Warn when editing objects with more than this number of fields (0 for no limit):"), this);
	warn_size_object_edit_box = new RKSpinBox (this);
	warn_size_object_edit_box->setIntMode (0, INT_MAX, warn_size_object_edit);
	warn_size_object_edit_box->setSpecialValueText (i18n ("No limit"));
	connect (warn_size_object_edit_box, SIGNAL (valueChanged (int)), this, SLOT (boxChanged (int)));
	main_vbox->addWidget (label);
	main_vbox->addWidget (warn_size_object_edit_box);

	main_vbox->addSpacing (2*RKGlobals::spacingHint ());

	label = new QLabel (i18n ("MDI window focus behavior"), this);
	mdi_focus_policy_chooser = new QComboBox (this);
	mdi_focus_policy_chooser->setEditable (false);
	mdi_focus_policy_chooser->insertItem (RKMDIClickFocus, i18n ("Click to focus"));
	mdi_focus_policy_chooser->insertItem (RKMDIFocusFollowsMouse, i18n ("Focus follows mouse"));
	mdi_focus_policy_chooser->setCurrentIndex (mdi_focus_policy);
	connect (mdi_focus_policy_chooser, SIGNAL (activated (int)), this, SLOT (boxChanged (int)));
	main_vbox->addWidget (label);
	main_vbox->addWidget (mdi_focus_policy_chooser);

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
	startup_action = static_cast<StartupDialog::Result> (startup_action_choser->currentIndex ());
	show_help_on_startup = show_help_on_startup_box->isChecked ();
	workplace_save_mode = static_cast<WorkplaceSaveMode> (workplace_save_chooser->checkedId ());
	warn_size_object_edit = warn_size_object_edit_box->intValue ();
	mdi_focus_policy = static_cast<RKMDIFocusPolicy> (mdi_focus_policy_chooser->currentIndex ());
}

void RKSettingsModuleGeneral::save (KConfig *config) {
	RK_TRACE (SETTINGS);
	saveSettings (config);
}

void RKSettingsModuleGeneral::saveSettings (KConfig *config) {
	RK_TRACE (SETTINGS);

	KConfigGroup cg;
	cg = config->group ("Logfiles");
	cg.writeEntry ("logfile dir", new_files_path);

	cg = config->group ("General");
	cg.writeEntry ("startup action", (int) startup_action);
	cg.writeEntry ("show help on startup", show_help_on_startup);

	cg = config->group ("Workplace");
	cg.writeEntry ("save mode", (int) workplace_save_mode);

	cg = config->group ("Editor");
	cg.writeEntry ("large object warning limit", warn_size_object_edit);

	cg = config->group ("MDI");
	cg.writeEntry ("focus policy", (int) mdi_focus_policy);
}

void RKSettingsModuleGeneral::loadSettings (KConfig *config) {
	RK_TRACE (SETTINGS);

	KConfigGroup cg;
	cg = config->group ("Logfiles");
	files_path = new_files_path = cg.readEntry ("logfile dir", QDir ().homePath () + "/.rkward/");

	cg = config->group ("General");
	startup_action = (StartupDialog::Result) cg.readEntry ("startup action", (int) StartupDialog::NoSavedSetting);
	show_help_on_startup = cg.readEntry ("show help on startup", true);

	cg = config->group ("Workplace");
	workplace_save_mode = (WorkplaceSaveMode) cg.readEntry ("save mode", (int) SaveWorkplaceWithWorkspace);

	cg = config->group ("Editor");
	warn_size_object_edit = cg.readEntry ("large object warning limit", 250000);

	cg = config->group ("MDI");
	mdi_focus_policy = (RKMDIFocusPolicy) cg.readEntry ("focus policy", (int) RKMDIClickFocus);
}

QString RKSettingsModuleGeneral::getSavedWorkplace (KConfig *config) {
	RK_TRACE (SETTINGS);

	KConfigGroup cg = config->group ("Workplace");
	return (cg.readEntry ("last saved layout", QString ()));
}

void RKSettingsModuleGeneral::setSavedWorkplace (const QString &description, KConfig *config) {
	RK_TRACE (SETTINGS);

	KConfigGroup cg = config->group ("Workplace");
	cg.writeEntry ("last saved layout", description);
}

#include "rksettingsmodulegeneral.moc"
