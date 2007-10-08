/***************************************************************************
                          rkvariable  -  description
                             -------------------
    begin                : Thu Aug 12 2004
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
#ifndef RKVARIABLE_H
#define RKVARIABLE_H

#include <qstring.h>
#include <q3intdict.h>

#include "robject.h"

class RContainerObject;

/** Abstract representation of a variable. A variable in this diction is an RObject, which is a vector of data. It may internally be a factor or a vector.
RKVariables are so far the only type of object that is really editable (data.frames are just a bundle of RKVariables). Therefore, for most practical purposes, the RKVariable represents a column in a table.

TODO: actually, for now, the data is always given to the backend as strings. Change that!
TODO: there should be "chunks" of column-data. This should be done at the level of rows, i.e. across columns. After all, if a row gets added/removed in one column, all other columns of the same table will also be affected.
TODO: which functions should do syncing by themselves, which should not? Or should all set... ()-functions have an extra parameter for this?

@author Thomas Friedrichsmeier
*/
class RKVariable : public RObject {
public:
/** constructs a new RKVariable as a child of the given parent and with the given name. Do not call directly, but let RContainerObject / RObjectList handle creation of new variables. */
	RKVariable (RContainerObject *parent, const QString &name);

	~RKVariable ();

/** set the VarType. If sync, the change will be communicated to the backend immediately. See RObject::RDataType */
	void setVarType (RObject::RDataType, bool sync=true);

/** reimplemented from RObject to also store value labels/factor levels (and in the future probably futher info) */
	void writeMetaData (RCommandChain *chain);
friend class RContainerObject;
	void rCommandDone (RCommand *command);
public:
////////////// BEGIN: data handling ////////////////////////
/** the Status enum is used for both keeping track of the entire row and inidvidual cells. For single cells the meaning should be obvious. The entire row
is set to Unused, if _no_ cell in the row is used, Valid if _all_ cells in the row are valid and Invalid if _one or more_ cells in the row are invalid, Unknown if _all_ cells in the row are unknown/updating. */
	enum Status { ValueUnused=0, ValueValid=1, ValueInvalid=2, ValueUnknown=4 };

/** sets whether changed data should be synced immediately or not. Set this to off for large paste operations. Rember to call setSyncing (true) and syncDataToR () after the paste is complete */
	void setSyncing (bool immediate);
/** syncs pending data changes to the backend */
	void syncDataToR ();
/** reimplemented from RObject */
	void updateDataFromR (RCommandChain *chain);
	
/** get the value at the given row in text-form - regardless of the storage mode.
@param pretty: get the text in pretty form, e.g. rounding numbers to a certain number of digits, replacing numeric values with value labels if available, etc. Formatting is done according to the meta-information stored in the RObject and global user preferences */
	QString getText (int row, bool pretty=false);
/** get the value at the given row in text-form suitable for submission to R. I.e. strings are quoted, numbers are not, empty values are returned as NA */
	QString getRText (int row);
/** set the value at the given row in text-form. Will try to convert the given string to the internal storage format if possible. */
	void setText (int row, const QString &text);

/** get a copy of the numeric values of rows starting from from_index, going to to_index. Do not use this before making sure that the rStorage () is really
numeric!  TODO: unused  */
	double *getNumeric (int from_row, int to_row);
/** set numeric values in the given range. Assumes you provide enough values for the range. If internalStorage is String, all values will be converted to strings, so you should use this function only, if you know you are dealing with a numeric object */
	void setNumeric (int from_row, int to_row, double *data);
/** like getNumeric, but returns values as an array of QString*s. TODO: unused */
	QString *getCharacter (int from_row, int to_row);
/** like setNumeric, but sets chars. If internalStorage () is numeric, attempts to convert the given strings to numbers. I.e. the function behaves essentially like setText (), but operates on a range of cells. */
	void setCharacter (int from_row, int to_row, QString *data);
	
/** returns the current status of the given cell */
	Status cellStatus (int row);

/** sets the status of the given range of cells to Unknown (the entire row if from_row and to_row are -1). Usually you call this, when you are about to update the given data-range, but haven't fetched the data for that, yet. The unknown-flag is cleared for the cells, as soon as data is written to those cells. The effect is that the cells will not be editable until the data was updated. */
	void setUnknown (int from_row=-1, int to_row=-1);

/** entirely remove the given row (i.e. the cell). Will also take care of updating the state (are there any invalid cells left?). Does not sync with the backend for technical reasons! You have to remove the row in the backend explicitly. */
	void removeRow (int row);
/** see removeRow (), but removes a range of rows (i.e. cells). Since data only needs to be copied once, this is more efficient than several single calls to removeRow (). Does not sync with the backend for technical reasons! You have to remove the row in the backend explicitly. */
	void removeRows (int from_row, int to_row);
/** inserts a row/cell (with empty value) just above the given index. Does not sync with the backend for technical reasons! You have to insert the row in the backend explicitly. */
	void insertRow (int row);
/** like insertRow (), but inserts count rows. Does not sync with the backend for technical reasons! You have to insert the row in the backend explicitly. */
	void insertRows (int row, int count);
/** Tells the object it has (data) length len. Usually this will only be called directly after creating a new object */
	void setLength (int len);

/** returns the map of value labels for this variable or 0 if no labels/levels are assigned. Does _not_ return a copy, but the real thing. Do not delete! */
	ValueLabels *getValueLabels ();
/** assigns a new map of labels. Also takes care of syncing with the backend. Ownership of the ValueLabels is transferred to the variable. Use setValueLabels (0) to remove all labels */
	void setValueLabels (ValueLabels *labels);
/** get value labels as string (for display) */
	QString getValueLabelString ();
/** set value labels from string (for paste operations) */
	void setValueLabelString (const QString &string);

/** Restores the variable including data and meta-data */
	void restore (RCommandChain *chain=0);

/** Stores formatting options set for this variable */
	struct FormattingOptions {
		enum Alignment { AlignDefault=0, AlignLeft=1, AlignRight=2 };
		enum Precision { PrecisionDefault=0, PrecisionRequired=1, PrecisionFixed=2 };

