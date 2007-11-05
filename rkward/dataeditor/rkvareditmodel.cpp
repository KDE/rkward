/***************************************************************************
                          rkvareditmodel  -  description
                             -------------------
    begin                : Mon Nov 05 2007
    copyright            : (C) 2007 by Thomas Friedrichsmeier
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

#include "rkvareditmodel.h"

#include <klocale.h>

#include "../core/rcontainerobject.h"
#include "../core/rkmodificationtracker.h"
#include "../rkglobals.h"

#include "../debug.h"

RKVarEditModel::RKVarEditModel (QObject *parent) : QAbstractTableModel (parent) {
	RK_TRACE (EDITOR);

	meta_model = 0;
	trailing_rows = trailing_cols = 0;
	edit_blocks = 0;

	connect (RKGlobals::tracker (), SIGNAL (objectRemoved(RObject*)), this, SLOT (objectRemoved(RObject*)));
}

RKVarEditModel::~RKVarEditModel () {
	RK_TRACE (EDITOR);
}

void RKVarEditModel::addObject (int index, RKVariable* object) {
	RK_TRACE (EDITOR);
	RK_ASSERT (object);

	if ((index < 0) || (index >= objects.size ())) index = objects.size ();

	beginInsertColumns (QModelIndex (), index, index);
	if (meta_model) meta_model->beginAddDataObject (index);
	objects.insert (index, object);
	if (meta_model) meta_model->endAddDataObject (index);
	endInsertColumns ();
}

void RKVarEditModel::objectRemoved (RObject* object) {
	RK_TRACE (EDITOR);

	int index = objects.indexOf (static_cast<RKVariable*> (object));	// no check for isVariable needed. we only need to look up, if we have this object, and where.
	if (index < 0) return;	// none of our buisiness

	beginRemoveColumns (QModelIndex (), index, index);
	if (meta_model) meta_model->beginRemoveDataObject (index);
	objects.removeAt (index);
	if (meta_model) meta_model->endRemoveDataObject (index);
	endRemoveColumns ();
}

void RKVarEditModel::doInsertColumn (int) {
	RK_TRACE (EDITOR);
	RK_ASSERT (false);	// should be implemented in a subclass, or never called
}

RKVarEditMetaModel* RKVarEditModel::getMetaModel () {
	RK_TRACE (EDITOR);

	if (!meta_model) meta_model = new RKVarEditMetaModel (this);

	return meta_model;
}

bool RKVarEditModel::insertRows (int row, int count, const QModelIndex& parent) {
	RK_TRACE (EDITOR);

	if (edit_blocks || parent.isValid () || objects.isEmpty () || (row > apparentRows ())) {
		RK_ASSERT (false);
		return false;
	}
	if (row > objects[0]->getLength ()) row = objects[0]->getLength ();
	int lastrow = row+count - 1;
	RK_ASSERT (row >= 0);
	RK_ASSERT (lastrow <= row);

	beginInsertRows (QModelIndex (), row, row+count);
	for (int i=0; i < objects.size (); ++i) {
		objects[i]->insertRows (row, count);
	}
	endInsertRows ();

	return true;
}

bool RKVarEditModel::removeRows (int row, int count, const QModelIndex& parent) {
	RK_TRACE (EDITOR);

	int lastrow = row+count - 1;
	if (edit_blocks || parent.isValid () || objects.isEmpty () || (lastrow >= (apparentRows ()))) {
		RK_ASSERT (false);
		return false;
	}
	if (lastrow >= objects[0]->getLength ()) lastrow = objects[0]->getLength () - 1;
	RK_ASSERT (row >= 0);
	RK_ASSERT (lastrow <= row);

	beginRemoveRows (QModelIndex (), row, lastrow);
	for (int i=0; i < objects.size (); ++i) {
		objects[i]->removeRows (row, lastrow);
	}
	endRemoveRows ();

	return true;
}

int RKVarEditModel::rowCount (const QModelIndex& parent) const {
	RK_TRACE (EDITOR);

	if (parent.isValid ()) return 0;
	if (objects.isEmpty ()) {
		RK_ASSERT (false);
		return 0;
	}
	return (apparentRows ());
}

int RKVarEditModel::columnCount (const QModelIndex& parent) const {
	RK_TRACE (EDITOR);

	if (parent.isValid ()) return 0;
	return (apparentCols ());
}

QVariant RKVarEditModel::data (const QModelIndex& index, int role) const {
	RK_TRACE (EDITOR);

	if (!index.isValid ()) return QVariant ();
	int row = index.row ();
	int col = index.column ();
	if ((col >= apparentCols ()) || (row >= apparentRows ())) {
		RK_ASSERT (false);
		return QVariant ();
	}

	// on a trailing row / col
	if ((col >= objects.size ()) || (row >= objects[0]->getLength ())) {
		if (role == Qt::BackgroundRole) return (Qt::gray);
		if (role == Qt::ToolTipRole) {
			if (col >= objects.size ()) return (i18n ("Type on these fields to add new columns"));
			else return (i18n ("Type on these fields to add new rows"));
		}
		return QVariant ();
	}

	// a normal cell
	RKVariable *var = objects[col];
	RK_ASSERT (var);

	if (role == Qt::DisplayRole) return var->getText (row, true);
	if (role == Qt::EditRole) return var->getText (row, false);

	RKVariable::Status status = var->cellStatus (row);
	if ((role == Qt::BackgroundRole) && (status == RKVariable::ValueInvalid)) return (Qt::red);
	if ((role == Qt::ForegroundRole) && (status == RKVariable::ValueUnknown)) return (Qt::lightGray);

	return QVariant ();
}

Qt::ItemFlags RKVarEditModel::flags (const QModelIndex& index) const {
	RK_TRACE (EDITOR);

	Qt::ItemFlags flags = 0;

	if (!index.isValid ()) return flags;
	int row = index.row ();
	int col = index.column ();
	if ((col >= apparentCols ()) || (row >= apparentRows ())) {
		RK_ASSERT (false);
		return flags;
	}

	if (!edit_blocks) flags |= Qt::ItemIsEditable | Qt::ItemIsEnabled;
	if ((col < objects.size ()) && (row >= objects[0]->getLength ())) flags |= Qt::ItemIsSelectable;

	return flags;
}

bool RKVarEditModel::setData (const QModelIndex& index, const QVariant& value, int role) {
	RK_TRACE (EDITOR);

	if (!index.isValid ()) return false;
	int row = index.row ();
	int col = index.column ();
	if (edit_blocks || (role != Qt::EditRole) || (col >= apparentCols ()) || (row >= apparentRows ())) {
		RK_ASSERT (false);
		return false;
	}

	if (col >= objects.size ()) {		// trailing col
		// somebody should add a column for us
		doInsertColumn (objects.size ());

		if (col >= objects.size ()) {
			// apparently, no column has been added in the above signal
			return false;
		}
	}
	if (row >= objects[0]->getLength ()) {		// trailing row
		insertRows (objects[0]->getLength (), 1);
	}

	// edit of normal cells
	RKVariable* var = objects[col];
	RK_ASSERT (var);
	var->setText (row, value.toString ());
	return true;
}

QVariant RKVarEditModel::headerData (int section, Qt::Orientation orientation, int role) const {
	RK_TRACE (EDITOR);

	if (role != Qt::DisplayRole) return QVariant ();

	if (orientation == Qt::Horizontal) {
		if (section >= objects.size ()) return QVariant ();
		return objects[section]->getShortName ();
	}

	return QString::number (section);
}










#include "rkvareditmodel.moc"
