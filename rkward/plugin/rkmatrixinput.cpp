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

#include "klocale.h"

#include "../misc/xmlhelper.h"

#include "../debug.h"

RKMatrixInput::RKMatrixInput (const QDomElement& element, RKComponent* parent_component, QWidget* parent_widget) : RKComponent (parent_component, parent_widget), QAbstractTableModel () {
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
		max = xml->getDoubleAttribute (element, "min", FLT_MAX, DL_INFO);
	} else {
		min = -FLT_MAX;
		max = FLT_MAX;
	}

	allow_missings = xml->getBoolAttribute (element, "allow_missings", false, DL_INFO);
	allow_user_resize_columns = xml->getBoolAttribute (element, "allow_user_resize_columns", true, DL_INFO);
	allow_user_resize_rows = xml->getBoolAttribute (element, "allow_user_resize_rows", true, DL_INFO);
	trailing_rows = allow_user_resize_rows ? 1 :0;
	trailing_columns = allow_user_resize_columns ? 1 :0;

	row_count = new RKComponentPropertyInt (this, false, xml->getIntAttribute (element, "rows", 2, DL_INFO));
	column_count = new RKComponentPropertyInt (this, false, xml->getIntAttribute (element, "columns", 2, DL_INFO));
	tsv_data = new RKComponentPropertyBase (this);
	row_count->setInternal (true);
	addChild (row_count);
	column_count->setInternal (true);
	addChild (column_count);
	addChild (tsv_data);
	connect (row_count, SIGNAL (valueChanged(RKComponentPropertyBase*)), this, SLOT (dimensionPropertyChanged(RKComponentPropertyBase*)));
	connect (column_count, SIGNAL (valueChanged(RKComponentPropertyBase*)), this, SLOT (dimensionPropertyChanged(RKComponentPropertyBase*)));
	connect (tsv_data, SIGNAL (valueChanged(RKComponentPropertyBase*)), this, SLOT (tsvPropertyChanged()));
	updating_tsv_data = false;
	updating_dimensions = false;

	display->setModel (this);
}


RKMatrixInput::~RKMatrixInput () {
	RK_TRACE (PLUGIN);
}

int RKMatrixInput::columnCount (const QModelIndex& parent) const {
	if (parent.isValid ()) return 0;
	return column_count->intValue () + trailing_columns;
}

int RKMatrixInput::rowCount (const QModelIndex& parent) const {
	if (parent.isValid ()) return 0;
	return row_count->intValue () + trailing_rows;
}

QVariant RKMatrixInput::data (const QModelIndex& index, int role) const {
	if (!index.isValid ()) return QVariant ();

	int row = index.row ();
	int column = index.column ();

	// handle trailing rows / cols in user expandable tables
	bool trailing = false;
	if (row > row_count->intValue ()) {
		if ((!allow_user_resize_rows) || (row > row_count->intValue () + trailing_rows)) {
			RK_ASSERT (false);
			return QVariant ();
		}
		trailing = true;
	}
	if (column > column_count->intValue ()) {
		if ((!allow_user_resize_columns) || (column > column_count->intValue () + trailing_columns)) {
			RK_ASSERT (false);
			return QVariant ();
		}
		trailing = true;
	};
	if (trailing) {
		if (role == Qt::BackgroundRole) return QVariant (QBrush (Qt::lightGray));
		if (role == Qt::ToolTipRole || role == Qt::StatusTipRole) return QVariant (i18n ("Type on these cells to expand the table"));
		return QVariant ();
	}

	// regular cells
	QString value;
	if (columns.size () > column) {	// column >= columns.size() can happen, legally. In this case the value is simply missing.
		value = columns[column].storage.value (row);
	}
	if ((role == Qt::DisplayRole) || (role == Qt::EditRole)) {
		return QVariant (value);
	} else if (role == Qt::BackgroundRole) {
		if (!is_valid && (value.isEmpty () && !allow_missings) || !isValueValid (value)) return QVariant (QBrush (Qt::red));
	} else if ((role == Qt::ToolTipRole) || (role == Qt::StatusTipRole)) {
		if (!is_valid && (value.isEmpty () && !allow_missings)) return QVariant (i18n ("Empty values are not allowed"));
		if (!is_valid && !isValueValid (value)) return QVariant (i18n ("This value is not allowed, here"));
	}
	return QVariant ();
}

