/***************************************************************************
                          editlabelsdialog  -  description
                             -------------------
    begin                : Tue Sep 21 2004
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
#include "editlabelsdialog.h"

#include <klocale.h>
#include <kdialogbase.h>

#include <qlistview.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qlineedit.h>

#include "../core/rkvariable.h"

#include "../debug.h"

EditLabelsDialog::EditLabelsDialog (QWidget *parent, RKVariable *var, int mode) : QDialog (parent) {
	RK_TRACE (EDITOR);
	RK_ASSERT (var);
	RK_ASSERT (var->objectOpened ());
	
	EditLabelsDialog::var = var;
	EditLabelsDialog::mode = mode;

	QVBoxLayout *mainvbox = new QVBoxLayout (this, KDialog::marginHint (), KDialog::spacingHint ());
	QLabel *label = new QLabel (i18n ("Levels can be assigned only to consecutive integers starting with 1. Warning: if you remove a level, the indices of the other levels may change, hence changing the values of the factor!"), this);
	label->setAlignment (Qt::AlignAuto | Qt::AlignVCenter | Qt::ExpandTabs | Qt::WordBreak);
	mainvbox->addWidget (label);
	
	QHBoxLayout *hbox = new QHBoxLayout (mainvbox, KDialog::spacingHint ());
	
	list = new QListView (this);
	list->addColumn (i18n ("Index"));
	list->addColumn (i18n ("Label"));
	list->setSorting (0);
	labels = var->getValueLabels ();
	if (labels) {
		int i=1;
		while (labels->contains (QString::number (i))) {
			new QListViewItem (list, QString::number (i), (*labels)[QString::number (i)]);
			++i;
		}
	} else {
		labels = new RObject::ValueLabels;
	}
	connect (list, SIGNAL (selectionChanged (QListViewItem *)), this, SLOT (listSelectionChanged (QListViewItem *)));
	hbox->addWidget (list);

	QVBoxLayout *buttonvbox = new QVBoxLayout (hbox, KDialog::spacingHint ());
	label_edit = new QLineEdit (this);
	connect (label_edit, SIGNAL (returnPressed ()), this, SLOT (labelEditEnterPressed ()));
	buttonvbox->addWidget (label_edit);
	
	add_button = new QPushButton (i18n ("Add"), this);
	connect (add_button, SIGNAL (clicked ()), this, SLOT (addButtonClicked ()));
	remove_button = new QPushButton (i18n ("Remove"), this);
	connect (remove_button, SIGNAL (clicked ()), this, SLOT (removeButtonClicked ()));
	remove_button->setEnabled (false);
	change_button = new QPushButton (i18n ("Change"), this);
	connect (change_button, SIGNAL (clicked ()), this, SLOT (changeButtonClicked ()));
	change_button->setEnabled (false);
	
	buttonvbox->addWidget (change_button);
	buttonvbox->addWidget (add_button);
	buttonvbox->addWidget (remove_button);
	
	QPushButton *ok_button = new QPushButton (i18n ("Ok"), this);
	connect (ok_button, SIGNAL (clicked ()), this, SLOT (accept ()));
	mainvbox->addWidget (ok_button);
	
	setCaption (i18n ("Levels / Value labels for '%1'").arg (var->getShortName ()));
}

EditLabelsDialog::~EditLabelsDialog () {
	RK_TRACE (EDITOR);
}

void EditLabelsDialog::addButtonClicked () {
	RK_TRACE (EDITOR);
	
	QString text = label_edit->text ();
	if (text.isEmpty ()) {
		text = "NA";
	}
	
	QString index = QString::number (list->childCount () + 1);
	new QListViewItem (list, index, text);
	labels->insert (index, text);
}

void EditLabelsDialog::removeButtonClicked () {
	RK_TRACE (EDITOR);

	QListViewItem *item = list->currentItem ();
	if (!item) {
		RK_ASSERT (false);
		return;
	}

	QString index = item->text (0);
	labels->remove (index);

	int ind = index.toInt ();
	RK_ASSERT ((ind >= 1) && (ind <= list->childCount ()));
	delete item;
	
	index = QString::number (++ind);
	while (labels->contains (index)) {
		item = list->findItem (index, 0);
		RK_ASSERT (item);
		delete (item);
		labels->insert (QString::number (ind - 1), (*labels)[index]);
		labels->remove (index);
		index = QString::number (++ind);
	}
}

void EditLabelsDialog::changeButtonClicked () {
	RK_TRACE (EDITOR);

	QListViewItem *item = list->currentItem ();
	if (!item) {
		RK_ASSERT (false);
		return;
	}
	
	QString text = label_edit->text ();
	if (text.isEmpty ()) {
		text = "NA";
	}
	
	item->setText (1, text);
	labels->insert (item->text (0), item->text (1));
}

void EditLabelsDialog::listSelectionChanged (QListViewItem *item) {
	RK_TRACE (EDITOR);
	
	if (item) {
		add_button->setEnabled (false);
		change_button->setEnabled (true);
		remove_button->setEnabled (true);

		label_edit->setText (item->text (1));
	} else {
		add_button->setEnabled (true);
		change_button->setEnabled (false);
		remove_button->setEnabled (false);
	}
}

void EditLabelsDialog::labelEditEnterPressed () {
	RK_TRACE (EDITOR);

	QListViewItem *item = list->currentItem ();
	if (item) {
		changeButtonClicked ();
	} else {
		addButtonClicked ();
	}
}

void EditLabelsDialog::accept () {
	RK_TRACE (EDITOR);

	if (labels->isEmpty ()) {
		var->setValueLabels (0);
	} else {
		var->setValueLabels (labels);
	}

	QDialog::accept ();
}

#include "editlabelsdialog.moc"
