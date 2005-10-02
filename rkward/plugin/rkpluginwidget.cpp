/***************************************************************************
                          rkpluginwidget.cpp  -  description
                             -------------------
    begin                : Thu Nov 7 2002
    copyright            : (C) 2002 by Thomas Friedrichsmeier
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

#include "rkpluginwidget.h"

#include <qdom.h>

#include "rkplugin.h"
#include "../debug.h"

RKPluginWidget::RKPluginWidget (const QDomElement &, QWidget *parent, RKPlugin *plugin) : QWidget (parent) {
	RK_TRACE (PLUGIN);
	_plugin = plugin;
	
	connect (this, SIGNAL (changed ()), _plugin, SLOT (changed ()));
}

RKPluginWidget::~RKPluginWidget () {
	RK_TRACE (PLUGIN);
}

QString RKPluginWidget::complaints () {
	RK_TRACE (PLUGIN);
	return QString::null;
}

QString RKPluginWidget::value (const QString &) {
	RK_TRACE (PLUGIN);
	return QString::null;
}

bool RKPluginWidget::isSatisfied () {
	RK_TRACE (PLUGIN);
	return true;
}

#include "rkpluginwidget.moc"
