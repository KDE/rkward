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

RKVarEditModel::RKVarEditModel (QObject *parent) : QAbstractTableModel (parent), RObjectListener (RObjectListener::DataModel, RObjectListener::ObjectRemoved) {
	RK_TRACE (EDITOR);

	meta_model = 0;
	trailing_rows = trailing_cols = 0;
	edit_blocks = 0;
}

RKVarEditModel::~RKVarEditModel () {
	RK_TRACE (EDITOR);

	for (int i = objects.size () - 1; i >= 0; --i) stopListenForObject (objects[i]);
}

void RKVarEditModel::addObject (int index, RKVariable* object) {
	RK_TRACE (EDITOR);
	RK_ASSERT (object);

	if ((index < 0) || (index >= objects.size ())) index = objects.size ();

	beginInsertColumns (QModelIndex (), index, index);
	if (meta_model) meta_model->beginAddDataObject (index);
	listenForObject (object);
	objects.insert (index, object);
	if (meta_model) meta_model->endAddDataObject ();
	endInsertColumns ();
}

void RKVarEditModel::objectRemoved (RObject* object) {
	RK_TRACE (EDITOR);

	int index = objects.indexOf (static_cast<RKVariable*> (object));	// no check for isVariable needed. we only need to look up, if we have this object, and where.
	if (index < 0) return;	// none of our buisiness

	beginRemoveColumns (QModelIndex (), index, index);
	if (meta_model) meta_model->beginRemoveDataObject (index);
	stopListenForObject (objects.takeAt (index));
	if (meta_model) meta_model->endRemoveDataObject ();
	endRemoveColumns ();

	if (objects.isEmpty ()) {
#warning TODO notify editor
	}
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
	if ((col < objects.size ()) && (row < objects[0]->getLength ())) flags |= Qt::ItemIsSelectable;

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


/////////////////// RKVarEditMetaModel ////////////////////////

RKVarEditMetaModel::RKVarEditMetaModel (RKVarEditModel* data_model) : QAbstractTableModel (data_model) {
	RK_TRACE (EDITOR);
	RK_ASSERT (data_model);

	RKVarEditMetaModel::data_model = data_model;
}

RKVarEditMetaModel::~RKVarEditMetaModel () {
	RK_TRACE (EDITOR);
}

void RKVarEditMetaModel::beginAddDataObject (int index) {
	RK_TRACE (EDITOR);

	beginInsertColumns (QModelIndex (), index, index);
}

void RKVarEditMetaModel::endAddDataObject () {
	RK_TRACE (EDITOR);

	endInsertColumns ();
}

void RKVarEditMetaModel::beginRemoveDataObject (int index) {
	RK_TRACE (EDITOR);

	beginRemoveColumns (QModelIndex (), index, index);
}

void RKVarEditMetaModel::endRemoveDataObject () {
	RK_TRACE (EDITOR);

	endRemoveColumns ();
}

int RKVarEditMetaModel::rowCount (const QModelIndex& parent) const {
	RK_TRACE (EDITOR);

	if (parent.isValid ()) return 0;
	return RowCount;
}

int RKVarEditMetaModel::columnCount (const QModelIndex& parent) const {
	RK_TRACE (EDITOR);

	return (data_model->columnCount (parent));
}

QVariant RKVarEditMetaModel::data (const QModelIndex& index, int role) const {
	RK_TRACE (EDITOR);

	if (!index.isValid ()) return QVariant ();
	int row = index.row ();
	int col = index.column ();
	if ((col >= data_model->apparentCols ()) || (row >= RowCount)) {
		RK_ASSERT (false);
		return QVariant ();
	}

	// on a trailing col
	if (col >= data_model->objects.size ()) {
		if (role == Qt::BackgroundRole) return (Qt::gray);
		if (role == Qt::ToolTipRole) return (i18n ("Type on these fields to add new columns"));
		return QVariant ();
	}

	if ((role == Qt::DisplayRole) || (role == Qt::EditRole)) {
		RKVariable *var = data_model->objects[col];
		RK_ASSERT (var);

		if (row == NameRow) return var->getShortName ();
		if (row == LabelRow) return var->getLabel ();
		if (row == TypeRow) {
			if (role == Qt::EditRole) return QString::number (var->getDataType ());
			return RObject::typeToText (var->getDataType ());
		}
		if (row == FormatRow) return var->getFormattingOptionsString ();
		if (row == LevelsRow) return var->getValueLabelString ();
	}

	return QVariant ();
}

Qt::ItemFlags RKVarEditMetaModel::flags (const QModelIndex& index) const {
	RK_TRACE (EDITOR);

	Qt::ItemFlags flags = 0;

	if (!index.isValid ()) return flags;
	int row = index.row ();
	int col = index.column ();
	if ((col >= data_model->apparentCols ()) || (row >= RowCount)) {
		RK_ASSERT (false);
		return flags;
	}

	if (!data_model->edit_blocks) flags |= Qt::ItemIsEditable | Qt::ItemIsEnabled;
	if ((col < data_model->objects.size ()) && (row < RowCount)) flags |= Qt::ItemIsSelectable;

	return flags;
}

bool RKVarEditMetaModel::setData (const QModelIndex& index, const QVariant& value, int role) {
	RK_TRACE (EDITOR);

	if (!index.isValid ()) return false;
	int row = index.row ();
	int col = index.column ();
	if (data_model->edit_blocks || (role != Qt::EditRole) || (col >= data_model->apparentCols ()) || (row >= RowCount)) {
		RK_ASSERT (false);
		return false;
	}

	if (col >= data_model->objects.size ()) {		// trailing col
		// somebody should add a column for us
		data_model->doInsertColumn (data_model->objects.size ());

		if (col >= data_model->objects.size ()) {
			// apparently, no column has been added in the above signal
			return false;
		}
	}

	// edit of normal cells
	RKVariable* var = data_model->objects[col];
	RK_ASSERT (var);

	if (row == NameRow) {
		if (var->getShortName () != value.toString ()) {
			RKGlobals::tracker ()->renameObject (var, var->getContainer ()->validizeName (value.toString ()));
		}
	} else if (row == LabelRow) {
		var->setLabel (value.toString (), true);
	} else if (row == TypeRow) {
		var->setVarType ((RObject::RDataType) value.toInt ());
	} else if (row == FormatRow) {
		var->setFormattingOptionsString (value.toString ());
	} else if (row == LevelsRow) {
		if (value.toString () != var->getValueLabelString ()) {
			var->setValueLabelString (value.toString ());
		}
	}

	return true;
}

QVariant RKVarEditMetaModel::headerData (int section, Qt::Orientation orientation, int role) const {
	RK_TRACE (EDITOR);

	if (orientation == Qt::Horizontal) {
		return data_model->headerData (section, orientation, role);
	}

	if (role == Qt::DisplayRole) {
		if (section == NameRow) return (i18n ("Name"));
		if (section == LabelRow) return (i18n ("Label"));
		if (section == TypeRow) return (i18n ("Type"));
		if (section == FormatRow) return (i18n ("Format"));
		if (section == LevelsRow) return (i18n ("Levels"));
	}

	return QVariant ();
}


/////////////////// RKVarEditDataFrameModel ////////////////////////


RKVarEditDataFrameModel::RKVarEditDataFrameModel (RContainerObject* dataframe, QObject *parent) : RKVarEditModel (parent) {
	RK_TRACE (EDITOR);

	RKVarEditDataFrameModel::dataframe = dataframe;

	trailing_rows = 1;
	trailing_cols = 1;

	addNotificationType (RObjectListener::ChildAdded);
	listenForObject (dataframe);

	for (int i = 0; i < dataframe->numChildren (); ++i) {
		RObject* obj = dataframe->findChildByIndex (i);
		RK_ASSERT (obj);
		RK_ASSERT (obj->isVariable ());
		addObject (i, static_cast<RKVariable*> (obj));
	}
}

RKVarEditDataFrameModel::~RKVarEditDataFrameModel () {
	RK_TRACE (EDITOR);

	if (dataframe) stopListenForObject (dataframe);
}

bool RKVarEditDataFrameModel::insertColumns (int column, int count, const QModelIndex& parent) {
	RK_TRACE (EDITOR);

/*	RObject *obj = static_cast<RContainerObject *> (getObject ())->createNewChild (static_cast<RContainerObject *> (getObject ())->validizeName (QString ()), col, this);
	RK_ASSERT (obj->isVariable ());
	RKGlobals::rInterface ()->issueCommand (new RCommand (".rk.data.frame.insert.column (" + getObject ()->getFullName () + ", \"" + obj->getShortName () + "\", " + QString::number (col+1) + ")", RCommand::App | RCommand::Sync));
	static_cast<RKVariable*> (obj)->setLength (dataview->numTrueRows ());
	obj->setCreatedInEditor (this); */

#warning TODO implement
}

bool RKVarEditDataFrameModel::removeColumns (int column, int count, const QModelIndex& parent) {
	RK_TRACE (EDITOR);

/*	RKGlobals::tracker ()->removeObject (obj); */
#warning TODO implement
}

void RKVarEditDataFrameModel::objectRemoved (RObject* object) {
	RK_TRACE (EDITOR);

	if (object == dataframe) {
		while (!objects.isEmpty ()) RKVarEditModel::objectRemoved (objects[0]);
#warning TODO: notify editor
		stopListenForObject (dataframe);
		dataframe = 0;
	}

	RKVarEditModel::objectRemoved (object);
}

void RKVarEditDataFrameModel::childAdded (int index, RObject* parent) {
	RK_TRACE (EDITOR);

	if (parent == dataframe) {
		RObject* child = dataframe->findChildByIndex (index);
		RK_ASSERT (child);

		if (child->isVariable ()) addObject (index, static_cast<RKVariable*> (child));
		else RK_ASSERT (false);
	}
}

void RKVarEditDataFrameModel::doInsertColumn (int index) {
	RK_TRACE (EDITOR);

	insertColumns (index, 1);
}

#include "rkvareditmodel.moc"
