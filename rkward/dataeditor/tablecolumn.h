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
#ifndef TABLECOLUMN_H
#define TABLECOLUMN_H

#include <qstring.h>
#include <qintdict.h>

class RObject;

/**
This is a helper class to be used in the TwinTable. It represents a column in the table along with the data in it, the data-type and the associated RObject. It comes with functions to read and set data in text format, regardless of how the data is stored internally. It also takes care of allocating memory in chunks, so frequent re-allocations are not necessary.

TODO: there should be "chunks" of column-data. This should be done at the level of rows, i.e. across columns. After all, if a row gets added/removed in one column, all other columns of the same table will also be affected.
TODO: which functions should do syncing by themselves, which should not? Or should all set... ()-functions have an extra parameter for this?

@author Thomas Friedrichsmeier
*/
class TableColumn {
public:
	TableColumn ();

	~TableColumn ();
	
/** the Status enum is used for both keeping track of the entire row and inidvidual cells. For single cells the meaning should be obvious. The entire row
is set to Unused, if _no_ cell in the row is used, Valid if _all_ cells in the row are valid and Invalid if _one or more_ cells in the row are invalid, Unknown if _all_ cells in the row are unknown/updating. */
	enum Status { Unused=0, Valid=1, Invalid=2, Unknown=4 };
/** The storage mode. For most vars this will be numeric. Note that if a single cell in a row is Invalid, the entire row will - in the R-backend - have to be stored as a string. */
	enum Storage { String=0, Numeric=1 };
/** See Storage enum. Returns how the row would like to be saved in the R-backend, and how it represents its data internally. The storage mode in the backend may be different due to invalid data. */
	Storage internalStorage ();
/** See Storage enum. Returns how the row is actually saved in the R-backend. */
	Storage rStorage ();
/** changes the internal storage mode, and also - if possible/necessary - the storage mode in the backend. Warning: this is an expensive operation, as it may involve conversion, deletion, reallocation and copying of data */
	void changeStorageMode (Storage new_mode);

/** get the value at the given row in text-form - regardless of the storage mode. */
	QString getText (int row);
/** set the value at the given row in text-form. Will try to convert the given string to the internal storage format if possible. */
	void setText (int row, const QString &text);
/** get the text in pretty form, e.g. rounding numbers to a certain number of digits, replacing numeric values with value labels if available, etc. Formatting is done according to the meta-information stored in the RObject and global user preferences */
	QString getFormatted (int row);
/** get a copy of the numeric values of rows starting from from_index, going to to_index. Do not use this before making sure that the rStorage () is really
numeric! */
	double *getNumeric (int from_row, int to_row);
/** set numeric values in the given range. Assumes you provide enough values for the range. If internalStorage is String, all values will be converted to strings, so you should use this function only, if you know you are dealing with a numeric object */
	void setNumeric (int from_row, int to_row, double *data);
/** like getNumeric, but returns values as an array of char*s */
	char *getCharacter (int from_row, int to_row);
/** like setNumeric, but sets chars. If internalStorage () is numeric, attempts to convert the given strings to numbers. I.e. the function behaves essentially like setText (), but operates on a range of cells. */
	void setCharacter (int from_row, int to_row, char *data);

/** sets the status of the given range of cells to Unknown (the entire row if from_row and to_row are -1). Usually you call this, when you are about to update the given data-range, but haven't fetched the data for that, yet. The unknown-flag is cleared for the cells, as soon as data is written to those cells. The effect is that the cells will not be editable until the data was updated. */
	void setUnknown (int from_row=-1, int to_row=-1);

/** entirely remove the given row (i.e. the cell). Will also take care of updating the state (are there any invalid cells left?), and syncing with the backend */
	void removeRow (int row);
/** see removeRow (), but removes a range of rows (i.e. cells). Since data only needs to be copied once, this is more efficient than several single calls to removeRow () */
	void removeRows (int from_row, int to_row);
/** inserts a row/cell (with empty value) just above the given index */
	void insertRow (int row);
/** like insertRow (), but inserts count rows */
	void insertRows (int row, int count);
private:
/// status of the row
	Status row_status;
/// array of status for the individual cells
	Status *cell_status;
/// array of numeric data for the cells. 0 if internalStorage is set to String
	double *cell_double_data;
/// array of string data for the cells. 0 if internalStorage is set to Numeric
	QCString *cell_string_data;
/// see cell_invalid_data
	typedef QIntDict<QCString> InvalidCellData;
/// For cells marked invalid, we need to store all invalid values as a string. We do this in a map so invalid cells can be added/removed easily and without taking up lots of memory
	InvalidCellData cell_invalid_data;
/// the currently allocated length of cell_double_data of cell_string_data. Used to determine, when a re-allocation is required
	int allocated_length;
/// pointer to the RObject associated with the column
	RObject *object;
	
};

#endif
