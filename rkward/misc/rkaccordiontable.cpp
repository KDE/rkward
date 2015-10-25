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

#include <QPointer>
#include <QVBoxLayout>
#include <QAbstractProxyModel>
#include <kvbox.h>

#include "../debug.h"

/** Maps from the Optionset data model to the model used internally in the RKAccordionTable
 *  (a dummy child item is inserted for each actual row). This item can _not_ actually be accessed
 *  in a meaningful way. The only purpose is to provide a placeholder to expand / collapse in the view. */
class RKAccordionDummyModel : public QAbstractProxyModel {
	Q_OBJECT
public:
	RKAccordionDummyModel (QObject *parent) : QAbstractProxyModel (parent) {};

	QModelIndex mapFromSource (const QModelIndex& sindex) const {
		if (!sindex.isValid ()) return QModelIndex ();
		// we're using Source row as "Internal ID", here. This _would_ fall on our feet when removing rows, _if_ we'd actually
		// have to be able to map the dummy rows back to their real parents.
		return (createIndex (sindex.row (), sindex.column (), real_item_id));
	}

	QModelIndex mapToSource (const QModelIndex& pindex) const {
		if (!pindex.isValid ()) return QModelIndex ();
		if (pindex.internalId () != real_item_id) {
			return sourceModel ()->index (pindex.internalId (), pindex.column ());
		} else {
			return sourceModel ()->index (pindex.row (), pindex.column ());
		}
	}

	Qt::ItemFlags flags (const QModelIndex& index) const {
		if (isFake (index)) return (Qt::NoItemFlags);
		return QAbstractProxyModel::flags (index);
	}

	int rowCount (const QModelIndex& parent) const {
		if (isFake (parent)) return 0;
		if (parent.isValid ()) return 1;
		return sourceModel ()->rowCount (mapToSource (parent));
	}

    QVariant data(const QModelIndex& proxyIndex, int role = Qt::DisplayRole) const {
		if (isFake (proxyIndex)) return QVariant ();
		return QAbstractProxyModel::data (proxyIndex, role);
	}

	bool hasChildren (const QModelIndex& parent) const {
		return (!isFake (parent));
	}

	int columnCount (const QModelIndex& parent) const {
		if (isFake (parent)) return 1;
		return sourceModel ()->columnCount (mapToSource (parent));
	}

	QModelIndex index (int row, int column, const QModelIndex& parent) const {
		if (!parent.isValid ()) return createIndex (row, column, real_item_id);
		RK_ASSERT (parent.internalId () == real_item_id);
		return createIndex (row, column, parent.row ());
	}

	QModelIndex parent (const QModelIndex& child) const {
		if (child.internalId () == real_item_id) return QModelIndex ();
		return createIndex (child.internalId (), 0, real_item_id);
	}

	void setSourceModel (QAbstractItemModel* source_model) {
		/* More than these would be needed for a proper proxy of any model, but in our case, we only have to support the RKOptionsetDisplayModel */
		connect (source_model, SIGNAL (rowsInserted(const QModelIndex&,int,int)), this, SLOT (rowsInserted(QModelIndex,int,int)));
		connect (source_model, SIGNAL (rowsRemoved(const QModelIndex&,int,int)), this, SLOT (rowsRemoved(QModelIndex,int,int)));
		connect (source_model, SIGNAL (layoutChanged()), this, SLOT (relayLayoutChange()));
		QAbstractProxyModel::setSourceModel (source_model);
	}

	bool isFake (const QModelIndex& index) const {
		return (index.isValid () && (index.internalId () != real_item_id));
	}

	static const quint32 real_item_id = 0xFFFFFFFF;
public slots:
	void rowsInserted (const QModelIndex& parent, int start, int end) {
		RK_TRACE (MISC);
		RK_ASSERT (!parent.isValid ());

		beginInsertRows (mapFromSource (parent), start, end);
		endInsertRows ();
	}
	void rowsRemoved (const QModelIndex& parent, int start, int end) {
		RK_TRACE (MISC);
		RK_ASSERT (!parent.isValid ());

		beginRemoveRows (mapFromSource (parent), start, end);
		endRemoveRows ();
	}
	void relayLayoutChange () {
 		RK_DEBUG (MISC, DL_ERROR, "reset");
		emit (layoutChanged());
	}
};


/** Protects the given child widget from deletion */
class RKWidgetGuard : public QWidget {
public:
	RKWidgetGuard (QWidget *parent, QWidget *widget_to_guard, QWidget *fallback_parent) : QWidget (parent) {
		RK_TRACE (MISC);
		RK_ASSERT (widget_to_guard);

		guarded = widget_to_guard;
		RKWidgetGuard::fallback_parent = fallback_parent;

		QVBoxLayout *layout = new QVBoxLayout (this);
		guarded->setParent (this);
		layout->addWidget (guarded);
	}

	~RKWidgetGuard () {
		RK_TRACE (MISC);
		if ((!guarded.isNull ()) && guarded->parent () == this) {
			guarded->setParent (fallback_parent);
		}
	}
private:
	QPointer<QWidget> guarded;
	QWidget *fallback_parent;
};

#include <QLabel>
RKAccordionTable::RKAccordionTable (QWidget* parent) : QTreeView (parent) {
	RK_TRACE (MISC);

	default_widget = new KVBox;
	new QLabel ("This is the content\nExcept it's just a dummy!!!!!!!!!!!!!!!", default_widget);
	setSelectionBehavior (SelectRows);
	setSelectionMode (SingleSelection);
	pmodel = new RKAccordionDummyModel (this);
	connect (this, SIGNAL (expanded(QModelIndex)), this, SLOT (rowExpanded(QModelIndex)));
}

RKAccordionTable::~RKAccordionTable () {
	RK_TRACE (MISC);

	delete default_widget;
}

void RKAccordionTable::currentChanged (const QModelIndex& current, const QModelIndex& previous) {
	RK_TRACE (MISC);
	RK_ASSERT (current.isValid ());
	Q_UNUSED (previous);
// TODO: needed?
	return;
	if (!isExpanded (current)) {
		expand (current);
	}
}

void RKAccordionTable::rowExpanded (QModelIndex row) {
	RK_TRACE (MISC);

	for (int i = 0; i < model ()->rowCount (); ++i) {
		if (i != row.row ()) setExpanded (model ()->index (i, 0), false);
	}
	setFirstColumnSpanned (0, row, true);
	setIndexWidget (model ()->index (0, 0, row), new RKWidgetGuard (0, default_widget, this));
}

void RKAccordionTable::setModel (QAbstractItemModel* model) {
	RK_TRACE (MISC);

	pmodel->setSourceModel (model);
	QTreeView::setModel (pmodel);

	if (pmodel->rowCount () > 0) expand (pmodel->index (0, 0));
}

void RKAccordionTable::resizeEvent (QResizeEvent* event) {
	// TODO
	QTreeView::resizeEvent (event);
}

// TODO
// - add buttons to each row
// - handle resize
// KF5 TODO: remove:
#include "rkaccordiontable.moc"
#include "rkaccordiontablemodel_moc.cpp"
