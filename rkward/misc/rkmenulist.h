/***************************************************************************
                          rkmenulist  -  description
                             -------------------
    begin                : Fri Aug 27 2004
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
#ifndef RKMENULIST_H
#define RKMENULIST_H

#include <qstring.h>
#include <qmap.h>

class QMenuBar;
class QPopupMenu;
class RKMenu;

/**
This class holds the entries in the menu-bar. Together with RKMenu it is used to identify where dynamically generated entries should go in the menu-hierarchy.

@author Thomas Friedrichsmeier
*/
class RKMenuList {
public:
    RKMenuList (QMenuBar *menubar);

    ~RKMenuList ();
/** register an existing QPopupMenu in the menu-list, giving it the identifier tag "id". If a menu by that id already exists, no new menu will be created,
but rather the corresponding RKMenu will be returned. */
	RKMenu *registerMenu (QPopupMenu *qmenu, const QString &id);
/** like registerMenu, except that a new QPopupMenu is created with the given label. */
	RKMenu *createMenu (const QString &id, const QString &label, int index);
/** clears all menus created via RKMenuList */
	void clear ();
private:
	typedef QMap<QString, RKMenu*> MenuMap;
	MenuMap menu_map;
	
	QMenuBar *menu_bar;
};

#endif
