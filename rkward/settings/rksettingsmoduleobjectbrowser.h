/*
rksettingsmodule - This file is part of the RKWard project. Created: Fri Apr 22 2005
SPDX-FileCopyrightText: 2005-2018 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef RKSETTINGSMODULEOBJECTBROWSER_H
#define RKSETTINGSMODULEOBJECTBROWSER_H

#include "rksettingsmodule.h"

#include "../misc/rkobjectlistview.h"

class QCheckBox;
class MultiStringSelector;

/** Configuration module for instances of RObjectListView
@see RKSettingsModule
@author Thomas Friedrichsmeier
*/
class RKSettingsModuleObjectBrowser : public RKSettingsModule {
	Q_OBJECT
public:
	RKSettingsModuleObjectBrowser (RKSettings *gui, QWidget *parent);
	~RKSettingsModuleObjectBrowser ();

/** applies current settings in this RKSettingsModule. This will only be called, if hasChanges () is true */
	void applyChanges () override;

/** @returns the caption ("Workspace Browser") */
	QString caption() const override;
	QIcon icon() const override;

	void save(KConfig *config) override { syncConfig(config, RKConfigBase::SaveConfig); };
	static void syncConfig(KConfig *config, RKConfigBase::ConfigSyncAction a);
	static void validateSettingsInteractive (QList<RKSetupWizardItem*>*) {};

	static bool isDefaultForWorkspace (RKObjectListViewSettings::PersistentSettings setting) { return workspace_settings[setting]; };
	static bool isDefaultForVarselector (RKObjectListViewSettings::PersistentSettings setting) { return varselector_settings[setting]; };
	static void setDefaultForWorkspace (RKObjectListViewSettings::PersistentSettings setting, bool state);
	static void setDefaultForVarselector (RKObjectListViewSettings::PersistentSettings setting, bool state);

	static bool isPackageBlacklisted (const QString &package_name);

	QUrl helpURL () override { return QUrl ("rkward://page/rkward_workspace_browser#settings"); };
public Q_SLOTS:
/** called when a checkbox has been changed. Signals change to RKSettings dialog to enable apply button */
	void boxChanged (int);
	void listChanged ();
	void addBlackList (QStringList *string_list);
private:
	MultiStringSelector *blacklist_choser;
	static RKConfigValue<QStringList> getstructure_blacklist;

	static RKConfigValue<bool> workspace_settings[RKObjectListViewSettings::SettingsCount];
	static RKConfigValue<bool> varselector_settings[RKObjectListViewSettings::SettingsCount];
};

#endif
