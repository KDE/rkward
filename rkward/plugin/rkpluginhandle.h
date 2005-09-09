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
#ifndef RKPLUGINHANDLE_H
#define RKPLUGINHANDLE_H

#include <qobject.h>

#include <qstring.h>

#include "rkcomponentmap.h"

class RKwardApp;

/**
@author Thomas Friedrichsmeier
*/
class RKPluginHandle : public QObject, public RKComponentHandle {
	Q_OBJECT
public:
    RKPluginHandle(const QString &filename, RKComponentType type);

    ~RKPluginHandle();
public slots:
/** Slot called, when the menu-item for this widget is selected. Responsible
	for creating the GUI. */
	void activated ();
};

#endif
