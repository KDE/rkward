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

#include "katepluginintegration.h"

#include "../debug.h"

#include <QWidget>

KatePluginIntegration::KatePluginIntegration (QObject *parent) : QObject (parent) {
	RK_TRACE (APP);
	RKDebug (APP, DL_ERROR, "ctor");

	main = new KTextEditor::MainWindow (this);
}

QWidget * KatePluginIntegration::createToolView (KTextEditor::Plugin *plugin, const QString &identifier, KTextEditor::MainWindow::ToolViewPosition pos, const QIcon &icon, const QString &text) {
	RK_TRACE (APP);
	RKDebug (APP, DL_ERROR, "createToolView");
	
	return new QWidget ();
}

