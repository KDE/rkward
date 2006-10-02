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
#include "rkvariable.h"

#include <qstringlist.h>
#include "float.h"
#include "math.h"

#include "rcontainerobject.h"
#include "robjectlist.h"

#include "../rbackend/rinterface.h"
#include "../rkglobals.h"
#include "rkmodificationtracker.h"

#define GET_STORAGE_MODE_COMMAND 10
#define GET_DATA_COMMAND 11
#define GET_FACTOR_LEVELS_COMMAND 12

#define MAX_PRECISION DBL_DIG

#include "../debug.h"

QString *RKVariable::na_char = new QString ("");
QString *RKVariable::unknown_char = new QString ("?");

RKVariable::RKVariable (RContainerObject *parent, const QString &name) : RObject (parent, name) {
	RK_TRACE (OBJECTS);
	type = Variable;
	var_type = Unknown;
}

RKVariable::~RKVariable () {
	RK_TRACE (OBJECTS);
}

QString RKVariable::getVarTypeString () {
	RK_TRACE (OBJECTS);
	return RObject::typeToText (var_type);
}

void RKVariable::setVarType (RObject::VarType new_type, bool sync) {
	RK_TRACE (OBJECTS);

	if (var_type == new_type) {
		// of course this is not harmful in any way, but in order to catch this kind of use, we raise an assert here for now.
		RK_ASSERT (false);
		return;
	}

	// if the variable is currently opened for editing, all values need to be rechecked / resynced
	if (myData ()) {
		bool internal_sync = myData ()->immediate_sync;
		setSyncing (false);
		// quick and dirty approach! TODO: make more efficient
		QStringList list;
		for (int i=0; i < getLength (); ++i) {
			list.append (getText (i));
		}
		var_type = new_type;
		int i = 0;
		for (QStringList::const_iterator it = list.constBegin (); it != list.constEnd (); ++it) {
			setText (i, *it);
			i++;
		}
		if (sync) {
			syncDataToR ();
		}
		setSyncing (internal_sync);
	} else {
		var_type = new_type;
	}

	setMetaProperty ("type", QString ().setNum ((int) new_type), sync);
}

void RKVariable::writeMetaData (RCommandChain *chain) {
	RK_TRACE (OBJECTS);

	writeValueLabels (chain);
	RObject::writeMetaData (chain);
}

void RKVariable::rCommandDone (RCommand *command) {
	RK_TRACE (OBJECTS);
	
	if (command->getFlags () == ROBJECT_UDPATE_STRUCTURE_COMMAND) {
		RObject::rCommandDone (command);
	} else if (command->getFlags () == GET_STORAGE_MODE_COMMAND) {
		RK_ASSERT (command->getDataType () == RData::IntVector);
		RK_ASSERT (command->getDataLength () == 2);
		if (!(command->getIntVector ()[1])) {
			RKGlobals::rInterface ()->issueCommand ("levels (" + getFullName () + ")", RCommand::App | RCommand::Sync | RCommand::GetStringVector, QString::null, this, GET_FACTOR_LEVELS_COMMAND);
			if (getVarType () == Unknown) {
				var_type = Factor;
			}
		}
		int command_type = RCommand::App | RCommand::Sync;
		if (command->getIntVector ()[0]) {
			command_type |= RCommand::GetRealVector;
			if (getVarType () == Unknown) {
				var_type = Number;
			}
		} else {
			command_type |= RCommand::GetStringVector;
			if (getVarType () == Unknown) {
				var_type = String;
			}
		}
		RKGlobals::rInterface ()->issueCommand (getFullName (), command_type, QString::null, this, GET_DATA_COMMAND);
	} else if (command->getFlags () == GET_DATA_COMMAND) {
		RK_ASSERT (myData ());
		// prevent resyncing of data
		setSyncing (false);
		int len = command->getDataLength ();
		RK_ASSERT (len == getLength ());
		if (command->getDataType () == RData::RealVector) {
			setNumeric (0, len - 1, command->getRealVector ());
		} else if (command->getDataType () == RData::StringVector) {
			setCharacter (0, len - 1, command->getStringVector ());
		} else {
			RK_ASSERT (false);
		}
		ChangeSet *set = new ChangeSet;
		set->from_index = 0;
		set->to_index = getLength ();
		RKGlobals::tracker ()->objectDataChanged (this, set);
		RKGlobals::tracker ()->objectMetaChanged (this);
		setSyncing (true);
	} else if (command->getFlags () == GET_FACTOR_LEVELS_COMMAND) {
		RK_ASSERT (myData ());
		// prevent resyncing of data
		setSyncing (false);
		RK_ASSERT (command->getDataLength ());
		RK_ASSERT (!myData ()->value_labels);
		myData ()->value_labels = new RObject::ValueLabels;
		for (unsigned int i=0; i < command->getDataLength (); ++i) {
			myData ()->value_labels->insert (QString::number (i+1), command->getStringVector ()[i]);
		}
		setSyncing (true);
	}
}


