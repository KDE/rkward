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

/**
This class basically keeps some static pointers which are needed all over the place, so they won't have to be passed around.

@author Thomas Friedrichsmeier
*/
class RKGlobals{
public:
    RKGlobals();

    ~RKGlobals();
	
	static RKwardApp *rkApp () { return app; }
	static RInterface *rInterface () { return rinter; };
	static RObjectList *rObjectList () { return list; };
	static RKEditorManager *editorManager () { return manager; };
	static RKModificationTracker *tracker () { return mtracker; };
private:
	friend class RKwardApp;
	static RKwardApp *app;
	static RInterface *rinter;
	static RObjectList *list;
	static RKEditorManager *manager;
	static RKModificationTracker *mtracker;
};

#endif
