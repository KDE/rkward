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

#include "rkplugin.h"
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

RKPlugin *RKMenu::place (const QDomElement &element, const QString &filename) {
	QDomNodeList children = element.childNodes ();
	QDomElement child = children.item (0).toElement ();

	if (child.tagName () == "location") {
		if (!submenus.contains (child.attribute ("tag"))) {
			RKMenu *sub = new RKMenu (this, child.attribute ("tag"), child.attribute ("label", "untitled"), app);
			submenus.insert (child.attribute ("tag"), sub);
			insertItem (sub->label (), sub);
		} 
		return submenus[child.attribute ("tag")]->place (child, filename);
	} else {		// entry
		RKPlugin *plug = new RKPlugin (app, child, filename);
		insertItem (plug->label (), plug, SLOT (activated ()));
		return plug;
	}
}