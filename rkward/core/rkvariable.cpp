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

#define GET_DATA_COMMAND 11

#define MAX_PRECISION DBL_DIG

#include "../debug.h"

QString *RKVariable::na_char = new QString ("");
QString *RKVariable::unknown_char = new QString ("?");

RKVariable::RKVariable (RContainerObject *parent, const QString &name) : RObject (parent, name) {
	RK_TRACE (OBJECTS);
	type = Variable;
	setDataType (RObject::DataNumeric);
}

RKVariable::~RKVariable () {
	RK_TRACE (OBJECTS);
}

void RKVariable::setVarType (RObject::RDataType new_type, bool sync) {
	RK_TRACE (OBJECTS);

	if (getDataType () == new_type) {
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

		setDataType (new_type);
		if (new_type == RObject::DataCharacter) {
			if (myData ()->cell_strings == 0) {
				delete [] (myData ()->cell_doubles);
				myData ()->cell_doubles = 0;
				myData ()->cell_strings = new QString[getLength ()];
			}
		} else {
			if (myData ()->cell_doubles == 0) {
				delete [] (myData ()->cell_strings);
				myData ()->cell_strings = 0;
				myData ()->cell_doubles = new double[getLength ()];
			}
		}

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
		setDataType (new_type);
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
	} else if (command->getFlags () == GET_DATA_COMMAND) {
		RK_ASSERT (myData ());
		// prevent resyncing of data
		setSyncing (false);

		RK_ASSERT (command->getDataType () == RData::StructureVector);
		RK_ASSERT (command->getDataLength () == 3);

		RData *data = command->getStructureVector ()[0];
		RData *levels = command->getStructureVector ()[1];
		RData *invalids = command->getStructureVector ()[2];

		// set factor levels first
		RK_ASSERT (levels->getDataType () == RData::StringVector);
		unsigned int levels_len = levels->getDataLength ();
		RK_ASSERT (levels_len >= 1);
		delete myData ()->value_labels;
		myData ()->value_labels = new RObject::ValueLabels;
		if ((levels_len == 1) && levels->getStringVector ()[0].isEmpty ()) {
			// no levels
		} else {
			for (unsigned int i=0; i < levels_len; ++i) {
				myData ()->value_labels->insert (QString::number (i+1), levels->getStringVector ()[i]);
			}
		}

		// now set the data
		RK_ASSERT (data->getDataLength () == (unsigned int) getLength ()); // not a problem due to the line below, I'd still like to know if / when this happens.
		extendToLength (data->getDataLength ());
		if (data->getDataType () == RData::StringVector) {
			setCharacter (0, getLength () - 1, data->getStringVector ());
		} else if (data->getDataType () == RData::RealVector) {
			setNumeric (0, getLength () - 1, data->getRealVector ());
		} else if (data->getDataType () == RData::IntVector) {
			unsigned int len = getLength ();
			double *dd = new double[len];
			for (unsigned int i = 0; i < len; ++i) {
				if (data->getIntVector ()[i] == INT_MIN) dd[i] = NAN;
				else dd[i] = (double) data->getIntVector ()[i];
			}
			setNumeric (0, getLength () - 1, dd);
			delete [] dd;
		}

		// now set the invalid fields (only if they are still NAs in the R data)
		myData ()->invalid_fields.clear ();
		if (invalids->getDataLength () <= 1) {
			// no invalids
		} else {
			RK_ASSERT (invalids->getDataType () == RData::StringVector);
			unsigned int invalids_length = invalids->getDataLength ();
			RK_ASSERT ((invalids_length % 2) == 0);
			unsigned int invalids_count = invalids_length / 2;
			for (unsigned int i=0; i < invalids_count; ++i) {
				int row = invalids->getStringVector ()[i].toInt () - 1;
				if (myData ()->cell_states[row] & RKVarEditData::NA) {
					setText (row, invalids->getStringVector ()[invalids_count + i]);
				}
			}
		}
		myData ()->formatting_options = parseFormattingOptionsString (getMetaProperty ("format"));

		ChangeSet *set = new ChangeSet;
		set->from_index = 0;
		set->to_index = getLength ();
		RKGlobals::tracker ()->objectDataChanged (this, set);
		RKGlobals::tracker ()->objectMetaChanged (this);
		myData ()->dirty = false;
		setSyncing (true);
	} else {
		RK_ASSERT (false);
	}
}


