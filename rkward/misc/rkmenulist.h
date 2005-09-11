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
#include <qdict.h>
#include <qmap.h>

class QMenuBar;
class QPopupMenu;
class RKMenu;

/**
This class holds the entries in the menu-bar. Together with RKMenu it is used to identify where dynamically generated entries should go in the menu-hierarchy. I.e. the most important function is to identify, when we try to create a menu with the same id as an already existing one. In this case,
instead of creating a new menu, the old one is returned.

In contrast to RKMenu, the RKMenuList is associated with the menu-bar, not a single menu.

@author Thomas Friedrichsmeier
*/
class RKMenuList {
public:
    RKMenuList (QMenuBar *menubar);

    ~RKMenuList ();
/** register an existing QPopupMenu in the menu-list, giving it the identifier tag "id". If a menu by that id already exists, no new menu will be created,
but rather the corresponding RKMenu will be returned. This function can be used to give access to a pre-existing menu (i.e. one that has been created
by static rather than dynamic methods, such as the "File"-menu). */
	RKMenu *registerMenu (QPopupMenu *qmenu, const QString &id);
/** create a new Menu with given id, label and at the given index. If a menu by that id is already known, no menu is actually created, but
rather a pointer to the existing menu gets returned. */
	RKMenu *createMenu (const QString &id, const QString &label, int index);
/** clears all menus created via RKMenuList */
	void clear ();
private:
	typedef QDict<RKMenu> MenuMap;
	MenuMap menu_map;
	
	QValueList<int> created_menu_ids;
	QMenuBar *menu_bar;
};

#endif
