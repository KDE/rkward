/***************************************************************************
                          rkmatrixinput  -  description
                             -------------------
    begin                : Tue Oct 09 2012
    copyright            : (C) 2012 by Thomas Friedrichsmeier
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

#include "rkmatrixinput.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QTableView>

#include "klocale.h"

#include "../misc/xmlhelper.h"

#include "../debug.h"

RKMatrixInput::RKMatrixInput (const QDomElement& element, RKComponent* parent_component, QWidget* parent_widget) : RKComponent (parent_component, parent_widget) {
	RK_TRACE (PLUGIN);

	// get xml-helper
	XMLHelper *xml = XMLHelper::getStaticHelper ();

	// create layout
	QVBoxLayout *vbox = new QVBoxLayout (this);
	vbox->setContentsMargins (0, 0, 0, 0);

	QLabel *label = new QLabel (xml->getStringAttribute (element, "label", i18n ("Enter data:"), DL_INFO), this);
	vbox->addWidget (label);

	display = new QTableView (this);
	vbox->addWidget (display);

	mode = static_cast<Mode> (xml->getMultiChoiceAttribute (element, "mode", "integer;real;string", 1, DL_WARNING));
	if (mode == Integer) {
		min = xml->getIntAttribute (element, "min", INT_MIN, DL_INFO) - .1;	// we'll only allow ints anyway. Make sure not to run into floating point precision issues.
		max = xml->getIntAttribute (element, "max", INT_MAX, DL_INFO) + .1;
	} else if (mode == Real) {
		min = xml->getDoubleAttribute (element, "min", -FLT_MAX, DL_INFO);
		max = xml->getDoubleAttribute (element, "max", FLT_MAX, DL_INFO);
	} else {
		min = -FLT_MAX;
		max = FLT_MAX;
	}

	allow_missings = xml->getBoolAttribute (element, "allow_missings", false, DL_INFO);
	allow_user_resize_columns = xml->getBoolAttribute (element, "allow_user_resize_columns", true, DL_INFO);
	allow_user_resize_rows = xml->getBoolAttribute (element, "allow_user_resize_rows", true, DL_INFO);
	trailing_rows = allow_user_resize_rows ? 1 : 0;
	trailing_columns = allow_user_resize_columns ? 1 : 0;

	row_count = new RKComponentPropertyInt (this, false, xml->getIntAttribute (element, "rows", 2, DL_INFO));
	column_count = new RKComponentPropertyInt (this, false, xml->getIntAttribute (element, "columns", 2, DL_INFO));
	tsv_data = new RKComponentPropertyBase (this, false);
	row_count->setInternal (true);
	addChild ("rows", row_count);
	column_count->setInternal (true);
	addChild ("columns", column_count);
	addChild ("tsv", tsv_data);
	connect (row_count, SIGNAL (valueChanged(RKComponentPropertyBase*)), this, SLOT (dimensionPropertyChanged(RKComponentPropertyBase*)));
	connect (column_count, SIGNAL (valueChanged(RKComponentPropertyBase*)), this, SLOT (dimensionPropertyChanged(RKComponentPropertyBase*)));
	connect (tsv_data, SIGNAL (valueChanged(RKComponentPropertyBase*)), this, SLOT (tsvPropertyChanged()));
	updating_tsv_data = false;
	updating_dimensions = false;

	model = new RKMatrixInputModel (this);
	QString headers = xml->getStringAttribute (element, "horiz_headers", QString (), DL_INFO);
	if (!headers.isEmpty ()) model->horiz_header = headers.split (';');
	headers = xml->getStringAttribute (element, "vert_headers", QString (), DL_INFO);
	if (!headers.isEmpty ()) model->vert_header = headers.split (';');
	updateDataAndDimensions ();
	display->setModel (model);
}

RKMatrixInput::~RKMatrixInput () {
	RK_TRACE (PLUGIN);
}

QString RKMatrixInput::value (const QString& modifier) {
	if (modifier == "cbind") {
		QStringList ret;
		for (int i = 0; i < column_count->intValue (); ++i) {
			ret.append ("\tc (" + makeColumnString (i, ", ") + ")");
		}
		return QString ("cbind (\n" + ret.join ("\n") + "\n)");
	}

	bool ok;
	int col = modifier.toInt (&ok);
	if ((col >= 0) && ok) return makeColumnString (col, "\t");
	return tsv_data->value (modifier);
}

bool RKMatrixInput::expandStorageForColumn (int column) {
	RK_TRACE (PLUGIN);

	if ((column < 0) || (column >= (column_count->intValue () + trailing_columns))) {
		RK_ASSERT (false);
		return false;
	}

	while (column >= columns.size ()) {
		Column col;
		col.valid_up_to_row = -1;
		col.filled_up_to_row = -1;
		columns.append (col);
	}
	return true;
}

void RKMatrixInput::setCellValue (int row, int column, const QString& value) {
	RK_TRACE (PLUGIN);

	if ((!expandStorageForColumn (column)) || (row < 0) || (row >= (row_count->intValue () + trailing_rows))) {
		RK_ASSERT (false);
		return;
	}

	Column &col = columns[column];
	while (row >= col.storage.size ()) {
		col.storage.append (QString ());
	}
	col.storage[row] = value;
	updateColumn (row, column);
	model->dataChanged (model->index (row, column), model->index (row, column));
}

void RKMatrixInput::setColumnValue (int column, const QString& value) {
	RK_TRACE (PLUGIN);

	if (!expandStorageForColumn (column)) return;
	columns[column].storage = value.split ('\t', QString::KeepEmptyParts);
	updateColumn (0, column);
	model->dataChanged (model->index (0, column), model->index (row_count->intValue () + trailing_rows, column));
}

void RKMatrixInput::updateColumn (int offset, int column) {
	RK_TRACE (PLUGIN);
	RK_ASSERT ((column >= 0) && (column < columns.size ()));

	Column &col = columns[column];

	// check for trailing empty rows:
	int last_row = col.storage.size () - 1;
	while ((last_row >= 0) && col.storage[last_row].isEmpty ()) {
		--last_row;
	}
	col.filled_up_to_row = last_row;

	offset = qMax (0, qMin (offset, col.valid_up_to_row));
	while (offset < col.storage.size ()) {
		if (!isValueValid (col.storage[offset])) break;
		offset++;
	}
	col.valid_up_to_row = offset - 1;
	col.cached_tab_joined_string.clear (); // == no valid cache

	updateDataAndDimensions ();
}

void RKMatrixInput::updateValidityFlag () {
	RK_TRACE (PLUGIN);

	is_valid = true;
	for (int i = 0; i < column_count->intValue (); ++i) {
		if (i >= columns.size ()) {	// hit end of data, before hitting any invalid strings
			is_valid = allow_missings;
			break;
		}

		Column &col = columns[i];
		if (col.valid_up_to_row >= (row_count->intValue () - 1)) continue;
		else if (allow_missings && (col.valid_up_to_row >= (col.storage.size () - 1))) continue;
		else {
			is_valid = false;
			break;
		}
	}
	changed ();
}

QString RKMatrixInput::makeColumnString (int column, const QString& sep) {
	RK_TRACE (PLUGIN);

	QStringList storage;
	if (column < columns.size ()) {
		storage = columns[column].storage;
	}
	QString ret;
	if (!storage.isEmpty ()) ret = QStringList (storage.mid (0, row_count->intValue ())).join (sep);
	for (int i = storage.size (); i < row_count->intValue (); ++i) {
		ret.append (sep);
	}
	return ret;
}

void RKMatrixInput::updateDataAndDimensions () {
	RK_TRACE (PLUGIN);

	if (updating_tsv_data) return;
	updating_tsv_data = true;

	int max_row = row_count->intValue () - 1;
	if (allow_user_resize_rows) {
		max_row = -1;
		for (int i = columns.size () - 1; i >= 0; --i) {
			max_row = qMax (max_row, columns[i].filled_up_to_row);
		}
		if (max_row != row_count->intValue () - 1) {
			updating_dimensions = true;
			model->layoutAboutToBeChanged ();
			row_count->setIntValue (max_row + 1);
			model->layoutChanged ();
			updating_dimensions = false;
		}
	}

	int max_col = column_count->intValue () - 1;
	if (allow_user_resize_columns) {
		for (max_col = columns.size () - 1; max_col >= 0; --max_col) {
			if (columns[max_col].filled_up_to_row >= 0) {
				break;
			}
		}
		if (max_col != column_count->intValue () - 1) {
			updating_dimensions = true;
			model->layoutAboutToBeChanged ();
			column_count->setIntValue (max_col + 1);
			model->layoutChanged ();
			updating_dimensions = false;
		}
	}

	QStringList tsv;
	int i = 0;
	for (; i < columns.size (); ++i) {
		Column& col = columns[i];
		if (col.cached_tab_joined_string.isEmpty ()) {
			col.cached_tab_joined_string = makeColumnString (i, "\t");
		}
		tsv.append (col.cached_tab_joined_string);
	}
	for (; i < max_col; ++i) {
		tsv.append (QString (max_row, '\t'));
	}
	tsv_data->setValue (tsv.join ("\n"));

	updating_tsv_data = false;
	updateValidityFlag ();
}

void RKMatrixInput::dimensionPropertyChanged (RKComponentPropertyBase *property) {
	RK_TRACE (PLUGIN);

	if (updating_dimensions) return;
	if (allow_user_resize_rows && (property == row_count)) {
		RK_ASSERT (false);
		return;
	}
	if (allow_user_resize_columns && (property == column_count)) {
		RK_ASSERT (false);
		return;
	}

	if (property == row_count) {		// invalidates column caches
		for (int i = columns.size () - 1; i >= 0; --i) {
			columns[i].cached_tab_joined_string.clear ();
		}
	}

	model->layoutAboutToBeChanged ();
	updateDataAndDimensions ();
	model->layoutChanged ();
}

void RKMatrixInput::tsvPropertyChanged () {
	RK_TRACE (PLUGIN);

	if (updating_tsv_data) return;
	updating_tsv_data = true;

	columns.clear ();
	QStringList coldata = tsv_data->value ().split ('\n', QString::KeepEmptyParts);
	for (int i = 0; i < coldata.size (); ++i) {
		setColumnValue (i, coldata[i]);
	}

	updating_tsv_data = false;
	updateDataAndDimensions ();
}

bool RKMatrixInput::isValueValid (const QString& value) const {
	if (mode == String) return true;
	if (value.isEmpty ()) return allow_missings;

	bool number_ok;
	double val;
	if (mode == Integer) {
		val = value.toInt (&number_ok);
	} else {
		val = value.toFloat (&number_ok);
	}
	if (!number_ok) return false;
	if (val < min) return false;
	return (val < max);
}



///////////////////////////////////////////////////////////////////////////////////////

RKMatrixInputModel::RKMatrixInputModel (RKMatrixInput* _matrix) : QAbstractTableModel (_matrix) {
	RK_TRACE (PLUGIN);

	matrix = _matrix;
}

RKMatrixInputModel::~RKMatrixInputModel () {
	RK_TRACE (PLUGIN);
}

int RKMatrixInputModel::columnCount (const QModelIndex& parent) const {
	if (parent.isValid ()) return 0;
	return matrix->column_count->intValue () + matrix->trailing_columns;
}

int RKMatrixInputModel::rowCount (const QModelIndex& parent) const {
	if (parent.isValid ()) return 0;
	return matrix->row_count->intValue () + matrix->trailing_rows;
}

QVariant RKMatrixInputModel::data (const QModelIndex& index, int role) const {
	if (!index.isValid ()) return QVariant ();

	int row = index.row ();
	int column = index.column ();

	// handle trailing rows / cols in user expandable tables
	bool trailing = false;
	if (row >= matrix->row_count->intValue ()) {
		if ((!matrix->allow_user_resize_rows) || (row >= matrix->row_count->intValue () + matrix->trailing_rows)) {
			RK_ASSERT (false);
			return QVariant ();
		}
		trailing = true;
	}
	if (column >= matrix->column_count->intValue ()) {
		if ((!matrix->allow_user_resize_columns) || (column >= matrix->column_count->intValue () + matrix->trailing_columns)) {
			RK_ASSERT (false);
			return QVariant ();
		}
		trailing = true;
	};
	if (trailing) {
		if (role == Qt::BackgroundRole) return QVariant (QBrush (Qt::gray));
		if (role == Qt::ToolTipRole || role == Qt::StatusTipRole) return QVariant (i18n ("Type on these cells to expand the table"));
		return QVariant ();
	}

	// regular cells
	QString value;
	if (matrix->columns.size () > column) {	// column >= columns.size() can happen, legally. In this case the value is simply missing.
		value = matrix->columns[column].storage.value (row);
	}
	if ((role == Qt::DisplayRole) || (role == Qt::EditRole)) {
		return QVariant (value);
	} else if (role == Qt::BackgroundRole) {
		if (!matrix->is_valid && !matrix->isValueValid (value)) return QVariant (QBrush (Qt::red));
	} else if ((role == Qt::ToolTipRole) || (role == Qt::StatusTipRole)) {
		if (!matrix->is_valid && (value.isEmpty () && !matrix->allow_missings)) return QVariant (i18n ("Empty values are not allowed"));
		if (!matrix->is_valid && !matrix->isValueValid (value)) return QVariant (i18n ("This value is not allowed, here"));
	}
	return QVariant ();
}

Qt::ItemFlags RKMatrixInputModel::flags (const QModelIndex& index) const {
	// handle trailing rows / cols in user expandable tables
	if ((index.row () > matrix->row_count->intValue ()) || (index.column () > matrix->column_count->intValue ())) {
		return (Qt::ItemIsEditable | Qt::ItemIsEnabled);
	}
	return (Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
}

QVariant RKMatrixInputModel::headerData (int section, Qt::Orientation orientation, int role) const {
	if (role == Qt::DisplayRole) {
		const QStringList* list;
		if (orientation == Qt::Horizontal) list = &horiz_header;
		else list = &vert_header;
		if (section < list->size ()) return QVariant ((*list)[section]);
		return QVariant (QString::number (section + 1));
	}
	return QVariant ();
}

bool RKMatrixInputModel::setData (const QModelIndex& index, const QVariant& value, int role) {
	RK_TRACE (PLUGIN);

	if (!index.isValid ()) return false;
	if (role != Qt::EditRole) return false;
	matrix->setCellValue (index.row (), index.column (), value.toString ());
	return true;
}

#include "rkmatrixinput.moc"
