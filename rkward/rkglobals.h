/***************************************************************************
                          rkglobals  -  description
                             -------------------
    begin                : Wed Aug 18 2004
    copyright            : (C) 2004, 2013 by Thomas Friedrichsmeier
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
#ifndef RKGLOBALS_H
#define RKGLOBALS_H

#include <QVariantMap>

class RKWardMainWindow;
class RInterface;
class RObjectList;
class RKModificationTracker;
class KHelpDlg;
class RControlWindow;
class QString;

/**
This class basically keeps some static pointers which are needed all over the place, so they won't have to be passed around.

TODO: move the static members to the respective classes instead. There's no point in having them here, and having to include rkglobals.h all over the place.

@author Thomas Friedrichsmeier
*/
class RKGlobals{
public:
/// static pointer to the RInterface
	static RInterface *rInterface () { return rinter; };
/// static pointer to the RKModificationTracker
	static RKModificationTracker *tracker () { return mtracker; };

/// returns KDialog::marginHint (), without the need to include kdialog.h in all the sources
	static int marginHint ();
/// returns KDialog::spacingHint (), without the need to include kdialog.h in all the sources
	static int spacingHint ();

	static QVariantMap startup_options;
private:
	friend class RKWardMainWindow;
	static RInterface *rinter;
	static RKModificationTracker *mtracker;
};

#endif