bool RKMatrixInput::setData (const QModelIndex& index, const QVariant& value, int role) {
	RK_TRACE (PLUGIN);

	if (!index.isValid ()) return false;
	if (role != Qt::EditRole) return false;
	setCellValue (index.row (), index.column (), value.toString ());
	return true;
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

	Column &col = columns[col];
	while (row >= col.storage.size ()) {
		col.storage.append (QString ());
	}
	col.storage[row] = value;
	updateColumn (row, column);
}

void RKMatrixInput::setColumnValue (int column, const QString& value) {
	RK_TRACE (PLUGIN);

	if (!expandStorageForColumn (column)) return;
	columns[column].storage = value.split ('\t', QString::KeepEmptyParts);
	updateColumn (0, column);
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
	updateValidityFlag ();
}

void RKMatrixInput::updateValidityFlag () {
	RK_TRACE (PLUGIN);

	for (int i = 0; i < column_count->intValue (); ++i) {
		if (i >= columns.size ()) {	// hit end of data, before hitting any invalid strings
			is_valid = allow_missings;
			return;
		}

		Column &col = columns[i];
		if (col.valid_up_to_row >= (row_count->intValue () - 1)) continue;
		else if (allow_missings && (col.valid_up_to_row >= (col.storage.size () - 1))) continue;
		else {
			is_valid = false;
			return;
		}
	}
	is_valid = true;
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
			row_count->setIntValue (max_row);
			updating_dimensions = false;
		}
	}

	int max_col = column_count->intValue () - 1;
	if (allow_user_resize_columns) {
		for (max_col = columns.size () - 1; max_col >= 0; --max_col) {
			if (columns[max_col].filled_up_to_row < 0) {
				columns.pop_back ();
			} else {
				break;
			}
		}
		if (max_col != column_count->intValue () - 1) {
			updating_dimensions = true;
			column_count->setIntValue (max_col);
			updating_dimensions = false;
		}
	}

	QStringList tsv;
	QString empty_column (max_row, '\t');	// NOTE: *Not* max_row+1, as the number of seps is rows - 1
	for (int i = 0; i < max_col; ++i) {
		if (i > (columns.size () - 1)) {
			tsv.append (empty_column);
		} else {
			Column& col = columns[i];
			if (col.cached_tab_joined_string.isNull ()) {
				QString cache = col.storage.mid (0, max_row + 1).join ('\t');
				if (col.storage.size () < max_row) {
					QString empty_trail (max_row - col.storage.size (), '\t');
					cache.append (empty_trail);
				}
				col.cached_tab_joined_string = cache;
			}
			tsv.append (col.cached_tab_joined_string);
		}
	}
	tsv_data->setValue (tsv);

	updating_tsv_data = false;
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

	updateValidityFlag ();
	updateDataAndDimensions ();
}

void RKMatrixInput::tsvPropertyChanged () {
	RK_TRACE (PLUGIN);

	bool old_updating_tsv_data = updating_tsv_data;
	updating_tsv_data = true;

	columns.clear ();
	QStringList coldata = tsv_data->value ().split ('\n', QString::KeepEmptyParts);
	for (int i = 0; i < coldata.size (); ++i) {
		setColumnValue (i, coldata[i]);
	}

	updating_tsv_data = old_updating_tsv_data;
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
		val = value.toDouble (&number_ok);
	}
	if (!number_ok) return false;
	if (val < min) return false;
	return (val < max);
}

#error TODO
#error TODO signal model layout and data changes

#include "rkmatrixinput.moc"
