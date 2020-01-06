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

#ifndef KATEPLUGININTEGRATION_H
#define KATEPLUGININTEGRATION_H

#include <KTextEditor/MainWindow>
#include <KPluginMetaData>
#include <KXMLGUIClient>

#include <QMap>

class KatePluginIntegration : public QObject, public KXMLGUIClient {
Q_OBJECT
public:
	KatePluginIntegration (QObject *parent);
	KTextEditor::MainWindow *mainWindow () const { return main; };
	QObject* loadPlugin (const QString& identifier);
public slots:
	QWidget *createToolView (KTextEditor::Plugin *plugin, const QString &identifier, KTextEditor::MainWindow::ToolViewPosition pos, const QIcon &icon, const QString &text);
	KXMLGUIFactory *guiFactory ();
	bool showToolView (QWidget *widget);
private:
	KTextEditor::MainWindow *main;
	QMap<QString, KPluginMetaData> known_plugins;
	QString idForPlugin(const KPluginMetaData &plugin) const;
};

#endif
