/*
rksettingsmodulekateplugins - This file is part of the RKWard project. Created: Thu Mar 26 2010
SPDX-FileCopyrightText: 2020 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef RKSETTINGSMODULEKATEPLUGINS_H
#define RKSETTINGSMODULEKATEPLUGINS_H

#include "rksettingsmodule.h"

#include <QStringList>

class QTreeWidget;

/** The settings-module for kate plugin specific settings
@author Thomas Friedrichsmeier
*/
class RKSettingsModuleKatePlugins : public RKSettingsModule {
public:
	RKSettingsModuleKatePlugins(RKSettings *gui, QWidget *parent);
	~RKSettingsModuleKatePlugins();

	void applyChanges() override;
	void save(KConfig *config) override { syncConfig(config, RKConfigBase::SaveConfig); };
	static void syncConfig(KConfig *config, RKConfigBase::ConfigSyncAction a);
	static void validateSettingsInteractive(QList<RKSetupWizardItem*>*) {};

	QString caption() const override;
	QIcon icon() const override;

	static QStringList pluginsToLoad() { return plugins_to_load; };
private:
	QTreeWidget *plugin_table;

	static RKConfigValue<QStringList> plugins_to_load;
};

#endif
