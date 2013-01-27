/***************************************************************************
                          multistringselector  -  description
                             -------------------
    begin                : Fri Sep 10 2005
    copyright            : (C) 2005, 2013 by Thomas Friedrichsmeier
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

#include <QTreeView>
#include <QPushButton>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QStringListModel>

#include <klocale.h>

#include "rkstandardicons.h"
#include "../debug.h"

class RKStringListModelWithColumnLabel : public QStringListModel {
public:
	RKStringListModelWithColumnLabel (QObject *parent, const QString& _label) : QStringListModel (parent), label (_label) {};
	~RKStringListModelWithColumnLabel () {};
	QVariant headerData (int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const {
		if ((section == 0) && (orientation == Qt::Horizontal) && (role == Qt::DisplayRole)) return label;
		return QVariant ();
	};
	QString label;
};

MultiStringSelector::MultiStringSelector (const QString& label, QWidget* parent) : RKMultiStringSelectorV2 (label, parent) {
	RK_TRACE (MISC);

	model = new RKStringListModelWithColumnLabel (this, i18n ("Filename"));
	connect (this, SIGNAL (swapRows(int,int)), this, SLOT (swapRowsImpl(int,int)));
	connect (this, SIGNAL (insertNewStrings(int)), this, SLOT (insertNewStringsImpl(int)));
	setModel (model);
}

MultiStringSelector::~MultiStringSelector () {
	RK_TRACE (MISC);

	// child widgets deleted by qt
}

QStringList MultiStringSelector::getValues () {
	RK_TRACE (MISC);

	return model->stringList ();
}

void MultiStringSelector::setValues (const QStringList& values) {
	RK_TRACE (MISC);

	model->setStringList (values);
}

void MultiStringSelector::insertNewStringsImpl (int above_row) {
	RK_TRACE (MISC);

	QStringList new_strings;
	emit (getNewStrings (&new_strings));
	model->insertRows (above_row, new_strings.size ());
	for (int i = new_strings.size () - 1; i >= 0; --i) {
		model->setData (model->index (above_row + i, 0), new_strings[i]);
	}
}

void MultiStringSelector::swapRowsImpl (int rowa, int rowb) {
	RK_TRACE (MISC);

	QVariant dummy = model->data (model->index (rowa, 0), Qt::DisplayRole);
	model->setData (model->index (rowa, 0), model->data (model->index (rowb, 0), Qt::DisplayRole), Qt::DisplayRole);
	model->setData (model->index (rowb, 0), dummy, Qt::DisplayRole);
}

RKMultiStringSelectorV2::RKMultiStringSelectorV2 (const QString& label, QWidget* parent) : QWidget (parent) {
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

	tree_view = new QTreeView (this);
	tree_view->setSelectionMode (QAbstractItemView::SingleSelection);
	tree_view->setSelectionBehavior (QAbstractItemView::SelectRows);
	tree_view->setSortingEnabled (false);
	tree_view->setRootIsDecorated (false);
	main_box->addWidget (tree_view);

	add_button = new QPushButton (i18n ("Add"), this);
	connect (add_button, SIGNAL (clicked ()), this, SLOT (buttonClicked()));
	button_box->addWidget (add_button);

	remove_button = new QPushButton (i18n ("Remove"), this);
	connect (remove_button, SIGNAL (clicked ()), this, SLOT (buttonClicked()));
	button_box->addWidget (remove_button);

	button_box->addSpacing (10);

	up_button = new QPushButton (RKStandardIcons::getIcon (RKStandardIcons::ActionMoveUp), i18n ("Up"), this);
	connect (up_button, SIGNAL (clicked ()), this, SLOT (buttonClicked()));
	button_box->addWidget (up_button);

	down_button = new QPushButton (RKStandardIcons::getIcon (RKStandardIcons::ActionMoveDown), i18n ("Down"), this);
	connect (down_button, SIGNAL (clicked ()), this, SLOT (buttonClicked()));
	button_box->addWidget (down_button);
}

RKMultiStringSelectorV2::~RKMultiStringSelectorV2 () {
	RK_TRACE (MISC);
}

void RKMultiStringSelectorV2::setModel (QAbstractItemModel* model, int main_column) {
	RK_TRACE (MISC);

	if (model == tree_view->model ()) return;

	if (tree_view->selectionModel ()) {
		disconnect (tree_view->selectionModel (), SIGNAL (currentChanged(QModelIndex,QModelIndex)), this, SLOT (updateButtons()));
	}
	if (tree_view->model ()) {
		disconnect (tree_view->model (), 0, this, SLOT (anyModelDataChange()));
	}
	tree_view->setModel (model);
	connect (tree_view->selectionModel (), SIGNAL (currentChanged(QModelIndex,QModelIndex)), this, SLOT (updateButtons()));
	connect (model, SIGNAL (dataChanged(QModelIndex,QModelIndex)), this, SLOT (anyModelDataChange()));
	connect (model, SIGNAL (layoutChanged()), this, SLOT (anyModelDataChange()));
	connect (model, SIGNAL (rowsInserted(const QModelIndex&,int,int)), this, SLOT (anyModelDataChange()));
	connect (model, SIGNAL (rowsRemoved(const QModelIndex&,int,int)), this, SLOT (anyModelDataChange()));
	connect (model, SIGNAL (reset()), this, SLOT (anyModelDataChange()));

	if (main_column >= 0) tree_view->resizeColumnToContents (main_column);
	
	updateButtons ();
}

void RKMultiStringSelectorV2::buttonClicked () {
	RK_TRACE (MISC);

	tree_view->setFocus ();
	if (!tree_view->model ()) {
		RK_ASSERT (false);
		return;
	}

	int row = -1;
	QModelIndex index = tree_view->currentIndex ();
	if (index.isValid ()) row = index.row ();

	if (sender () == add_button) {
		if (row < 0) row = tree_view->model ()->rowCount ();
		emit (insertNewStrings (row));
	} else if (row < 0) {	// all actions below need a valid row
		RK_ASSERT (false);
	} else if (sender () == remove_button) {
		tree_view->model ()->removeRow (row);
	} else {
		int rowb;
		if (sender () == up_button) {
			RK_ASSERT (row >= 0);
			rowb = qMax (row - 1, 0);
		} else {
			RK_ASSERT (sender () == down_button);
			RK_ASSERT (row < tree_view->model ()->rowCount ());
			rowb = qMin (row + 1, tree_view->model ()->rowCount () - 1);
		}
		emit (swapRows (row, rowb));
		tree_view->setCurrentIndex (tree_view->model ()->index (rowb, 0));
	}
	anyModelDataChange ();
}

void RKMultiStringSelectorV2::updateButtons () {
	RK_TRACE (MISC);

	QModelIndex index = tree_view->currentIndex ();
	if (!index.isValid ()) {
		remove_button->setEnabled (false);
		up_button->setEnabled (false);
		down_button->setEnabled (false);
	} else {
		remove_button->setEnabled (true);
		up_button->setEnabled (index.row () > 0);
		down_button->setEnabled (index.row () < tree_view->model ()->rowCount () - 1);
	}
}

void RKMultiStringSelectorV2::anyModelDataChange () {
	RK_TRACE (MISC);
	emit (listChanged ());
}

#include "multistringselector.moc"
