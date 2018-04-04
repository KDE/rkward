/***************************************************************************
                          katepluginintegration  -  description
                             -------------------
    begin                : Mon Jun 12 2017
    copyright            : (C) 2017 by Thomas Friedrichsmeier
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

#include <ktexteditor/mainwindow.h>

class KatePluginIntegration : public QObject {
Q_OBJECT
public:
	KatePluginIntegration (QObject *parent);
	KTextEditor::MainWindow *mainWindow () const { return main; };
public slots:
	QWidget *createToolView (KTextEditor::Plugin *plugin, const QString &identifier, KTextEditor::MainWindow::ToolViewPosition pos, const QIcon &icon, const QString &text);
private:
	KTextEditor::MainWindow *main;
};

#endif