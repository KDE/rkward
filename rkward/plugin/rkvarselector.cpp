/***************************************************************************
                          rkvarselector.cpp  -  description
                             -------------------
    begin                : Thu Nov 7 2002
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

#include "rkvarselector.h"

#include <qwidget.h>
#include <qlistview.h>
#include <qdom.h>
#include <qlabel.h>

#include "../rkvariable.h"

#include "../rkglobals.h"

// TODO: remove these!
#include "../rkward.h"
#include "../rkwarddoc.h"


RKVarSelector::RKVarSelector (const QDomElement &element, QWidget *parent, RKPlugin *plugin, QLayout *layout) : RKPluginWidget (element, parent, plugin, layout) {
	qDebug ("creating varselector");
	label = new QLabel (element.attribute ("label", "Select Variable(s)"), parent);
	addWidget (label);

	list_view = new QListView (parent);
	list_view->setSorting (100);
    list_view->addColumn ("Name");
    list_view->addColumn ("Label");
    list_view->addColumn ("Type");
	list_view->setSelectionMode (QListView::Extended);
	QListViewItem *main_table;
	main_table = new QListViewItem (list_view, "data", "Main table");
	main_table->setOpen (true);
	main_table->setSelectable (false);
	list_view->insertItem (main_table);
	
	RKwardDoc *doc = RKGlobals::rkApp ()->getDocument ();
	for (int i = doc->numCols () - 1; i >= 0; --i) {
		RKVariable *variable = new RKVariable;
		variable->table = "rk.data";
		variable->name = doc->varname (i);
		variable->label = doc->label (i);
		variable->type = doc->typeString (i);
		item_map.insert (new QListViewItem (main_table, doc->varname (i), doc->label (i), doc->typeString (i)), variable);
	}

	addWidget (list_view);
}

RKVarSelector::~RKVarSelector(){
	for (ItemMap::iterator it = item_map.begin (); it != item_map.end (); ++it) {
		delete it.data ();
	}
}

QValueList<RKVariable*> RKVarSelector::selectedVars () {
	QValueList<RKVariable*> selected;

	QListViewItem *current;
	current = list_view->firstChild ();
	while (current->itemBelow ()) {
		current = current->itemBelow ();
		if (current->isSelected ()) {
			selected.append (item_map[current]);
		}
	}

	return selected;
}

int RKVarSelector::numSelectedVars () {
	int i=0;

	QListViewItem *current;
	current = list_view->firstChild ();
	while (current->itemBelow ()) {
		current = current->itemBelow ();
		if (current->isSelected ()) {
			++i;
		}
	}

	return i;
}