////////////////////// BEGIN: data-handling //////////////////////////////
#define ALLOC_STEP 2
#define INITIAL_ALLOC 100

#define RECHECK_VALID { if ((!myData ()->invalid_count) && (!myData ()->previously_valid)) restoreStorageInBackend (); }

void RKVariable::setLength (int len) {
	RK_TRACE (OBJECTS);
	RK_ASSERT (!getLength ());	// should only be called once
	RK_ASSERT (dimensions);

	dimensions[0] = len;
}

void RKVariable::restoreStorageInBackend () {
	RK_TRACE (OBJECTS);
	RK_ASSERT (myData ());
	
	if (getVarType () == Number) {
		RKGlobals::rInterface ()->issueCommand ("mode (" + getFullName () + ") <- \"numeric\"", RCommand::App | RCommand::Sync);
	} else if (getVarType () == Factor) {
		RKGlobals::rInterface ()->issueCommand ("rk.restore.factor (" + getFullName () + ")", RCommand::App | RCommand::Sync);
	}
}

void RKVariable::deleteStringData (int row) {
	RK_ASSERT (myData ());

	if (myData ()->cell_string_data[row] && (myData ()->cell_string_data[row] != na_char) && (myData ()->cell_string_data[row] != unknown_char)) {
		delete myData ()->cell_string_data[row];
		if (getVarType () != String) {
			myData ()->invalid_count--;
		}
	}
	myData ()->cell_string_data[row] = 0;
}

// virtual
void RKVariable::allocateEditData () {
	RK_TRACE (OBJECTS);

	// this assert should stay even when more than one editor is allowed per object. After all, the edit-data should only ever be allocated once!
	RK_ASSERT (!myData ());
	
	data = new RKVarEditData;
	myData ()->cell_double_data = 0;
	myData ()->cell_string_data = 0;
	myData ()->allocated_length = 0;
	myData ()->immediate_sync = true;
	myData ()->changes = 0;
	myData ()->invalid_count = 0;
	myData ()->value_labels = 0;
	myData ()->formatting_options = 0;
	myData ()->previously_valid = true;
	
	extendToLength (getLength ());
}

// virtual
void RKVariable::initializeEditData (bool to_empty) {
	RK_TRACE (OBJECTS);
	RK_ASSERT (myData ());
	
	if (to_empty) {
		for (int row=0; row < getLength (); ++row) {
			myData ()->cell_string_data[row] = na_char;
			myData ()->cell_double_data[row] = RKGlobals::na_double;
		}
	} else {
		RKGlobals::rInterface ()->issueCommand ("c (is.numeric (" + getFullName () + "), is.null (levels (" + getFullName () + ")))", RCommand::App | RCommand::Sync | RCommand::GetIntVector, QString::null, this, GET_STORAGE_MODE_COMMAND);
		myData ()->formatting_options = parseFormattingOptionsString (getMetaProperty ("format"));
	}
}

