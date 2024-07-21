/*
rksettingsmodulekateplugins - This file is part of the RKWard project. Created: Thu Mar 26 2010
SPDX-FileCopyrightText: 2020-2024 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef RKSETTINGSMODULEKATEPLUGINS_H
#define RKSETTINGSMODULEKATEPLUGINS_H

#include "rksettingsmodule.h"

#include <QStringList>

class QTreeWidget;
namespace KTextEditor {
	class Plugin;
}

/** The settings-module for kate plugin specific settings
@author Thomas Friedrichsmeier
*/
class RKSettingsModuleKatePlugins : public RKSettingsModule {
public:
	explicit RKSettingsModuleKatePlugins(QObject *parent);
	~RKSettingsModuleKatePlugins() override;

	void syncConfig(KConfig *config, RKConfigBase::ConfigSyncAction) override;
	void createPages(RKSettings *parent) override;
	static constexpr PageId page_id = QLatin1String("kateplugins");

	static QStringList pluginsToLoad() { return plugins_to_load; };
private:
	void addPluginSettingsPages(RKSettings *parent, KTextEditor::Plugin *plugin);
friend class RKSettingsPageKatePlugins;
	static RKConfigValue<QStringList> plugins_to_load;
};

#endif
