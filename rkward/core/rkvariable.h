/*
rkvariable - This file is part of the RKWard project. Created: Thu Aug 12 2004
SPDX-FileCopyrightText: 2004-2012 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef RKVARIABLE_H
#define RKVARIABLE_H

#include <QHash>
#include <QStringList>

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
	RKVariable(RContainerObject *parent, const QString &name);

	~RKVariable();

	/** set the VarType. If sync, the change will be communicated to the backend immediately. See RObject::RDataType */
	void setVarType(RObject::RDataType, bool sync = true);

	/** reimplemented from RObject to also store value labels/factor levels (and in the future probably further info) */
	void writeMetaData(RCommandChain *chain) override;

  public:
	////////////// BEGIN: data handling ////////////////////////
	/** the Status enum is used for both keeping track of the entire row and individual cells. For single cells the meaning should be obvious. The entire row
	is set to Unused, if _no_ cell in the row is used, Valid if _all_ cells in the row are valid and Invalid if _one or more_ cells in the row are invalid, Unknown if _all_ cells in the row are unknown/updating. */
	enum Status { ValueUnused = 0,
		          ValueValid = 1,
		          ValueInvalid = 2,
		          ValueUnknown = 4 };

	/** sets whether changed data should be synced immediately or not. Set this to off for large paste operations. Remember to call setSyncing (true) and syncDataToR () after the paste is complete */
	void lockSyncing(bool lock);
	/** syncs pending data changes to the backend */
	void syncDataToR();
	/** reimplemented from RObject */
	void updateDataFromR(RCommandChain *chain) override;

	bool hasInvalidFields() const;

	/** get the value at the given row in text-form - regardless of the storage mode.
	@param pretty: get the text in pretty form, e.g. rounding numbers to a certain number of digits, replacing numeric values with value labels if available, etc. Formatting is done according to the meta-information stored in the RObject and global user preferences */
	QString getText(int row, bool pretty = false) const;
	/** get the value at the given row in text-form suitable for submission to R. I.e. strings are quoted, numbers are not, empty values are returned as NA */
	QString getRText(int row) const;
	/** set the value at the given row in text-form. Will try to convert the given string to the internal storage format if possible. */
	virtual void setText(int row, const QString &text);

	/** get a copy of the text values of rows from from_index to to_index. TODO: This could be made, but currently is not, more efficient than calling getText in a loop. */
	QString *getCharacter(int from_row, int to_row) const;

	/** returns the current status of the given cell */
	Status cellStatus(int row) const;

	/** entirely remove the given rows (i.e. the cells). Will also take care of updating the state (are there any invalid cells left?). Does not sync with the backend for technical reasons! You have to remove the row in the backend explicitly. */
	virtual void removeRows(int from_row, int to_row);
	/** inserts count rows (with empty values) just above the given index. Does not sync with the backend for technical reasons! You have to insert the row in the backend explicitly. */
	virtual void insertRows(int row, int count);
	/** Tells the object it has (data) length len. Usually this will only be called directly after creating a new object */
	void setLength(int len);

	/** returns (a copy of) the map of value labels for this variable or and empty map, if no labels/levels are assigned. */
	ValueLabels getValueLabels() const;
	/** assigns a new map of labels. Also takes care of syncing with the backend. */
	void setValueLabels(const ValueLabels &labels);
	/** re-check a factor variable after editing its value labels, and sync labels to R */
	void updateValueLabels();
	/** get value labels as string (for display) */
	QString getValueLabelString() const;
	/** set value labels from string (for paste operations) */
	void setValueLabelString(const QString &string);

	/** Restores the variable including data and meta-data */
	void restore(RCommandChain *chain = nullptr);

	/** Stores formatting options set for this variable */
	struct FormattingOptions {
		enum Alignment { AlignDefault = 0,
			             AlignLeft = 1,
			             AlignRight = 2 };
		enum Precision { PrecisionDefault = 0,
			             PrecisionRequired = 1,
			             PrecisionFixed = 2 };

		Alignment alignment;
		Precision precision_mode;
		int precision;
	};

	/** assigns new formatting options. Ownership of the FormattingOptions -struct is transferred to the variable. Use setFormatting (0) to remove all options */
	void setFormattingOptions(const FormattingOptions new_options);
	/** get the formatting options for this variable */
	FormattingOptions getFormattingOptions() const;
	/** get formatting options as a string (for display) TODO: redundant -> remove */
	QString getFormattingOptionsString() const;
	/** parse formatting options from the given string TODO: redundant -> remove */
	void setFormattingOptionsString(const QString &string);

	/** This enum describes the alignment of text inside a table cell */
	enum CellAlign { AlignCellLeft = 0,
		             AlignCellRight = 1 };
	/** returns alignment to use for this variable */
	CellAlign getAlignment() const;

	/** creates/parses formatting options from the stored meta-property string. See also: getFormattingOptions () */
	static FormattingOptions parseFormattingOptionsString(const QString &string);
	/** inverse of parseFormattingOptionsString () */
	static QString formattingOptionsToString(const FormattingOptions &options);
	/** changes the allocated storage to contain a least length elements. More data may be allocated than actually needed. This function only ever does upsizing. */
	void extendToLength(int length);

  protected:
	/** Discards pending unsynced changes. */
	void discardUnsyncedChanges();
	/** like setNumeric, but sets chars. If internalStorage () is numeric, attempts to convert the given strings to numbers. I.e. the function behaves essentially like setText (), but operates on a range of cells. Code may assume that all data comes directly from R, is entirely valid in R. */
	virtual void setCharacterFromR(int from_row, int to_row, const QStringList &data);
	/** set numeric values in the given range. Assumes you provide enough values for the range. If internalStorage is String, all values will be converted to strings, so you should use this function only, if you know you are dealing with a numeric object. Code may assume that all data comes directly from R, is entirely valid in R. */
	void setNumericFromR(int from_row, int to_row, const QVector<double> &data);
	/** reimplemented from RObject to change the internal data storage mode, if the var is being edited */
	bool updateType(RData *new_data) override;
	/** Extended from RObject::EditData to actually contain data. */
	struct RKVarEditData {
		QStringList cell_strings;
		QList<double> cell_doubles;
		enum CellState {
			Unknown = 0,
			Invalid = 1,
			NA = 2,
			Valid = 4,
			UnsyncedInvalidState = 8
		};
		QList<int> cell_states;

		/// see setSyncing
		int sync_locks;
		/// stores changes if syncing is not immediate
		ChangeSet changes;
		/// stores whether there were preivously invalid cells. If so, and there are no longer, now, we may change the mode in the backend.
		bool previously_valid;
		/** the value-labels or factor levels assigned to this variable. 0 if no values/levels given. TODO: Should this be made a regular (non-pointer) member, or is the saved mem really worth the trouble? */
		ValueLabels *value_labels;
		/// the formatting options set for this var (see FormattingOptions) */
		FormattingOptions formatting_options;
		/// storage for invalid fields
		QHash<int, QString> invalid_fields;
		/// how many models need our data?
		int num_listeners;
	};
	RKVarEditData *data;

	/** reimplemented from RObject */
	void beginEdit() override;
	/** reimplemented from RObject */
	void endEdit() override;

	/** takes care of syncing the given range of cells */
	void cellsChanged(int from_row, int to_row);
	/** writes the given range of cells to the backend (regardless of whether syncing should be immediate) */
	virtual void writeData(int from_row, int to_row, RCommandChain *chain = nullptr);
	void writeInvalidFields(QList<int> rows, RCommandChain *chain = nullptr);
	/** writes the values labels to the backend */
	void writeValueLabels(RCommandChain *chain) const;

	/** allocate edit data (cells initialized to NAs) */
	void allocateEditData();
	/** discard edit data */
	void discardEditData();
	/////////////////// END: data-handling //////////////////////
};

#endif