// virtual
void RKVariable::discardEditData () {
	RK_TRACE (OBJECTS);

	RK_ASSERT (myData ());

	delete [] myData ()->cell_double_data;
	for (int i = 0; i < myData ()->allocated_length; ++i) {
		deleteStringData (i);
	}
	delete [] myData ()->cell_string_data;

	RK_ASSERT (!(myData ()->changes));
	delete myData ()->value_labels;
	delete myData ()->formatting_options;
	delete myData ();
	data = 0;
}

void RKVariable::setSyncing (bool immediate) {
	RK_TRACE (OBJECTS);
	RK_ASSERT (myData ());
	
	myData ()->immediate_sync = immediate;
	if (!immediate) {
		if (!myData ()->changes) {
			myData ()->changes = new ChangeSet;
			myData ()->changes->from_index = -1;
			myData ()->changes->to_index = -1;
		}
	} else {
		delete myData ()->changes;
		myData ()->changes = 0;
	}
}

void RKVariable::syncDataToR () {
	RK_TRACE (OBJECTS);
	if (!(myData ()->changes)) return;
	
	// TODO
	writeData (myData ()->changes->from_index, myData ()->changes->to_index);
	myData ()->changes->from_index = -1;
	myData ()->changes->to_index = -1;
}

void RKVariable::restore (RCommandChain *chain) {
	RK_TRACE (OBJECTS);
	RK_ASSERT (myData ());

	writeData (0, getLength () - 1, chain);
	delete myData ()->changes;
	writeMetaData (chain);
	if (getVarType () == Factor) {
		RKGlobals::rInterface ()->issueCommand ("rk.restore.factor (" + getFullName () + ")", RCommand::App | RCommand::Sync, QString::null, 0, 0, chain);
	}
}

void RKVariable::writeData (int from_row, int to_row, RCommandChain *chain) {
	RK_TRACE (OBJECTS);
	if (from_row == -1) return;

	// TODO: try to sync in correct storage mode
	if (from_row == to_row) {
		RKGlobals::rInterface ()->issueCommand (getFullName () + "[" + QString::number (from_row+1) + "] <- " + getRText (from_row), RCommand::App | RCommand::Sync, QString::null, 0,0, chain);
	} else {
		QString data_string = "c (";
		for (int row = from_row; row <= to_row; ++row) {
			// TODO: use getCharacter and direct setting of vectors.
			data_string.append (getRText (row));
			if (row != to_row) {
				data_string.append (", ");
			}
		}
		data_string.append (")");
		RKGlobals::rInterface ()->issueCommand (getFullName () + "[" + QString::number (from_row + 1) + ":" + QString::number (to_row + 1) + "] <- " + data_string, RCommand::App | RCommand::Sync, QString::null, 0,0, chain);
	}

	ChangeSet *set = new ChangeSet;
	set->from_index = from_row;
	set->to_index = to_row;
	RKGlobals::tracker ()->objectDataChanged (this, set);
}

void RKVariable::cellChanged (int row) {
	RK_TRACE (OBJECTS);
	if (myData ()->immediate_sync) {
		writeData (row, row);
	} else {
		RK_ASSERT (myData ()->changes);
		if ((myData ()->changes->from_index > row) || (myData ()->changes->from_index == -1)) myData ()->changes->from_index = row;
		if (myData ()->changes->to_index < row) myData ()->changes->to_index = row;
	}
	RECHECK_VALID
}

void RKVariable::cellsChanged (int from_row, int to_row) {
	RK_TRACE (OBJECTS);
	if (myData ()->immediate_sync) {
		writeData (from_row, to_row);
	} else {
		RK_ASSERT (myData ()->changes);
		if ((myData ()->changes->from_index > from_row) || (myData ()->changes->from_index == -1)) myData ()->changes->from_index = from_row;
		if (myData ()->changes->to_index < to_row) myData ()->changes->to_index = to_row;
	}
	RECHECK_VALID
}

