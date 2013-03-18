/***************************************************************************
                          rksettingsmodulegeneral  -  description
                             -------------------
    begin                : Fri Jul 30 2004
    copyright            : (C) 2004-2013 by Thomas Friedrichsmeier
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
#include "../misc/rkcommonfunctions.h"
#include "../rkglobals.h"
#include "../version.h"
#include "../debug.h"

// static members
QString RKSettingsModuleGeneral::files_path;
QString RKSettingsModuleGeneral::new_files_path;
StartupDialog::Result RKSettingsModuleGeneral::startup_action;
RKSettingsModuleGeneral::WorkplaceSaveMode RKSettingsModuleGeneral::workplace_save_mode;
bool RKSettingsModuleGeneral::cd_to_workspace_dir_on_load;
bool RKSettingsModuleGeneral::show_help_on_startup;
int RKSettingsModuleGeneral::warn_size_object_edit;
RKSettingsModuleGeneral::RKMDIFocusPolicy RKSettingsModuleGeneral::mdi_focus_policy;
RKSettingsModuleGeneral::RKWardConfigVersion RKSettingsModuleGeneral::stored_config_version;
bool RKSettingsModuleGeneral::config_exists;
RKSettingsModuleGeneral::InitialDirectory RKSettingsModuleGeneral::initial_dir;
QString RKSettingsModuleGeneral::initial_dir_specification;
bool RKSettingsModuleGeneral::rkward_version_changed;

RKSettingsModuleGeneral::RKSettingsModuleGeneral (RKSettings *gui, QWidget *parent) : RKSettingsModule (gui, parent) {
	RK_TRACE (SETTINGS);

	QVBoxLayout *main_vbox = new QVBoxLayout (this);
	QLabel *label = new QLabel (i18n ("Settings marked with (*) do not take effect until you restart RKWard"), this);
	label->setWordWrap (true);
	main_vbox->addWidget (label);

	main_vbox->addSpacing (2*RKGlobals::spacingHint ());

	files_choser = new GetFileNameWidget (this, GetFileNameWidget::ExistingDirectory, true, i18n ("Directory where rkward may store files (*)"), QString::null, new_files_path);
	connect (files_choser, SIGNAL (locationChanged ()), this, SLOT (settingChanged ()));
	main_vbox->addWidget (files_choser);

	main_vbox->addSpacing (2*RKGlobals::spacingHint ());

	main_vbox->addWidget (new QLabel (i18n ("Startup Action (*)"), this));
	startup_action_choser = new QComboBox (this);
	startup_action_choser->setEditable (false);
	startup_action_choser->addItem (i18n ("Start with an empty workspace"), (int) StartupDialog::EmptyWorkspace);
	startup_action_choser->addItem (i18n ("Load .RData-file from current directory, if available (R option '--restore')"), (int) StartupDialog::RestoreFromWD);
	startup_action_choser->addItem (i18n ("Start with an empty table"), (int) StartupDialog::EmptyTable);
	startup_action_choser->addItem (i18n ("Ask for a file to open"), (int) StartupDialog::ChoseFile);
	startup_action_choser->addItem (i18n ("Show selection dialog (default)"), (int) StartupDialog::NoSavedSetting);
	startup_action_choser->setCurrentIndex (startup_action_choser->findData (startup_action));
	connect (startup_action_choser, SIGNAL (activated (int)), this, SLOT (settingChanged()));
	main_vbox->addWidget (startup_action_choser);

	show_help_on_startup_box = new QCheckBox (i18n ("Show RKWard Help on Startup"), this);
	show_help_on_startup_box->setChecked (show_help_on_startup);
	connect (show_help_on_startup_box, SIGNAL (stateChanged (int)), this, SLOT (settingChanged()));
	main_vbox->addWidget (show_help_on_startup_box);

	QGroupBox* group_box = new QGroupBox (i18n ("Intial working directory (*)"), this);
	QHBoxLayout *hlayout = new QHBoxLayout (group_box);
	initial_dir_chooser = new QComboBox (group_box);
	initial_dir_chooser->setEditable (false);
	initial_dir_chooser->addItem (i18n ("Do not change current directory on startup"), (int) CurrentDirectory);
	initial_dir_chooser->addItem (i18n ("RKWard files directory (as specified, above)"), (int) RKWardDirectory);
	initial_dir_chooser->addItem (i18n ("User home directory"), (int) UserHomeDirectory);
	initial_dir_chooser->addItem (i18n ("Last used directory"), (int) LastUsedDirectory);
	initial_dir_chooser->addItem (i18n ("The following directory (please specify):"), (int) CustomDirectory);
	initial_dir_chooser->setCurrentIndex (initial_dir_chooser->findData ((int) initial_dir));
	connect (initial_dir_chooser, SIGNAL (activated(int)), this, SLOT (settingChanged()));
	hlayout->addWidget (initial_dir_chooser);
	initial_dir_custom_chooser = new GetFileNameWidget (group_box, GetFileNameWidget::ExistingDirectory, true, QString(), i18n ("Initial working directory"), initial_dir_specification);
	initial_dir_custom_chooser->setEnabled (initial_dir == CustomDirectory);
	connect (initial_dir_custom_chooser, SIGNAL (locationChanged()), this, SLOT (settingChanged()));
	hlayout->addWidget (initial_dir_custom_chooser);
	RKCommonFunctions::setTips (i18n ("<p>The initial working directory to use. Note that if you are loading a workspace on startup, and you have configured RKWard to change to the directory of loaded workspaces, that directory will take precedence.</p>"), group_box, initial_dir_chooser, initial_dir_custom_chooser);
	main_vbox->addWidget (group_box);

	main_vbox->addSpacing (2*RKGlobals::spacingHint ());

	label = new QLabel (i18n ("The workplace layout (i.e. which script-, data-, help-windows are open) may be saved (and loaded) per R workspace, or independent of the R workspace. Which do you prefer?"), this);
	label->setWordWrap (true);
	main_vbox->addWidget (label);

	workplace_save_chooser = new QButtonGroup (this);
	group_box = new QGroupBox (this);
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
	connect (workplace_save_chooser, SIGNAL (buttonClicked (int)), this, SLOT (settingChanged()));
	main_vbox->addWidget (group_box);

	main_vbox->addSpacing (2*RKGlobals::spacingHint ());

	cd_to_workspace_dir_on_load_box = new QCheckBox (i18n ("When loading a workspace, change to the corresponding directory."), this);
	cd_to_workspace_dir_on_load_box->setChecked (cd_to_workspace_dir_on_load);
	connect (cd_to_workspace_dir_on_load_box, SIGNAL (stateChanged (int)), this, SLOT (settingChanged()));
	main_vbox->addWidget (cd_to_workspace_dir_on_load_box);

	main_vbox->addSpacing (2*RKGlobals::spacingHint ());

	label = new QLabel (i18n ("Warn when editing objects with more than this number of fields (0 for no limit):"), this);
	warn_size_object_edit_box = new RKSpinBox (this);
	warn_size_object_edit_box->setIntMode (0, INT_MAX, warn_size_object_edit);
	warn_size_object_edit_box->setSpecialValueText (i18n ("No limit"));
	connect (warn_size_object_edit_box, SIGNAL (valueChanged (int)), this, SLOT (settingChanged()));
	main_vbox->addWidget (label);
	main_vbox->addWidget (warn_size_object_edit_box);

	main_vbox->addSpacing (2*RKGlobals::spacingHint ());

	label = new QLabel (i18n ("MDI window focus behavior"), this);
	mdi_focus_policy_chooser = new QComboBox (this);
	mdi_focus_policy_chooser->setEditable (false);
	mdi_focus_policy_chooser->insertItem (RKMDIClickFocus, i18n ("Click to focus"));
	mdi_focus_policy_chooser->insertItem (RKMDIFocusFollowsMouse, i18n ("Focus follows mouse"));
	mdi_focus_policy_chooser->setCurrentIndex (mdi_focus_policy);
	connect (mdi_focus_policy_chooser, SIGNAL (activated (int)), this, SLOT (settingChanged()));
	main_vbox->addWidget (label);
	main_vbox->addWidget (mdi_focus_policy_chooser);

	main_vbox->addStretch ();
}

RKSettingsModuleGeneral::~RKSettingsModuleGeneral() {
	RK_TRACE (SETTINGS);
}

QString RKSettingsModuleGeneral::initialWorkingDirectory () {
	if (initial_dir == CurrentDirectory) return QString ();
	if (initial_dir == RKWardDirectory) return filesPath ();
	if (initial_dir == UserHomeDirectory) return QDir::homePath ();
	return initial_dir_specification;
}

void RKSettingsModuleGeneral::settingChanged () {
	RK_TRACE (SETTINGS);
	int dummy = initial_dir_chooser->itemData (initial_dir_chooser->currentIndex ()).toInt ();
	initial_dir_custom_chooser->setEnabled (dummy == CustomDirectory);
	change ();
}

QString RKSettingsModuleGeneral::caption () {
	RK_TRACE (SETTINGS);
	return (i18n ("General"));
}

void RKSettingsModuleGeneral::applyChanges () {
	RK_TRACE (SETTINGS);
	new_files_path = files_choser->getLocation ();
	startup_action = static_cast<StartupDialog::Result> (startup_action_choser->itemData (startup_action_choser->currentIndex ()).toInt ());
	show_help_on_startup = show_help_on_startup_box->isChecked ();
	workplace_save_mode = static_cast<WorkplaceSaveMode> (workplace_save_chooser->checkedId ());
	cd_to_workspace_dir_on_load = cd_to_workspace_dir_on_load_box->isChecked ();
	warn_size_object_edit = warn_size_object_edit_box->intValue ();
	mdi_focus_policy = static_cast<RKMDIFocusPolicy> (mdi_focus_policy_chooser->currentIndex ());
	initial_dir = static_cast<InitialDirectory> (initial_dir_chooser->itemData (initial_dir_chooser->currentIndex ()).toInt ());
	initial_dir_specification = initial_dir_custom_chooser->getLocation ();
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
	cg.writeEntry ("initial dir mode", (int) initial_dir);
	cg.writeEntry ("initial dir spec", (initial_dir == LastUsedDirectory) ? QDir::currentPath() : initial_dir_specification);

	cg = config->group ("Workplace");
	cg.writeEntry ("save mode", (int) workplace_save_mode);
	cg.writeEntry ("cd to workspace on load", cd_to_workspace_dir_on_load);

	cg = config->group ("Editor");
	cg.writeEntry ("large object warning limit", warn_size_object_edit);

	cg = config->group ("MDI");
	cg.writeEntry ("focus policy", (int) mdi_focus_policy);

	cg = config->group ("Internal");
	cg.writeEntry ("config file version", (int) RKWardConfig_Latest);
	cg.writeEntry ("previous runtime version", QString (RKWARD_VERSION));
}

void RKSettingsModuleGeneral::loadSettings (KConfig *config) {
	RK_TRACE (SETTINGS);

	config_exists = config->hasGroup ("General");	// one of the very oldest groups in the config

	KConfigGroup cg;
	cg = config->group ("Logfiles");
	files_path = new_files_path = cg.readEntry ("logfile dir", QDir ().homePath () + "/.rkward/");

	cg = config->group ("General");
	startup_action = (StartupDialog::Result) cg.readEntry ("startup action", (int) StartupDialog::NoSavedSetting);
	show_help_on_startup = cg.readEntry ("show help on startup", true);
	initial_dir = (InitialDirectory) cg.readEntry ("initial dir mode",
#ifndef Q_WS_WIN
		(int) CurrentDirectory
#else
		(int) RKWardDirectory
#endif
	);
	initial_dir_specification = cg.readEntry ("initial dir spec", QString ());

	cg = config->group ("Workplace");
	workplace_save_mode = (WorkplaceSaveMode) cg.readEntry ("save mode", (int) SaveWorkplaceWithWorkspace);
	cd_to_workspace_dir_on_load = cg.readEntry ("cd to workspace on load", true);

	cg = config->group ("Editor");
	warn_size_object_edit = cg.readEntry ("large object warning limit", 250000);

	cg = config->group ("MDI");
	mdi_focus_policy = (RKMDIFocusPolicy) cg.readEntry ("focus policy", (int) RKMDIClickFocus);

	cg = config->group ("Internal");
	stored_config_version = (RKWardConfigVersion) cg.readEntry ("config file version", (int) RKWardConfig_Pre0_5_7);
	rkward_version_changed = (cg.readEntry ("previous runtime version", QString ()) != RKWARD_VERSION);
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
