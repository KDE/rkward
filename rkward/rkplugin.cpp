/***************************************************************************
                          rkplugin.cpp  -  description
                             -------------------
    begin                : Wed Nov 6 2002
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

#include "rkplugin.h"

#include <qdom.h>

#include "rkmenu.h"

RKPlugin::RKPlugin(RKMenu *parent, const QDomElement &element, QString filename) {
	RKPlugin::parent = parent;
	RKPlugin::filename = filename;
	_label = element.attribute ("label", "untitled");
}

RKPlugin::~RKPlugin(){
}

void RKPlugin::activated () {
	qDebug ("activated plugin: " + filename);
}