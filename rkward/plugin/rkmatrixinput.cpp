/*
rkmatrixinput - This file is part of RKWard (https://rkward.kde.org). Created: Tue Oct 09 2012
SPDX-FileCopyrightText: 2012-2016 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "rkmatrixinput.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QHeaderView>

#include <KLocalizedString>
#include <KStandardAction>

#include "../misc/rktableview.h"
#include "../dataeditor/rktextmatrix.h"
#include "../misc/xmlhelper.h"

#include "../debug.h"

RKMatrixInput::RKMatrixInput (const QDomElement& element, RKComponent* parent_component, QWidget* parent_widget) : RKComponent (parent_component, parent_widget) {
	RK_TRACE (PLUGIN);

	is_valid = true;

	// get xml-helper
	XMLHelper *xml = parent_component->xmlHelper ();

	// create layout
	QVBoxLayout *vbox = new QVBoxLayout (this);
	vbox->setContentsMargins (0, 0, 0, 0);

	QLabel *label = new QLabel (xml->i18nStringAttribute (element, "label", i18n ("Enter data:"), DL_INFO), this);
	vbox->addWidget (label);

	display = new RKTableView (this);
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

	min_rows = xml->getIntAttribute (element, "min_rows", 0, DL_INFO);
	min_columns = xml->getIntAttribute (element, "min_columns", 0, DL_INFO);

	// Note: string type matrix allows missings, implicitly (treating them as empty strings)
	allow_missings = xml->getBoolAttribute (element, "allow_missings", false, DL_INFO);
	if (mode == String) allow_missings = true;
	allow_user_resize_columns = xml->getBoolAttribute (element, "allow_user_resize_columns", true, DL_INFO);
	allow_user_resize_rows = xml->getBoolAttribute (element, "allow_user_resize_rows", true, DL_INFO);
	trailing_rows = allow_user_resize_rows ? 1 : 0;
	trailing_columns = allow_user_resize_columns ? 1 : 0;

	row_count = new RKComponentPropertyInt (this, false, xml->getIntAttribute (element, "rows", qMax (2, min_rows), DL_INFO));
	column_count = new RKComponentPropertyInt (this, false, xml->getIntAttribute (element, "columns", qMax (2, min_columns), DL_INFO));
	tsv_data = new RKComponentPropertyBase (this, false);
	row_count->setInternal (true);
	addChild ("rows", row_count);
	column_count->setInternal (true);
	addChild ("columns", column_count);
	addChild ("tsv", tsv_data);
	connect (row_count, &RKComponentPropertyBase::valueChanged, this, &RKMatrixInput::dimensionPropertyChanged);
	connect (column_count, &RKComponentPropertyBase::valueChanged, this, &RKMatrixInput::dimensionPropertyChanged);
	connect (tsv_data, &RKComponentPropertyBase::valueChanged, this, &RKMatrixInput::tsvPropertyChanged);
	updating_tsv_data = false;

	model = new RKMatrixInputModel (this);
	QString headers = xml->getStringAttribute (element, "horiz_headers", QString (), DL_INFO);
	if (!headers.isEmpty ()) model->horiz_header = headers.split (';');
	else if (!headers.isNull ()) display->horizontalHeader ()->hide ();	// attribute explicitly set to ""
	headers = xml->getStringAttribute (element, "vert_headers", QString (), DL_INFO);
	if (!headers.isEmpty ()) model->vert_header = headers.split (';');
	else if (!headers.isNull ()) display->verticalHeader ()->hide ();
	updateAll ();
	display->setModel (model);
	display->setAlternatingRowColors (true);
	if (xml->getBoolAttribute (element, "fixed_width", false, DL_INFO)) {
		display->horizontalHeader ()->setStretchLastSection (true);
	}
	if (xml->getBoolAttribute (element, "fixed_height", false, DL_INFO)) {
		int max_row = row_count->intValue () - 1;
		display->setHorizontalScrollBarPolicy (Qt::ScrollBarAlwaysOff);
		display->setFixedHeight (display->horizontalHeader ()->height () + display->rowViewportPosition (max_row) + display->rowHeight (max_row));
	}

	// define standard actions
	QAction *acut = KStandardAction::cut(this, &RKMatrixInput::cut, this);
	display->addAction(acut);
	QAction *acopy = KStandardAction::copy(this, &RKMatrixInput::copy, this);
	display->addAction(acopy);
	QAction *apaste = KStandardAction::paste(this, &RKMatrixInput::paste, this);
	display->addAction(apaste);
	display->setContextMenuPolicy(Qt::ActionsContextMenu);

	display->setRKItemDelegate (new RKItemDelegate (display, model, true));
	connect (display, &RKTableView::blankSelectionRequest, this, &RKMatrixInput::clearSelectedCells);
}

RKMatrixInput::~RKMatrixInput () {
	RK_TRACE (PLUGIN);
}

QVariant RKMatrixInput::value (const QString& modifier) {
	if (modifier.isEmpty () || (modifier == "cbind")) {
		QStringList ret;
		for (int i = 0; i < column_count->intValue (); ++i) {
			ret.append ("\tc (" + makeColumnString (i, ", ") + ')');
		}
		return QString ("cbind (\n" + ret.join (",\n") + "\n)");
	} else if (modifier.startsWith (QLatin1String ("row."))) {
		bool ok;
		int row = QStringView(modifier).mid(4).toInt(&ok);
		if ((row >= 0) && ok) {
			return (rowStrings (row));
		}
	}

	bool ok;
	int col = modifier.toInt (&ok);
	if ((col >= 0) && ok) {
		QStringList l;
		if (col < columns.size ()) l = columns[col].storage;
		while (l.size () < row_count->intValue ()) {
			l.append (QString ());
		}
		if (l.size () > row_count->intValue ()) l = l.mid (0, row_count->intValue ());
		return l;
	}
	return (QString ("Modifier '" + modifier + "' not recognized\n"));
}

bool RKMatrixInput::expandStorageForColumn (int column) {
	RK_TRACE (PLUGIN);

	if ((column < 0) || (!allow_user_resize_columns && (column >= column_count->intValue ()))) {
		RK_ASSERT (false);
		return false;
	}

	while (column >= columns.size ()) {
		Column col;
		col.last_valid_row = -1;
		columns.append (col);
	}
	return true;
}

QString RKMatrixInput::cellValue (int row, int column) const {
	if ((column < 0) || (column >= columns.size ())) return (QString ());
	return columns[column].storage.value (row);
}


void RKMatrixInput::setCellValue (int row, int column, const QString& value) {
	RK_TRACE (PLUGIN);

	if ((!expandStorageForColumn (column)) || (row < 0) || (!allow_user_resize_rows && (row >= row_count->intValue ()))) {
		RK_ASSERT (false);
		return;
	}

	Column &col = columns[column];
	if (col.storage.value (row) == value) return;

	while (row >= col.storage.size ()) {
		col.storage.append (QString ());
	}
	col.storage[row] = value;
	updateColumn (column);
	Q_EMIT model->dataChanged(model->index(row, column), model->index(row, column));
}

void RKMatrixInput::setColumnValue (int column, const QString& value) {
	RK_TRACE (PLUGIN);

	if (!expandStorageForColumn (column)) return;
	columns[column].storage = value.split ('\t', Qt::KeepEmptyParts);
	updateColumn (column);
	Q_EMIT model->dataChanged (model->index(0, column), model->index(row_count->intValue() + trailing_rows, column));
}

void RKMatrixInput::updateColumn (int column) {
	RK_TRACE (PLUGIN);
	RK_ASSERT ((column >= 0) && (column < columns.size ()));

	Column &col = columns[column];

	// check for trailing empty rows:
	int last_row = col.storage.size ();
	while ((--last_row >= min_rows) && col.storage[last_row].isEmpty ()) {	// strip empty trailing strings
		col.storage.pop_back ();
	}

	col.last_valid_row = -1;
	col.cached_tab_joined_string.clear (); // == no valid cache

	updateAll ();
}

QString pasteableValue (const QString &in, bool string) {
	if (string) return (RObject::rQuote (in));
	else if (in.isEmpty ()) return ("NA");
	else return in;
}

QString RKMatrixInput::makeColumnString (int column, const QString& sep, bool r_pasteable) {
	RK_TRACE (PLUGIN);

	QStringList storage;
	if (column < columns.size ()) {
		storage = columns[column].storage;
	}
	QString ret;
	ret.reserve (3 * row_count->intValue ());	// a rather conservative estimate for most purposes
	for (int i = 0; i < row_count->intValue (); ++i) {
		if (i > 0) ret.append (sep);
		const QString val = storage.value (i);
		if (r_pasteable) ret.append (pasteableValue (val, mode == String));
		else ret.append (val);
	}
	return ret;
}

QStringList RKMatrixInput::rowStrings (int row) {
	RK_TRACE (PLUGIN);

	QStringList ret;
	ret.reserve (column_count->intValue ());
	for (int i = 0; i < column_count->intValue (); ++i) {
		if (i < columns.size ()) ret.append (columns[i].storage.value (row));
		else ret.append (QString ());
	}
	return ret;
}

void RKMatrixInput::updateAll () {
	RK_TRACE (PLUGIN);

	if (updating_tsv_data) return;
	updating_tsv_data = true;

	int max_row = row_count->intValue () - 1;
	if (allow_user_resize_rows) {
		max_row = -1;
		for (int i = columns.size () - 1; i >= 0; --i) {
			max_row = qMax (max_row, columns[i].storage.size () - 1);
		}
	}
	max_row = qMax (min_rows - 1, max_row);
	if (max_row != row_count->intValue () - 1) {
		row_count->setIntValue (max_row + 1);
	}

	int max_col = column_count->intValue () - 1;
	if (allow_user_resize_columns) {
		for (max_col = columns.size () - 1; max_col >= 0; --max_col) {
			if (!columns[max_col].storage.isEmpty ()) {
				break;
			}
		}
	}
	max_col = qMax (min_columns - 1, max_col);
	if (max_col != column_count->intValue () - 1) {
		column_count->setIntValue (max_col + 1);
	}

	QStringList tsv;
	int i = 0;
	for (; i < columns.size (); ++i) {
		Column& col = columns[i];
		if (col.cached_tab_joined_string.isEmpty ()) {
			col.cached_tab_joined_string = makeColumnString (i, "\t", false);
		}
		tsv.append (col.cached_tab_joined_string);
	}
	for (; i < max_col; ++i) {
		tsv.append (QString (max_row, '\t'));
	}
	tsv_data->setValue (tsv.join ("\n"));

	updating_tsv_data = false;

	// finally, check whether table is valid, and signal change
	bool new_valid = true;
	for (int i = 0; i < column_count->intValue (); ++i) {
		if (!isColumnValid (i)) {
			new_valid = false;
			break;
		}
	}
	if (new_valid != is_valid) {
		is_valid = new_valid;
		Q_EMIT model->headerDataChanged(Qt::Horizontal, 0, column_count->intValue() - 1);
	}
	changed ();
}

void RKMatrixInput::dimensionPropertyChanged (RKComponentPropertyBase *property) {
	RK_TRACE (PLUGIN);

	if (allow_user_resize_rows && (property == row_count)) {
		RK_ASSERT (updating_tsv_data);
	}
	if (allow_user_resize_columns && (property == column_count)) {
		RK_ASSERT (updating_tsv_data);
	}

	if (property == row_count) {		// invalidates column caches
		for (int i = columns.size () - 1; i >= 0; --i) {
			columns[i].last_valid_row = qMin (columns[i].last_valid_row, row_count->intValue () - 1);
			columns[i].cached_tab_joined_string.clear ();
		}
	}

	Q_EMIT model->layoutAboutToBeChanged();
	updateAll();
	Q_EMIT model->layoutChanged();
}

void RKMatrixInput::tsvPropertyChanged () {
	if (updating_tsv_data) return;
	updating_tsv_data = true;
	RK_TRACE (PLUGIN);

	columns.clear ();
	QStringList coldata = fetchStringValue (tsv_data).split ('\n', Qt::KeepEmptyParts);
	for (int i = 0; i < coldata.size (); ++i) {
		setColumnValue (i, coldata[i]);
	}

	updating_tsv_data = false;
	updateAll ();
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
	return (val <= max);
}

bool RKMatrixInput::isColumnValid (int column) {
	if (column < 0) {
		RK_ASSERT (false);
		return false;
	}

	if (mode == String) return true;

	if (column >= columns.size ()) return (allow_missings || (row_count->intValue () == 0));

	Column &col = columns[column];
	if (col.last_valid_row >= (row_count->intValue () - 1)) {
		return true;
	} else if (allow_missings && (col.last_valid_row >= (col.storage.size () - 1))) {
		return true;
	}

	int row = col.last_valid_row + 1;
	for (; row < col.storage.size (); ++row) {
		if (!isValueValid (col.storage[row])) {
			col.last_valid_row = row - 1;
			return false;
		}
	}
	col.last_valid_row = row - 1;

	if (col.last_valid_row < (row_count->intValue () - 1)) {
		return allow_missings;
	}
	return true;
}

void RKMatrixInput::clearSelectedCells () {
	RK_TRACE (PLUGIN);

	QItemSelectionRange range = display->getSelectionBoundaries ();
	if (!range.isValid ()) return;

	updating_tsv_data = true;
	for (int col = range.left (); col <= range.right (); ++col) {
		for (int row = range.top (); row <= range.bottom (); ++row) {
			setCellValue (row, col, QString ());
		}
	}
	updating_tsv_data = false;
	updateAll ();
}

void RKMatrixInput::copy () {
	RK_TRACE (PLUGIN);

	QItemSelectionRange range = display->getSelectionBoundaries ();
	if (!range.isValid ()) return;

	RKTextMatrix ret;
	for (int col = range.left (); col <= range.right (); ++col) {
		for (int row = range.top (); row <= range.bottom (); ++row) {
			ret.setText (row - range.top (), col - range.left (), cellValue (row, col));
		}
	}
	ret.copyToClipboard ();
}

void RKMatrixInput::cut () {
	RK_TRACE (PLUGIN);

	copy ();
	clearSelectedCells ();
}

void RKMatrixInput::paste () {
	RK_TRACE (PLUGIN);

	int left = 0;
	int top = 0;
	QItemSelectionRange range = display->getSelectionBoundaries ();
	if (range.isValid ()) {
		left = range.left ();
		top = range.top ();
	}

	RKTextMatrix pasted = RKTextMatrix::matrixFromClipboard ();
	int height = allow_user_resize_rows ? pasted.numRows () : qMin (pasted.numRows (), row_count->intValue () - top);
	int width = allow_user_resize_columns ? pasted.numColumns () : qMin (pasted.numColumns (), column_count->intValue () - left);
	
	updating_tsv_data = true;
	for (int c = 0; c < width; ++c) {
		for (int r = 0; r < height; ++r) {
			setCellValue (r + top, c + left, pasted.getText (r, c));
		}
	}
	updating_tsv_data = false;
	updateAll ();
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
	QString value = matrix->cellValue (row, column);
	if ((role == Qt::DisplayRole) || (role == Qt::EditRole)) {
		return QVariant (value);
	} else if (role == Qt::BackgroundRole) {
		if (!matrix->is_valid && !matrix->isValueValid (value)) return QVariant (QColor (Qt::red));
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
	} else if (orientation == Qt::Horizontal) {
		if (section < matrix->column_count->intValue ()) {
			if ((role == Qt::BackgroundRole) && !matrix->isColumnValid (section)) return QVariant (QColor (Qt::red));
			if (((role == Qt::ToolTipRole) || (role == Qt::StatusTipRole)) && !matrix->isColumnValid (section)) return QVariant (i18n ("This column contains illegal values in some of its cells"));
		}
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

