/***************************************************************************
                          rkobjectlistview  -  description
                             -------------------
    begin                : Wed Sep 1 2004
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
#include "rkobjectlistview.h"

#include <klocale.h>
#include <kiconloader.h>

#include "../debug.h"
#include "../rkglobals.h"
#include "../core/robjectlist.h"
#include "../core/rkvariable.h"
#include "../core/rkmodificationtracker.h"

RKObjectListView::RKObjectListView (QWidget *parent) : QListView (parent) {
	RK_TRACE (APP);
	setSorting (100);
	addColumn (i18n("Name"));
	addColumn (i18n("Label"));
	addColumn (i18n("Type"));
	addColumn (i18n("Class"));
}

RKObjectListView::~RKObjectListView () {
	RK_TRACE (APP);
}

void RKObjectListView::initialize (bool fetch_list) {
	RK_TRACE (APP);
	addObject (0, RKGlobals::rObjectList (), fetch_list);
	
	connect (RKGlobals::tracker (), SIGNAL (objectRemoved (RObject *)), this, SLOT (objectRemoved (RObject*)));
	connect (RKGlobals::tracker (), SIGNAL (objectPropertiesChanged (RObject *)), this, SLOT (objectPropertiesChanged (RObject*)));
	connect (RKGlobals::tracker (), SIGNAL (objectAdded (RObject *)), this, SLOT (objectAdded (RObject*)));

	connect (RKGlobals::rObjectList (), SIGNAL (updateComplete ()), this, SLOT (updateComplete ()));
	connect (RKGlobals::rObjectList (), SIGNAL (updateStarted ()), this, SLOT (updateStarted ()));

	emit (listChanged ());
	update_in_progress = false;
	changes = false;
}

void RKObjectListView::updateComplete () {
	RK_TRACE (APP);
	setEnabled (true);
	update_in_progress = false;
	if (changes) {
		emit (listChanged ());
		changes = false;
	}
}

void RKObjectListView::updateStarted () {
	RK_TRACE (APP);
	setEnabled (false);
	update_in_progress = true;
}

void RKObjectListView::objectAdded (RObject *object) {
	RK_TRACE (APP);

	QListViewItem *parent = findObjectItem (object->getContainer ());
	RK_ASSERT (parent);
	addObject (parent, object, false);
	
	if (update_in_progress) {
		changes = true;
	} else {
		emit (listChanged ());
	}
}

void RKObjectListView::objectRemoved (RObject *object) {
	RK_TRACE (APP);

	QListViewItem *item = findObjectItem (object);
	RK_ASSERT (item);
	object_map.remove (item);
	delete item;
	
	if (update_in_progress) {
		changes = true;
	} else {
		emit (listChanged ());
	}
}

void RKObjectListView::objectPropertiesChanged (RObject *object) {
	RK_TRACE (APP);

	QListViewItem *item = findObjectItem (object);
	RK_ASSERT (item);
	updateItem (item, object);

	if (update_in_progress) {
		changes = true;
	} else {
		emit (listChanged ());
	}
}

QListViewItem *RKObjectListView::findObjectItem (RObject *object) {
	RK_TRACE (APP);
	for (ObjectMap::iterator it = object_map.begin (); it != object_map.end (); ++it) {
		if (it.data () == object) return it.key ();
	}
	return 0;
}

RObject *RKObjectListView::findItemObject (QListViewItem *item) {
	RK_TRACE (APP);
	if (!item) return 0;
	if (object_map.find (item) == object_map.end ()) {
		return 0;
	} else {
		return object_map[item];
	}
}

void RKObjectListView::updateItem (QListViewItem *item, RObject *object) {
	RK_TRACE (APP);

	// if the objec is hidden, it shouldn't appear
	QString  temp  = (QString) object->getFullName()  ;
	if (temp.left(1).latin1() == (QString) ".") return ;

		
	item->setText (0, object->getShortName ());
	item->setText (1, object->getLabel ());
	if (object->isContainer ()) {
		item->setText (3, static_cast<RContainerObject*> (object)->makeClassString ("; "));
	} else if (object->isVariable ()) {
		item->setText (2, static_cast<RKVariable*> (object)->getVarTypeString ());
		item->setText (3, static_cast<RKVariable*> (object)->makeClassString (""));
	}

	if (object->isDataFrame ()) {
		item->setPixmap (0, SmallIcon("spreadsheet"));
	} else {
		switch(static_cast<RKVariable*> (object)->getVarType ()) {
			case RObject::Number:
				item->setPixmap (0, SmallIcon("ledblue",8));
				break;
			case RObject::Factor:
				item->setPixmap (0, SmallIcon("ledgreen",8));
				break;
			case RObject::String:
				item->setPixmap (0, SmallIcon("ledyellow",8));
				break;
			case RObject::Invalid:
				item->setPixmap (0, SmallIcon("cancel",8));
				break;
			case RObject::Unknown:
				item->setPixmap (0, SmallIcon("ledred",8));
				break;
		}
	}
}

void RKObjectListView::addObject (QListViewItem *parent, RObject *object, bool recursive) {
	RK_TRACE (APP);
	
	// if the objec is hidden, it shouldn't appear
	QString  temp  = (QString) object->getFullName()  ;
	if (temp.left(1).latin1() == (QString) ".") return ;

	QListViewItem *item;

	if (parent) {
		item = new QListViewItem (parent);
	} else {
		item = new QListViewItem (this);
	}

	updateItem (item, object);
	object_map.insert (item, object);

	if (recursive) {
		RObject **children = object->children ();
		for (int i=0; i < object->numChildren (); ++i) {
			addObject (item, children[i], true);
		}
	}


	
// special treatment for the workspace object
	if (!parent) {
		item->setPixmap (0, SmallIcon("view_tree"));
		item->setText (0, i18n ("[Objects]"));
		item->setOpen (true);
	}
// code below won't work, as objects get added before editor is opened. Need to call from RKEditor(Manager)
/*	if (object->numChildren () && RKGlobals::editorManager ()->objectOpened (object)) {
		item->setOpen (true);
		while (item->parent ()) {
			item = item->parent ();
			item->setOpen (true);
		}
	} */
}

