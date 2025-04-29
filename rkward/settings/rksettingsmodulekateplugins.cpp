/*
rksettingsmodulekateplugins - This file is part of RKWard (https://rkward.kde.org). Created: Thu Mar 26 2010
SPDX-FileCopyrightText: 2020-2024 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "rksettingsmodulekateplugins.h"

#include <QIcon>
#include <QLabel>
#include <QTreeWidget>
#include <QVBoxLayout>

#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>
#include <KPluginMetaData>
#include <KTextEditor/Application>
#include <KTextEditor/Editor>
#include <KTextEditor/Plugin>

#include "../misc/rkcommonfunctions.h"
#include "../rkward.h"
#include "../windows/katepluginintegration.h"
#include "rksettings.h"
#include "rksettingsmodulecommandeditor.h"
#include "rksettingsmoduleplugins.h"

#include "../debug.h"

RKConfigValue<QStringList> RKSettingsModuleKatePlugins::plugins_to_load{"Plugins to load", QStringList() << QStringLiteral("katesearchplugin") << QStringLiteral("kateprojectplugin") << QStringLiteral("katesnippetsplugin")};

class RKSettingsPageKatePlugins : public RKSettingsModuleWidget {
  public:
	RKSettingsPageKatePlugins(QWidget *parent, RKSettingsModule *parent_module) : RKSettingsModuleWidget(parent, parent_module, RKSettingsModuleKatePlugins::page_id, RKSettingsModulePlugins::addons_superpage_id) {
		RK_TRACE(SETTINGS);

		setWindowTitle(i18n("Kate Plugins"));
		setWindowIcon(QIcon::fromTheme(QStringLiteral("kate")));

		/* Known kate plugins at the time of this writing (March 2020): katesearchplugin katexmltoolsplugin katexmlcheckplugin katectagsplugin katefiletreeplugin 	katecloseexceptplugin katebacktracebrowserplugin tabswitcherplugin kterustcompletionplugin katekonsoleplugin katesnippetsplugin katefilebrowserplugin katereplicodeplugin ktexteditor_lumen kateprojectplugin kateopenheaderplugin katesymbolviewerplugin ktexteditorpreviewplugin katesqlplugin kategdbplugin katebuildplugin textfilterplugin */
		QStringList recommended_plugins = QStringList({u"katesearchplugin"_s, u"katecloseexceptplugin"_s, u"katekonsoleplugin"_s, u"katesnippetsplugin"_s, u"katefiletreeplugin"_s, u"kateprojectplugin"_s, u"ktexteditorpreviewplugin"_s, u"textfilterplugin"_s});

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
			item->setCheckState(0, RKSettingsModuleKatePlugins::plugins_to_load.get().contains(key) ? Qt::Checked : Qt::Unchecked);
			plugin_table->addTopLevelItem(item);
		}
		plugin_table->resizeColumnToContents(0);
		plugin_table->resizeColumnToContents(1);
		plugin_table->sortItems(1, Qt::AscendingOrder);
		vbox->addWidget(plugin_table);

		connect(plugin_table, &QTreeWidget::itemChanged, this, &RKSettingsPageKatePlugins::change);
	}
	void applyChanges() override {
		RK_TRACE(SETTINGS);

		QStringList p;
		for (int i = plugin_table->topLevelItemCount() - 1; i >= 0; --i) {
			QTreeWidgetItem *item = plugin_table->topLevelItem(i);
			if (item->checkState(0) == Qt::Checked) {
				p.append(item->data(1, Qt::UserRole).toString());
			}
		}
		RKSettingsModuleKatePlugins::plugins_to_load = p;
		RKWardMainWindow::getMain()->katePluginIntegration()->loadPlugins(p);
	}

  private:
	QTreeWidget *plugin_table;
};

RKSettingsModuleKatePlugins::RKSettingsModuleKatePlugins(QObject *parent) : RKSettingsModule(parent) {
	RK_TRACE(SETTINGS);
}

RKSettingsModuleKatePlugins::~RKSettingsModuleKatePlugins() {
	RK_TRACE(SETTINGS);
}

void RKSettingsModuleKatePlugins::addPluginSettingsPages(RKSettings *parent, KTextEditor::Plugin *plugin) {
	RK_TRACE(SETTINGS);

	for (int i = 0; i < plugin->configPages(); ++i) {
		auto _page = plugin->configPage(i, nullptr);
		auto page = new RKTextEditorConfigPageWrapper(parent, this, RKSettingsModuleKatePlugins::page_id, _page);
		parent->addSettingsPage(page);
		connect(KTextEditor::Editor::instance()->application(), &KTextEditor::Application::pluginDeleted, page, [parent, plugin, page](QString, const KTextEditor::Plugin *plug) {
			if (plug == plugin) parent->removeSettingsPage(page);
			// NOTE: The page will be deleted, above, thus also removing this connection
		});
	}
}

void RKSettingsModuleKatePlugins::createPages(RKSettings *parent) {
	RK_TRACE(SETTINGS);

	parent->addSettingsPage(new RKSettingsPageKatePlugins(parent, this));

	auto loaded_plugins = RKWardMainWindow::getMain()->katePluginIntegration()->loadedPlugins();
	for (auto it = loaded_plugins.constBegin(); it != loaded_plugins.constEnd(); ++it) {
		addPluginSettingsPages(parent, *it);
	}
	// any future plugins loaded during the lifetime of the settings dialog
	connect(KTextEditor::Editor::instance()->application(), &KTextEditor::Application::pluginCreated, parent, [this, parent](QString, KTextEditor::Plugin *plugin) {
		addPluginSettingsPages(parent, plugin);
	});
}

void RKSettingsModuleKatePlugins::syncConfig(KConfig *config, RKConfigBase::ConfigSyncAction a) {
	RK_TRACE(SETTINGS);

	if (a == RKConfigBase::SaveConfig) {
		// if no kate plugins are known (installation problem), don't save any config
		if (!RKWardMainWindow::getMain()->katePluginIntegration()->knownPluginCount()) return;
	}

	KConfigGroup cg = config->group(QStringLiteral("Kate Plugins"));
	plugins_to_load.syncConfig(cg, a);
}
