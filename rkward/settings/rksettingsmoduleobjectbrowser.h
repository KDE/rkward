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
	explicit RKSettingsModuleObjectBrowser(QObject *parent);
	~RKSettingsModuleObjectBrowser() override;

	void syncConfig(KConfig *config, RKConfigBase::ConfigSyncAction) override;
	QList<RKSettingsModuleWidget*> createPages(QWidget *parent) override;
	static constexpr PageId page_id = QLatin1String("browser");

	static bool isDefaultForWorkspace (RKObjectListViewSettings::PersistentSettings setting) { return workspace_settings[setting]; };
	static bool isDefaultForVarselector (RKObjectListViewSettings::PersistentSettings setting) { return varselector_settings[setting]; };
	static void setDefaultForWorkspace (RKObjectListViewSettings::PersistentSettings setting, bool state);
	static void setDefaultForVarselector (RKObjectListViewSettings::PersistentSettings setting, bool state);

	static bool isPackageBlacklisted (const QString &package_name);
private:
friend class RKSettingsPageObjectBrowser;
	static RKConfigValue<QStringList> getstructure_blacklist;

	static RKConfigValue<bool> workspace_settings[RKObjectListViewSettings::SettingsCount];
	static RKConfigValue<bool> varselector_settings[RKObjectListViewSettings::SettingsCount];
};

#endif
