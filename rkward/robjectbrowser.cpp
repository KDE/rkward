/***************************************************************************
                          robjectbrowser  -  description
                             -------------------
    begin                : Thu Aug 19 2004
    copyright            : (C) 2004 by Thomas Friedrichsmeier
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
#include "robjectbrowser.h"

#include <qlayout.h>
#include <qlistview.h>
#include <qpushbutton.h>

#include <klocale.h>

#include "rkglobals.h"
#include "core/robjectlist.h"
#include "core/rkvariable.h"
#include "rkeditormanager.h"

#include "debug.h"

RObjectBrowser::RObjectBrowser () : RKToggleWidget () {
	QGridLayout *grid = new QGridLayout (this, 1, 1);
	QVBoxLayout *vbox = new QVBoxLayout ();
	grid->addLayout (vbox, 0, 0);
	
	list_view = new QListView (this);
	list_view->setSorting (100);
    list_view->addColumn ("Name");
    list_view->addColumn ("Label");
    list_view->addColumn ("Type");
    list_view->addColumn ("Class(es)");
	vbox->addWidget (list_view);

	update_button = new QPushButton (i18n ("Udpate"), this);
	vbox->addWidget (update_button);
	
	setCaption (i18n ("Objects in the R workspace"));
}

RObjectBrowser::~RObjectBrowser () {
}

void RObjectBrowser::initialize () {
	connect (RKGlobals::rObjectList (), SIGNAL (updateComplete (bool)), this, SLOT (updateComplete (bool)));
	connect (update_button, SIGNAL (clicked ()), this, SLOT (updateButtonClicked ()));
	connect (list_view, SIGNAL (clicked (QListViewItem*)), this, SLOT (itemClicked (QListViewItem*)));
}

void RObjectBrowser::updateButtonClicked () {
	list_view->setEnabled (false);
	RKGlobals::rObjectList ()->updateFromR ();
}

void RObjectBrowser::addObject (QListViewItem *parent, RObject *object) {
	RK_TRACE (APP);
	
	QListViewItem *item;

	if (parent) {
		item = new QListViewItem (parent);
	} else {
		item = new QListViewItem (list_view);
	}
	item->setText (0, object->getShortName ());
	item->setText (1, object->getLabel ());
	if (object->isContainer ()) {
		item->setText (3, static_cast<RContainerObject*> (object)->makeClassString ("; "));
	} else if (object->isVariable ()) {
		item->setText (2, static_cast<RKVariable*> (object)->getTypeString ());
	}

	RObject **children = object->children ();
	for (int i=0; i < object->numChildren (); ++i) {
		addObject (item, children[i]);
	}
	
	if (object->numChildren ()) {
		item->setOpen (true);
	}
	
	object_map.insert (item, object);
}

void RObjectBrowser::updateComplete (bool changed) {
	list_view->clear ();
	
	addObject (0, RKGlobals::rObjectList());
	
	list_view->setEnabled (true);
}

void RObjectBrowser::itemClicked (QListViewItem *item) {
	if (item) RKGlobals::editorManager ()->editObject (object_map[item]);
}

#include "robjectbrowser.moc"
