/***************************************************************************
                          tablecolumn  -  description
                             -------------------
    begin                : Fri Sep 10 2004
    copyright            : (C) 2004 by Thomas Friedrichsmeier
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
#include "tablecolumn.h"

#include "../rbackend/rinterface.h"
#include "../debug.h"

#define ALLOC_STEP 100

#define DELETE_STRING_DATA(row) if (cell_string_data && (cell_string_data[row] != empty) && (cell_string_data[row] != unknown)) { delete cell_string_data[row]; } cell_string_data[row] = 0;

// static
char *TableColumn::empty = qstrdup ("");
char *TableColumn::unknown = qstrdup ("?");

TableColumn::TableColumn (RObject *object) {
	TableColumn::object = object;
	
	cell_string_data = new char*[ALLOC_STEP];
	for (int i=0; i < ALLOC_STEP; ++i) {
		cell_string_data[i] = empty;
	}
	cell_double_data = new double[ALLOC_STEP];
	allocated_length = ALLOC_STEP;
}

TableColumn::~TableColumn () {
	delete [] cell_double_data;
	for (int i = 0; i < allocated_length; ++i) {
		DELETE_STRING_DATA (i);
	}
	delete [] cell_string_data;
}

/** See Storage enum. Returns how the row would like to be saved in the R-backend, and how it represents its data internally. The storage mode in the backend may be different due to invalid data. */
TableColumn::Storage TableColumn::internalStorage () {
	//TODO: implement correctly!
	return String;
}

/** See Storage enum. Returns how the row is actually saved in the R-backend. */
TableColumn::Storage TableColumn::rStorage () {
	//TODO: implement correctly!
	return String;
}

QString TableColumn::getText (int row) {
	if (row >= allocated_length) {
		RK_ASSERT (false);
		return "";
	}
	if (internalStorage () == String) {
		return QString::fromLocal8Bit (cell_string_data[row]);
	} else {
		if (cell_string_data[row] != 0) {
			return QString::fromLocal8Bit (cell_string_data[row]);
		} else {
			return QString::number (cell_double_data[row]);
		}
	}
}

void TableColumn::setText (int row, const QString &text) {
	if (row >= allocated_length) {
		// TODO: allocate more data!
		RK_ASSERT (false);
		return;
	}

	// delete previous string data, unless it's a special value
	DELETE_STRING_DATA (row);

	if (internalStorage () == String) {
		if (text.isEmpty ()) cell_string_data[row] = empty;
		else cell_string_data[row] = qstrdup (text.local8Bit ());
		return;
	}
		
	bool ok;
	cell_double_data[row] = text.toDouble (&ok);
	if (!ok) {
		if (text.isEmpty ()) cell_string_data[row] = empty;
		else cell_string_data[row] = qstrdup (text.local8Bit ());
		return;
	}
}

/** get the text in pretty form, e.g. rounding numbers to a certain number of digits, replacing numeric values with value labels if available, etc. Formatting is done according to the meta-information stored in the RObject and global user preferences */
QString TableColumn::getFormatted (int row) {
	// TODO: implement!
	// for now just return the text
	return getText (row);
}

/** get a copy of the numeric values of rows starting from from_index, going to to_index. Do not use this before making sure that the rStorage () is really
numeric! */
double *TableColumn::getNumeric (int from_row, int) {
	// we simply return the whole array starting at the given offset for now. Change this, if the storage mechanism gets changed!
	return &(cell_double_data[from_row]);
}

void TableColumn::setNumeric (int from_row, int to_row, double *data) {
	for (int row=from_row; row <= to_row; ++row) {
		DELETE_STRING_DATA (row);
	}
	
	if (internalStorage () == String) {
		RK_ASSERT (false);		// asserting false to catch cases of this use for now. it's not really a problem, though
		int i = 0;
		for (int row=from_row; row <= to_row; ++row) {
			cell_string_data[row] = qstrdup (QString::number (data[i++]));
		}
	} else {
		int i = 0;
		for (int row=from_row; row <= to_row; ++row) {
			cell_double_data[row] = data[i++];
		}
	}
}

