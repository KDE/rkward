/***************************************************************************
                          rkglobals  -  description
                             -------------------
    begin                : Wed Aug 18 2004
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
#include "rkglobals.h"

RKwardApp *RKGlobals::app;
RInterface *RKGlobals::rinter;
RObjectList *RKGlobals::list;
RKEditorManager *RKGlobals::manager;
RKModificationTracker *RKGlobals::mtracker;

/* statics
- empty_char
- unknown_char
- na_char
- na_double
defined in REmbedInternal
*/

RKGlobals::RKGlobals () {
}


RKGlobals::~RKGlobals () {
}