////////////////////// BEGIN: data-handling //////////////////////////////
#define ALLOC_STEP 2
#define INITIAL_ALLOC 100

void RKVariable::setLength (int len) {
	RK_TRACE (OBJECTS);
	RK_ASSERT (!getLength ());	// should only be called once
	RK_ASSERT (dimensions);

	dimensions[0] = len;
}

// virtual
void RKVariable::allocateEditData () {
	RK_TRACE (OBJECTS);

	// this assert should stay even when more than one editor is allowed per object. After all, the edit-data should only ever be allocated once!
	RK_ASSERT (!myData ());
	
	data = new RKVarEditData;
	myData ()->cell_strings = 0;
	myData ()->cell_doubles = 0;
	myData ()->cell_states = 0;
	myData ()->allocated_length = 0;
	myData ()->immediate_sync = true;
	myData ()->changes = 0;
	myData ()->value_labels = 0;
	myData ()->formatting_options = 0;
	myData ()->previously_valid = true;
	myData ()->invalid_fields.setAutoDelete (true);
	myData ()->dirty = false;

	extendToLength (getLength ());

	for (int i = 0; i < getLength (); ++i) {
		myData ()->cell_states[i] = RKVarEditData::NA;
	}
}

bool RKVariable::updateType (RData *new_data) {
	RK_TRACE (OBJECTS);

	if (myData ()) {
		int old_type = type;
		bool ret = RObject::updateType (new_data);
		int new_type = type;
		type = old_type;		// needed to read out the old data
		setVarType (typeToDataType (new_type), false);
		type = new_type;
		return ret;
	}
	return RObject::updateType (new_data);
}

// virtual
void RKVariable::initializeEditDataToEmpty () {
	RK_TRACE (OBJECTS);
	RK_ASSERT (myData ());
	
	for (int row=0; row < getLength (); ++row) {
		myData ()->cell_states[row] = RKVarEditData::NA;
	}
}

void RKVariable::updateDataFromR (RCommandChain *chain) {
	RK_TRACE (OBJECTS);
	RK_ASSERT (myData ());

	RKGlobals::rInterface ()->issueCommand (".rk.get.vector.data (" + getFullName () + ')', RCommand::App | RCommand::Sync | RCommand::GetStructuredData, QString::null, this, GET_DATA_COMMAND, chain);
}

// virtual
void RKVariable::discardEditData () {
	RK_TRACE (OBJECTS);

	RK_ASSERT (myData ());

	if (getDataType () == RObject::DataCharacter) {
		delete [] myData ()->cell_strings;
		RK_ASSERT (myData ()->cell_doubles == 0);
	} else {
		delete [] myData ()->cell_doubles;
		RK_ASSERT (myData ()->cell_strings == 0);
	}
	delete [] myData ()->cell_states;

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
}

void RKVariable::writeInvalidField (int row, RCommandChain *chain) {
	RK_TRACE (OBJECTS);

	if (myData ()->invalid_fields[row]) {
		RKGlobals::rInterface ()->issueCommand (".rk.set.invalid.field (" + getFullName () + ", " + QString::number (row+1) + ", " + rQuote (*(myData ()->invalid_fields[row])) + ')', RCommand::App | RCommand::Sync, QString::null, 0,0, chain);
	} else {
		RKGlobals::rInterface ()->issueCommand (".rk.set.invalid.field (" + getFullName () + ", " + QString::number (row+1) + ", NULL)", RCommand::App | RCommand::Sync, QString::null, 0,0, chain);
	}
	myData ()->cell_states[row] -= (myData ()->cell_states[row] & RKVarEditData::UnsyncedInvalidState);
}