		Alignment alignment;
		Precision precision_mode;
		int precision;
	};

/** returns the formatting options, or 0 if no options specified (i.e. all set to default). Does _not_ return a copy, but the real thing. Do not delete! */
	FormattingOptions *getFormattingOptions ();
/** assigns new formatting options. Ownership of the FormattingOptions -struct is transferred to the variable. Use setFormatting (0) to remove all options */
	void setFormattingOptions (FormattingOptions *formatting_options);
/** get formatting options as a string (for display) */
	QString getFormattingOptionsString ();
/** parse formatting options from the given string */
	void setFormattingOptionsString (const QString &string);

/** This enum describes the alignment of text inside a table cell */
	enum CellAlign { AlignCellLeft=0, AlignCellRight=1 };
/** returns alignment to use for this variable */
	CellAlign getAlignment ();

/// an empty char
	static QString *na_char;
/// an unknown value
	static QString *unknown_char;

protected:
/** reimplemented from RObject to change the internal data storage mode, if the var is being edited */
	bool updateType (RData *new_data);
/** Extended from RObject::EditData to actually contain data. */
	struct RKVarEditData : public EditData {
		QString *cell_strings;
		double *cell_doubles;
		enum CellState {
			Unknown=0,
			Invalid=1,
			NA=2,
			Valid=4,
			UnsyncedInvalidState=8
		};
		int *cell_states;

/// the currently allocated length of cell_data and cell_states. Used to determine, when a re-allocation is required
		int allocated_length;
/// see setSyncing
		bool immediate_sync;
/// stores changes if syncing is not immediate
		ChangeSet *changes;
/// stores whether there were preivously invalid cells. If so, and there are no longer, now, we may change the mode in the backend.
		bool previously_valid;
/// the value-labels or factor levels assigned to this variable. 0 if no values/levels given
		ValueLabels *value_labels;
/// the formatting options set for this var (see FormattingOptions) */
		FormattingOptions *formatting_options;
/// storage for invalid fields
		Q3IntDict<QString> invalid_fields;
	};
/** reimplemented from RObject */
	void allocateEditData (RKEditor *editor);
/** reimplemented from RObject */
	void initializeEditDataToEmpty ();
/** reimplemented from RObject */
	void discardEditData ();
private:
/** changes the allocated storage to contain a least length elements. More data may be allocated than acutally needed. This function only ever does upsizing. */
	void extendToLength (int length);
/** changes the allocated storage to contain a least getLength elements. More data may be allocated than acutally needed. This function only ever does downsizing. */
	void downSize ();
/** convenience function to avoid typing static_cast... */
	RKVarEditData *myData () { return static_cast<RKVarEditData*> (data); };
/** takes care of syncing the given cell */
	void cellChanged (int row);
/** takes care of syncing the given range of cells */
	void cellsChanged (int from_row, int to_row);
/** writes the given range of cells to the backend (regardless of whether syncing should be immediate) */
	void writeData (int from_row, int to_row, RCommandChain *chain=0);
	void writeInvalidField (int row, RCommandChain *chain=0);
/** writes the values labels to the backend */
	void writeValueLabels (RCommandChain *chain);
/** creates/parses formatting options from the stored meta-property string. See also: getFormattingOptions () */
	FormattingOptions *parseFormattingOptionsString (const QString &string);
/** tries to match a value-label to the value in the given cell. Returns the label, or - if there is no label - the original value in textual representation */
	QString getLabeled (int row);
/////////////////// END: data-handling //////////////////////
};

typedef RKVariable* RKVarPtr;

#endif
