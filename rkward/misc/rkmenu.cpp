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

RKMenu::RKMenu () {
	RK_TRACE (MISC);
}

RKMenu::~RKMenu() {
	RK_TRACE (MISC);
	for (QMap<int, RKMenu*>::iterator it = submenus.begin (); it != submenus.end (); ++it) {
		delete it.data ();
	}
	for (QMap<int, RKPluginHandle*>::iterator it = entry_plugins.begin (); it != entry_plugins.end (); ++it) {
		delete it.data ();
	}
}

RKMenu *RKMenu::createSubMenu () {
	RK_TRACE (MISC);
	QPopupMenu *qmenu = new QPopupMenu (menu);
	RKMenu *ret = new RKMenu ();
	ret->menu = qmenu;
	
	return ret;
}

RKMenu *RKMenu::addSubMenu (const QString &id, const QString &label, int index) {
	RK_TRACE (MISC);
	int mid;
	RKMenu *ret;
	if (submenu_ids.find (id) == submenu_ids.end ()) {
		ret = createSubMenu ();
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
		delete entry_plugins[mid];
		menu->removeItem (mid);
		mid = menu->insertItem (label, plugin, SLOT (activated ()), 0, mid, index);
	}
	entry_plugins.insert (mid, plugin);
	entry_ids.insert (id, mid);
}
