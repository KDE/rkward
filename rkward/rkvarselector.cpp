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

#include <rkplugin.h>
#include <rkward.h>
#include <rkwarddoc.h>

RKVarSelector::RKVarSelector(const QDomElement &element, QWidget *parent, RKPlugin *plugin) : RKPluginWidget (element, parent, plugin) {
	qDebug ("creating varselector");
	label = new QLabel (element.attribute ("label", "Select Variable(s)"), parent);
	addWidget (label);

	list_view = new QListView (parent);
    list_view->addColumn ("Name");
    list_view->addColumn ("Label");
    list_view->addColumn ("Type");
	list_view->setSelectionMode (QListView::Extended);
	QListViewItem *main_table;
	main_table = new QListViewItem (list_view, "data", "Main table");
	main_table->setOpen (true);
	main_table->setSelectable (false);
	list_view->insertItem (main_table);
	
	RKwardDoc *doc = plugin->getApp ()->getDocument ();
	for (int i = 0; i < doc->numCols (); i++) {
		new QListViewItem (main_table, doc->varname (i));
	}

	addWidget (list_view);
}

RKVarSelector::~RKVarSelector(){
}

QStrList RKVarSelector::selectedVars () {
	QStrList selected;

	QListViewItem *current;
	current = list_view->firstChild ();
	while (current->itemBelow ()) {
		current = current->itemBelow ();
		if (current->isSelected ()) {
			selected.append ("rk." + current->parent ()->text (0) + "[\"" + current->text (0) + "\"]");
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
