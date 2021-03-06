/***************************************************************************
                          rkglobals  -  description
                             -------------------
    begin                : Wed Aug 18 2004
    copyright            : (C) 2004 by Thomas Friedrichsmeier
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
#include "rkglobals.h"

#include <qstring.h>

RInterface *RKGlobals::rinter;
RKModificationTracker *RKGlobals::mtracker;
QVariantMap RKGlobals::startup_options;

#include <QApplication>
#include <QStyle>

int RKGlobals::marginHint () {
	return QApplication::style ()->pixelMetric (QStyle::PM_DefaultChildMargin);
}

int RKGlobals::spacingHint () {
	return QApplication::style ()->pixelMetric (QStyle::PM_DefaultLayoutSpacing);
}

