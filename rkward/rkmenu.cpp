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

#include "rkpluginhandle.h"
#include "rkward.h"

RKMenu::RKMenu(RKMenu *parent, QString tag, QString label, RKwardApp *app) : QPopupMenu (parent) {
	is_top_level = false;
	_tag = tag;
	_label = label;
	RKMenu::app = app;
}

RKMenu::RKMenu(QMenuBar *parent, QString tag, QString label, RKwardApp *app) : QPopupMenu (parent) {
	is_top_level = true;
	_tag = tag;
	_label = label;
	RKMenu::app = app;
}

RKMenu::~RKMenu(){
	// TODO: delete submenus and items
}

QString RKMenu::label () {
	return _label;
}

void RKMenu::addSubMenu (const QString &id, RKMenu *submenu) {
	submenus.insert (id, submenu);
	insertItem (submenu->label (), submenu);
}

void RKMenu::addEntry (const QString &id, RKPluginHandle *plugin, const QString &label) {
	entries.insert (id, plugin);
	insertItem (label, plugin, SLOT (activated ()));
}
