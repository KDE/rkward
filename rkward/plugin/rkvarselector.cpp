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

#include "../core/rkvariable.h"

#include "../rkglobals.h"

#include "../core/robjectlist.h"

#include "../debug.h"

RKVarSelector::RKVarSelector (const QDomElement &element, QWidget *parent, RKPlugin *plugin, QLayout *layout) : RKPluginWidget (element, parent, plugin, layout) {
	RK_TRACE (PLUGIN);
	
	label = new QLabel (element.attribute ("label", "Select Variable(s)"), parent);
	addWidget (label);

	list_view = new QListView (parent);
	list_view->setSorting (100);
    list_view->addColumn ("Name");
    list_view->addColumn ("Label");
    list_view->addColumn ("Type");
	list_view->setSelectionMode (QListView::Extended);
	
	addObject (0, RKGlobals::rObjectList ());

	addWidget (list_view);
}

RKVarSelector::~RKVarSelector(){
	RK_TRACE (PLUGIN);
}

void RKVarSelector::addObject (QListViewItem *parent, RObject *object) {
	RK_TRACE (PLUGIN);
	QListViewItem *item;
	
	if (parent) {
		item = new QListViewItem (parent);
	} else {
		item = new QListViewItem (list_view);
	}
	item_map.insert (item, object);
	item->setText (0, object->getShortName ());
	item->setText (1, object->getLabel ());
	if (object->isContainer ()) {
		item->setText (3, static_cast<RContainerObject*> (object)->makeClassString ("; "));
	} else if (object->isVariable ()) {
		item->setText (2, static_cast<RKVariable*> (object)->getVarTypeString ());
	}

	RObject **children = object->children ();
	for (int i=0; i < object->numChildren (); ++i) {
		addObject (item, children[i]);
	}
	
	if (object->numChildren ()) {
		item->setOpen (true);
	}
}

QValueList<RKVariable*> RKVarSelector::selectedVars () {
	RK_TRACE (PLUGIN);
	QValueList<RKVariable*> selected;

	QListViewItem *current;
	current = list_view->firstChild ();
	while (current->itemBelow ()) {
		current = current->itemBelow ();
		if (current->isSelected ()) {
			RObject *obj = item_map[current];
			RK_ASSERT (obj);
			if (obj->isVariable ()) {
				selected.append (static_cast<RKVariable*> (obj));
			}
		}
	}

	return selected;
}

int RKVarSelector::numSelectedVars () {
	RK_TRACE (PLUGIN);
	int i=0;

	QListViewItem *current;
	current = list_view->firstChild ();
	while (current->itemBelow ()) {
		current = current->itemBelow ();
		if (current->isSelected ()) {
			RObject *obj = item_map[current];
			RK_ASSERT (obj);
			if (obj->isVariable ()) {
				++i;
			}
		}
	}

	return i;
}