void RKVariable::extendToLength (int length) {
	RK_TRACE (OBJECTS);

	if (length <= 0) length = 1;
	if (length < (myData ()->allocated_length - 1)) {
		dimensions[0] = length;
		return;
	}

	int target = myData ()->allocated_length;
	if (!target) target = INITIAL_ALLOC;
	while (target < length) target = target * ALLOC_STEP;
	RK_DO (qDebug ("resizing from %d to %d", myData ()->allocated_length, target), OBJECTS, DL_DEBUG);

	QString **new_string_data = new QString*[target];
	double *new_double_data = new double[target];
	
	if (myData ()->cell_string_data) {		// not starting from 0
		qmemmove (new_string_data, myData ()->cell_string_data, myData ()->allocated_length * sizeof (QString*));
		qmemmove (new_double_data, myData ()->cell_double_data, myData ()->allocated_length * sizeof (double));
	
		delete [] (myData ()->cell_string_data);
		delete [] (myData ()->cell_double_data);
	}
	for (int i=myData ()->allocated_length; i < target; ++i) {
		new_string_data[i] = unknown_char;
	}

	myData ()->cell_string_data = new_string_data;
	myData ()->cell_double_data = new_double_data;

	myData ()->allocated_length = target;
	dimensions[0] = length;
}

void RKVariable::downSize () {
	RK_TRACE (OBJECTS);

	// TODO: downsizing to values other than 0
	if (getLength () <= 0) {
		delete [] myData ()->cell_double_data;
		myData ()->cell_double_data = 0;
		for (int i = 0; i < myData ()->allocated_length; ++i) {
#warning inefficient
			deleteStringData (i);
		}
		delete [] myData ()->cell_string_data;
		myData ()->cell_string_data = 0;
	}
}

QString RKVariable::getText (int row) {
	if (row >= getLength ()) {
		RK_ASSERT (false);
		return (*unknown_char);
	}
	if (getVarType () == String) {
		return (*(myData ()->cell_string_data[row]));
	} else {
		if (myData ()->cell_string_data[row] != 0) {
			return (*(myData ()->cell_string_data[row]));
		} else {
			return QString::number (myData ()->cell_double_data[row], 'g', MAX_PRECISION);
		}
	}
}

QString RKVariable::getRText (int row) {
	RK_TRACE (OBJECTS);
	
	Status cell_state = cellStatus (row);
	
	if (cell_state == ValueUnused) {
		return ("NA");
	} else if (getVarType () == Factor) {
		return (rQuote (getLabeled (row)));
	} else if ((getVarType () == String) || (cell_state == ValueInvalid)) {
		return (rQuote (getText (row)));
	} else {
		return (QString::number (myData ()->cell_double_data[row], 'g', MAX_PRECISION));
	}
}

void RKVariable::setText (int row, const QString &text) {
	RK_TRACE (OBJECTS);
	RK_ASSERT (row < getLength ());
	// delete previous string data, unless it's a special value
	deleteStringData (row);

	if (getVarType () == String) {
		if (text.isNull ()) {
			myData ()->cell_string_data[row] = na_char;
		} else {
			myData ()->cell_string_data[row] = new QString (text);
		}
	} else if (getVarType () == Factor) {
		if (text.isNull ()) {
			myData ()->cell_string_data[row] = na_char;
		} else if (myData ()->value_labels && myData ()->value_labels->contains (text)) {
			myData ()->cell_double_data[row] = text.toInt ();
		} else {
			myData ()->invalid_count++;
			myData ()->previously_valid = false;
			myData ()->cell_string_data[row] = new QString (text);
		}
	} else {
		bool ok;
		myData ()->cell_double_data[row] = text.toDouble (&ok);
		if (!ok) {
			if (text.isNull ()) {
				myData ()->cell_string_data[row] = na_char;
			} else {
				myData ()->invalid_count++;
				myData ()->previously_valid = false;
				myData ()->cell_string_data[row] = new QString (text);
			}
		}
	}
	cellChanged (row);
}

