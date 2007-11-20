/***************************************************************************
                          multistringselector  -  description
                             -------------------
    begin                : Fri Sep 10 2005
    copyright            : (C) 2005 by Thomas Friedrichsmeier
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

#include "multistringselector.h"

#include <QTreeWidget>
#include <qpushbutton.h>
#include <qlabel.h>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include <klocale.h>

#include "rkstandardicons.h"
#include "../debug.h"

MultiStringSelector::MultiStringSelector (const QString& label, QWidget* parent) : QWidget (parent) {
	RK_TRACE (MISC);

	QHBoxLayout *hbox = new QHBoxLayout (this);
	hbox->setContentsMargins (0, 0, 0, 0);

	QVBoxLayout *main_box = new QVBoxLayout ();
	hbox->addLayout (main_box);
	main_box->setContentsMargins (0, 0, 0, 0);
	QVBoxLayout *button_box = new QVBoxLayout ();
	hbox->addLayout (button_box);
	button_box->setContentsMargins (0, 0, 0, 0);

	QLabel *label_widget = new QLabel (label, this);
	main_box->addWidget (label_widget);

	tree_view = new QTreeWidget (this);
	tree_view->setHeaderLabel (i18n ("Filename"));
	tree_view->setSelectionMode (QAbstractItemView::SingleSelection);
	tree_view->setSortingEnabled (false);
	connect (tree_view, SIGNAL (itemSelectionChanged ()), this, SLOT (listSelectionChanged ()));
	main_box->addWidget (tree_view);

	add_button = new QPushButton (i18n ("Add"), this);
	connect (add_button, SIGNAL (clicked ()), this, SLOT (addButtonClicked ()));
	button_box->addWidget (add_button);

	remove_button = new QPushButton (i18n ("Remove"), this);
	connect (remove_button, SIGNAL (clicked ()), this, SLOT (removeButtonClicked ()));
	button_box->addWidget (remove_button);

	button_box->addSpacing (10);

	up_button = new QPushButton (RKStandardIcons::getIcon (RKStandardIcons::ActionMoveUp), i18n ("Up"), this);
	connect (up_button, SIGNAL (clicked ()), this, SLOT (upButtonClicked ()));
	button_box->addWidget (up_button);

	down_button = new QPushButton (RKStandardIcons::getIcon (RKStandardIcons::ActionMoveDown), i18n ("Down"), this);
	connect (down_button, SIGNAL (clicked ()), this, SLOT (downButtonClicked ()));
	button_box->addWidget (down_button);

	listSelectionChanged ();		// causes remove, up, and down buttons to be disabled (since no item is selected)
}

MultiStringSelector::~MultiStringSelector () {
	RK_TRACE (MISC);

	// child widgets and listviewitems deleted by qt
}

QStringList MultiStringSelector::getValues () {
	RK_TRACE (MISC);

	QStringList list;
	for (int i = 0; i < tree_view->topLevelItemCount (); ++i) {
		QTreeWidgetItem* item = tree_view->topLevelItem (i);
		RK_ASSERT (item);
		list.append (item->text (0));

	}

	return list;
}

void MultiStringSelector::setValues (const QStringList& values) {
	RK_TRACE (MISC);

	tree_view->clear ();
	for (QStringList::const_iterator it = values.begin (); it != values.end (); ++it) {
		QTreeWidgetItem* item = new QTreeWidgetItem (tree_view);
		item->setText (0, (*it));
	}
	listSelectionChanged ();
	emit (listChanged ());
}

void MultiStringSelector::addButtonClicked () {
	RK_TRACE (MISC);

	tree_view->setFocus ();
	QStringList new_strings;
	emit (getNewStrings (&new_strings));
	for (QStringList::const_iterator it = new_strings.begin (); it != new_strings.end (); ++it) {
		QTreeWidgetItem* item = new QTreeWidgetItem (tree_view);
		item->setText (0, (*it));
	}
	emit (listChanged ());
	listSelectionChanged ();		// update button states
}

QTreeWidgetItem* MultiStringSelector::treeSelectedItem () const {
	RK_TRACE (MISC);

	QList<QTreeWidgetItem *> sel = tree_view->selectedItems ();
	if (sel.isEmpty ()) return 0;
	RK_ASSERT (sel.count () == 1);
	return sel[0];
}

void MultiStringSelector::removeButtonClicked () {
	RK_TRACE (MISC);

	tree_view->setFocus ();
	delete (treeSelectedItem ());
	emit (listChanged ());
}

void MultiStringSelector::upButtonClicked () {
	RK_TRACE (MISC);

	tree_view->setFocus ();
	QTreeWidgetItem* sel = treeSelectedItem ();
	if (!sel) {
		RK_ASSERT (false);
		return;
	}
	int pos = tree_view->indexOfTopLevelItem (sel);
	if (pos <= 0) return;	// already at top

	tree_view->insertTopLevelItem (pos - 1, tree_view->takeTopLevelItem (pos));
	tree_view->setCurrentItem (sel);
	emit (listChanged ());
}

void MultiStringSelector::downButtonClicked () {
	RK_TRACE (MISC);

	tree_view->setFocus ();
	QTreeWidgetItem* sel = treeSelectedItem ();
	if (!sel) {
		RK_ASSERT (false);
		return;
	}
	int pos = tree_view->indexOfTopLevelItem (sel);
	if (pos >= (tree_view->topLevelItemCount () - 1)) return;	// already at bottom

	tree_view->insertTopLevelItem (pos + 1, tree_view->takeTopLevelItem (pos));
	tree_view->setCurrentItem (sel);
	emit (listChanged ());
}

void MultiStringSelector::listSelectionChanged () {
	RK_TRACE (MISC);

	if (treeSelectedItem ()) {
		remove_button->setEnabled (true);
		up_button->setEnabled (true);
		down_button->setEnabled (true);
	} else {
		remove_button->setEnabled (false);
		up_button->setEnabled (false);
		down_button->setEnabled (false);
	}
}

#include "multistringselector.moc"
