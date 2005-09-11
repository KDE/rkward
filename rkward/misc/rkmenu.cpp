/***************************************************************************
                          rkmenu.cpp  -  description
                             -------------------
    begin                : Wed Nov 6 2002
    copyright            : (C) 2002 by Thomas Friedrichsmeier
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

#include "rkmenu.h"

#include <qmenubar.h>
#include <qdom.h>

#include "../plugin/rkpluginhandle.h"

#include "../debug.h"

RKMenu::RKMenu (QPopupMenu *menu, bool pre_existing) {
	RK_TRACE (MISC);

	RKMenu::menu = menu;
	delete_q_menu = !pre_existing;
}

RKMenu::~RKMenu() {
	RK_TRACE (MISC);

	for (QMap<int, RKMenu*>::iterator it = submenus.begin (); it != submenus.end (); ++it) {
		menu->removeItem (it.key ());
		delete it.data ();
	}

	if (delete_q_menu) {
		delete menu;
	} else{
		// delete the dynamically created entries
		for (QMap<QString, int>::iterator it = entry_ids.begin (); it != entry_ids.end (); ++it) {
			menu->removeItem (it.data ());
		}
	}
}

RKMenu *RKMenu::addSubMenu (const QString &id, const QString &label, int index) {
	RK_TRACE (MISC);
	int mid;
	RKMenu *ret;
	if (submenu_ids.find (id) == submenu_ids.end ()) {
		ret = new RKMenu (new QPopupMenu (menu, false));
		mid = menu->insertItem (label, ret->menu, -1, index);
	} else {
		mid = submenu_ids[id];
		menu->changeItem (mid, label);
		ret = submenus [mid];
	}
	submenu_ids.insert (id, mid);
	submenus.insert (mid, ret);
	
	return ret;
}

void RKMenu::addEntry (const QString &id, RKPluginHandle *plugin, const QString &label, int index) {
	RK_TRACE (MISC);
	int mid;
	if (entry_ids.find (id) == entry_ids.end ()) {
		mid = menu->insertItem (label, plugin, SLOT (activated ()), 0, -1, index);
	} else {
		mid = entry_ids[id];
		menu->removeItem (mid);
		mid = menu->insertItem (label, plugin, SLOT (activated ()), 0, mid, index);
	}
	entry_ids.insert (id, mid);
}
