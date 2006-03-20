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

	// initialize properties
	addChild ("source", source = new RKComponentPropertyRObjects (this, false));
	addChild ("available", available = new RKComponentPropertyRObjects (this, true));
	addChild ("selected", selected = new RKComponentPropertyRObjects (this, false));

	// find out about options
	if (multi = xml->getBoolAttribute (element, "multi", false, DL_INFO)) {
		available->setListLength (xml->getIntAttribute (element, "min_vars", 1, DL_INFO), xml->getIntAttribute (element, "min_vars_if_any", 0, DL_INFO), xml->getIntAttribute (element, "max_vars", 0, DL_INFO));
		connect (list, SIGNAL (selectionChanged ()), this, SLOT (listSelectionChanged ()));
	} else {
		available->setListLength (1, 1, 1);

		// make it look like a line-edit
		list->header ()->hide ();
		list->setFixedHeight (list->fontMetrics ().height () + 2*list->itemMargin () + 4);	// the height of a single line including margins
		list->setColumnWidthMode (0, QListView::Manual);
		list->setColumnWidth (0, 0);
		list->setHScrollBarMode (QScrollView::AlwaysOff);
		list->setVScrollBarMode (QScrollView::AlwaysOff);
		g_layout->setRowStretch (3, 1);		// so the label does not get separated from the view
	}

	// initialize filters
	available->setClassFilter (QStringList::split (" ", xml->getStringAttribute (element, "classes", QString::null, DL_INFO)));
	available->setRequired (xml->getBoolAttribute (element, "required", true, DL_INFO));
	available->setTypeFilter (QStringList::split (" ", xml->getStringAttribute (element, "types", QString::null, DL_INFO)));

	connect (available, SIGNAL (valueChanged (RKComponentPropertyBase *)), this, SLOT (availablePropertyChanged (RKComponentPropertyBase *)));
	availablePropertyChanged (available);		// initialize
}

RKVarSlot::~RKVarSlot (){
	RK_TRACE (PLUGIN);
}

void RKVarSlot::setSelectButton (bool add) {
	if (add) {
		select->setPixmap (SmallIcon ("1rightarrow"));
		add_mode = true;
	} else {
		select->setPixmap (SmallIcon ("1leftarrow"));
		add_mode = false;
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

	setSelectButton ((!selection) && (!available->atMaxLength ()));
}

void RKVarSlot::availablePropertyChanged (RKComponentPropertyBase *) {
	RK_TRACE (PLUGIN);

	list->clear ();
	item_map.clear ();

	RK_DO (qDebug ("contained in varslot: %s", available->value ().latin1 ()), PLUGIN, DL_DEBUG);

	ObjectList objlist = available->objectList ();
	ObjectList::const_iterator it = objlist.begin ();
	int i = 1;
	while (it != objlist.end ()) {
		QListViewItem *new_item = new QListViewItem (list, QString::number (i++), (*it)->getShortName ());
		list->insertItem (new_item);
		++it;
	}

	listSelectionChanged ();		// takes care of updating the select button

	changed ();
}

void RKVarSlot::updateLook () {
	RK_TRACE (PLUGIN);

	if (isEnabled ()) {
		if (!isSatisfied ()) {
			list->setPaletteBackgroundColor (QColor (255, 0, 0));
		} else {
			list->setPaletteBackgroundColor (QColor (255, 255, 255));
		}
	} else {
		if (!isSatisfied ()) {
			list->setPaletteBackgroundColor (QColor (200, 0, 0));
		} else {
			list->setPaletteBackgroundColor (QColor (200, 200, 200));
		}
	}
}

void RKVarSlot::selectPressed () {
	RK_TRACE (PLUGIN);

	RK_DO (qDebug ("select press in varslot: mode %d, source %s, selected %s", add_mode, source->value ().latin1 (), selected->value ().latin1 ()), PLUGIN, DL_DEBUG);

	// first update the properties
	if (add_mode) {
		if (multi) {
			ObjectList objlist = source->objectList ();
			ObjectList::const_iterator it = objlist.begin ();
			while (it != objlist.end ()) {
				available->addObjectValue (*it);
				++it;
			}
		} else {
			available->setObjectValue (source->objectValue ());
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
}

#include "rkvarslot.moc"
