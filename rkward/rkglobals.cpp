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

#include <qstring.h>

RInterface *RKGlobals::rinter;
RObjectList *RKGlobals::list;
RKModificationTracker *RKGlobals::mtracker;
RKComponentMap *RKGlobals::cmap;
KHelpDlg *RKGlobals::helpdlg;
RControlWindow *RKGlobals::rcontrol;

double RKGlobals::na_double;

RKGlobals::RKGlobals () {
}


RKGlobals::~RKGlobals () {
}

#include <kdialog.h>

int RKGlobals::marginHint () {
	return KDialog::marginHint ();
}

int RKGlobals::spacingHint () {
	return KDialog::spacingHint ();
}

