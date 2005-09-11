/***************************************************************************
                          rkpluginhandle  -  description
                             -------------------
    begin                : Tue Aug 10 2004
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
#include "rkpluginhandle.h"

#include "rkplugin.h"
#include "../rkward.h"
#include "../rkglobals.h"
#include "../debug.h"

RKPluginHandle::RKPluginHandle (const QString &filename, RKComponentType type) : QObject (RKGlobals::rkApp ()), RKComponentHandle (filename, type) {
	RK_TRACE (PLUGIN);
}


RKPluginHandle::~RKPluginHandle () {
	RK_TRACE (PLUGIN);
}

void RKPluginHandle::activated () {
	RK_TRACE (PLUGIN);
	new RKPlugin (getFilename ());
}

#include "rkpluginhandle.moc"
