/***************************************************************************
                          rkvarslot.cpp  -  description
                             -------------------
    begin                : Thu Nov 7 2002
    copyright            : (C) 2002, 2007, 2008, 2009, 2011, 2012 by Thomas Friedrichsmeier
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
#include <QTreeWidget>
#include <QHeaderView>
#include <qstringlist.h>
#include <QGridLayout>

#include <klocale.h>

#include "rkvarselector.h"
#include "../core/robject.h"
#include "../rkglobals.h"
#include "../debug.h"
#include "../misc/xmlhelper.h"
#include "../misc/rkstandardicons.h"

RKVarSlot::RKVarSlot (const QDomElement &element, RKComponent *parent_component, QWidget *parent_widget) : RKComponent (parent_component, parent_widget) {
	RK_TRACE (PLUGIN);

	XMLHelper *xml = XMLHelper::getStaticHelper ();

	// basic layout
	QGridLayout *g_layout = new QGridLayout (this);

	QLabel *label = new QLabel (xml->getStringAttribute (element, "label", i18n ("Variable:"), DL_INFO), this);
	g_layout->addWidget (label, 0, 2);

	select = new QPushButton (QString::null, this);
	setSelectButton (false);
	connect (select, SIGNAL (clicked ()), this, SLOT (selectPressed ()));
	g_layout->addWidget (select, 1, 0);
	g_layout->addItem (new QSpacerItem (5, 0), 0, 1);

	list = new QTreeWidget (this);
	list->setSelectionMode (QAbstractItemView::ExtendedSelection);
	list->setHeaderLabels (QStringList () << " " << i18n ("Name"));
	list->setSortingEnabled (false);
	list->setUniformRowHeights (true);
	list->setRootIsDecorated (false);
	g_layout->addWidget (list, 1, 2);

	// initialize properties
	addChild ("source", source = new RKComponentPropertyRObjects (this, false));
	source->setInternal (true);
	addChild ("available", available = new RKComponentPropertyRObjects (this, true));
	addChild ("selected", selected = new RKComponentPropertyRObjects (this, false));
	selected->setInternal (true);

	// find out about options
	if ((multi = xml->getBoolAttribute (element, "multi", false, DL_INFO))) {
		available->setListLength (xml->getIntAttribute (element, "min_vars", 1, DL_INFO), xml->getIntAttribute (element, "min_vars_if_any", 1, DL_INFO), xml->getIntAttribute (element, "max_vars", 0, DL_INFO));
		connect (list, SIGNAL (itemSelectionChanged ()), this, SLOT (listSelectionChanged ()));
	} else {
		available->setListLength (1, 1, 1);

		// make it look like a line-edit
		list->header ()->hide ();
		QTreeWidgetItem dummy (list);
		dummy.setText (0, "Tg");
		int top, left, bottom, right;
		list->getContentsMargins (&left, &top, &right, &bottom);
		list->setFixedHeight (list->visualItemRect (&dummy).height () + 2*list->visualItemRect (&dummy).top () + top + bottom);
		list->header ()->setStretchLastSection (true);
		list->hideColumn (0);
		list->setHorizontalScrollBarPolicy (Qt::ScrollBarAlwaysOff);
		list->setVerticalScrollBarPolicy (Qt::ScrollBarAlwaysOff);
		g_layout->setRowStretch (3, 1);		// so the label does not get separated from the view
	}

	// initialize filters
	available->setClassFilter (xml->getStringAttribute (element, "classes", QString (), DL_INFO).split (" ", QString::SkipEmptyParts));
	setRequired (xml->getBoolAttribute (element, "required", false, DL_INFO));
	available->setTypeFilter (xml->getStringAttribute (element, "types", QString::null, DL_INFO).split (" ", QString::SkipEmptyParts));
	available->setDimensionFilter (xml->getIntAttribute (element, "num_dimensions", 0, DL_INFO), xml->getIntAttribute (element, "min_length", 0, DL_INFO), xml->getIntAttribute (element, "max_length", INT_MAX, DL_INFO));

	connect (available, SIGNAL (valueChanged (RKComponentPropertyBase *)), this, SLOT (availablePropertyChanged (RKComponentPropertyBase *)));
	availablePropertyChanged (available);		// initialize
}

RKVarSlot::~RKVarSlot (){
	RK_TRACE (PLUGIN);
}

void RKVarSlot::setSelectButton (bool add) {
	if (add) {
		select->setIcon (RKStandardIcons::getIcon (RKStandardIcons::ActionAddRight));
		add_mode = true;
	} else {
		select->setIcon (RKStandardIcons::getIcon (RKStandardIcons::ActionRemoveLeft));
		add_mode = false;
	}
}

void RKVarSlot::listSelectionChanged () {
	RK_TRACE (PLUGIN);

	RObject::ObjectList sellist;
	QList<QTreeWidgetItem*> selitems = list->selectedItems ();
	for (int i = 0; i < selitems.count (); ++i) sellist.append (item_map.value (selitems[i]));

	selected->setObjectList (sellist);

	setSelectButton (((!multi) || (selitems.isEmpty ())) && (!available->atMaxLength ()));
}

void RKVarSlot::availablePropertyChanged (RKComponentPropertyBase *) {
	RK_TRACE (PLUGIN);

	list->clear ();
	item_map.clear ();

	RK_DO (qDebug ("contained in varslot: %s", qPrintable (fetchStringValue (available))), PLUGIN, DL_DEBUG);

	RObject::ObjectList objlist = available->objectList ();
	for (int i = 0; i < objlist.count (); ++i) {
		RObject *object = objlist[i];

		QTreeWidgetItem *new_item = new QTreeWidgetItem (list);
		new_item->setText (0, QString::number (i + 1));
		new_item->setText (1, object->getShortName ());
		QString probs = available->objectProblems (i);
		if (!probs.isEmpty ()) {
			new_item->setToolTip (1, i18n ("<p>This object is not allowed, here, for the following reason(s):</p>") + probs);
			new_item->setIcon (1, RKStandardIcons::getIcon (RKStandardIcons::ActionDeleteVar));
		}
		item_map.insert (new_item, object);
	}
	if (multi) list->resizeColumnToContents (0);

	listSelectionChanged ();		// takes care of updating the select button

	changed ();
}

void RKVarSlot::updateLook () {
	RK_TRACE (PLUGIN);

	QPalette palette = list->palette ();
	if (!isSatisfied ()) {		// implies that it is enabled
		palette.setColor (QPalette::Window, QColor (255, 0, 0));
	} else {
		if (isEnabled ()) {
			palette.setColor (QPalette::Window, QColor (255, 255, 255));
		} else {
			palette.setColor (QPalette::Window, QColor (200, 200, 200));
		}
	}
	if (!multi) palette.setColor (QPalette::Base, palette.color (QPalette::Window));
	list->setPalette(palette);
}

void RKVarSlot::selectPressed () {
	RK_TRACE (PLUGIN);

	RK_DO (qDebug ("select press in varslot: mode %d, source %s, selected %s", add_mode, qPrintable (fetchStringValue (source)), qPrintable (fetchStringValue (selected))), PLUGIN, DL_DEBUG);

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
