/***************************************************************************
                          rkvarslot.cpp  -  description
                             -------------------
    begin                : Thu Nov 7 2002
    copyright            : (C) 2002, 2007 by Thomas Friedrichsmeier
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

#include <qlabel.h>
#include <qpushbutton.h>
#include <q3listview.h>
#include <qlayout.h>
#include <q3header.h>
#include <qstringlist.h>
//Added by qt3to4:
#include <Q3GridLayout>

#include <klocale.h>
#include <kiconloader.h>

#include "rkvarselector.h"
#include "../core/robject.h"
#include "../rkglobals.h"
#include "../debug.h"
#include "../misc/xmlhelper.h"


RKVarSlot::RKVarSlot (const QDomElement &element, RKComponent *parent_component, QWidget *parent_widget) : RKComponent (parent_component, parent_widget) {
	RK_TRACE (PLUGIN);

	XMLHelper *xml = XMLHelper::getStaticHelper ();

	// basic layout
	Q3GridLayout *g_layout = new Q3GridLayout (this, 3, 3, RKGlobals::spacingHint ());

	QLabel *label = new QLabel (xml->getStringAttribute (element, "label", i18n ("Variable:"), DL_INFO), this);
	g_layout->addWidget (label, 0, 2);

	select = new QPushButton (QString::null, this);
	setSelectButton (false);
	connect (select, SIGNAL (clicked ()), this, SLOT (selectPressed ()));
	g_layout->addWidget (select, 1, 0);
	g_layout->addColSpacing (1, 5);

	list = new Q3ListView (this);
	list->setSelectionMode (Q3ListView::Extended);
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
		available->setListLength (xml->getIntAttribute (element, "min_vars", 1, DL_INFO), xml->getIntAttribute (element, "min_vars_if_any", 1, DL_INFO), xml->getIntAttribute (element, "max_vars", 0, DL_INFO));
		connect (list, SIGNAL (selectionChanged ()), this, SLOT (listSelectionChanged ()));
	} else {
		available->setListLength (1, 1, 1);

		// make it look like a line-edit
		list->header ()->hide ();
		list->setFixedHeight (list->fontMetrics ().height () + 2*list->itemMargin () + 4);	// the height of a single line including margins
		list->setColumnWidthMode (0, Q3ListView::Manual);
		list->setColumnWidth (0, 0);
		list->setHScrollBarMode (Q3ScrollView::AlwaysOff);
		list->setVScrollBarMode (Q3ScrollView::AlwaysOff);
		g_layout->setRowStretch (3, 1);		// so the label does not get separated from the view
	}

	// initialize filters
	available->setClassFilter (QStringList::split (" ", xml->getStringAttribute (element, "classes", QString::null, DL_INFO)));
	setRequired (xml->getBoolAttribute (element, "required", false, DL_INFO));
	available->setTypeFilter (QStringList::split (" ", xml->getStringAttribute (element, "types", QString::null, DL_INFO)));
	available->setDimensionFilter (xml->getIntAttribute (element, "num_dimensions", 0, DL_INFO), xml->getIntAttribute (element, "min_length", 0, DL_INFO), xml->getIntAttribute (element, "max_length", INT_MAX, DL_INFO));

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

	RObject::ObjectList sellist;
	Q3ListViewItem *item = list->firstChild ();
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

	setSelectButton (((!multi) || (!selection)) && (!available->atMaxLength ()));
}

void RKVarSlot::availablePropertyChanged (RKComponentPropertyBase *) {
	RK_TRACE (PLUGIN);

	list->clear ();
	item_map.clear ();

	RK_DO (qDebug ("contained in varslot: %s", available->value ().toLatin1 ().data ()), PLUGIN, DL_DEBUG);

	RObject::ObjectList objlist = available->objectList ();
	RObject::ObjectList::const_iterator it = objlist.begin ();
	int i = 1;
	while (it != objlist.end ()) {
		Q3ListViewItem *new_item = new Q3ListViewItem (list, QString::number (i++), (*it)->getShortName ());
		list->insertItem (new_item);
		item_map.insert (new_item, *it);
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

	RK_DO (qDebug ("select press in varslot: mode %d, source %s, selected %s", add_mode, source->value ().toLatin1 ().data (), selected->value ().toLatin1 ().data ()), PLUGIN, DL_DEBUG);

	// first update the properties
	if (add_mode) {
		if (multi) {
			RObject::ObjectList objlist = source->objectList ();
			RObject::ObjectList::const_iterator it = objlist.constBegin ();
			while (it != objlist.constEnd ()) {
				available->addObjectValue (*it);
				++it;
			}
		} else {
			if (source->objectValue ()) available->setObjectValue (source->objectValue ());
		}
	} else {		// remove-mode
		RObject::ObjectList objlist;
		if (multi) {
			objlist = selected->objectList ();
		} else {
			objlist = available->objectList ();
		}

		RObject::ObjectList::const_iterator it = objlist.begin ();
		while (it != objlist.end ()) {
			available->removeObjectValue (*it);
			selected->removeObjectValue (*it);
			++it;
		}
	}
}

#include "rkvarslot.moc"
