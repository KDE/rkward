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
class RKEditorManager;
class RKModificationTracker;
class RKComponentMap;
class KHelpDlg;

// deletes the given char*, if it is not a special value. Does not set to 0.
#define DELETE_STRING(x) if (x && (x != RKGlobals::empty_char) && (x != RKGlobals::unknown_char)) { delete x; };

/**
This class basically keeps some static pointers which are needed all over the place, so they won't have to be passed around.

@author Thomas Friedrichsmeier
*/
class RKGlobals{
public:
/** constructor. Do not use. No instance needed of this class. Only static stuff inside */
	RKGlobals ();
/** destructor. Do not use. No instance needed of this class. Only static stuff inside */
	~RKGlobals ();

/// static pointer to the app
	static RKwardApp *rkApp () { return app; }
/// static pointer to the RInterface
	static RInterface *rInterface () { return rinter; };
/// static pointer to the RObjectList
	static RObjectList *rObjectList () { return list; };
/// static pointer to the RKEditorManager
	static RKEditorManager *editorManager () { return manager; };
/// static pointer to the RKModificationTracker
	static RKModificationTracker *tracker () { return mtracker; };
/// static pointer to the RKComponentMap
	static RKComponentMap *componentMap () { return cmap; };
/// static pointer to the RKHelpDlg
	static KHelpDlg *helpDialog () { return helpdlg; };

/// an empty char
	static char *empty_char;
/// an unknown value
	static char *unknown_char;
/// a NA double
	static double na_double;
	
/// returns KDialog::marginHint (), without the need to include kdialog.h in all the sources
	static int marginHint ();
/// returns KDialog::spacingHint (), without the need to include kdialog.h in all the sources
	static int spacingHint ();
private:
	friend class RKwardApp;
	static RKwardApp *app;
	static RInterface *rinter;
	static RObjectList *list;
	static RKEditorManager *manager;
	static RKModificationTracker *mtracker;
	static RKComponentMap *cmap;
	static KHelpDlg *helpdlg;
};

#endif
