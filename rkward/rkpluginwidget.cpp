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

#include <qwidget.h>
#include <qdom.h>

RKPluginWidget::RKPluginWidget(const QDomElement &element, QWidget *parent, RKPlugin *plugin, QLayout *layout) : QBoxLayout (layout, QBoxLayout::TopToBottom, 6) {
	_parent = parent;
	_plugin = plugin;
	layout->setMargin (6);
}

RKPluginWidget::~RKPluginWidget(){
	qDebug ("widget deleted");
}

QString RKPluginWidget::complaints () {
	return "";
}

QString RKPluginWidget::value (const QString &modifier) {
	return "";
}

bool RKPluginWidget::isSatisfied () {
	return true;
}