/** get the text in pretty form, e.g. rounding numbers to a certain number of digits, replacing numeric values with value labels if available, etc. Formatting is done according to the meta-information stored in the RObject and global user preferences */
QString RKVariable::getFormatted (int row) {
	if (row >= getLength ()) {
		RK_ASSERT (false);
		return (*unknown_char);
	}

	if (myData ()->value_labels) {
		if (myData ()->value_labels->contains (getText (row))) {
			return (*(myData ()->value_labels))[getText (row)];
		}
	}

	if (getVarType () == String) {
		return (*(myData ()->cell_string_data[row]));
	} else {
		if (myData ()->cell_string_data[row] != 0) {
			return (*(myData ()->cell_string_data[row]));
		} else {
			if (myData ()->formatting_options && (myData ()->formatting_options->precision_mode != FormattingOptions::PrecisionDefault)) {
				if (myData ()->formatting_options->precision_mode == FormattingOptions::PrecisionRequired) {
					return QString::number (myData ()->cell_double_data[row], 'g', MAX_PRECISION);
				} else {
					return QString::number (myData ()->cell_double_data[row], 'f', myData ()->formatting_options->precision);
				}
			} else {
				// TODO: use global (configurable) defaults!
				return QString::number (myData ()->cell_double_data[row], 'g', MAX_PRECISION);
			}
		}
	}

	return getText (row);
}

/** tries to match a value-label to the value in the given cell. Returns the label, or - if there is no label - the original value in textual representation */
QString RKVariable::getLabeled (int row) {
	if (myData ()->value_labels) {
		if (myData ()->value_labels->contains (getText (row))) {
			return (*(myData ()->value_labels))[getText (row)];
		}
	}
	return getText (row);
}

/** get a copy of the numeric values of rows starting from from_index, going to to_index. Do not use this before making sure that the rStorage () is really numeric! */
double *RKVariable::getNumeric (int from_row, int to_row) {
	if (to_row >= getLength ()) {
		RK_ASSERT (false);
		return 0;
	}
	RK_ASSERT (from_row <= to_row);

	// TODO: no, this is not good. Return a _copy_!
	// we simply return the whole array starting at the given offset for now. Change this, if the storage mechanism gets changed!
	return &(myData ()->cell_double_data[from_row]);
}

void RKVariable::setNumeric (int from_row, int to_row, double *data) {
	RK_ASSERT (to_row < getLength ());
	
	for (int row=from_row; row <= to_row; ++row) {
#warning inefficient
		deleteStringData (row);
	}
	
	if (getVarType () == String) {
		RK_ASSERT (false);		// asserting false to catch cases of this use for now. it's not really a problem, though
		int i = 0;
		for (int row=from_row; row <= to_row; ++row) {
			myData ()->cell_string_data[row] = new QString (QString::number (data[i++], 'g', MAX_PRECISION));
		}
	} else if (getVarType () == Factor) {
		int i = 0;
		for (int row=from_row; row <= to_row; ++row) {
			setText (row, QString::number (data[i++], 'g', MAX_PRECISION));
		}
	} else {
		int i = 0;
		for (int row=from_row; row <= to_row; ++row) {
			if (isnan (data[i])) myData ()->cell_string_data[row] = na_char;
			myData ()->cell_double_data[row] = data[i++];
		}
	}
	cellsChanged (from_row, to_row);
}

QString *RKVariable::getCharacter (int from_row, int to_row) {
	if (to_row >= getLength ()) {
		RK_ASSERT (false);
		return 0;
	}
	RK_ASSERT (from_row <= to_row);

	QString *ret = new QString[(to_row - from_row) + 1];
	
	int i = 0;
	for (int row = from_row; row <= to_row; ++row) {
		if (myData ()->cell_string_data[row]) {
			ret[i] = *(myData ()->cell_string_data[row]);
		} else {
			ret[i] = getText (row).local8Bit ();
		}
		i++;
	}

	return ret;
}

