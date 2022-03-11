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
#ifndef RKSETTINGSMODULEGENERALFILES_H
#define RKSETTINGSMODULEGENERALFILES_H

#include "rksettingsmodule.h"
#include "../dialogs/startupdialog.h"

class GetFileNameWidget;
class QComboBox;
class QButtonGroup;
class RKSpinBox;

/**
@author Thomas Friedrichsmeier
*/
class RKSettingsModuleGeneral : public RKSettingsModule {
	Q_OBJECT
public:
	RKSettingsModuleGeneral (RKSettings *gui, QWidget *parent);
	~RKSettingsModuleGeneral ();

	enum WorkplaceSaveMode {	// don't change the int values of this enum, or you'll ruin users saved settings. Append new values at the end
		SaveWorkplaceWithWorkspace=0,
		SaveWorkplaceWithSession=1,
		DontSaveWorkplace=2
	};

	enum InitialDirectory {	// don't change the int values of this enum, or you'll ruin users saved settings. Append new values at the end
		CurrentDirectory=0,
		RKWardDirectory=1,
		UserHomeDirectory=2,
		LastUsedDirectory=3,
		CustomDirectory=4
	};

	enum RKMDIFocusPolicy {		// don't change the int values of this enum, or you'll ruin users saved settings. Append new values at the end
		RKMDIClickFocus=0,
		RKMDIFocusFollowsMouse=1
	};

	void applyChanges () override;
	void save(KConfig *config) override { syncConfig(config, RKConfigBase::SaveConfig); };
	static void syncConfig(KConfig *config, RKConfigBase::ConfigSyncAction a);
	static void validateSettingsInteractive (QList<RKSetupWizardItem*>*) {};

	QString caption () override;

/// returns the directory-name where the logfiles should reside
	static QString &filesPath () { return files_path; };
	static StartupDialog::Result startupAction () { return startup_action; };
	static bool showHelpOnStartup () { return show_help_on_startup; };
	static void setStartupAction (StartupDialog::Result action) { startup_action = action; };
	static WorkplaceSaveMode workplaceSaveMode () { return workplace_save_mode; };
/** retrieve the saved workplace description. Meaningful only is workplaceSaveMode () == SaveWorkplaceWithSession */
	static QString getSavedWorkplace (KConfig *config);
/** set the saved workplace description. Meaningful only is workplaceSaveMode () == SaveWorkplaceWithSession */
	static void setSavedWorkplace (const QString &description, KConfig *config);
	static bool cdToWorkspaceOnLoad () { return cd_to_workspace_dir_on_load; };
	static unsigned long warnLargeObjectThreshold () { return warn_size_object_edit; };
	static RKMDIFocusPolicy mdiFocusPolicy () { return mdi_focus_policy; }
	static QString workspaceFilenameFilter () { return ("*.RData *.RDA"); };
	static QString initialWorkingDirectory ();
/** get the url last used for @param thing, where thing is simply a key string.
    Urls are saved and restored across sessions, _except_ for the empty key, which will keep track of the last used
    "generic" url within a session (and defaults to the current working directory, while not set). */
	static QUrl lastUsedUrlFor (const QString& thing);
/** update the url used for @param thing, @see lastUsedUrlFor(). */
	static void updateLastUsedUrl (const QString& thing, const QUrl& new_path);
/** if the installation-path of KDE seems to have moved since the last startup, *and* the given path is relative to the
 *  KDE data path, adjust the given path (probably loaded from config), accordingly. @See RKCommonFunctions::getRKWardDataDir()
 *  TODO: similar, but not quite identical functionality in rkworkplace.cpp checkAdjustRestoredUrl(). Might be mergeable. */
	static QString checkAdjustLoadedPath (const QString &localpath);

	enum RKWardConfigVersion {
		RKWardConfig_Pre0_5_7,
		RKWardConfig_0_5_7,
		RKWardConfig_0_6_1,
		RKWardConfig_0_6_3,
		RKWardConfig_0_6_4,
		RKWardConfig_0_7_1,
		RKWardConfig_0_7_3,
		RKWardConfig_Next,		/**< add new configuration versions above / before this entry */
		RKWardConfig_Latest = RKWardConfig_Next - 1
	};
	/** Which version did an existing config file appear to have?
	 * @return RKWardConfig_Pre0_5_7, if no config file could be read. See anyExistingConfig()
	 * @note Not all versions of RKWard have a config file version of their own, not even necessarily when new entries were added. Only when not-quite-compatible changes are needed, new config versions will be added. */
	static RKWardConfigVersion storedConfigVersion () { return stored_config_version; };
	/** Did a config file already exist? */
	static bool anyExistingConfig () { return config_exists; };
	/** Returns true, if the runtime version of RKWard has changed since the previous session. */
	static bool rkwardVersionChanged () { return rkward_version_changed; };
	/** Returns true, if rkward seems to have started from a different path than on the previous run. */
	static bool installationMoved () { return installation_moved; };
public slots:
	void settingChanged ();
private:
	GetFileNameWidget *files_choser;
	QComboBox *startup_action_choser;
	QButtonGroup *workplace_save_chooser;
	RKSpinBox *warn_size_object_edit_box;
	QComboBox *mdi_focus_policy_chooser;
	QComboBox *initial_dir_chooser;
	GetFileNameWidget *initial_dir_custom_chooser;

	static RKConfigValue<StartupDialog::Result, int> startup_action;
	static QString files_path;
/** since changing the files_path can not easily be done while in an active session, the setting should only take effect on the next start. This string stores a changed setting, while keeping the old one intact as long as RKWard is running */
	static RKConfigValue<QString> new_files_path;
	static RKConfigValue<WorkplaceSaveMode, int> workplace_save_mode;
	static RKConfigValue<bool> cd_to_workspace_dir_on_load;
	static RKConfigValue<bool> show_help_on_startup;
	static RKConfigValue<int> warn_size_object_edit;
	static RKConfigValue<RKMDIFocusPolicy, int> mdi_focus_policy;
	static RKConfigValue<InitialDirectory, int> initial_dir;
	static RKConfigValue<QString> initial_dir_specification;
	static QString previous_rkward_data_dir;
	static RKWardConfigVersion stored_config_version;
	static bool rkward_version_changed;
	static bool config_exists;
	static bool installation_moved;
	static QUrl generic_filedialog_start_url;
};

#endif