/** like getNumeric, but returns values as an array of char*s */
char **TableColumn::getCharacter (int from_row, int to_row) {
	RK_ASSERT (from_row <= to_row);
	if (internalStorage () == String) {
		return &(cell_string_data[from_row]);
	} else {
		char **ret = new char*[(to_row - from_row) + 1];
		int i=0;
		for (int row=from_row; row <= to_row; ++row) {
			// TODO: converting to QString and then back to local8Bit ()+string copying is inefficient!
			ret[i++] = qstrdup (getText (row).local8Bit ());
		}
		return ret;
	}
}

/** like setNumeric, but sets chars. If internalStorage () is numeric, attempts to convert the given strings to numbers. I.e. the function behaves essentially like setText (), but operates on a range of cells. */
void TableColumn::setCharacter (int from_row, int to_row, char **data) {
	if (internalStorage () == String) {
		int i=0;
		for (int row=from_row; row <= to_row; ++row) {
			DELETE_STRING_DATA (row);
			cell_string_data[row] = data[i++];
		}
	} else {
		int i=0;
		for (int row=from_row; row <= to_row; ++row) {
			// TODO: converting to QString and then back to local8Bit () is inefficient!
			// note: old string pointers are deleted inside setText ();
			setText (row, data[i++]);
		}
	}
}

void TableColumn::setUnknown (int from_row, int to_row) {
	if ((from_row < 0)) from_row = 0;
	if ((to_row < 0)) to_row = allocated_length - 1;
		
	for (int row=from_row; row <= to_row; ++row) {
		DELETE_STRING_DATA (row);
		cell_string_data[row] = unknown;
	}
}

TableColumn::Status TableColumn::cellStatus (int row) {
	if (cell_string_data[row] == empty) {
		return Unused;
	} else if (cell_string_data[row] == unknown) {
		return Unknown;
	} else if ((internalStorage () == Numeric) && (cell_string_data[row] != 0)) {
		return Invalid;
	}
	return Valid;
}

/** entirely remove the given row (i.e. the cell). Will also take care of updating the state (are there any invalid cells left?), and syncing with the backend */
void TableColumn::removeRow (int row) {
	removeRows (row, row);
}

/** see removeRow (), but removes a range of rows (i.e. cells). Since data only needs to be copied once, this is more efficient than several single calls to removeRow () */
void TableColumn::removeRows (int from_row, int to_row) {
	for (int row = from_row; row <= to_row; ++row) {
		DELETE_STRING_DATA (row);
	}

	if (to_row < (allocated_length - 1)) {	// not the last rows
		qmemmove (&(cell_string_data[to_row+1]), &(cell_string_data[from_row]), (allocated_length - to_row - 1) * sizeof (char*));
		qmemmove (&(cell_double_data[to_row+1]), &(cell_double_data[from_row]), (allocated_length - to_row - 1) * sizeof (double*));
	}

	for (int row = (allocated_length - 1 - (to_row - from_row)); row < allocated_length; ++row) {
		cell_string_data[allocated_length - 1] = empty;
		cell_double_data[allocated_length - 1] = 0;
	}
}

void TableColumn::insertRow (int row) {
	insertRows (row, 1);
}

void TableColumn::insertRows (int row, int count) {
	// TODO: allocate more memory if needed!!!
	qmemmove (&(cell_string_data[row]), &(cell_string_data[row+count]), (allocated_length - row - count) * sizeof (char*));
	qmemmove (&(cell_double_data[row]), &(cell_double_data[row+count]), (allocated_length - row - count) * sizeof (double));
	
	for (int i=row+count; i >= row; --i) {
		cell_string_data[i] = empty;
		cell_double_data[i] = 0;
	}
}
