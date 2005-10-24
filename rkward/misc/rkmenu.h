/***************************************************************************
                          rkmenu.h  -  description
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
#if 0
#ifndef RKMENU_H
#define RKMENU_H

#include <qpopupmenu.h>

#include <qmap.h>
#include <qstring.h>

class QMenuBar;
class RKPluginHandle;
class QDomElement;

/** The purpose of this class is to attach some additional information to a QPopupMenu, needed to create and delete dynamic menu-entries. See RKMenuList for more detailed information.
  *@author Thomas Friedrichsmeier
  */

class RKMenu {
public:
/** create a new RKMenu. If pre_existing==false, when the RKMenu gets deleted, the corresponding QPopupMenu will be deleted as well
Use this for menus that are entirely dynamically created.

If pre_existing==true, when the RKMenu gets deleted, this QPopupMenu will continue to live. Use this for pre-existing menus that only contain some dynamic items. */
	RKMenu (QPopupMenu *menu, bool pre_existing=false);
/** destructor */
	~RKMenu ();
/** adds a new submenu to this menu at the given index. If a menu with the given tag already exists, that menu will be returned, so entries can be merged (and the index will be ignored) */
	RKMenu *addSubMenu (const QString &id, const QString &label, int index=-1);
/** adds a new plugin to this menu at the given index. If a plugin with the given tag already exists, it will be deleted and overwritten, i.e. replaced with the newer plugin */
	void addEntry (const QString &id, RKPluginHandle *plugin, const QString &label, int index=-1);
private:
friend class RKMenuList;
	QMap<QString, int> submenu_ids;
	QMap<int, RKMenu*> submenus;
	QMap<QString, int> entry_ids;
/** the associated menu, the entries get placed in */
	QPopupMenu *menu;
/** whether the QPopupMenu should be deleted along with the RKMenu. See constructor. */
	bool pre_existing;
};

#endif

#endif
