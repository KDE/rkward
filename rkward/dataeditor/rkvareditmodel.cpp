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

#include "../core/rkvariable.h"
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

RKVarEditMetaModel* RKVarEditModel::getMetaModel () {
	RK_TRACE (EDITOR);

	if (!meta_model) meta_model = new RKVarEditMetaModel (this);

	return meta_model;
}

bool RKVarEditModel::insertRows (int row, int count, const QModelIndex& parent) {
	RK_TRACE (EDITOR);

	int lastrow = row+count - 1;
	if (edit_blocks || parent.isValid () || objects.isEmpty () || (row > objects[0]->getLength ())) {
		RK_ASSERT (false);
		return false;
	}
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
	if (edit_blocks || parent.isValid () || objects.isEmpty () || (lastrow > objects[0]->getLength ())) {
		RK_ASSERT (false);
		return false;
	}
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
	return objects[0]->getLength ();
}

int RKVarEditModel::columnCount (const QModelIndex& parent) const {
	RK_TRACE (EDITOR);

	if (parent.isValid ()) return 0;
	return objects.size ();
}

QVariant RKVarEditModel::data (const QModelIndex& index, int role) const {
	RK_TRACE (EDITOR);
#warning implement
}

Qt::ItemFlags RKVarEditModel::flags (const QModelIndex& index) const {
	RK_TRACE (EDITOR);
#warning implement
}

bool RKVarEditModel::setData (const QModelIndex& index, const QVariant& value, int role) {
	RK_TRACE (EDITOR);
#warning implement
}

QVariant RKVarEditModel::headerData (int section, Qt::Orientation orientation, int role) const {
	RK_TRACE (EDITOR);
#warning implement
}










#include "rkvareditmodel.moc"
