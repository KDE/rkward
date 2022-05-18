/*
rksettingsmodulekateplugins - This file is part of RKWard (https://rkward.kde.org). Created: Thu Mar 26 2010
SPDX-FileCopyrightText: 2020 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "rksettingsmodulekateplugins.h"

#include <QTreeWidget>
#include <QVBoxLayout>
#include <QLabel>

#include <KPluginMetaData>
#include <KLocalizedString>
#include <KConfigGroup>
#include <KConfig>
#include <QIcon>

#include "../windows/katepluginintegration.h"
#include "../misc/rkcommonfunctions.h"
#include "../rkward.h"

#include "../debug.h"

RKConfigValue<QStringList> RKSettingsModuleKatePlugins::plugins_to_load {"Plugins to load", QStringList() << "katesearchplugin" << "kateprojectplugin" << "katesnippetsplugin"};

RKSettingsModuleKatePlugins::RKSettingsModuleKatePlugins(RKSettings *gui, QWidget *parent) : RKSettingsModule(gui, parent) {
	RK_TRACE(SETTINGS);

/* Known kate plugins at the time of this writing (March 2020): katesearchplugin katexmltoolsplugin katexmlcheckplugin katectagsplugin katefiletreeplugin katecloseexceptplugin katebacktracebrowserplugin tabswitcherplugin kterustcompletionplugin katekonsoleplugin katesnippetsplugin katefilebrowserplugin katereplicodeplugin ktexteditor_lumen kateprojectplugin kateopenheaderplugin katesymbolviewerplugin ktexteditorpreviewplugin katesqlplugin kategdbplugin katebuildplugin textfilterplugin */
	QStringList recommended_plugins = QStringList () << "katesearchplugin" << "katecloseexceptplugin" << "katekonsoleplugin" << "katesnippetsplugin" << "katefiletreeplugin" << "kateprojectplugin" << "ktexteditorpreviewplugin" << "textfilterplugin";

	QVBoxLayout *vbox = new QVBoxLayout(this);
	vbox->setContentsMargins(0, 0, 0, 0);
	vbox->addWidget(RKCommonFunctions::wordWrappedLabel(i18n("<p>Kate plugins to load in RKWard. Note that some loaded plugins will not become visible until certain conditions are met, e.g. you are loading a version controlled file for the <i>Project</i> plugin.</p><p>The plugins listed here have not been developed specifically for RKWard, and several do not make a whole lot of sense in the context of RKWard. Plugins shown in <b>bold</b> have been reported as \"useful\" by RKWard users.</p>")));

	plugin_table = new QTreeWidget();
	QFont boldfont = plugin_table->font();
	boldfont.setBold(true);
	plugin_table->setHeaderLabels(QStringList() << QString() << i18n("Name") << i18n("Description"));
	KatePluginIntegrationApp *pluginapp = RKWardMainWindow::getMain()->katePluginIntegration();
	const auto keys = pluginapp->known_plugins.keys();
	for (const QString &key : keys) {
		QTreeWidgetItem *item = new QTreeWidgetItem();
		KPluginMetaData plugindata = pluginapp->known_plugins.value(key).data;
		item->setData(1, Qt::DisplayRole, plugindata.name());
		if (recommended_plugins.contains(key)) item->setData(1, Qt::FontRole, boldfont); 
		item->setData(2, Qt::DisplayRole, plugindata.description());
		item->setData(1, Qt::DecorationRole, QIcon::fromTheme(plugindata.iconName()));
		item->setData(1, Qt::UserRole, key);
		item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsUserCheckable);
		item->setCheckState(0, plugins_to_load.get().contains(key) ? Qt::Checked : Qt::Unchecked);
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

	QStringList p;
	for (int i = plugin_table->topLevelItemCount() - 1; i >= 0; --i) {
		QTreeWidgetItem *item = plugin_table->topLevelItem(i);
		if (item->checkState(0) == Qt::Checked) {
			p.append (item->data(1, Qt::UserRole).toString());
		}
	}
	plugins_to_load = p;
	RKWardMainWindow::getMain()->katePluginIntegration()->loadPlugins(plugins_to_load);
}

void RKSettingsModuleKatePlugins::syncConfig(KConfig *config, RKConfigBase::ConfigSyncAction a) {
	RK_TRACE(SETTINGS);

	if (a == RKConfigBase::SaveConfig) {
		// if no kate plugins are known (installation problem), don't save any config
		if (!RKWardMainWindow::getMain()->katePluginIntegration()->knownPluginCount()) return;
	}

	KConfigGroup cg = config->group("Kate Plugins");
	plugins_to_load.syncConfig(cg, a);
}

QString RKSettingsModuleKatePlugins::caption() const {
	RK_TRACE(SETTINGS);
	return i18n("Kate Plugins");
}

QIcon RKSettingsModuleKatePlugins::icon() const {
	RK_TRACE(SETTINGS);
	return QIcon::fromTheme("kate");
}
