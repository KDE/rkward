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
#include <KPluginMetaData>
#include <KLocalizedString>
#include <QLabel>

#include "../windows/katepluginintegration.h"
#include "../misc/rkcommonfunctions.h"
#include "../rkward.h"

#include "../debug.h"

QStringList RKSettingsModuleKatePlugins::plugins_to_load;

RKSettingsModuleKatePlugins::RKSettingsModuleKatePlugins (RKSettings *gui, QWidget *parent) : RKSettingsModule (gui, parent) {
	RK_TRACE (SETTINGS);

	QVBoxLayout *vbox = new QVBoxLayout (this);
	vbox->setContentsMargins (0, 0, 0, 0);
	vbox->addWidget (RKCommonFunctions::wordWrappedLabel (i18n ("Kate plugins to load in RKWard. Note that some loaded plugins will not become visible until certain conditions are met, e.g. you are loading a version controlled file for the <i>Project</i> plugin. Also, not all plugins listed here may make much sense in the context of RKWard.")));

	plugin_table = new QTreeWidget ();
	plugin_table->setHeaderLabels (QStringList () << QString () << i18n ("Name") << i18n ("Description"));
	KatePluginIntegrationApp *pluginapp = RKWardMainWindow::getMain()->katePluginIntegration ();
	foreach (const QString &key, pluginapp->known_plugins.keys()) {
		QTreeWidgetItem *item = new QTreeWidgetItem();
		KPluginMetaData plugindata = pluginapp->known_plugins.value (key).data;
		item->setData (1, Qt::DisplayRole, plugindata.name ());
		item->setData (2, Qt::DisplayRole, plugindata.description ());
		item->setData (1, Qt::DecorationRole, QIcon::fromTheme (plugindata.iconName ()));
		item->setFlags (Qt::ItemIsEnabled | Qt::ItemIsUserCheckable);
		item->setCheckState (0, plugins_to_load.contains (key) ? Qt::Checked : Qt::Unchecked);
		plugin_table->addTopLevelItem (item);
	}
	plugin_table->resizeColumnToContents (0);
	plugin_table->resizeColumnToContents (1);
	vbox->addWidget (plugin_table);
}

RKSettingsModuleKatePlugins::~RKSettingsModuleKatePlugins () {
	RK_TRACE (SETTINGS);
}

void RKSettingsModuleKatePlugins::applyChanges () {
	RK_TRACE (SETTINGS);
}

void RKSettingsModuleKatePlugins::save (KConfig *config) {
	RK_TRACE (SETTINGS);
}

void RKSettingsModuleKatePlugins::saveSettings (KConfig *config) {
	RK_TRACE (SETTINGS);
}

void RKSettingsModuleKatePlugins::loadSettings (KConfig *config) {
	RK_TRACE (SETTINGS);
}

QString RKSettingsModuleKatePlugins::caption () {
	RK_TRACE (SETTINGS);

	return i18n ("Kate Plugins");
}

void RKSettingsModuleKatePlugins::settingChanged () {
	RK_TRACE (SETTINGS);
	change ();
}