void RKVariable::setCharacter (int from_row, int to_row, QString *data) {
	RK_ASSERT (to_row < getLength ());
	
	if (getVarType () == String) {
		int i=0;
		for (int row=from_row; row <= to_row; ++row) {
#warning inefficient
			deleteStringData (row);
			myData ()->cell_string_data[row] = new QString (data[i++]);
		}
	} else {
		int i=0;
		for (int row=from_row; row <= to_row; ++row) {
			setText (row, data[i++]);
		}
		return;
	}
	cellsChanged (from_row, to_row);
}

void RKVariable::setUnknown (int from_row, int to_row) {
	RK_ASSERT (to_row < getLength ());

	if ((from_row < 0)) from_row = 0;
	if ((to_row < 0)) to_row = myData ()->allocated_length - 1;
		
	for (int row=from_row; row <= to_row; ++row) {
#warning inefficient
		deleteStringData (row);
		myData ()->cell_string_data[row] = unknown_char;
	}
}

RKVariable::Status RKVariable::cellStatus (int row) {
	if (myData ()->cell_string_data[row] == na_char) {
		return ValueUnused;
	} else if (myData ()->cell_string_data[row] == unknown_char) {
		return ValueUnknown;
	} else if ((getVarType () != String) && (myData ()->cell_string_data[row] != 0)) {
		return ValueInvalid;
	}
	return ValueValid;
}

/** entirely remove the given row (i.e. the cell). Will also take care of updating the state (are there any invalid cells left?), and syncing with the backend */
void RKVariable::removeRow (int row) {
	removeRows (row, row);
}

/** see removeRow (), but removes a range of rows (i.e. cells). Since data only needs to be copied once, this is more efficient than several single calls to removeRow () */
void RKVariable::removeRows (int from_row, int to_row) {
	for (int row = from_row; row <= to_row; ++row) {
#warning inefficient
		deleteStringData (row);
	}

	for (int row=from_row; row <= to_row; ++row) {
		if (cellStatus (row) == ValueInvalid) myData ()->invalid_count--;
	}
	
	if (to_row < (myData ()->allocated_length - 1)) {	// not the last rows
		qmemmove (&(myData ()->cell_string_data[from_row]), &(myData ()->cell_string_data[to_row+1]), (myData ()->allocated_length - to_row - 1) * sizeof (QString*));
		qmemmove (&(myData ()->cell_double_data[from_row]), &(myData ()->cell_double_data[to_row+1]), (myData ()->allocated_length - to_row - 1) * sizeof (double));
	}

	for (int row = (myData ()->allocated_length - 1 - (to_row - from_row)); row < myData ()->allocated_length; ++row) {
		myData ()->cell_string_data[myData ()->allocated_length - 1] = unknown_char;
		myData ()->cell_double_data[myData ()->allocated_length - 1] = 0;
	}

	dimensions[0] -= (to_row - from_row) + 1;	
	downSize ();
	RECHECK_VALID
}

void RKVariable::insertRow (int row) {
	insertRows (row, 1);
}

void RKVariable::insertRows (int row, int count) {
	int old_len = getLength ();
	extendToLength (getLength () + count);

	for (int i=old_len; i <= row+count; ++i) {
		myData ()->cell_string_data[i] = na_char;
		myData ()->cell_double_data[i] = 0;
	}

	if (row >= getLength () && (count == 1)) {		// important special case
		myData ()->cell_string_data[row+count] = myData ()->cell_string_data[row];
		myData ()->cell_double_data[row+count] = myData ()->cell_double_data[row];
	} else { 
		qmemmove (&(myData ()->cell_string_data[row+count]), &(myData ()->cell_string_data[row]), (myData ()->allocated_length - (row + count) - 1) * sizeof (QString*));
		qmemmove (&(myData ()->cell_double_data[row+count]), &(myData ()->cell_double_data[row]), (myData ()->allocated_length - (row + count) - 1) * sizeof (double));
	}
	
	for (int i=row+count-1; i >= row; --i) {
		myData ()->cell_string_data[i] = na_char;
		myData ()->cell_double_data[i] = 0;
	}
}

