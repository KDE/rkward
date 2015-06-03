/***************************************************************************
                          rkvarslot.cpp  -  description
                             -------------------
    begin                : Thu Nov 7 2002
    copyright            : (C) 2002-2015 by Thomas Friedrichsmeier
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
#include <QVBoxLayout>

#include <klocale.h>

#include "rkvarselector.h"
#include "../core/robject.h"
#include "../rkglobals.h"
#include "../debug.h"
#include "../misc/xmlhelper.h"
#include "../misc/rkstandardicons.h"

RKVarSlot::RKVarSlot (const QDomElement &element, RKComponent *parent_component, QWidget *parent_widget) : RKComponent (parent_component, parent_widget) {
	RK_TRACE (PLUGIN);

	XMLHelper *xml = parent_component->xmlHelper ();
	updating = false;

	// basic layout
	QGridLayout *g_layout = new QGridLayout (this);

	label_string = xml->i18nStringAttribute (element, "label", i18n ("Variable:"), DL_INFO);
	if (!label_string.isEmpty ()) {
		QLabel *label = new QLabel (label_string, this);
		g_layout->addWidget (label, 0, 2);
	}

	QVBoxLayout *button_layout = new QVBoxLayout ();
	select_button = new QPushButton (QString (), this);
	select_button->setIcon (RKStandardIcons::getIcon (RKStandardIcons::ActionAddRight));
	connect (select_button, SIGNAL (clicked()), this, SLOT (selectPressed()));
	button_layout->addWidget (select_button);
	remove_button = new QPushButton (QString (), this);
	remove_button->setIcon (RKStandardIcons::getIcon (RKStandardIcons::ActionRemoveLeft));
	connect (remove_button, SIGNAL (clicked()), this, SLOT (removePressed()));
	button_layout->addWidget (remove_button);
	button_layout->addStretch ();
	g_layout->addLayout (button_layout, 1, 0);
	g_layout->addItem (new QSpacerItem (5, 0), 0, 1);

	list = new QTreeWidget (this);
	list->setSelectionMode (QAbstractItemView::ExtendedSelection);
	list->setHeaderLabels (QStringList () << i18n ("Name"));
	list->setSortingEnabled (false);
	list->setUniformRowHeights (true);
	list->setRootIsDecorated (false);
	g_layout->addWidget (list, 1, 2);

	mode = (element.tagName () == "valueslot") ? Valueslot : Varslot;

	// initialize properties
	if (mode == Valueslot) {
		addChild ("source", source = new RKComponentPropertyStringList (this, false));
		addChild ("available", available = new RKComponentPropertyStringList (this, true));
		addChild ("selected", selected = new RKComponentPropertyStringList (this, false));
	} else {
		addChild ("source", source = new RKComponentPropertyRObjects (this, false));
		addChild ("available", available = new RKComponentPropertyRObjects (this, true));
		addChild ("selected", selected = new RKComponentPropertyRObjects (this, false));
	}
	source->setInternal (true);
	selected->setInternal (true);

	// find out about options
	if ((multi = xml->getBoolAttribute (element, "multi", false, DL_INFO))) {
		available->setAllowedLength (xml->getIntAttribute (element, "min_vars", 1, DL_INFO), xml->getIntAttribute (element, "min_vars_if_any", 1, DL_INFO), xml->getIntAttribute (element, "max_vars", 0, DL_INFO));
		connect (list, SIGNAL (itemSelectionChanged()), this, SLOT (listSelectionChanged()));
	} else {
		available->setAllowedLength (1, 1, 1);

		// make it look like a line-edit
		list->header ()->hide ();
		QTreeWidgetItem dummy (list);
		dummy.setText (0, "Tg");
		int top, left, bottom, right;
		list->getContentsMargins (&left, &top, &right, &bottom);
		list->setFixedHeight (list->visualItemRect (&dummy).height () + 2*list->visualItemRect (&dummy).top () + top + bottom);
		list->header ()->setStretchLastSection (true);
		list->setHorizontalScrollBarPolicy (Qt::ScrollBarAlwaysOff);
		list->setVerticalScrollBarPolicy (Qt::ScrollBarAlwaysOff);
		g_layout->setRowStretch (3, 1);		// so the label does not get separated from the view
	}

	if (mode == Varslot) {
		// initialize filters
		static_cast<RKComponentPropertyRObjects*> (available)->setClassFilter (xml->getStringAttribute (element, "classes", QString (), DL_INFO).split (' ', QString::SkipEmptyParts));
		static_cast<RKComponentPropertyRObjects*> (available)->setTypeFilter (xml->getStringAttribute (element, "types", QString (), DL_INFO).split (' ', QString::SkipEmptyParts));
		static_cast<RKComponentPropertyRObjects*> (available)->setDimensionFilter (xml->getIntAttribute (element, "num_dimensions", 0, DL_INFO), xml->getIntAttribute (element, "min_length", 0, DL_INFO), xml->getIntAttribute (element, "max_length", INT_MAX, DL_INFO));
	}
	available->setStripDuplicates (!xml->getBoolAttribute (element, "allow_duplicates", false, DL_INFO));
	setRequired (xml->getBoolAttribute (element, "required", false, DL_INFO));

	connect (available, SIGNAL (valueChanged(RKComponentPropertyBase*)), this, SLOT (availablePropertyChanged(RKComponentPropertyBase*)));
	availablePropertyChanged (available);	// initialize
}

RKVarSlot::~RKVarSlot (){
	RK_TRACE (PLUGIN);
}

void RKVarSlot::listSelectionChanged () {
	RK_TRACE (PLUGIN);

	QModelIndexList selrows = list->selectionModel ()->selectedRows (0);
	if (mode == Valueslot) {
		QStringList sellist;
		for (int i = 0; i < selrows.count (); ++i) sellist.append (static_cast<RKComponentPropertyStringList*> (available)->valueAt (selrows[i].row ()));
		static_cast<RKComponentPropertyStringList*> (selected)->setValueList (sellist);
	} else {
		RObject::ObjectList sellist;
		for (int i = 0; i < selrows.count (); ++i) sellist.append (static_cast<RKComponentPropertyRObjects*> (available)->objectAt (selrows[i].row ()));
		static_cast<RKComponentPropertyRObjects*> (selected)->setObjectList (sellist);
	}

	if (multi) remove_button->setEnabled (!selrows.isEmpty ());
	else {
		select_button->setVisible (available->listLength () == 0);
		remove_button->setVisible (available->listLength () > 0);
	}
}

void RKVarSlot::availablePropertyChanged (RKComponentPropertyBase *) {
	RK_TRACE (PLUGIN);

	if (updating) return;

	list->clear ();

	RK_DEBUG (PLUGIN, DL_DEBUG, "contained in varslot: %s", qPrintable (fetchStringValue (available)));

	int len = available->listLength ();
	for (int i = 0; i < len; ++i) {
		QTreeWidgetItem *new_item = new QTreeWidgetItem (list);

		if (mode == Valueslot) {
			new_item->setText (0, static_cast<RKComponentPropertyStringList*> (available)->valueAt (i));
		} else {
			RObject *object = static_cast<RKComponentPropertyRObjects*> (available)->objectAt (i);
			new_item->setText (0, object->getShortName ());
			QString probs = static_cast<RKComponentPropertyRObjects*> (available)->objectProblems (i);
			if (!probs.isEmpty ()) {
				new_item->setToolTip (0, i18n ("<p>This object is not allowed, here, for the following reason(s):</p>") + probs);
				new_item->setIcon (0, RKStandardIcons::getIcon (RKStandardIcons::ActionDeleteVar));
			}
		}
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

void RKVarSlot::addOrRemove (bool add) {
	RK_TRACE (PLUGIN);
	RK_DEBUG (PLUGIN, DL_DEBUG, "select press in varslot: mode %d, source %s, selected %s", add, qPrintable (fetchStringValue (source)), qPrintable (fetchStringValue (selected)));

	updating = true;
	// first update the properties
	if (add) {
		if (!multi) available->setValueList (QStringList ());  // replace
		int len = source->listLength ();
		for (int i = 0; i < len; ++i) {
			if (mode == Valueslot) {
				static_cast<RKComponentPropertyStringList*> (available)->setValueAt (static_cast<RKComponentPropertyStringList*> (available)->listLength (), static_cast<RKComponentPropertyStringList*> (source)->valueAt (i));
			} else {
				static_cast<RKComponentPropertyRObjects*> (available)->addObjectValue (static_cast<RKComponentPropertyRObjects*> (source)->objectAt (i));
			}
		}
	} else {		// remove-mode
		QModelIndexList removed = list->selectionModel ()->selectedRows (0);       // Note: list contains no dupes, but is unsorted.
		QList<int> removed_rows;
		for (int i = 0; i < removed.size (); ++i) {
			removed_rows.append (removed[i].row ());
		}
		qSort (removed_rows);
		if (!multi && removed_rows.isEmpty ()) removed_rows.append (0);
		for (int i = removed_rows.size () - 1; i >= 0; --i) {
			available->removeAt (removed_rows[i]);
		}
		selected->setValueList (QStringList ());
	}
	updating = false;
	availablePropertyChanged (available);
}

void RKVarSlot::selectPressed () {
	RK_TRACE (PLUGIN);
	addOrRemove (true);
}

void RKVarSlot::removePressed () {
	RK_TRACE (PLUGIN);
	addOrRemove (false);
}

QStringList RKVarSlot::getUiLabelPair () const {
	RK_TRACE (PLUGIN);

	QStringList ret (label_string);
	ret.append (available->value ().toString ());
	return ret;
}

#include "rkvarslot.moc"
