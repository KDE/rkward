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
#include <qpopupmenu.h>

#include <klocale.h>
#include <kinputdialog.h>
#include <kmessagebox.h>

#include "rkglobals.h"
#include "core/robjectlist.h"
#include "core/rkvariable.h"
#include "rkeditormanager.h"
#include "core/rkmodificationtracker.h"
#include "dataeditor/rkeditor.h"
#include "robjectviewer.h"

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

	update_button = new QPushButton (i18n ("Update"), this);
	vbox->addWidget (update_button);
	
	setCaption (i18n ("Objects in the R workspace"));

	menu = new QPopupMenu (this);
	menu->insertItem (i18n ("Edit"), this, SLOT (popupEdit ()), 0, Edit);
	menu->insertItem (i18n ("View"), this, SLOT (popupView ()), 0, View);
	menu->insertItem (i18n ("Rename"), this, SLOT (popupRename ()), 0, Rename);
	menu->insertItem (i18n ("Delete"), this, SLOT (popupDelete ()), 0, Delete);
}

RObjectBrowser::~RObjectBrowser () {
}

void RObjectBrowser::initialize () {
	addObject (0, RKGlobals::rObjectList());
	
	connect (RKGlobals::tracker (), SIGNAL (objectRemoved (RObject *)), this, SLOT (objectRemoved (RObject*)));
	connect (RKGlobals::tracker (), SIGNAL (objectPropertiesChanged (RObject *)), this, SLOT (objectPropertiesChanged (RObject*)));
	connect (RKGlobals::tracker (), SIGNAL (objectAdded (RObject *)), this, SLOT (objectAdded (RObject*)));

	connect (RKGlobals::rObjectList (), SIGNAL (updateComplete ()), this, SLOT (updateComplete ()));
	connect (update_button, SIGNAL (clicked ()), this, SLOT (updateButtonClicked ()));
	connect (list_view, SIGNAL (contextMenuRequested (QListViewItem*, const QPoint &, int)), this, SLOT (requestedContextMenu (QListViewItem*, const QPoint &, int)));
}

void RObjectBrowser::updateButtonClicked () {
	list_view->setEnabled (false);
	RKGlobals::rObjectList ()->updateFromR ();
}

void RObjectBrowser::objectAdded (RObject *object) {
	RK_TRACE (APP);

	QListViewItem *parent = findObjectItem (object->getContainer ());
	RK_ASSERT (parent);
	addObject (parent, object);
}

void RObjectBrowser::objectRemoved (RObject *object) {
	RK_TRACE (APP);

	QListViewItem *item = findObjectItem (object);
	RK_ASSERT (item);
	object_map.remove (item);
	delete item;
}

void RObjectBrowser::objectPropertiesChanged (RObject *object) {
	RK_TRACE (APP);

	QListViewItem *item = findObjectItem (object);
	RK_ASSERT (item);
	updateItem (item, object);
}

void RObjectBrowser::updateItem (QListViewItem *item, RObject *object) {
	RK_TRACE (APP);
	
	item->setText (0, object->getShortName ());
	item->setText (1, object->getLabel ());
	if (object->isContainer ()) {
		item->setText (3, static_cast<RContainerObject*> (object)->makeClassString ("; "));
	} else if (object->isVariable ()) {
		item->setText (2, static_cast<RKVariable*> (object)->getVarTypeString ());
	}
}

void RObjectBrowser::addObject (QListViewItem *parent, RObject *object) {
	RK_TRACE (APP);
	
	QListViewItem *item;

	if (parent) {
		item = new QListViewItem (parent);
	} else {
		item = new QListViewItem (list_view);
		item->setOpen (true);
	}

	updateItem (item, object);
	object_map.insert (item, object);
	
// code below won't work, as objects get added before editor is opened. Need to call from RKEditor(Manager)
/*	if (object->numChildren () && RKGlobals::editorManager ()->objectOpened (object)) {
		item->setOpen (true);
		while (item->parent ()) {
			item = item->parent ();
			item->setOpen (true);
		}
	} */
}

void RObjectBrowser::updateComplete () {
	list_view->setEnabled (true);
}

void RObjectBrowser::popupEdit () {
	if (menu_object) RKGlobals::editorManager ()->editObject (menu_object);
}

void RObjectBrowser::popupView () {
	RKGlobals::editorManager ()->flushAll ();
	new RObjectViewer (0, menu_object);
}

void RObjectBrowser::popupDelete () {
	RKGlobals::tracker ()->removeObject (menu_object);
}

void RObjectBrowser::popupRename () {
	bool ok;
	QString name = KInputDialog::getText (i18n ("Rename object"), i18n ("Enter the new name (make it a valid one - no checks so far)"), menu_object->getShortName (), &ok, this);

	if (ok) {
		RKGlobals::tracker ()->renameObject (menu_object, name);
	}
}

void RObjectBrowser::requestedContextMenu (QListViewItem *item, const QPoint &pos, int) {
	if (item) {
		RObject *object = object_map[item];
		menu->setItemEnabled (Edit, RKGlobals::editorManager ()->canEditObject (object));
		menu_object = object;
		menu->popup (pos);
	}
}

QListViewItem *RObjectBrowser::findObjectItem (RObject *object) {
	for (ObjectMap::iterator it = object_map.begin (); it != object_map.end (); ++it) {
		if (it.data () == object) return it.key ();
	}
	return 0;
}

#include "robjectbrowser.moc"