RObject::ValueLabels *RKVariable::getValueLabels () {
	RK_ASSERT (myData ());
	return (myData ()->value_labels);
}

void RKVariable::setValueLabels (ValueLabels *labels) {
	RK_TRACE (OBJECTS);
	RK_ASSERT (myData ());
	
	if (labels != myData ()->value_labels) {
		delete (myData ()->value_labels);
		myData ()->value_labels = labels;
	}

	writeValueLabels (0);
	RKGlobals::tracker ()->objectMetaChanged (this);

	// find out which values got valid / invalid and change those
	for (int i=0; i < getLength (); ++i) {
		if (cellStatus (i) == ValueInvalid) {
			if (labels && labels->contains (getText (i))) {
				setText (i, getText (i));
			}
		} else {
			if (!(labels && labels->contains (getText (i)))) {
				setText (i, getText (i));
			}
		}
	}

	// also update display of all values:
	ChangeSet *set = new ChangeSet;
	set->from_index = 0;
	set->to_index = getLength () - 1;	
	RKGlobals::tracker ()->objectDataChanged (this, set);

	// TODO: find out whether the object is valid after the operation and update accordingly!
}

void RKVariable::writeValueLabels (RCommandChain *chain) {
	RK_TRACE (OBJECTS);
	RK_ASSERT (myData ());
	
	if (myData ()->value_labels) {
		int i = 1;
		QString level_string = "c (";
		while (myData ()->value_labels->contains (QString::number (i))) {
			level_string.append (rQuote ((*(myData ()->value_labels))[QString::number (i)]));
			if (myData ()->value_labels->contains (QString::number (++i))) {
				level_string.append (", ");
			}
		}
		level_string.append (")");
		// using attr (..., "levels) instead of levels (...) in order to bypass checking
		RKGlobals::rInterface ()->issueCommand ("attr (" + getFullName () + ", \"levels\") <- " + level_string, RCommand::App | RCommand::Sync, QString::null, 0, 0, chain);
	} else {
		RKGlobals::rInterface ()->issueCommand ("attr (" + getFullName () + ", \"levels\") <- NULL", RCommand::App | RCommand::Sync, QString::null, 0, 0, chain);
	}
}

QString RKVariable::getValueLabelString () {
	RK_TRACE (OBJECTS);
	RK_ASSERT (myData ());

	if (myData ()->value_labels) {
		int i = 1;
		QString level_string;
		while (myData ()->value_labels->contains (QString::number (i))) {
			level_string.append ((*(myData ()->value_labels))[QString::number (i)]);
			if (myData ()->value_labels->contains (QString::number (++i))) {
				level_string.append ("#,#");
			}
		}
		
		return level_string;
	} else {
		return QString::null;
	}
}

void RKVariable::setValueLabelString (const QString &string) {
	RK_TRACE (OBJECTS);
	RK_ASSERT (myData ());

	QStringList list = QStringList::split ("#,#", string);
	
	if (list.empty ()) {
		setValueLabels (0);
		return;
	}
	
	int i = 1;
	ValueLabels *new_labels = new ValueLabels;
	for (QStringList::const_iterator it = list.constBegin (); it != list.constEnd (); ++it) {
		new_labels->insert (QString::number (i), *it);
		++i;
	}
	setValueLabels (new_labels);
}

RKVariable::FormattingOptions *RKVariable::getFormattingOptions () {
	RK_TRACE (OBJECTS);
	RK_ASSERT (myData ());

	return myData ()->formatting_options;
}

