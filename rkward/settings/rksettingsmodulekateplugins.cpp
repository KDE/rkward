/***************************************************************************
                          rksettingsmodulekateplugins  -  description
                             -------------------
    begin                : Thu Mar 26 2010
    copyright            : (C) 2020 by Thomas Friedrichsmeier
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

#include "rksettingsmodulekateplugins.h"

#include <QTreeWidget>
#include <QVBoxLayout>
#include <QLabel>

#include <KPluginMetaData>
#include <KLocalizedString>
#include <KConfigGroup>
#include <KConfig>

#include "../windows/katepluginintegration.h"
#include "../misc/rkcommonfunctions.h"
#include "../rkward.h"

#include "../debug.h"

QStringList RKSettingsModuleKatePlugins::plugins_to_load;

RKSettingsModuleKatePlugins::RKSettingsModuleKatePlugins(RKSettings *gui, QWidget *parent) : RKSettingsModule(gui, parent) {
	RK_TRACE(SETTINGS);

	QVBoxLayout *vbox = new QVBoxLayout(this);
	vbox->setContentsMargins(0, 0, 0, 0);
	vbox->addWidget(RKCommonFunctions::wordWrappedLabel(i18n("Kate plugins to load in RKWard. Note that some loaded plugins will not become visible until certain conditions are met, e.g. you are loading a version controlled file for the <i>Project</i> plugin. Also, many of the plugins listed here do not make a whole lot of sense in the context of RKWard.")));

	plugin_table = new QTreeWidget();
	plugin_table->setHeaderLabels(QStringList() << QString() << i18n("Name") << i18n("Description"));
	KatePluginIntegrationApp *pluginapp = RKWardMainWindow::getMain()->katePluginIntegration();
	foreach (const QString &key, pluginapp->known_plugins.keys()) {
		QTreeWidgetItem *item = new QTreeWidgetItem();
		KPluginMetaData plugindata = pluginapp->known_plugins.value(key).data;
		item->setData(1, Qt::DisplayRole, plugindata.name());
		item->setData(2, Qt::DisplayRole, plugindata.description());
		item->setData(1, Qt::DecorationRole, QIcon::fromTheme(plugindata.iconName()));
		item->setData(1, Qt::UserRole, key);
		item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsUserCheckable);
		item->setCheckState(0, plugins_to_load.contains(key) ? Qt::Checked : Qt::Unchecked);
		plugin_table->addTopLevelItem(item);
	}
	plugin_table->resizeColumnToContents(0);
	plugin_table->resizeColumnToContents(1);
	plugin_table->sortItems(1, Qt::AscendingOrder);
	vbox->addWidget(plugin_table);

	connect(plugin_table, &QTreeWidget::itemChanged, this, &RKSettingsModuleKatePlugins::change);
}

RKSettingsModuleKatePlugins::~RKSettingsModuleKatePlugins() {
	RK_TRACE(SETTINGS);
}

void RKSettingsModuleKatePlugins::applyChanges() {
	RK_TRACE(SETTINGS);

	plugins_to_load.clear();
	for (int i = plugin_table->topLevelItemCount() - 1; i >= 0; --i) {
		QTreeWidgetItem *item = plugin_table->topLevelItem(i);
		if (item->checkState(0) == Qt::Checked) {
			plugins_to_load.append (item->data(1, Qt::UserRole).toString());
		}
	}
	RKWardMainWindow::getMain()->katePluginIntegration()->loadPlugins(plugins_to_load);
}

void RKSettingsModuleKatePlugins::save(KConfig *config) {
	RK_TRACE(SETTINGS);

	saveSettings(config);
}

void RKSettingsModuleKatePlugins::saveSettings(KConfig *config) {
	RK_TRACE(SETTINGS);

	KConfigGroup cg = config->group("Kate Plugins");
	cg.writeEntry("Plugins to load", plugins_to_load);
}

void RKSettingsModuleKatePlugins::loadSettings(KConfig *config) {
	RK_TRACE(SETTINGS);

	KConfigGroup cg = config->group("Kate Plugins");
	plugins_to_load = cg.readEntry("Plugins to load", QStringList() << "katesearchplugin" << "kateprojectplugin" << "katesnippetsplugin");
}

QString RKSettingsModuleKatePlugins::caption() {
	RK_TRACE(SETTINGS);

	return i18n("Kate Plugins");
}