void RKVariable::writeData (int from_row, int to_row, RCommandChain *chain) {
	RK_TRACE (OBJECTS);
	if (from_row == -1) return;

	// TODO: try to sync in correct storage mode
	if (from_row == to_row) {
		RKGlobals::rInterface ()->issueCommand (getFullName () + '[' + QString::number (from_row+1) + "] <- " + getRText (from_row), RCommand::App | RCommand::Sync, QString::null, 0,0, chain);
		if (myData ()->cell_states[from_row] & RKVarEditData::UnsyncedInvalidState) writeInvalidField (from_row, chain);
	} else {
		QString data_string = "c (";
		for (int row = from_row; row <= to_row; ++row) {
			// TODO: use getCharacter and direct setting of vectors.
			data_string.append (getRText (row));
			if (row != to_row) {
				data_string.append (", ");
			}
			if (myData ()->cell_states[row] & RKVarEditData::UnsyncedInvalidState) writeInvalidField (row, chain);
		}
		data_string.append (")");
		RKGlobals::rInterface ()->issueCommand (getFullName () + '[' + QString::number (from_row + 1) + ':' + QString::number (to_row + 1) + "] <- " + data_string, RCommand::App | RCommand::Sync, QString::null, 0,0, chain);
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
}

void RKVariable::extendToLength (int length) {
	RK_TRACE (OBJECTS);

	if (length <= 0) length = 1;
	if (length < (myData ()->allocated_length - 1)) {
		dimensions[0] = length;
		return;
	}

	int ilength = length + 1;		// be a little generous
	int target = myData ()->allocated_length;
	if (!target) target = INITIAL_ALLOC;
	while (target <= ilength) target = target * ALLOC_STEP;
	RK_DO (qDebug ("resizing from %d to %d", myData ()->allocated_length, target), OBJECTS, DL_DEBUG);

	// allocate new memory and copy
	if (getDataType () == RObject::DataCharacter) {
		RK_ASSERT (myData ()->cell_doubles == 0);
		QString *new_data = new QString[target];
		if (myData ()->allocated_length) {		// if not yet allocated, don't mem-move
			qmemmove (new_data, myData ()->cell_strings, myData ()->allocated_length * sizeof (QString));
		}
		delete [] (myData ()->cell_strings);
		myData ()->cell_strings = new_data;
	} else {
		RK_ASSERT (myData ()->cell_strings == 0);
		double *new_data = new double[target];
		if (myData ()->allocated_length) {		// if not yet allocated, don't mem-move
			qmemmove (new_data, myData ()->cell_doubles, myData ()->allocated_length * sizeof (double));
		}
		delete [] (myData ()->cell_doubles);
		myData ()->cell_doubles = new_data;
	}
	int *new_states = new int[target];
	if (myData ()->allocated_length) {		// if not yet allocated, don't mem-move
		qmemmove (new_states, myData ()->cell_states, myData ()->allocated_length * sizeof (int));
	}
	delete [] (myData ()->cell_states);
	myData ()->cell_states = new_states;

	// set allocated but unused rows to Unknown
	for (int i=myData ()->allocated_length; i < target; ++i) {
		myData ()->cell_states[i] = RKVarEditData::Unknown;
	}

	myData ()->allocated_length = target;
	dimensions[0] = length;
}

void RKVariable::downSize () {
	RK_TRACE (OBJECTS);

	// TODO: downsizing to values other than 0
	if (getLength () <= 0) {
		delete [] myData ()->cell_doubles;
		myData ()->cell_doubles = 0;
		delete [] myData ()->cell_strings;
		myData ()->cell_strings = 0;
		delete [] myData ()->cell_states;
		myData ()->cell_states = 0;
	}
}

QString RKVariable::getText (int row, bool pretty) {
	if (row >= getLength ()) {
		RK_ASSERT (false);
		return (*unknown_char);
	}

	if (myData ()->cell_states[row] & RKVarEditData::Invalid) {
		RK_ASSERT (myData ()->invalid_fields[row] != 0);
		return (*(myData ()->invalid_fields[row]));
	}

	if (myData ()->cell_states[row] & RKVarEditData::NA) {
		return (*na_char);
	}

	if (pretty && (myData ()->value_labels)) {
		QString otext = getText (row);
		if (myData ()->value_labels->contains (otext)) {
			return (*(myData ()->value_labels))[otext];
		}
	}

	if (getDataType () == DataCharacter) {
		RK_ASSERT (myData ()->cell_strings != 0);
		return (myData ()->cell_strings[row]);
	} else {
		RK_ASSERT (myData ()->cell_doubles != 0);
		if (pretty && myData ()->formatting_options && (myData ()->formatting_options->precision_mode != FormattingOptions::PrecisionDefault)) {
			if (myData ()->formatting_options->precision_mode == FormattingOptions::PrecisionRequired) {
				return QString::number (myData ()->cell_doubles[row], 'g', MAX_PRECISION);
			}
			return QString::number (myData ()->cell_doubles[row], 'f', myData ()->formatting_options->precision);
		}
		return QString::number (myData ()->cell_doubles[row], 'g', MAX_PRECISION);
	}
}

QString RKVariable::getRText (int row) {
	RK_TRACE (OBJECTS);
	
	Status cell_state = cellStatus (row);
	
	if ((cell_state == ValueUnused) || (cell_state == ValueInvalid)) {
		return ("NA");
	} else if (getDataType () == DataFactor) {
		return (rQuote (getLabeled (row)));
	} else if (getDataType () == DataCharacter) {
		return (rQuote (getText (row)));
	} else {
		RK_ASSERT (myData ()->cell_doubles != 0);
		return (QString::number (myData ()->cell_doubles[row], 'g', MAX_PRECISION));
	}
}

void RKVariable::setText (int row, const QString &text) {
	RK_TRACE (OBJECTS);
	RK_ASSERT (row < getLength ());

	if (myData ()->cell_states[row] & RKVarEditData::Invalid) {
		myData ()->cell_states[row] = RKVarEditData::UnsyncedInvalidState;
		myData ()->invalid_fields.remove (row);
	} else {
		myData ()->cell_states[row] = 0;
	}

	if (text.isNull ()) {
		myData ()->cell_states[row] |= RKVarEditData::NA;
	} else {
		if (getDataType () == DataCharacter) {
			RK_ASSERT (myData ()->cell_strings != 0);
			myData ()->cell_strings[row] = text;
			myData ()->cell_states[row] |= RKVarEditData::Valid;
		} else if (getDataType () == DataFactor) {
			RK_ASSERT (myData ()->cell_doubles != 0);
			if (text.isEmpty ()) {
				myData ()->cell_states[row] |= RKVarEditData::NA;
			} else if (myData ()->value_labels && myData ()->value_labels->contains (text)) {
				myData ()->cell_doubles[row] = text.toInt ();
				myData ()->cell_states[row] |= RKVarEditData::Valid;
			} else {
				myData ()->invalid_fields.replace (row, new QString (text));
				myData ()->cell_states[row] |= RKVarEditData::Invalid | RKVarEditData::UnsyncedInvalidState;
			}
		} else {
			RK_ASSERT (myData ()->cell_doubles != 0);
			bool ok;
			if (text.isEmpty ()) {
				myData ()->cell_states[row] |= RKVarEditData::NA;
			} else {
				myData ()->cell_doubles[row] = text.toDouble (&ok);
				if (ok) {
					myData ()->cell_states[row] |= RKVarEditData::Valid;
				} else {
					myData ()->invalid_fields.replace (row, new QString (text));
					myData ()->cell_states[row] |= RKVarEditData::Invalid | RKVarEditData::UnsyncedInvalidState;
				}
			}
		}
	}
	cellChanged (row);
}

QString RKVariable::getLabeled (int row) {
	if (myData ()->value_labels) {
		QString otext = getText (row);
		if (myData ()->value_labels->contains (otext)) {
			return (*(myData ()->value_labels))[otext];
		}
	}
	return getText (row);
}

double *RKVariable::getNumeric (int from_row, int to_row) {
	RK_TRACE (OBJECTS);
	if (to_row >= getLength ()) {
		RK_ASSERT (false);
		return 0;
	}
	RK_ASSERT (from_row <= to_row);

	// TODO: no, this is not good. Return a _copy_!
	// we simply return the whole array starting at the given offset for now. Change this, if the storage mechanism gets changed!
	return &(myData ()->cell_doubles[from_row]);
}

void RKVariable::setNumeric (int from_row, int to_row, double *data) {
	RK_TRACE (OBJECTS);
	RK_ASSERT (to_row < getLength ());

	if (getDataType () == DataCharacter) {
		RK_ASSERT (false);		// asserting false to catch cases of this use for now. it's not really a problem, though
		int i = 0;
		for (int row=from_row; row <= to_row; ++row) {
			setText (row, QString::number (data[i++], 'g', MAX_PRECISION));
		}
	} else if (getDataType () == DataFactor) {
		int i = 0;
		for (int row=from_row; row <= to_row; ++row) {
			if (myData ()->cell_states[row] & RKVarEditData::Invalid) myData ()->cell_states[row] = RKVarEditData::UnsyncedInvalidState;
			else myData ()->cell_states[row] = 0;

			if (isnan (data[i]) || (!myData ()->value_labels) || (!myData ()->value_labels->contains (QString::number (data[i])))) {
				myData ()->cell_states[row] |= RKVarEditData::NA;
			} else {
				myData ()->cell_states[row] |= RKVarEditData::Valid;
				myData ()->cell_doubles[row] = data[i];
			}
			++i;
		}
	} else {
		int i = 0;
		for (int row=from_row; row <= to_row; ++row) {
			if (myData ()->cell_states[row] & RKVarEditData::Invalid) myData ()->cell_states[row] = RKVarEditData::UnsyncedInvalidState;
			else myData ()->cell_states[row] = 0;

			if (isnan (data[i])) {
				myData ()->cell_states[row] |= RKVarEditData::NA;
			} else {
				myData ()->cell_states[row] |= RKVarEditData::Valid;
				myData ()->cell_doubles[row] = data[i];
			}
			++i;
		}
	}
	cellsChanged (from_row, to_row);
}

QString *RKVariable::getCharacter (int from_row, int to_row) {
	RK_TRACE (OBJECTS);
	if (to_row >= getLength ()) {
		RK_ASSERT (false);
		return 0;
	}
	RK_ASSERT (from_row <= to_row);

	QString *ret = new QString[(to_row - from_row) + 1];
	
	int i = 0;
	for (int row = from_row; row <= to_row; ++row) {
		ret[i] = getText (row);
		i++;
	}

	return ret;
}

void RKVariable::setCharacter (int from_row, int to_row, QString *data) {
	RK_TRACE (OBJECTS);
	RK_ASSERT (to_row < getLength ());
	
	if (getDataType () == DataCharacter) {
		int i=0;
		for (int row=from_row; row <= to_row; ++row) {
			if (myData ()->cell_states[row] & RKVarEditData::Invalid) myData ()->cell_states[row] = RKVarEditData::UnsyncedInvalidState;
			else myData ()->cell_states[row] = 0;

			if (data[i].isNull ()) myData ()->cell_states[row] |= RKVarEditData::NA;
			else myData ()->cell_states[row] |= RKVarEditData::Valid;

			myData ()->cell_strings[row] = data[i++];
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
	RK_TRACE (OBJECTS);
	RK_ASSERT (to_row < getLength ());

	if ((from_row < 0)) from_row = 0;
	if ((to_row < 0)) to_row = myData ()->allocated_length - 1;
		
	for (int row=from_row; row <= to_row; ++row) {
		myData ()->cell_strings[row] = RKVarEditData::Unknown;
	}
}

RKVariable::Status RKVariable::cellStatus (int row) {
	if (myData ()->cell_states[row] == RKVarEditData::Unknown) return ValueUnknown;
	if (myData ()->cell_states[row] & RKVarEditData::NA) return ValueUnused;
	if (myData ()->cell_states[row] & RKVarEditData::Invalid) return ValueInvalid;
	return ValueValid;
}

void RKVariable::removeRow (int row) {
	RK_TRACE (OBJECTS);
	removeRows (row, row);
}

void RKVariable::removeRows (int from_row, int to_row) {
	RK_TRACE (OBJECTS);
	for (int row = from_row; row <= to_row; ++row) {
		myData ()->invalid_fields.remove (row);
	}

	if (to_row < (myData ()->allocated_length - 1)) {	// not the last rows
		if (myData ()->cell_strings) {
			qmemmove (&(myData ()->cell_strings[from_row]), &(myData ()->cell_strings[to_row+1]), (myData ()->allocated_length - to_row - 1) * sizeof (QString));
		} else {
			qmemmove (&(myData ()->cell_doubles[from_row]), &(myData ()->cell_doubles[to_row+1]), (myData ()->allocated_length - to_row - 1) * sizeof (double));
		}
		qmemmove (&(myData ()->cell_states[from_row]), &(myData ()->cell_states[to_row+1]), (myData ()->allocated_length - to_row - 1) * sizeof (int));
	}

	for (int row = (myData ()->allocated_length - 1 - (to_row - from_row)); row < myData ()->allocated_length; ++row) {
		myData ()->cell_states[myData ()->allocated_length - 1] = RKVarEditData::Unknown;
	}

	dimensions[0] -= (to_row - from_row) + 1;	
	downSize ();
}

void RKVariable::insertRow (int row) {
	RK_TRACE (OBJECTS);
	insertRows (row, 1);
}

void RKVariable::insertRows (int row, int count) {
	RK_TRACE (OBJECTS);
	int old_len = getLength ();
	extendToLength (getLength () + count);

	for (int i=old_len; i <= row+count; ++i) {
		myData ()->cell_states[i] = RKVarEditData::NA;
	}

	if (row >= getLength () && (count == 1)) {		// important special case
		if (myData ()->cell_strings) myData ()->cell_strings[row+count] = myData ()->cell_strings[row];
		if (myData ()->cell_doubles) myData ()->cell_doubles[row+count] = myData ()->cell_doubles[row];
		myData ()->cell_states[row+count] = myData ()->cell_states[row];
	} else {
		if (myData ()->cell_strings) qmemmove (&(myData ()->cell_strings[row+count]), &(myData ()->cell_strings[row]), (myData ()->allocated_length - (row + count) - 1) * sizeof (QString));
		if (myData ()->cell_doubles) qmemmove (&(myData ()->cell_doubles[row+count]), &(myData ()->cell_doubles[row]), (myData ()->allocated_length - (row + count) - 1) * sizeof (double));
		qmemmove (&(myData ()->cell_states[row+count]), &(myData ()->cell_states[row]), (myData ()->allocated_length - (row + count) - 1) * sizeof (int));
	}
	
	for (int i=row+count-1; i >= row; --i) {
		myData ()->cell_states[i] = RKVarEditData::NA;
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
		return QString ();
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
		if ((getDataType () == DataCharacter) || (getDataType () == DataFactor)) {
			return AlignCellLeft;
		} else {
			return AlignCellRight;
		}
	}
}

/////////////////// END: data-handling ///////////////////////////
