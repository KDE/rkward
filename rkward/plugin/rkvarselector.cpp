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

#include <qlayout.h>
#include <qlistview.h>
#include <qdom.h>
#include <qlabel.h>

#include "../core/rkvariable.h"
#include "../rkglobals.h"
#include "../misc/rkobjectlistview.h"

#include "../debug.h"

RKVarSelector::RKVarSelector (const QDomElement &element, QWidget *parent, RKPlugin *plugin) : RKPluginWidget (element, parent, plugin) {
	RK_TRACE (PLUGIN);
	QVBoxLayout  *vbox = new QVBoxLayout (this, RKGlobals::spacingHint ());
	depend = element.attribute ("depend", "");
	
	label = new QLabel (element.attribute ("label", "Select Variable(s)"), this);
	vbox->addWidget (label);

	list_view = new RKObjectListView (this);
	list_view->setSelectionMode (QListView::Extended);
	connect (list_view, SIGNAL (listChanged ()), this, SLOT (objectListChanged ()));

	vbox->addWidget (list_view);
	list_view->initialize (true);
}

RKVarSelector::~RKVarSelector(){
	RK_TRACE (PLUGIN);
}

void RKVarSelector::objectListChanged () {
	RK_TRACE (PLUGIN);
	// forward the change-notification
	emit (changed ());
}

QValueList<RKVariable*> RKVarSelector::selectedVars () {
	RK_TRACE (PLUGIN);
	QValueList<RKVariable*> selected;

	QListViewItem *current;
	current = list_view->firstChild ();
	while (current->itemBelow ()) {
		current = current->itemBelow ();
		if (current->isSelected ()) {
			RObject *obj = list_view->findItemObject (current);
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
			RObject *obj = list_view->findItemObject (current);
			RK_ASSERT (obj);
			if (obj->isVariable ()) {
				++i;
			}
		}
	}

	return i;
}

bool RKVarSelector::containsObject (RObject *object) {
	RK_TRACE (PLUGIN);

	QListViewItem *current;
	current = list_view->firstChild ();
	while (current->itemBelow ()) {
		current = current->itemBelow ();
		if (list_view->findItemObject (current) == object) return true;
	}

	return false;
}

void RKVarSelector::setEnabled(bool checked){
  list_view->setEnabled(checked);
  }

void RKVarSelector::active(){
bool isOk = list_view->isEnabled();
list_view->setEnabled(! isOk) ;
}

void RKVarSelector::active(bool isOk){
list_view->setEnabled(isOk) ;
}
