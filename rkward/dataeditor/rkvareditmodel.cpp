/***************************************************************************
                          rkvareditmodel  -  description
                             -------------------
    begin                : Mon Nov 05 2007
    copyright            : (C) 2007, 2010 by Thomas Friedrichsmeier
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

#include <QColor>

#include "../core/rcontainerobject.h"
#include "../core/rkmodificationtracker.h"
#include "../core/rkrownames.h"
#include "../rbackend/rinterface.h"
#include "../rkglobals.h"

#include "../debug.h"

RKVarEditModel::RKVarEditModel (QObject *parent) : RKVarEditModelBase (parent), RObjectListener (RObjectListener::DataModel) {
	RK_TRACE (EDITOR);

	meta_model = 0;
	trailing_rows = trailing_cols = 0;
	edit_blocks = 0;

	addNotificationType (RObjectListener::ObjectRemoved);
	addNotificationType (RObjectListener::MetaChanged);
	addNotificationType (RObjectListener::DataChanged);
}

RKVarEditModel::~RKVarEditModel () {
	RK_TRACE (EDITOR);

	for (int i = objects.size () - 1; i >= 0; --i) stopListenForObject (objects[i]);
}

RKVariable* RKVarEditModel::getObject (int index) const {
	RK_TRACE (EDITOR);

	if (index >= trueCols ()) {
		RK_ASSERT (false);
		return 0;
	}
	return objects[index];
}

void RKVarEditModel::addObject (int index, RKVariable* object) {
	RK_TRACE (EDITOR);
	RK_ASSERT (object);

	if ((index < 0) || (index >= objects.size ())) index = objects.size ();

	beginInsertColumns (QModelIndex (), index, index);
	if (meta_model) meta_model->beginAddDataObject (index);
	if (object->isPending ()) object->setLength (trueRows ());	// probably we just created it ourselves
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

	if (objects.isEmpty ()) emit (modelDepleted ());	// editor may or may want to auto-destruct
}

void RKVarEditModel::objectMetaChanged (RObject* changed) {
	RK_TRACE (EDITOR);

	int cindex = objects.indexOf (static_cast<RKVariable*> (changed));	// no check for isVariable needed. we only need to look up, if we have this object, and where.
	if (cindex < 0) return;	// none of our buisiness

	emit (dataChanged (index (0, cindex), index (trueRows (), cindex)));
	if (meta_model) meta_model->objectMetaChanged (cindex);
}

void RKVarEditModel::objectDataChanged (RObject* object, const RObject::ChangeSet *changes) {
	RK_TRACE (EDITOR);

	if (object == rownames) {
		RK_ASSERT (changes);
		emit (headerDataChanged (Qt::Vertical, changes->from_index, changes->to_index));
		return;
	}

	int cindex = objects.indexOf (static_cast<RKVariable*> (object));	// no check for isVariable needed. we only need to look up, if we have this object, and where.
	if (cindex < 0) return;	// none of our buisiness

	RK_ASSERT (changes);

	emit (dataChanged (index (changes->from_index, cindex), index (changes->to_index, cindex)));
}

void RKVarEditModel::doInsertColumns (int, int) {
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
	if (row > trueRows ()) row = trueRows ();
	int lastrow = row+count - 1;
	RK_ASSERT (row >= 0);
	RK_ASSERT (lastrow >= row);

	beginInsertRows (QModelIndex (), row, row+count-1);
	for (int i=0; i < objects.size (); ++i) {
// TODO: this does not emit any data change notifications to other editors
		objects[i]->insertRows (row, count);
	}
	rownames->insertRows (row, count);
	doInsertRowsInBackend (row, count);
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
	RK_ASSERT (lastrow >= row);

	beginRemoveRows (QModelIndex (), row, lastrow);
	for (int i=0; i < objects.size (); ++i) {
// TODO: this does not emit any data change notifications to other editors
		objects[i]->removeRows (row, lastrow);
	}
	doRemoveRowsInBackend (row, lastrow - row + 1);
	endRemoveRows ();

	return true;
}

void RKVarEditModel::doInsertRowsInBackend (int, int) {
	RK_TRACE (EDITOR);

	// TODO: implement
	RK_ASSERT (false);
}

void RKVarEditModel::doRemoveRowsInBackend (int, int) {
	RK_TRACE (EDITOR);

	// TODO: implement
	RK_ASSERT (false);
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
	if ((role == Qt::BackgroundRole)) {
		if (status == RKVariable::ValueInvalid) return (Qt::red);
		if (row % 2) return (QColor (240, 255, 240));	// very light green
	}
	if ((role == Qt::ForegroundRole) && (status == RKVariable::ValueUnknown)) return (Qt::lightGray);
	if (role == Qt::TextAlignmentRole) {
		if (var->getAlignment () == RKVariable::AlignCellLeft) return ((int) Qt::AlignLeft | Qt::AlignVCenter);
		else return ((int) Qt::AlignRight | Qt::AlignVCenter);
	}

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
		doInsertColumns (objects.size (), 1);

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
		if (section >= objects.size ()) return i18n ("#New Variable#");
		return objects[section]->getShortName ();
	}

	if (section < rownames->getLength ()) {
		return rownames->getText (section);
	}
	return QVariant ();
}

RKTextMatrix RKVarEditModel::getTextMatrix (const QItemSelectionRange& range) const {
	RK_TRACE (EDITOR);

	if ((!range.isValid ()) || objects.isEmpty ()) return RKTextMatrix ();

// NOTE: of course, when the range is small, this is terribly inefficient. On the other hand, it doesn't really matter, then, either.
	QItemSelectionRange erange = range.intersected (QItemSelectionRange (index (0, 0), index (trueRows () - 1, trueCols () - 1)));
	int top = erange.top ();
	int bottom = erange.bottom ();
	int left = erange.left ();
	int right = erange.right ();

	RKTextMatrix ret;
	int tcol = 0;
	for (int col = left; col <= right; ++col) {
		QString* data = objects[col]->getCharacter (top, bottom);
		RK_ASSERT (data);
		ret.setColumn (tcol, data, bottom - top + 1);
		delete [] data;
		++tcol;
	}

	return ret;
}

void RKVarEditModel::blankRange (const QItemSelectionRange& range) {
	RK_TRACE (EDITOR);

	if ((!range.isValid ()) || objects.isEmpty ()) return;

// NOTE: of course, when the range is small, this is terribly inefficient. On the other hand, it doesn't really matter, then, either.
	QItemSelectionRange erange = range.intersected (QItemSelectionRange (index (0, 0), index (trueRows () - 1, trueCols () - 1)));
	int top = erange.top ();
	int bottom = erange.bottom ();
	int left = erange.left ();
	int right = erange.right ();

	for (int col = left; col <= right; ++col) {
		RKVariable* var = objects[col];
		for (int row = top; row <= bottom; ++row) {
			var->setText (row, QString ());
		}
	}
}

void RKVarEditModel::setTextMatrix (const QModelIndex& offset, const RKTextMatrix& text, const QItemSelectionRange& confine_to) {
	RK_TRACE (EDITOR);

// NOTE: of course, when the range is small, this is terribly inefficient. On the other hand, it doesn't really matter, then, either. Single cells will be set using setData()
	if ((!offset.isValid ()) || text.isEmpty ()) return;

	int top = offset.row ();
	int left = offset.column ();
	int bottom = top + text.numRows () - 1;
	int right = left + text.numColumns () - 1;
	if (confine_to.isValid ()) {
		if (confine_to.top () > top) return;	// can't confine top-left. Should have been set by caller
		if (confine_to.left () > left) return;
		bottom = qMin (confine_to.bottom (), bottom);
		right = qMin (confine_to.right (), right);
	}

// TODO: some models might not support column addition.
	if (right >= trueCols ()) doInsertColumns (objects.size (), right - trueCols () + 1);
	RK_ASSERT (right < trueCols ());
	int current_rows = objects[0]->getLength ();
	if (bottom >= current_rows) insertRows (current_rows, bottom - current_rows + 1);

	int tcol = 0;
	for (int col = left; col <= right; ++col) {
		RKVariable* var = objects[col];
		QString* data = text.getColumn (tcol);
		RK_ASSERT (data);
		var->setCharacter (top, bottom, data);
		delete [] data;		// hm, why doesn't RKVariable take ownership?
		++tcol;
	}
}

void RKVarEditModel::restoreObject (RObject* object, RCommandChain* chain) {
	RK_TRACE (EDITOR);

	RK_ASSERT (objects.contains (static_cast<RKVariable*> (object)));
	static_cast<RKVariable*> (object)->restore (chain);
}

/////////////////// RKVarEditMetaModel ////////////////////////

RKVarEditMetaModel::RKVarEditMetaModel (RKVarEditModel* data_model) : RKVarEditModelBase (data_model) {
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

void RKVarEditMetaModel::objectMetaChanged (int atcolumn) {
	RK_TRACE (EDITOR);

	emit (dataChanged (index (0, atcolumn), index (RowCount - 1, atcolumn)));
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
		data_model->doInsertColumns (data_model->objects.size (), 1);

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

RObject::ValueLabels RKVarEditMetaModel::getValueLabels (int column) const {
	RK_TRACE (EDITOR);

	if (column >= trueCols ()) return RObject::ValueLabels ();
	return (getObject (column)->getValueLabels ());
}

void RKVarEditMetaModel::setValueLabels (int column, const RObject::ValueLabels& labels) {
	RK_TRACE (EDITOR);

	if (column >= data_model->apparentCols ()) {
		RK_ASSERT (false);
		return;
	}
	if (column >= trueCols ()) {
		data_model->doInsertColumns (trueCols (), 1);
	}
	RKVariable* var = getObject (column);
	RK_ASSERT (var);
	var->setValueLabels (labels);
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

RKTextMatrix RKVarEditMetaModel::getTextMatrix (const QItemSelectionRange& range) const {
	RK_TRACE (EDITOR);

	if ((!range.isValid ()) || data_model->objects.isEmpty ()) return RKTextMatrix ();

// NOTE: of course, when the range is small, this is terribly inefficient. On the other hand, it doesn't really matter, then, either.
	QItemSelectionRange erange = range.intersected (QItemSelectionRange (index (0, 0), index (trueRows () - 1, trueCols () - 1)));
	int top = erange.top ();
	int bottom = erange.bottom ();
	int left = erange.left ();
	int right = erange.right ();

	RKTextMatrix ret;
	int tcol = 0;
	for (int col = left; col <= right; ++col) {
		int trow = 0;
		for (int row = top; row <= bottom; ++row) {
			QVariant celldata = data (index (row, col), Qt::EditRole);
			if (!celldata.isValid ()) {
				RK_ASSERT (false);
				break;
			}
			ret.setText (trow, tcol, celldata.toString ());
			++trow;
		}
		++tcol;
	}

	return ret;
}

void RKVarEditMetaModel::blankRange (const QItemSelectionRange& range) {
	RK_TRACE (EDITOR);

	if ((!range.isValid ()) || data_model->objects.isEmpty ()) return;

// NOTE: of course, when the range is small, this is terribly inefficient. On the other hand, it doesn't really matter, then, either.
	QItemSelectionRange erange = range.intersected (QItemSelectionRange (index (0, 0), index (trueRows () - 1, trueCols () - 1)));
	int top = erange.top ();
	int bottom = erange.bottom ();
	int left = erange.left ();
	int right = erange.right ();

	for (int col = left; col <= right; ++col) {
		RKVariable* var = data_model->objects[col];
		var->setSyncing (false);
		for (int row = top; row <= bottom; ++row) {
			setData (createIndex (row, col), QString (), Qt::EditRole);
		}
		var->setSyncing (true);
	}
}

void RKVarEditMetaModel::setTextMatrix (const QModelIndex& offset, const RKTextMatrix& text, const QItemSelectionRange& confine_to) {
	RK_TRACE (EDITOR);

	if ((!offset.isValid ()) || text.isEmpty ()) return;

	int top = offset.row ();
	int left = offset.column ();
	int bottom = top + text.numRows () - 1;
	int right = left + text.numColumns () - 1;
	if (confine_to.isValid ()) {
		if (confine_to.top () > top) return;	// can't confine top-left. Should have been set by caller
		if (confine_to.left () > left) return;
		bottom = qMin (confine_to.bottom (), bottom);
		right = qMin (confine_to.right (), right);
	}

// TODO: some models might not support column addition.
	if (right >= trueCols ()) data_model->doInsertColumns (trueCols (), right - trueCols () + 1);
	RK_ASSERT (right < data_model->objects.size ());
	bottom = qMin (bottom, trueRows () - 1);

	int tcol = 0;
	for (int col = left; col <= right; ++col) {
		int trow = 0;
		RKVariable* var = data_model->objects[col];
		var->setSyncing (false);
		for (int row = top; row <= bottom; ++row) {
			setData (index (row, col), text.getText (trow, tcol), Qt::EditRole);
			++trow;
		}
		var->syncDataToR ();
		var->setSyncing (true);
		++tcol;
	}
}


/////////////////// RKVarEditDataFrameModel ////////////////////////


RKVarEditDataFrameModel::RKVarEditDataFrameModel (RContainerObject* dataframe, QObject* parent) : RKVarEditModel (parent) {
	RK_TRACE (EDITOR);

	init (dataframe);
}

RKVarEditDataFrameModel::RKVarEditDataFrameModel (const QString& validized_name, RContainerObject* parent_object, RCommandChain* chain, int initial_cols, QObject* parent) : RKVarEditModel (parent) {
	RK_TRACE (EDITOR);

	RObject *object = parent_object->createPendingChild (validized_name, -1, true, true);
	RK_ASSERT (object->isDataFrame ());
	RContainerObject* df = static_cast<RContainerObject*> (object);

// initialize the new object
	for (int i = 0; i < initial_cols; ++i) {
		RObject* child = df->createPendingChild (df->validizeName (QString ()), -1, false, false);
		RK_ASSERT (child->isVariable ());
	}

	init (df);

	// creates the pending object in the backend
	pushTable (chain);
}

void RKVarEditDataFrameModel::init (RContainerObject* dataframe) {
	RK_TRACE (EDITOR);

	RKVarEditDataFrameModel::dataframe = dataframe;

	trailing_rows = 1;
	trailing_cols = 1;

	addNotificationType (RObjectListener::ChildAdded);
	addNotificationType (RObjectListener::ChildMoved);
	listenForObject (dataframe);

	for (int i = 0; i < dataframe->numChildren (); ++i) {
		RObject* obj = dataframe->findChildByIndex (i);
		RK_ASSERT (obj);
		RK_ASSERT (obj->isVariable ());
		addObject (i, static_cast<RKVariable*> (obj));
	}
	rownames = dataframe->rowNames ();
	listenForObject (rownames);
}

RKVarEditDataFrameModel::~RKVarEditDataFrameModel () {
	RK_TRACE (EDITOR);

	if (dataframe) stopListenForObject (dataframe);
	if (rownames) stopListenForObject (rownames);
}

bool RKVarEditDataFrameModel::insertColumns (int column, int count, const QModelIndex& parent) {
	RK_TRACE (EDITOR);

	if (parent.isValid ()) {
		RK_ASSERT (false);
		return false;
	}

	if (column > trueCols ()) column = trueCols ();
	for (int col = column; col < (column + count); ++col) {
		RObject *obj = dataframe->createPendingChild (dataframe->validizeName (QString ()), col);
		RK_ASSERT (obj->isVariable ());
//		addObject (col, obj);	// the object will be added via RKModificationTracker::addObject -> this::childAdded. That will also take care of calling beginInsertColumns()/endInsertColumns()
	
		RKGlobals::rInterface ()->issueCommand (new RCommand (".rk.data.frame.insert.column (" + dataframe->getFullName () + ", \"" + obj->getShortName () + "\", " + QString::number (col+1) + ")", RCommand::App | RCommand::Sync));
	}

	return true;
}

bool RKVarEditDataFrameModel::removeColumns (int column, int count, const QModelIndex& parent) {
	RK_TRACE (EDITOR);

	if (parent.isValid ()) {
		RK_ASSERT (false);
		return false;
	}

	while ((column + count) > objects.size ()) --count;
	for (int i = column + count - 1; i >= column; --i) {	// we start at the end so that the index remains valid
		RKGlobals::tracker ()->removeObject (objects[i]);
		// the comment in insertColumns, above: The object will be removed from our list in objectRemoved().
	}
	return true;
}

void RKVarEditDataFrameModel::doInsertRowsInBackend (int row, int count) {
	RK_TRACE (EDITOR);

	// TODO: most of the time we're only adding one row at a time, still we should have a function to add multiple rows at once.
	for (int i = row; i < row + count; ++i) {
		RKGlobals::rInterface ()->issueCommand (new RCommand (".rk.data.frame.insert.row (" + dataframe->getFullName () + ", " + QString::number (i+1) + ')', RCommand::App | RCommand::Sync));
	}
}

void RKVarEditDataFrameModel::doRemoveRowsInBackend (int row, int count) {
	RK_TRACE (EDITOR);

	for (int i = row + count - 1; i >= row; --i) {
		RKGlobals::rInterface ()->issueCommand (new RCommand (".rk.data.frame.delete.row (" + dataframe->getFullName () + ", " + QString::number (i+1) + ')', RCommand::App | RCommand::Sync));
	}
}

void RKVarEditDataFrameModel::objectRemoved (RObject* object) {
	RK_TRACE (EDITOR);

	if (object == dataframe) {
		while (!objects.isEmpty ()) RKVarEditModel::objectRemoved (objects[0]);
		stopListenForObject (dataframe);
		dataframe = 0;
	}

	RKVarEditModel::objectRemoved (object);

	// if the dataframe is gone, the editor will most certainly want to auto-destruct.
	// since the model will be taken down as well, this has to come last in the function.
	if (!dataframe) emit (modelObjectDestroyed ());
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

void RKVarEditDataFrameModel::childMoved (int old_index, int new_index, RObject* parent) {
	RK_TRACE (EDITOR);

	if (parent == dataframe) {
		RObject *child = dataframe->findChildByIndex (new_index);	// it's at the new position, already
		RK_ASSERT (objects.size () > old_index);
		RK_ASSERT (child == objects[old_index]);
		// if an object has changed position, there should be at least two objects left. Hence, the objectRemoved-call will never lead to editor destruction
		RK_ASSERT (objects.size () >= 2);
		objectRemoved (child);

		RK_ASSERT (child->isVariable ());
		addObject (new_index, static_cast<RKVariable*> (child));
	} else {
		// even though we are listening for the child objects as well, we should see move notifications
		// only for children of the parent.
		RK_ASSERT (false);
	}
}

void RKVarEditDataFrameModel::doInsertColumns (int index, int count) {
	RK_TRACE (EDITOR);

	insertColumns (index, count);
}

void RKVarEditDataFrameModel::pushTable (RCommandChain *sync_chain) {
	RK_TRACE (EDITOR);

	// first push real data
	QString command = dataframe->getFullName ();
	command.append (" <- data.frame (");

	RK_ASSERT (objects.size ());
	RKVariable* child = objects[0];
	QString na_vector = "=as.numeric (rep (NA, " + QString::number (child->getLength ()) + "))";
	for (int col=0; col < objects.size (); ++col) {
		if (col != 0) command.append (", ");
		command.append (objects[col]->getShortName () + na_vector);
	}
	command.append (")");

	// push all children
	RKGlobals::rInterface ()->issueCommand (new RCommand (command, RCommand::Sync | RCommand::ObjectListUpdate), sync_chain);
	for (int col=0; col < objects.size (); ++col) {
		objects[col]->restore (sync_chain);
	}

	// now store the meta-data
	dataframe->writeMetaData (sync_chain);
}

void RKVarEditDataFrameModel::restoreObject (RObject* object, RCommandChain* chain) {
	RK_TRACE (EDITOR);

	if (object == dataframe) {
		pushTable (chain);
	} else {
		RKVarEditModel::restoreObject (object, chain);
	}
}


#include "rkvareditmodel.moc"