void RKVariable::setFormattingOptions (FormattingOptions *formatting_options) {
	RK_TRACE (OBJECTS);
	RK_ASSERT (myData ());
	
	if (formatting_options != myData ()->formatting_options) {
		delete myData ()->formatting_options;
	}

	myData ()->formatting_options = formatting_options;

	if (!formatting_options) {
		setMetaProperty ("format", QString::null);
	} else {
		QString format_string;
		if (formatting_options->alignment != (int) FormattingOptions::AlignDefault) {
			format_string.append ("align:");
			format_string.append (QString::number (formatting_options->alignment));
		}
	
		if (formatting_options->precision_mode != (int) FormattingOptions::PrecisionDefault) {
			if (!format_string.isEmpty ()) format_string.append ("#");
			format_string.append ("prec:");
			if (formatting_options->precision_mode == (int) FormattingOptions::PrecisionRequired) {
				format_string.append ("v");
			} else {
				format_string.append (QString::number (formatting_options->precision));
			}
		}
	
		setMetaProperty ("format", format_string);
	}

	// also update display of all values:
	ChangeSet *set = new ChangeSet;
	set->from_index = 0;
	set->to_index = getLength () - 1;	
	RKGlobals::tracker ()->objectDataChanged (this, set);
}

QString RKVariable::getFormattingOptionsString () {
	RK_TRACE (OBJECTS);
	RK_ASSERT (myData ());

	return getMetaProperty ("format");
}

void RKVariable::setFormattingOptionsString (const QString &string) {
	RK_TRACE (OBJECTS);
	RK_ASSERT (myData ());

	setFormattingOptions (parseFormattingOptionsString (string));
}

RKVariable::FormattingOptions *RKVariable::parseFormattingOptionsString (const QString &string) {
	RK_TRACE (OBJECTS);
	RK_ASSERT (myData ());

	FormattingOptions *formatting_options = new FormattingOptions;
	formatting_options->alignment = FormattingOptions::AlignDefault;
	formatting_options->precision_mode = FormattingOptions::PrecisionDefault;
	formatting_options->precision = 0;
	bool empty = true;

	QStringList list = QStringList::split ("#", string);
	QString option, parameter;
	for (QStringList::const_iterator it = list.constBegin (); it != list.constEnd (); ++it) {
		option = (*it).section (':', 0, 0);
		parameter = (*it).section (':', 1, 1);
		
		if (parameter.isEmpty ()) continue;
		
		if (option == "align") {
			int al = parameter.toInt ();
			if ((al >= (int) FormattingOptions::AlignDefault) && (al <= (int) FormattingOptions::AlignRight)) {
				empty = false;
				formatting_options->alignment = (FormattingOptions::Alignment) al;
			}
		} else if (option == "prec") {
			if (parameter == "d") {
				empty = false;
				formatting_options->precision_mode = FormattingOptions::PrecisionDefault;
			} else if (parameter == "v") {
				empty = false;
				formatting_options->precision_mode = FormattingOptions::PrecisionRequired;
			} else {
				int dig = parameter.toInt ();
				if ((dig >= 0) && (dig <= 15)) {
					empty = false;
					formatting_options->precision_mode = FormattingOptions::PrecisionFixed;
					formatting_options->precision = dig;
				}
			}
		} else {
			RK_ASSERT (false);
		}
	}
	
	if (empty) {
		delete formatting_options;
		return 0;
	} else {
		return formatting_options;
	}
}

/** returns alignment to use for this variable */
RKVariable::CellAlign RKVariable::getAlignment () {
	RK_ASSERT (myData ());
	
	if (myData ()->formatting_options && (myData ()->formatting_options->alignment != FormattingOptions::AlignDefault)) {
		if (myData ()->formatting_options->alignment == FormattingOptions::AlignLeft) return AlignCellLeft;
		return AlignCellRight;
	} else {
	// TODO: use global (configurable) defaults, if not specified
		if ((getVarType () == String) || (getVarType () == Factor)) {
			return AlignCellLeft;
		} else {
			return AlignCellRight;
		}
	}
}

/////////////////// END: data-handling ///////////////////////////
