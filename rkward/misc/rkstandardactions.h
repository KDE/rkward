/***************************************************************************
                          rkstandardactions  -  description
                             -------------------
    begin                : Sun Nov 18 2007
    copyright            : (C) 2007, 2009 by Thomas Friedrichsmeier
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

#ifndef RKSTANDARDACTIONS_H
#define RKSTANDARDACTIONS_H

class KAction;
class QString;
class QObject;
class RKMDIWindow;

/** This namespace provides functions to generate some standard actions, i.e. actions which are needed at more than one place.

@author Thomas Friedrichsmeier */
namespace RKStandardActions {
	KAction* runLine (RKMDIWindow *window, const QString &name, const QObject *receiver=0, const char *member=0);
	KAction* runSelection (RKMDIWindow *window, const QString &name, const QObject *receiver=0, const char *member=0);
	KAction* runAll (RKMDIWindow *window, const QString &name, const QObject *receiver=0, const char *member=0);

	KAction* functionHelp (RKMDIWindow *window, const QString &name, const QObject *receiver=0, const char *member=0);
};

#endif
