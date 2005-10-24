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
#if 0
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
	clear ();
}

void RKMenuList::clear () {
	RK_TRACE (MISC);

	for (QValueList<int>::const_iterator it = created_menu_ids.begin (); it != created_menu_ids.end (); ++it) {
		menu_bar->removeItem (*it);
	}
	created_menu_ids.clear ();

// remove only RKMenus, which were not pre-existing
	QDictIterator<RKMenu> it (menu_map);
	while (it.current ()) {
		if (!(it.current ()->pre_existing)) {
			delete it.current ();
			menu_map.remove (it.currentKey ());
		} else {
			++it;
		}
	}
}

RKMenu *RKMenuList::registerMenu (QPopupMenu *qmenu, const QString &id) {
	RK_TRACE (MISC);
	RKMenu *ret;
	if (!(ret = menu_map.find (id))) {
		ret = new RKMenu (qmenu, true);
		menu_map.insert (id, ret);
	}
	return ret;
}

RKMenu *RKMenuList::createMenu (const QString &id, const QString &label, int index) {
	RK_TRACE (MISC);
	RKMenu *ret;

	if (!(ret = menu_map.find (id))) {
		QPopupMenu *qmenu = new QPopupMenu (menu_bar);
		created_menu_ids.append (menu_bar->insertItem (label, qmenu, -1, index));
		ret = new RKMenu (qmenu, false);
		menu_map.insert (id, ret);
	}
	return ret;
}

#endif
