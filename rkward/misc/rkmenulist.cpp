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
#include "rkmenulist.h"

#include <qmenubar.h>
#include <qpopupmenu.h>

#include "rkmenu.h"

#include "../debug.h"

RKMenuList::RKMenuList (QMenuBar *menubar) {
	RK_TRACE (MISC);
	menu_bar = menubar;
}

RKMenuList::~RKMenuList () {
	RK_TRACE (MISC);
	for (MenuMap::iterator it = menu_map.begin (); it != menu_map.end (); ++it) {
		delete it.data ();
	}
}

RKMenu *RKMenuList::registerMenu (QPopupMenu *qmenu, const QString &id) {
	RK_TRACE (MISC);
	RKMenu *ret;
	if (menu_map.find (id) == menu_map.end ()) {
		ret = new RKMenu ();
		ret->menu = qmenu;
		menu_map.insert (id, ret);
	} else {
		ret = menu_map[id];
	}
	return ret;
}

RKMenu *RKMenuList::createMenu (const QString &id, const QString &label, int index) {
	RK_TRACE (MISC);
	RKMenu *ret;
	if (menu_map.find (id) == menu_map.end ()) {
		ret = new RKMenu ();
		QPopupMenu *qmenu = new QPopupMenu (menu_bar);
		menu_bar->insertItem (label, qmenu, -1, index);
		ret->menu = qmenu;
		menu_map.insert (id, ret);
	} else {
		ret = menu_map[id];
	}
	return ret;
}
