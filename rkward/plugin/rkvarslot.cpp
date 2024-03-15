/*
rkvarslot.cpp - This file is part of RKWard (https://rkward.kde.org). Created: Thu Nov 7 2002
SPDX-FileCopyrightText: 2002-2015 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "rkvarslot.h"

#include <qlabel.h>
#include <qpushbutton.h>
#include <QTreeWidget>
#include <QHeaderView>
#include <qstringlist.h>
#include <QEvent>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QIcon>

#include <KLocalizedString>

#include "rkvarselector.h"
#include "../core/robject.h"
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
	connect (select_button, &QPushButton::clicked, this, &RKVarSlot::selectPressed);
	button_layout->addWidget (select_button);
	remove_button = new QPushButton (QString (), this);
	remove_button->setIcon (RKStandardIcons::getIcon (RKStandardIcons::ActionRemoveLeft));
	connect (remove_button, &QPushButton::clicked, this, &RKVarSlot::removePressed);
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
		connect (list, &QTreeWidget::itemSelectionChanged, this, &RKVarSlot::listSelectionChanged);
	} else {
		available->setAllowedLength (1, 1, 1);

		// make it look like a line-edit
		list->header ()->hide ();
		QTreeWidgetItem dummy (list);
		dummy.setText (0, "Tg");
		QMargins margins = list->contentsMargins();
		list->setFixedHeight(list->visualItemRect(&dummy).height() + 2*list->visualItemRect(&dummy).top() + margins.top() + margins.bottom());
		list->header ()->setStretchLastSection (true);
		list->setHorizontalScrollBarPolicy (Qt::ScrollBarAlwaysOff);
		list->setVerticalScrollBarPolicy (Qt::ScrollBarAlwaysOff);
		g_layout->setRowStretch (3, 1);		// so the label does not get separated from the view
	}

	if (mode == Varslot) {
		// initialize filters
		static_cast<RKComponentPropertyRObjects*> (available)->setClassFilter (xml->getStringAttribute (element, "classes", QString (), DL_INFO).split (' ', Qt::SkipEmptyParts));
		static_cast<RKComponentPropertyRObjects*> (available)->setTypeFilter (xml->getStringAttribute (element, "types", QString (), DL_INFO).split (' ', Qt::SkipEmptyParts));
		static_cast<RKComponentPropertyRObjects*> (available)->setDimensionFilter (xml->getIntAttribute (element, "num_dimensions", 0, DL_INFO), xml->getIntAttribute (element, "min_length", 0, DL_INFO), xml->getIntAttribute (element, "max_length", INT_MAX, DL_INFO));
		static_cast<RKComponentPropertyRObjects*> (available)->setObjectProblemsAreErrors (false);
	}
	available->setStripDuplicates (!xml->getBoolAttribute (element, "allow_duplicates", false, DL_INFO));
	setRequired (xml->getBoolAttribute (element, "required", false, DL_INFO));

	connect (available, &RKComponentPropertyBase::valueChanged, this, &RKVarSlot::availablePropertyChanged);
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
				new_item->setToolTip (0, i18n ("<p>Using this object, here, may lead to failures or unexpected results, for the following reason(s):</p>") + probs);
				new_item->setIcon (0, QIcon::fromTheme ("task-attention"));
			}
		}
	}
	if (multi) list->resizeColumnToContents (0);

	listSelectionChanged ();		// takes care of updating the select button

	changed ();
}

void RKVarSlot::updateLook () {
	RK_TRACE (PLUGIN);

	if (!isSatisfied ()) {		// implies that it is enabled
		list->setStyleSheet(QString("background: red; color: black"));
	} else {
		if (isEnabled ()) {
			list->setStyleSheet(QString(""));
		} else {
			list->setStyleSheet(QString("background: rgb(200, 200, 200); color: black"));
		}
	}
}

void RKVarSlot::changeEvent (QEvent* event) {
	if (event->type () == QEvent::EnabledChange) updateLook ();
	RKComponent::changeEvent (event);
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
		std::sort(removed_rows.begin(), removed_rows.end());
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

