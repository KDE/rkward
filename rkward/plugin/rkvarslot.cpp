/***************************************************************************
                          rkvarslot.cpp  -  description
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

#include "rkvarslot.h"

#include <qlineedit.h>
#include <qdom.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qlistview.h>
#include <qlayout.h>
#include <qheader.h>
#include <qstringlist.h>

#include <klocale.h>
#include <kiconloader.h>

#include "rkvarselector.h"
#include "rkplugin.h"
#include "../rkglobals.h"
#include "../debug.h"
#include "../core/rkvariable.h"
#include "../core/rcontainerobject.h"
#include "../misc/xmlhelper.h"


RKVarSlot::RKVarSlot (const QDomElement &element, RKComponent *parent_component, QWidget *parent_widget) : RKComponent (parent_component, parent_widget) {
	RK_TRACE (PLUGIN);

	XMLHelper *xml = XMLHelper::getStaticHelper ();

	// basic layout
	QGridLayout *g_layout = new QGridLayout (this, 4, 3, RKGlobals::spacingHint ());

	QLabel *label = new QLabel (xml->getStringAttribute (element, "label", i18n ("Variable:"), DL_INFO), this);
	g_layout->addWidget (label, 0, 2);

	select = new QPushButton (QString::null, this);
	setSelectButton (false);
	connect (select, SIGNAL (clicked ()), this, SLOT (selectPressed ()));
	g_layout->addWidget (select, 1, 0);
	g_layout->addColSpacing (1, 5);

	list = new QListView (this);
	list->setSelectionMode (QListView::Extended);
	list->addColumn (" ");		// for counter
	list->addColumn (i18n ("Name"));
	list->setSorting (2);
	g_layout->addWidget (list, 1, 2);

	g_layout->setRowStretch (3, 1);

	// initialize properties
	addChild ("source", source = new RKComponentPropertyRObjects (this, false));
	addChild ("available", available = new RKComponentPropertyRObjects (this, true));
	addChild ("selected", selected = new RKComponentPropertyRObjects (this, false));

	// find out about options
	if (xml->getBoolAttribute (element, "multi", false, DL_INFO)) {
		available->setListLength (xml->getIntAttribute (element, "min_vars", 1, DL_INFO), xml->getIntAttribute (element, "min_vars_fi_any", 0, DL_INFO), xml->getIntAttribute (element, "max_vars", 0, DL_INFO));
		connect (list, SIGNAL (selectionChanged ()), this, SLOT (listSelectionChanged ()));
	} else {
		list->header ()->hide ();
		//list->setSizePolicy (QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
		list->setFixedHeight (20);
		list->setColumnWidth (0, 0);
	}

	// initialize filters
	available->setClassFilter (xml->getStringAttribute (element, "classes", QString::null, DL_INFO));
	available->setRequired (xml->getBoolAttribute (element, "required", true, DL_INFO));
	available->setTypeFilter (xml->getStringAttribute (element, "types", QString::null, DL_INFO));

	connect (available, SIGNAL (valueChanged (RKComponentPropertyBase *)), this, SLOT (availablePropertyChanged (RKComponentPropertyBase *)));
}

RKVarSlot::~RKVarSlot (){
	RK_TRACE (PLUGIN);
}

void RKVarSlot::setSelectButton (bool add) {
	if (add) {
		select->setPixmap (SmallIcon ("1leftarrow"));
		add_mode = false;
	} else {
		select->setPixmap (SmallIcon ("1rightarrow"));
		add_mode = true;
	}
}

void RKVarSlot::listSelectionChanged () {
	RK_TRACE (PLUGIN);

	bool selection = false;

	ObjectList sellist;
	QListViewItem *item = list->firstChild ();
	while (item) {
		if (item->isSelected ()) {
			selection = true;
			RObject *robj = item_map[item];
			RK_ASSERT (robj);
			sellist.append (robj);
		}
		item = item->nextSibling ();
	}
	selected->setObjectList (sellist);

	setSelectButton (!selection);
}

void RKVarSlot::availablePropertyChanged (RKComponentPropertyBase *) {
	RK_TRACE (PLUGIN);

	list->clear ();
	item_map.clear ();

	ObjectList objlist = available->objectList ();
	ObjectList::const_iterator it = objlist.begin ();
	int i = 1;
	while (it != objlist.end ()) {
		QListViewItem *new_item = new QListViewItem (list, QString::number (i++), (*it)->getShortName ());
		list->insertItem (new_item);
	}

	if (!isSatisfied ()) {
		list->setPaletteBackgroundColor (QColor (255, 0, 0));
	} else {
		list->setPaletteBackgroundColor (QColor (255, 255, 255));
	}
}

void RKVarSlot::selectPressed () {
	RK_TRACE (PLUGIN);

	// first update the properties
	if (add_mode) {
		ObjectList objlist = source->objectList ();
		ObjectList::const_iterator it = objlist.begin ();
		while (it != objlist.end ()) {
			available->addObjectValue (*it);
			++it;
		}
	} else {		// remove-mode
		ObjectList objlist;
		if (multi) {
			ObjectList objlist = selected->objectList ();
		} else {
			objlist = available->objectList ();
		}

		ObjectList::const_iterator it = objlist.begin ();
		while (it != objlist.end ()) {
			available->removeObjectValue (*it);
			selected->removeObjectValue (*it);
			++it;
		}
	}

	setSelectButton (false);
}

#include "rkvarslot.moc"
