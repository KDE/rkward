/***************************************************************************
                          katepluginintegration  -  description
                             -------------------
    begin                : Mon Jun 12 2017
    copyright            : (C) 2017-2020 by Thomas Friedrichsmeier
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

#include "katepluginintegration.h"

#include <QWidget>
#include <QFileInfo>
#include <QVBoxLayout>

#include <KPluginFactory>
#include <KPluginLoader>
#include <KPluginMetaData>
#include <KTextEditor/Editor>
#include <KTextEditor/Application>
#include <KTextEditor/Plugin>

#include "../rkward.h"

#include "../debug.h"

KatePluginIntegration::KatePluginIntegration (QObject *parent) : QObject (parent), KXMLGUIClient () {
	RK_TRACE (APP);

	main = new KTextEditor::MainWindow (this);

	// enumerate all available kate plugins
	QVector<KPluginMetaData> plugins = KPluginLoader::findPlugins (QStringLiteral ("ktexteditor"), [](const KPluginMetaData &md) { return md.serviceTypes().contains(QLatin1String("KTextEditor/Plugin")); });
    for (int i = plugins.size() -1; i >= 0; --i) {
		KPluginMetaData &plugin = plugins[i];
		// Note: creates a lookup-table *and* eliminates potential dupes later in the search path 
		known_plugins.insert (idForPlugin (plugin), plugin);

		qDebug ("%s", qPrintable(plugin.fileName()));
	}
}

QWidget * KatePluginIntegration::createToolView (KTextEditor::Plugin *plugin, const QString &identifier, KTextEditor::MainWindow::ToolViewPosition pos, const QIcon &icon, const QString &text) {
	RK_TRACE (APP);
	RKDebug (APP, DL_ERROR, "createToolView");

	QWidget *dummy = new QWidget();
	dummy->setLayout (new QVBoxLayout ());
//	dummy->show();
	return dummy;
}

bool KatePluginIntegration::showToolView (QWidget *widget) {
	RK_TRACE (APP);
	RKDebug (APP, DL_ERROR, "showToolView");
	widget->show ();
	return true;
}

QObject* KatePluginIntegration::loadPlugin (const QString& identifier) {
	RK_TRACE (APP);

	if (!known_plugins.contains (identifier)) {
		RK_DEBUG (APP, DL_WARNING, "Plugin %s is not known", qPrintable (identifier));
		return 0;
	}

	KPluginFactory *factory = KPluginLoader (known_plugins[identifier].fileName ()).factory ();
	if (factory) {
		KTextEditor::Plugin *plugin = factory->create<KTextEditor::Plugin> (this, QVariantList () << identifier);
		if (plugin) {
			emit KTextEditor::Editor::instance ()->application ()->pluginCreated (identifier, plugin);
			plugin->createView (main);
			return plugin;
		}
	}

    return 0;
}

KXMLGUIFactory *KatePluginIntegration::guiFactory () {
	RK_TRACE (APP);

	return RKWardMainWindow::getMain()->guiFactory();
}

QString KatePluginIntegration::idForPlugin(const KPluginMetaData &plugin) const {
	return QFileInfo(plugin.fileName()).baseName();
}
