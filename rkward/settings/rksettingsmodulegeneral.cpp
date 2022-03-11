/***************************************************************************
                          rksettingsmodulegeneral  -  description
                             -------------------
    begin                : Fri Jul 30 2004
    copyright            : (C) 2004-2022 by Thomas Friedrichsmeier
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
#include "rksettingsmodulegeneral.h"

#include <KLocalizedString>
#include <KSharedConfig>
#include <KConfigGroup>

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
RKConfigValue<QString> RKSettingsModuleGeneral::new_files_path { "logfile dir", QString() }; // NOTE: default initialized at runtime!
RKConfigValue<StartupDialog::Result, int> RKSettingsModuleGeneral::startup_action { "startup action", StartupDialog::NoSavedSetting };
RKConfigValue<RKSettingsModuleGeneral::WorkplaceSaveMode, int> RKSettingsModuleGeneral::workplace_save_mode { "save mode", SaveWorkplaceWithWorkspace };
RKConfigValue<bool> RKSettingsModuleGeneral::cd_to_workspace_dir_on_load {"cd to workspace on load", true};
RKConfigValue<bool> RKSettingsModuleGeneral::show_help_on_startup {"show help on startup", true};
RKConfigValue<int> RKSettingsModuleGeneral::warn_size_object_edit {"large object warning limit", 250000};
RKConfigValue<RKSettingsModuleGeneral::RKMDIFocusPolicy, int> RKSettingsModuleGeneral::mdi_focus_policy {"focus policy", RKMDIClickFocus};
RKSettingsModuleGeneral::RKWardConfigVersion RKSettingsModuleGeneral::stored_config_version;
bool RKSettingsModuleGeneral::config_exists;
RKConfigValue<RKSettingsModuleGeneral::InitialDirectory, int> RKSettingsModuleGeneral::initial_dir {"initial dir mode",
#ifndef Q_OS_WIN
	CurrentDirectory
#else
	RKWardDirectory
#endif
};
RKConfigValue<QString> RKSettingsModuleGeneral::initial_dir_specification { "initial dir spec", QString() };
bool RKSettingsModuleGeneral::rkward_version_changed;
bool RKSettingsModuleGeneral::installation_moved = false;
QString RKSettingsModuleGeneral::previous_rkward_data_dir;
QUrl RKSettingsModuleGeneral::generic_filedialog_start_url;

RKSettingsModuleGeneral::RKSettingsModuleGeneral (RKSettings *gui, QWidget *parent) : RKSettingsModule (gui, parent) {
	RK_TRACE (SETTINGS);

	QVBoxLayout *main_vbox = new QVBoxLayout (this);
	main_vbox->addWidget (RKCommonFunctions::wordWrappedLabel (i18n ("Settings marked with (*) do not take effect until you restart RKWard")));

	main_vbox->addSpacing (2*RKGlobals::spacingHint ());

	files_choser = new GetFileNameWidget (this, GetFileNameWidget::ExistingDirectory, true, i18n ("Directory where rkward may store files (*)"), QString (), new_files_path);
	connect (files_choser, &GetFileNameWidget::locationChanged, this, &RKSettingsModuleGeneral::settingChanged);
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
	startup_action_choser->setCurrentIndex (startup_action_choser->findData (startup_action.get()));
	connect (startup_action_choser, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), this, &RKSettingsModuleGeneral::settingChanged);
	main_vbox->addWidget (startup_action_choser);

	main_vbox->addWidget(show_help_on_startup.makeCheckbox(i18n("Show RKWard Help on Startup"), this));

	QGroupBox* group_box = new QGroupBox (i18n ("Initial working directory (*)"), this);
	QHBoxLayout *hlayout = new QHBoxLayout (group_box);
	initial_dir_chooser = new QComboBox (group_box);
	initial_dir_chooser->setEditable (false);
	initial_dir_chooser->addItem (i18n ("Do not change current directory on startup"), (int) CurrentDirectory);
	initial_dir_chooser->addItem (i18n ("RKWard files directory (as specified, above)"), (int) RKWardDirectory);
	initial_dir_chooser->addItem (i18n ("User home directory"), (int) UserHomeDirectory);
	initial_dir_chooser->addItem (i18n ("Last used directory"), (int) LastUsedDirectory);
	initial_dir_chooser->addItem (i18n ("The following directory (please specify):"), (int) CustomDirectory);
	initial_dir_chooser->setCurrentIndex (initial_dir_chooser->findData ((int) initial_dir));
	connect (initial_dir_chooser, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), this, &RKSettingsModuleGeneral::settingChanged);
	hlayout->addWidget (initial_dir_chooser);
	initial_dir_custom_chooser = new GetFileNameWidget (group_box, GetFileNameWidget::ExistingDirectory, true, QString(), i18n ("Initial working directory"), initial_dir_specification);
	initial_dir_custom_chooser->setEnabled (initial_dir == CustomDirectory);
	connect (initial_dir_custom_chooser, &GetFileNameWidget::locationChanged, this, &RKSettingsModuleGeneral::settingChanged);
	hlayout->addWidget (initial_dir_custom_chooser);
	RKCommonFunctions::setTips (i18n ("<p>The initial working directory to use. Note that if you are loading a workspace on startup, and you have configured RKWard to change to the directory of loaded workspaces, that directory will take precedence.</p>"), group_box, initial_dir_chooser, initial_dir_custom_chooser);
	main_vbox->addWidget (group_box);

	main_vbox->addSpacing (2*RKGlobals::spacingHint ());

	main_vbox->addWidget (RKCommonFunctions::wordWrappedLabel (i18n ("The workplace layout (i.e. which script-, data-, help-windows are open) may be saved (and loaded) per R workspace, or independent of the R workspace. Which do you prefer?")));

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
	connect (workplace_save_chooser, static_cast<void (QButtonGroup::*)(int)>(&QButtonGroup::buttonClicked), this, &RKSettingsModuleGeneral::settingChanged);
	main_vbox->addWidget (group_box);

	main_vbox->addSpacing (2*RKGlobals::spacingHint ());

	main_vbox->addWidget(cd_to_workspace_dir_on_load.makeCheckbox(i18n("When loading a workspace, change to the corresponding directory."), this));

	main_vbox->addSpacing (2*RKGlobals::spacingHint ());

	QLabel* label = new QLabel (i18n ("Warn when editing objects with more than this number of fields (0 for no limit):"), this);
	warn_size_object_edit_box = new RKSpinBox (this);
	warn_size_object_edit_box->setIntMode (0, INT_MAX, warn_size_object_edit);
	warn_size_object_edit_box->setSpecialValueText (i18n ("No limit"));
	connect (warn_size_object_edit_box, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &RKSettingsModuleGeneral::settingChanged);
	main_vbox->addWidget (label);
	main_vbox->addWidget (warn_size_object_edit_box);

	main_vbox->addSpacing (2*RKGlobals::spacingHint ());

	label = new QLabel (i18n ("MDI window focus behavior"), this);
	mdi_focus_policy_chooser = new QComboBox (this);
	mdi_focus_policy_chooser->setEditable (false);
	mdi_focus_policy_chooser->insertItem (RKMDIClickFocus, i18n ("Click to focus"));
	mdi_focus_policy_chooser->insertItem (RKMDIFocusFollowsMouse, i18n ("Focus follows mouse"));
	mdi_focus_policy_chooser->setCurrentIndex (mdi_focus_policy);
	connect (mdi_focus_policy_chooser, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), this, &RKSettingsModuleGeneral::settingChanged);
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

QUrl RKSettingsModuleGeneral::lastUsedUrlFor (const QString& thing) {
	RK_TRACE (SETTINGS);

	if (thing.isEmpty ()) {
		if (generic_filedialog_start_url.isEmpty ()) return QUrl::fromLocalFile (QDir::currentPath ());
		return generic_filedialog_start_url;
	}
	KConfigGroup cg (KSharedConfig::openConfig (), "FileDialogUrls");
	return (cg.readEntry (thing, QUrl ()));
}

void RKSettingsModuleGeneral::updateLastUsedUrl (const QString& thing, const QUrl& new_path) {
	RK_TRACE (SETTINGS);

	if (thing.isEmpty ()) {
		generic_filedialog_start_url = new_path;
	} else {
		KConfigGroup cg (KSharedConfig::openConfig (), "FileDialogUrls");
		cg.writeEntry (thing, new_path);
	}
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
	workplace_save_mode = static_cast<WorkplaceSaveMode> (workplace_save_chooser->checkedId ());
	warn_size_object_edit = warn_size_object_edit_box->intValue ();
	mdi_focus_policy = static_cast<RKMDIFocusPolicy> (mdi_focus_policy_chooser->currentIndex ());
	initial_dir = static_cast<InitialDirectory> (initial_dir_chooser->itemData (initial_dir_chooser->currentIndex ()).toInt ());
	initial_dir_specification = initial_dir_custom_chooser->getLocation ();
}

void RKSettingsModuleGeneral::syncConfig(KConfig *config, RKConfigBase::ConfigSyncAction a) {
	RK_TRACE (SETTINGS);

	if (a == RKConfigBase::LoadConfig) {
		config_exists = config->hasGroup("General");	// one of the very oldest groups in the config
	}

	KConfigGroup cg;
	cg = config->group("Logfiles");
	if (a == RKConfigBase::LoadConfig) {
		// default not yet set, first time loading config
		if (new_files_path.get().isNull()) new_files_path = QString(QDir().homePath() + "/.rkward/");

		new_files_path.loadConfig(cg);
		files_path = new_files_path;
	} else {
		new_files_path.saveConfig(cg);
	}

	cg = config->group("General");
	if (a == RKConfigBase::LoadConfig) {
		previous_rkward_data_dir = cg.readEntry("last known data dir", RKCommonFunctions::getRKWardDataDir());
		installation_moved = (previous_rkward_data_dir != RKCommonFunctions::getRKWardDataDir ()) && !previous_rkward_data_dir.isEmpty ();
	} else {
		cg.writeEntry("last known data dir", RKCommonFunctions::getRKWardDataDir());
	}
	startup_action.syncConfig(cg, a);
	show_help_on_startup.syncConfig(cg, a);
	initial_dir.syncConfig(cg, a);
	if ((a == RKConfigBase::SaveConfig) && (initial_dir = LastUsedDirectory)) {
		cg.writeEntry(initial_dir_specification.key(), QDir::currentPath());
	} else {
		initial_dir_specification.syncConfig(cg, a);
	}

	cg = config->group ("Workplace");
	workplace_save_mode.syncConfig(cg, a);
	cd_to_workspace_dir_on_load.syncConfig(cg, a);

	cg = config->group ("Editor");
	warn_size_object_edit.syncConfig(cg, a);

	cg = config->group ("MDI");
	mdi_focus_policy.syncConfig(cg, a);

	cg = config->group ("Internal");
	if (a == RKConfigBase::LoadConfig) {
		stored_config_version = (RKWardConfigVersion) cg.readEntry("config file version", (int) RKWardConfig_Pre0_5_7);
		rkward_version_changed = (cg.readEntry("previous runtime version", QString()) != RKWARD_VERSION);
	} else {
		cg.writeEntry("config file version", (int) RKWardConfig_Latest);
		cg.writeEntry("previous runtime version", QString(RKWARD_VERSION));
	}
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

QString RKSettingsModuleGeneral::checkAdjustLoadedPath (const QString& localpath) {
	RK_TRACE (SETTINGS);

	if (!installation_moved) return localpath;
	if (QUrl::fromLocalFile (previous_rkward_data_dir).isParentOf (QUrl::fromLocalFile (localpath))) { // old data path is parent of given path
		QString adjusted = QDir (previous_rkward_data_dir).relativeFilePath (localpath);
		QDir new_data_dir (RKCommonFunctions::getRKWardDataDir ());
		return QDir::cleanPath (new_data_dir.absoluteFilePath (adjusted));
	}
	return localpath;
}
