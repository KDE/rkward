/*
multistringselector - This file is part of RKWard (https://rkward.kde.org). Created: Fri Sep 10 2005
SPDX-FileCopyrightText: 2005-2013 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "multistringselector.h"

#include <QTreeView>
#include <QPushButton>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QStringListModel>

#include <KLocalizedString>

#include "rkstandardicons.h"
#include "../debug.h"

class RKStringListModelWithColumnLabel : public QStringListModel {
public:
	RKStringListModelWithColumnLabel (QObject *parent, const QString& _label) : QStringListModel (parent), label (_label) {};
	~RKStringListModelWithColumnLabel () {};
	QVariant headerData (int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override {
		if ((section == 0) && (orientation == Qt::Horizontal) && (role == Qt::DisplayRole)) return label;
		return QVariant ();
	};
	QString label;
};

MultiStringSelector::MultiStringSelector (const QString& label, QWidget* parent) : RKMultiStringSelectorV2 (label, parent) {
	RK_TRACE (MISC);

	model = new RKStringListModelWithColumnLabel (this, i18n ("Filename"));
	connect (this, &MultiStringSelector::swapRows, this, &MultiStringSelector::swapRowsImpl);
	connect (this, &MultiStringSelector::insertNewStrings, this, &MultiStringSelector::insertNewStringsImpl);
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
	Q_EMIT getNewStrings(&new_strings);
	model->insertRows (above_row, new_strings.size ());
	for (int i = new_strings.size () - 1; i >= 0; --i) {
		model->setData(model->index (above_row + i, 0), new_strings[i]);
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

	add_at_bottom = false;
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
	connect (add_button, &QPushButton::clicked, this, &RKMultiStringSelectorV2::buttonClicked);
	button_box->addWidget (add_button);

	remove_button = new QPushButton (i18n ("Remove"), this);
	connect (remove_button, &QPushButton::clicked, this, &RKMultiStringSelectorV2::buttonClicked);
	button_box->addWidget (remove_button);

	button_box->addSpacing (10);

	up_button = new QPushButton (RKStandardIcons::getIcon (RKStandardIcons::ActionMoveUp), i18n ("Up"), this);
	connect (up_button, &QPushButton::clicked, this, &RKMultiStringSelectorV2::buttonClicked);
	button_box->addWidget (up_button);

	down_button = new QPushButton (RKStandardIcons::getIcon (RKStandardIcons::ActionMoveDown), i18n ("Down"), this);
	connect (down_button, &QPushButton::clicked, this, &RKMultiStringSelectorV2::buttonClicked);
	button_box->addWidget (down_button);
}

RKMultiStringSelectorV2::~RKMultiStringSelectorV2 () {
	RK_TRACE (MISC);
}

void RKMultiStringSelectorV2::setModel (QAbstractItemModel* model, int main_column) {
	RK_TRACE (MISC);

	if (model == tree_view->model ()) return;

	if (tree_view->selectionModel ()) {
		disconnect (tree_view->selectionModel (), &QItemSelectionModel::currentChanged, this, &RKMultiStringSelectorV2::updateButtons);
	}
	if (tree_view->model ()) {
		// NOTE: Commented version gives compile error. Fortunately, we do not connect the model to any other slots, so the version below is ok.
		//disconnect (tree_view->model (), 0, this, &RKMultiStringSelectorV2::anyModelDataChange);
		disconnect(tree_view->model(), nullptr, this, nullptr);
	}
	tree_view->setModel (model);
	connect (tree_view->selectionModel (), &QItemSelectionModel::currentChanged, this, &RKMultiStringSelectorV2::updateButtons);
	connect (model, &QAbstractItemModel::dataChanged, this, &RKMultiStringSelectorV2::anyModelDataChange);
	connect (model, &QAbstractItemModel::layoutChanged, this, &RKMultiStringSelectorV2::anyModelDataChange);
	connect (model, &QAbstractItemModel::rowsInserted, this, &RKMultiStringSelectorV2::anyModelDataChange);
	connect (model, &QAbstractItemModel::rowsRemoved, this, &RKMultiStringSelectorV2::anyModelDataChange);
	connect (model, &QAbstractItemModel::modelReset, this, &RKMultiStringSelectorV2::anyModelDataChange);

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
		if (add_at_bottom || (row < 0)) row = tree_view->model ()->rowCount ();
		Q_EMIT insertNewStrings(row);
		tree_view->setCurrentIndex (tree_view->model ()->index (row, 0));
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
		Q_EMIT swapRows(row, rowb);
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
	Q_EMIT listChanged();
}

