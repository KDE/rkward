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

#ifndef RKMENU_H
#define RKMENU_H

#include <qpopupmenu.h>

#include <qmap.h>
#include <qstring.h>

class QMenuBar;
class RKPluginHandle;
class QDomElement;

/**
  *@author Thomas Friedrichsmeier
  */

class RKMenu {
public: 
	~RKMenu();
/** adds a new submenu to this menu at the given index. If a menu with the given tag already exists, that menu will be returned, so entries can be merged (and the index will be ignored) */
	RKMenu *addSubMenu (const QString &id, const QString &label, int index=-1);
/** adds a new plugin to this menu at the given index. If a plugin with the given tag already exists, it will be deleted and overwritten, i.e. replaced with the newer plugin */
	void addEntry (const QString &id, RKPluginHandle *plugin, const QString &label, int index=-1);
protected:
	RKMenu ();
private:
friend class RKMenuList;
	RKMenu *createSubMenu ();
/** TODO: Probably we neither need to keep the tag, nor is_top_level. */
	QMap<QString, int> submenu_ids;
	QMap<int, RKMenu*> submenus;
	QMap<QString, int> entry_ids;
	QMap<int, RKPluginHandle*> entry_plugins;
/** the associated menu, the entries get placed in */
	QPopupMenu *menu;
};

#endif
