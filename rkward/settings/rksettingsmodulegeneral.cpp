/*
rksettingsmodulegeneral - This file is part of RKWard (https://rkward.kde.org). Created: Fri Jul 30 2004
SPDX-FileCopyrightText: 2004-2024 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
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
#include "../misc/rkcommandlineargs.h"
#include "../misc/rkstandardicons.h"
#include "../misc/rkstyle.h"

#include "../version.h"
#include "../debug.h"

// static members
QString RKSettingsModuleGeneral::files_path;
RKConfigValue<QString> RKSettingsModuleGeneral::new_files_path { "logfile dir", QString() }; // NOTE: default initialized at runtime!
RKConfigValue<bool> RKSettingsModuleGeneral::autorestore_from_wd { "autorestore from wd", false };
RKConfigValue<RKSettingsModuleGeneral::WorkplaceSaveMode, int> RKSettingsModuleGeneral::workplace_save_mode { "save mode", SaveWorkplaceWithWorkspace };
RKConfigValue<bool> RKSettingsModuleGeneral::cd_to_workspace_dir_on_load {"cd to workspace on load", true};
RKConfigValue<bool> RKSettingsModuleGeneral::show_help_on_startup {"show help on startup", true};
RKConfigValue<int> RKSettingsModuleGeneral::warn_size_object_edit {"large object warning limit", 250000};
RKConfigValue<RKSettingsModuleGeneral::RKMDIFocusPolicy, int> RKSettingsModuleGeneral::mdi_focus_policy {"focus policy", RKMDIClickFocus};
RKSettingsModuleGeneral::RKWardConfigVersion RKSettingsModuleGeneral::stored_config_version;
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
RKConfigValue<int> RKSettingsModuleGeneral::num_recent_files { "Max number of recent files", 8 };

RKSettingsModuleGeneral::RKSettingsModuleGeneral (RKSettings *gui, QWidget *parent) : RKSettingsModule (gui, parent) {
	RK_TRACE (SETTINGS);

	QVBoxLayout *main_vbox = new QVBoxLayout (this);
	files_choser = new GetFileNameWidget (this, GetFileNameWidget::ExistingDirectory, true, i18n ("Directory where rkward may store files (setting takes effect after restarting RKWard)"), QString (), new_files_path);
	connect (files_choser, &GetFileNameWidget::locationChanged, this, &RKSettingsModuleGeneral::change);
	main_vbox->addWidget (files_choser);

	main_vbox->addSpacing (2*RKStyle::spacingHint ());

	auto group = new QGroupBox(i18n("Startup behavior"));
	auto vbox = new QVBoxLayout(group);
	vbox->addWidget(autorestore_from_wd.makeCheckbox(i18n("Load .RData-file from startup directory, if available (R option '--restore')"), this));
	vbox->addWidget(show_help_on_startup.makeCheckbox(i18n("Show RKWard Help on Startup"), this));

	QGroupBox* group_box = new QGroupBox (i18n ("Initial working directory"), this);
	QHBoxLayout *hlayout = new QHBoxLayout (group_box);
	auto initial_dir_chooser = initial_dir.makeDropDown(RKConfigBase::LabelList(
		{{CurrentDirectory, i18n("Do not change current directory on startup")}, {RKWardDirectory, i18n("RKWard files directory (as specified, above)")}, {UserHomeDirectory, i18n("User home directory")}, {LastUsedDirectory, i18n("Last used directory")}, {CustomDirectory, i18n("The following directory (please specify):")}}
	), this);
	hlayout->addWidget (initial_dir_chooser);
	initial_dir_custom_chooser = new GetFileNameWidget (group_box, GetFileNameWidget::ExistingDirectory, true, QString(), i18n ("Initial working directory"), initial_dir_specification);
	initial_dir_custom_chooser->setEnabled (initial_dir == CustomDirectory);
	connect (initial_dir_custom_chooser, &GetFileNameWidget::locationChanged, this, &RKSettingsModuleGeneral::change);
	connect(initial_dir_chooser, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [initial_dir_chooser, this]() { this->initial_dir_custom_chooser->setEnabled(initial_dir_chooser->currentData()==CustomDirectory); });
	hlayout->addWidget (initial_dir_custom_chooser);
	RKCommonFunctions::setTips (i18n ("<p>The initial working directory to use. Note that if you are loading a workspace on startup, and you have configured RKWard to change to the directory of loaded workspaces, that directory will take precedence.</p>"), group_box, initial_dir_chooser, initial_dir_custom_chooser);
	vbox->addWidget (group_box);
	main_vbox->addWidget(group);

	main_vbox->addSpacing (2*RKStyle::spacingHint ());

	auto num_recent_files_box = num_recent_files.makeSpinBox(1, INT_MAX, this);
	RKCommonFunctions::setTips (i18n ("<p>The number of recent files to remember (in the Open Recent R Script File menu).</p>") + RKCommonFunctions::noteSettingsTakesEffectAfterRestart (), num_recent_files_box, num_recent_files_box);
	vbox->addWidget(new QLabel(i18n("Maximum number of recently used files to remember per category")));
	vbox->addWidget(num_recent_files_box);

	main_vbox->addSpacing (2*RKStyle::spacingHint ());

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
	connect (workplace_save_chooser, &QButtonGroup::idClicked, this, &RKSettingsModuleGeneral::change);
	main_vbox->addWidget (group_box);

	main_vbox->addSpacing (2*RKStyle::spacingHint ());

	main_vbox->addWidget(cd_to_workspace_dir_on_load.makeCheckbox(i18n("When loading a workspace, change to the corresponding directory."), this));

	main_vbox->addSpacing (2*RKStyle::spacingHint ());

	main_vbox->addWidget (new QLabel(i18n("Warn when editing objects with more than this number of fields (0 for no limit):")));
	main_vbox->addWidget (warn_size_object_edit.makeSpinBox(0, INT_MAX, this));

	main_vbox->addSpacing (2*RKStyle::spacingHint ());

	main_vbox->addWidget(new QLabel(i18n("MDI window focus behavior"), this));
	auto mdi_focus_policy_chooser = mdi_focus_policy.makeDropDown(RKConfigBase::LabelList(
		{{RKMDIClickFocus, i18n("Click to focus")}, {RKMDIFocusFollowsMouse, i18n("Focus follows mouse")}}
	), this);
	main_vbox->addWidget(mdi_focus_policy_chooser);

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

QString RKSettingsModuleGeneral::caption() const {
	RK_TRACE(SETTINGS);
	return(i18n("General"));
}

QIcon RKSettingsModuleGeneral::icon() const {
	RK_TRACE(SETTINGS);
	return RKStandardIcons::getIcon(RKStandardIcons::RKWardIcon);
}

void RKSettingsModuleGeneral::applyChanges () {
	RK_TRACE (SETTINGS);
	new_files_path = files_choser->getLocation ();
	workplace_save_mode = static_cast<WorkplaceSaveMode> (workplace_save_chooser->checkedId ());
	initial_dir_specification = initial_dir_custom_chooser->getLocation ();
}

void RKSettingsModuleGeneral::syncConfig(KConfig *config, RKConfigBase::ConfigSyncAction a) {
	RK_TRACE (SETTINGS);

	KConfigGroup cg;
	bool config_exists = config->hasGroup("General");	// one of the very oldest groups in the config
	cg = config->group ("Internal");
	if (a == RKConfigBase::LoadConfig) {
		stored_config_version = (RKWardConfigVersion) cg.readEntry("config file version", (int) RKWardConfig_Pre0_5_7_Obsolete);
		if (config_exists && stored_config_version < RKWardConfig_0_6_3) {
			QStringList groups = config->groupList();
			for (int i = 0; i < groups.size(); ++i) {
				config->deleteGroup(groups[i]);
			}
		}
		rkward_version_changed = (cg.readEntry("previous runtime version", QString()) != RKWARD_VERSION) || RKCommandLineArgs::get(RKCommandLineArgs::Setup).toBool();
	} else {
		cg.writeEntry("config file version", (int) RKWardConfig_Latest);
		cg.writeEntry("previous runtime version", QString(RKWARD_VERSION));
	}

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
	autorestore_from_wd.syncConfig(cg, a);
	show_help_on_startup.syncConfig(cg, a);
	num_recent_files.syncConfig(cg, a);
	initial_dir.syncConfig(cg, a);
	if ((a == RKConfigBase::SaveConfig) && (initial_dir == LastUsedDirectory)) {
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

	if ((a == RKConfigBase::LoadConfig) && (stored_config_version < RKWardConfig_0_7_4)) {
		show_help_on_startup = true;
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
