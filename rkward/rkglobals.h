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
#ifndef RKGLOBALS_H
#define RKGLOBALS_H

class RKwardApp;
class RInterface;
class RObjectList;
class RKModificationTracker;
class RKComponentMap;
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
/** constructor. Do not use. No instance needed of this class. Only static stuff inside */
	RKGlobals ();
/** destructor. Do not use. No instance needed of this class. Only static stuff inside */
	~RKGlobals ();

/// static pointer to the RInterface
	static RInterface *rInterface () { return rinter; };
/// static pointer to the RKModificationTracker
	static RKModificationTracker *tracker () { return mtracker; };
/// static pointer to the RKComponentMap
	static RKComponentMap *componentMap () { return cmap; };
/// static pointer to the RKHelpDlg
	static KHelpDlg *helpDialog () { return helpdlg; };
/// static pointer to the RControlWindow
	static RControlWindow *controlWindow () { return rcontrol; };

/// a NA double
	static double na_double;
	
/// returns KDialog::marginHint (), without the need to include kdialog.h in all the sources
	static int marginHint ();
/// returns KDialog::spacingHint (), without the need to include kdialog.h in all the sources
	static int spacingHint ();

	static void deleteStrings (QString **strings, int count);

private:
	friend class RKwardApp;
	static RInterface *rinter;
	static RKModificationTracker *mtracker;
	static RKComponentMap *cmap;
	static KHelpDlg *helpdlg;
	static RControlWindow *rcontrol;
};

#endif
