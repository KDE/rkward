/***************************************************************************
                          rkpluginguiwidget  -  description
                             -------------------
    begin                : Tue Jul 27 2004
    copyright            : (C) 2004 by Thomas Friedrichsmeier
    email                : tfry@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "rkpluginguiwidget.h"

#include "rkplugin.h"
#include "qevent.h"

RKPluginGUIWidget::RKPluginGUIWidget(RKPlugin *parent) : QWidget() {
	plugin = parent;
}


RKPluginGUIWidget::~RKPluginGUIWidget() {
}

void RKPluginGUIWidget::closeEvent (QCloseEvent *e) {
	e->accept ();
	plugin->try_destruct ();
}

#include "rkpluginguiwidget.moc"
