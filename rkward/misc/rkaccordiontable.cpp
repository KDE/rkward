/***************************************************************************
                          rkaccordiontable  -  description
                             -------------------
    begin                : Fri Oct 24 2015
    copyright            : (C) 2015 by Thomas Friedrichsmeier
    email                : thomas.friedrichsmeier@kdemail.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "rkaccordiontable.h"

#include <kvbox.h>

#include "../debug.h"

class RKAccordionDummyModel : public QAbstractProxyModel {
public:
	RKAccordionDummyModel (QObject *parent) : QAbstractProxyModel (parent) {};

	QModelIndex mapFromSource (const QModelIndex& sindex) const {
		return (createIndex (sindex.row () * 2 + 1, sindex.column ()));
	}

    QModelIndex mapToSource (const QModelIndex& pindex) const {
		int srow;
		if (pindex.row () % 2) srow = (pindex.row () - 1) / 2;
		else srow = pindex.row () / 2;
		return (createIndex (srow, pindex.column ()));
	}
	
    Qt::ItemFlags flags (const QModelIndex& index) const {
		if (index.row () % 2) return (Qt::NoItemFlags);
		return QAbstractProxyModel::flags (index);
	}
	
    int rowCount (const QModelIndex& parent) const {
		return sourceModel ()->rowCount (parent) * 2;
	}

	int columnCount(const QModelIndex& parent) const {
		return sourceModel ()->columnCount (parent);
	}
};

#include <QLabel>
RKAccordionTable::RKAccordionTable (QWidget* parent) : QTableView (parent) {
	RK_TRACE (MISC);

	default_widget = new KVBox;
	new QLabel ("This is the content\nExcept it's just a dummy!!!!!!!!!!!!!!!", default_widget);
	setSelectionBehavior (SelectRows);
	setSelectionMode (SingleSelection);
	pmodel = new RKAccordionDummyModel (this);
}

RKAccordionTable::~RKAccordionTable () {
	RK_TRACE (MISC);

	delete default_widget;
}

void RKAccordionTable::currentChanged (const QModelIndex& current, const QModelIndex& previous) {
	RK_TRACE (MISC);
	RK_ASSERT (current.isValid ());

	int cur = current.row;
	if (previous.isValid ()) {
		int prev = previous.row;
		int height = rowHeight (prev + 1);
		hideRow (prev + 1);
		setRowHeight (cur + 1, height);
	}
	showRow (cur + 1);
}

void RKAccordionTable::setModel (QAbstractItemModel* model) {
	RK_TRACE (MISC);

	pmodel->setSourceModel (model);
	QTableView::setModel (pmodel);

	for (int i = 1; i < model->rowCount (); ++i) {
		hideRow (i * 2 + 1);
	}
	if (model->rowCount () > 0) showRow (1);
}

void RKAccordionTable::resizeEvent (QResizeEvent* event) {
	// TODO
	QAbstractItemView::resizeEvent (event);
}


// TODO
// - add buttons to each row
// - handle row removals
// - handle resize

// KF5 TODO: remove:
#include "rkaccordiontable.moc"
