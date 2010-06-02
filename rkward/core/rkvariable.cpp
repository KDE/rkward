/***************************************************************************
                          rkvariable  -  description
                             -------------------
    begin                : Thu Aug 12 2004
    copyright            : (C) 2004, 2007, 2008, 2010 by Thomas Friedrichsmeier
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
#include "limits.h"

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
	data = 0;
	setDataType (RObject::DataNumeric);
}

RKVariable::~RKVariable () {
	RK_TRACE (OBJECTS);

	RK_ASSERT (!data);	// endEdit() should have been called
}

void RKVariable::setVarType (RObject::RDataType new_type, bool sync) {
	RK_TRACE (OBJECTS);

	if (getDataType () == new_type) return;
	if ((new_type < RObject::MinKnownDataType) || (new_type > RObject::MaxKnownDataType)) {
		new_type = RObject::DataCharacter;
	}

	// if the variable is currently opened for editing, all values need to be rechecked / resynced
	if (data) {
		bool internal_sync = data->immediate_sync;
		// quick and dirty approach! TODO: make more efficient
		QStringList list;
		for (int i=0; i < getLength (); ++i) {
			list.append (getText (i));
		}

		if (data->changes) {	// all pending changes are moot
			delete data->changes;
			data->changes = 0;
		}

		// store what we want to keep of the edit data
		int num_listeners = data->num_listeners;
		ValueLabels *value_labels = data->value_labels;
		data->value_labels = 0;	// prevent destruction
		FormattingOptions formatting_options = data->formatting_options;

		// destroy and re-allocate edit data
		data->num_listeners = 0;	// to avoid the otherwise useful assert in discardEditData
		discardEditData ();
		setDataType (new_type);
		allocateEditData ();

		// re-set presistent aspects of the edit data
		data->value_labels = value_labels;
		data->formatting_options = formatting_options;
		data->num_listeners = num_listeners;

		// re-set all data
		setSyncing (false);
		int i = 0;
		for (QStringList::const_iterator it = list.constBegin (); it != list.constEnd (); ++it) {
			setText (i, *it);
			i++;
		}
		if (sync) {
			QString command = ".rk.set.vector.mode(" + getFullName () + ", ";
			if (new_type == RObject::DataCharacter) command += "as.character";
			else if (new_type == RObject::DataNumeric) command += "as.numeric";
			else if (new_type == RObject::DataLogical) command += "as.logical";
			else if (new_type == RObject::DataFactor) command += "as.factor";
			command += ")";
			RKGlobals::rInterface ()->issueCommand (command, RCommand::App | RCommand::Sync, QString::null);

			syncDataToR ();
		}
		setSyncing (internal_sync);
	} else {
		setDataType (new_type);
	}
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
		RK_ASSERT (data);
		// prevent resyncing of data
		setSyncing (false);

		RK_ASSERT (command->getDataType () == RData::StructureVector);
		RK_ASSERT (command->getDataLength () == 3);

		RData *cdata = command->getStructureVector ()[0];
		RData *levels = command->getStructureVector ()[1];
		RData *invalids = command->getStructureVector ()[2];

		// set factor levels first
		RK_ASSERT (levels->getDataType () == RData::StringVector);
		unsigned int levels_len = levels->getDataLength ();
		RK_ASSERT (levels_len >= 1);
		delete data->value_labels;
		data->value_labels = new RObject::ValueLabels;
		if ((levels_len == 1) && levels->getStringVector ()[0].isEmpty ()) {
			// no levels
		} else {
			for (unsigned int i=0; i < levels_len; ++i) {
				data->value_labels->insert (QString::number (i+1), levels->getStringVector ()[i]);
			}
		}

		// now set the data
		RK_ASSERT (cdata->getDataLength () == (unsigned int) getLength ()); // not a problem due to the line below, I'd still like to know if / when this happens.
		extendToLength (cdata->getDataLength ());
		if (cdata->getDataType () == RData::StringVector) {
			setCharacter (0, getLength () - 1, cdata->getStringVector ());
		} else if (cdata->getDataType () == RData::RealVector) {
			setNumeric (0, getLength () - 1, cdata->getRealVector ());
		} else if (cdata->getDataType () == RData::IntVector) {
			unsigned int len = getLength ();
			double *dd = new double[len];
			for (unsigned int i = 0; i < len; ++i) {
				if (cdata->getIntVector ()[i] == INT_MIN) dd[i] = NAN;
				else dd[i] = (double) cdata->getIntVector ()[i];
			}
			setNumeric (0, getLength () - 1, dd);
			delete [] dd;
		}

		// now set the invalid fields (only if they are still NAs in the R data)
		data->invalid_fields.clear ();
		if (invalids->getDataLength () <= 1) {
			// no invalids
		} else {
			RK_ASSERT (invalids->getDataType () == RData::StringVector);
			unsigned int invalids_length = invalids->getDataLength ();
			RK_ASSERT ((invalids_length % 2) == 0);
			unsigned int invalids_count = invalids_length / 2;
			for (unsigned int i=0; i < invalids_count; ++i) {
				int row = invalids->getStringVector ()[i].toInt () - 1;
				if (data->cell_states[row] & RKVarEditData::NA) {
					setText (row, invalids->getStringVector ()[invalids_count + i]);
				}
			}
		}
		data->formatting_options = parseFormattingOptionsString (getMetaProperty ("format"));

		ChangeSet *set = new ChangeSet;
		set->from_index = 0;
		set->to_index = getLength ();
		RKGlobals::tracker ()->objectDataChanged (this, set);
		RKGlobals::tracker ()->objectMetaChanged (this);
		type -= (type & NeedDataUpdate);
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

bool RKVariable::updateType (RData *new_data) {
	RK_TRACE (OBJECTS);

	if (data) {
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
void RKVariable::beginEdit () {
	RK_TRACE (OBJECTS);

	if (!data) {
		allocateEditData ();
		if (!isPending ()) updateDataFromR (0);
	}
	++(data->num_listeners);
}

// virtual
void RKVariable::endEdit () {
	RK_TRACE (OBJECTS);

	RK_ASSERT (data);
	RK_ASSERT (data->num_listeners > 0);
	--(data->num_listeners);
	if (!data->num_listeners) discardEditData ();
}

void RKVariable::allocateEditData () {
	RK_TRACE (OBJECTS);

	// edit data should only be allocated once, even if there are multiple editors
	RK_ASSERT (!data);
	
	data = new RKVarEditData;
	data->cell_strings = 0;
	data->cell_doubles = 0;
	data->cell_states = 0;
	data->allocated_length = 0;
	data->immediate_sync = true;
	data->changes = 0;
	data->value_labels = 0;
	data->formatting_options.alignment = FormattingOptions::AlignDefault;
	data->formatting_options.precision_mode = FormattingOptions::PrecisionDefault;
	data->formatting_options.precision = 0;
	data->previously_valid = true;
	data->num_listeners = 0;

	extendToLength (getLength ());

	for (int i = 0; i < getLength (); ++i) {
		data->cell_states[i] = RKVarEditData::NA;
	}
}

void RKVariable::discardEditData () {
	RK_TRACE (OBJECTS);

	RK_ASSERT (data);
	RK_ASSERT (!(data->num_listeners));

	if (getDataType () == RObject::DataCharacter) {
		delete [] data->cell_strings;
		RK_ASSERT (data->cell_doubles == 0);
	} else {
		delete [] data->cell_doubles;
		RK_ASSERT (data->cell_strings == 0);
	}
	delete [] data->cell_states;

	RK_ASSERT (!(data->changes));
	delete data->value_labels;
	delete data;
	data = 0;

	if (isPending ())  (type -= Pending);
}

void RKVariable::updateDataFromR (RCommandChain *chain) {
	RK_TRACE (OBJECTS);
	if (!data) return;

	RKGlobals::rInterface ()->issueCommand (".rk.get.vector.data (" + getFullName () + ')', RCommand::App | RCommand::Sync | RCommand::GetStructuredData, QString::null, this, GET_DATA_COMMAND, chain);
}

void RKVariable::setSyncing (bool immediate) {
	RK_TRACE (OBJECTS);
	RK_ASSERT (data);
	
	data->immediate_sync = immediate;
	if (!immediate) {
		if (!data->changes) {
			data->changes = new ChangeSet;
			data->changes->from_index = -1;
			data->changes->to_index = -1;
		}
	} else {
		delete data->changes;
		data->changes = 0;
	}
}

void RKVariable::syncDataToR () {
	RK_TRACE (OBJECTS);
	if (!(data->changes)) return;
	
	// TODO
	writeData (data->changes->from_index, data->changes->to_index);
	data->changes->from_index = -1;
	data->changes->to_index = -1;
}

void RKVariable::restore (RCommandChain *chain) {
	RK_TRACE (OBJECTS);
	RK_ASSERT (data);

	writeData (0, getLength () - 1, chain);
	delete data->changes;
	writeMetaData (chain);
}

void RKVariable::writeInvalidField (int row, RCommandChain *chain) {
	RK_TRACE (OBJECTS);

	if (data->invalid_fields.contains (row)) {
		RKGlobals::rInterface ()->issueCommand (".rk.set.invalid.field (" + getFullName () + ", " + QString::number (row+1) + ", " + rQuote (data->invalid_fields.value (row)) + ')', RCommand::App | RCommand::Sync, QString::null, 0,0, chain);
	} else {
		RKGlobals::rInterface ()->issueCommand (".rk.set.invalid.field (" + getFullName () + ", " + QString::number (row+1) + ", NULL)", RCommand::App | RCommand::Sync, QString (), 0,0, chain);
	}
	data->cell_states[row] -= (data->cell_states[row] & RKVarEditData::UnsyncedInvalidState);
}

void RKVariable::writeData (int from_row, int to_row, RCommandChain *chain) {
	RK_TRACE (OBJECTS);
	if (from_row == -1) return;

	// TODO: try to sync in correct storage mode
	if (from_row == to_row) {
		RKGlobals::rInterface ()->issueCommand (getFullName () + '[' + QString::number (from_row+1) + "] <- " + getRText (from_row), RCommand::App | RCommand::Sync, QString::null, 0,0, chain);
		if (data->cell_states[from_row] & RKVarEditData::UnsyncedInvalidState) writeInvalidField (from_row, chain);
	} else {
		QString data_string = "c (";
		for (int row = from_row; row <= to_row; ++row) {
			// TODO: use getCharacter and direct setting of vectors.
			data_string.append (getRText (row));
			if (row != to_row) {
				data_string.append (", ");
			}
			if (data->cell_states[row] & RKVarEditData::UnsyncedInvalidState) writeInvalidField (row, chain);
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
	if (data->immediate_sync) {
		writeData (row, row);
	} else {
		RK_ASSERT (data->changes);
		if ((data->changes->from_index > row) || (data->changes->from_index == -1)) data->changes->from_index = row;
		if (data->changes->to_index < row) data->changes->to_index = row;
	}
}

void RKVariable::cellsChanged (int from_row, int to_row) {
	RK_TRACE (OBJECTS);
	if (data->immediate_sync) {
		writeData (from_row, to_row);
	} else {
		RK_ASSERT (data->changes);
		if ((data->changes->from_index > from_row) || (data->changes->from_index == -1)) data->changes->from_index = from_row;
		if (data->changes->to_index < to_row) data->changes->to_index = to_row;
	}
}

void RKVariable::extendToLength (int length) {
	RK_TRACE (OBJECTS);

	if (length <= 0) length = 1;
	if (length < (data->allocated_length - 1)) {
		dimensions[0] = length;
		return;
	}

	int ilength = length + 1;		// be a little generous
	int target = data->allocated_length;
	if (!target) target = INITIAL_ALLOC;
	while (target <= ilength) target = target * ALLOC_STEP;
	RK_DO (qDebug ("resizing from %d to %d", data->allocated_length, target), OBJECTS, DL_DEBUG);

	// allocate new memory and copy
	if (getDataType () == RObject::DataCharacter) {
		RK_ASSERT (data->cell_doubles == 0);
		QString *new_data = new QString[target];
		if (data->allocated_length) {		// if not yet allocated, don't mem-move
			memMoveQStrings (new_data, data->cell_strings, data->allocated_length);
		}
		delete [] (data->cell_strings);
		data->cell_strings = new_data;
	} else {
		RK_ASSERT (data->cell_strings == 0);
		double *new_data = new double[target];
		if (data->allocated_length) {		// if not yet allocated, don't mem-move
			memmove (new_data, data->cell_doubles, data->allocated_length * sizeof (double));
		}
		delete [] (data->cell_doubles);
		data->cell_doubles = new_data;
	}
	int *new_states = new int[target];
	if (data->allocated_length) {		// if not yet allocated, don't mem-move
		memmove (new_states, data->cell_states, data->allocated_length * sizeof (int));
	}
	delete [] (data->cell_states);
	data->cell_states = new_states;

	// set allocated but unused rows to Unknown
	for (int i=data->allocated_length; i < target; ++i) {
		data->cell_states[i] = RKVarEditData::Unknown;
	}

	data->allocated_length = target;
	dimensions[0] = length;
}

void RKVariable::downSize () {
	RK_TRACE (OBJECTS);

	// TODO: downsizing to values other than 0
	if (getLength () <= 0) {
		delete [] data->cell_doubles;
		data->cell_doubles = 0;
		delete [] data->cell_strings;
		data->cell_strings = 0;
		delete [] data->cell_states;
		data->cell_states = 0;
	}
}

QString RKVariable::getText (int row, bool pretty) const {
	if (row >= getLength ()) {
		RK_ASSERT (false);
		return (*unknown_char);
	}

	if (data->cell_states[row] & RKVarEditData::Invalid) {
		RK_ASSERT (data->invalid_fields.contains (row));
		return (data->invalid_fields.value (row));
	}

	if (data->cell_states[row] & RKVarEditData::NA) {
		return (*na_char);
	}

	if (pretty && (data->value_labels)) {
		QString otext = getText (row);
		if (data->value_labels->contains (otext)) {
			return (*(data->value_labels))[otext];
		}
	}

	if (getDataType () == DataCharacter) {
		RK_ASSERT (data->cell_strings != 0);
		return (data->cell_strings[row]);
	} else {
		RK_ASSERT (data->cell_doubles != 0);
		if (pretty && (data->formatting_options.precision_mode != FormattingOptions::PrecisionDefault)) {
			if (data->formatting_options.precision_mode == FormattingOptions::PrecisionRequired) {
				return QString::number (data->cell_doubles[row], 'g', MAX_PRECISION);
			}
			return QString::number (data->cell_doubles[row], 'f', data->formatting_options.precision);
		}
		return QString::number (data->cell_doubles[row], 'g', MAX_PRECISION);
	}
}

QString RKVariable::getRText (int row) const {
	RK_TRACE (OBJECTS);
	
	Status cell_state = cellStatus (row);
	
	if ((cell_state == ValueUnused) || (cell_state == ValueInvalid)) {
		return ("NA");
	} else if (getDataType () == DataFactor) {
		return (rQuote (getLabeled (row)));
	} else if (getDataType () == DataCharacter) {
		return (rQuote (getText (row)));
	} else if (getDataType () == DataLogical) {
		RK_ASSERT (data->cell_doubles != 0);
		if (data->cell_doubles[row] == 0) return ("FALSE");
		else return ("TRUE");
	} else {
		RK_ASSERT (data->cell_doubles != 0);
		return (QString::number (data->cell_doubles[row], 'g', MAX_PRECISION));
	}
}

#warning this could/should be merged with setCharacter (which is currently less complete)
void RKVariable::setText (int row, const QString &text) {
	RK_TRACE (OBJECTS);
	RK_ASSERT (row < getLength ());

	// clear previous state
	if (data->cell_states[row] & RKVarEditData::Invalid) {
		data->cell_states[row] = RKVarEditData::UnsyncedInvalidState;
		data->invalid_fields.remove (row);
	} else {
		data->cell_states[row] = 0;
	}

	if (text.isNull ()) {
		data->cell_states[row] |= RKVarEditData::NA;
	} else {
		if (getDataType () == DataCharacter) {
			RK_ASSERT (data->cell_strings != 0);
			data->cell_strings[row] = text;
			data->cell_states[row] |= RKVarEditData::Valid;
		} else if (getDataType () == DataFactor) {
			RK_ASSERT (data->cell_doubles != 0);
			if (text.isEmpty ()) {
				data->cell_states[row] |= RKVarEditData::NA;
			} else if (data->value_labels && data->value_labels->contains (text)) {
				data->cell_doubles[row] = text.toInt ();
				data->cell_states[row] |= RKVarEditData::Valid;
			} else {
				data->invalid_fields.insert (row, text);
				data->cell_states[row] |= RKVarEditData::Invalid | RKVarEditData::UnsyncedInvalidState;
			}
		} else {
			RK_ASSERT (data->cell_doubles != 0);
			bool ok;
			if (text.isEmpty ()) {
				data->cell_states[row] |= RKVarEditData::NA;
			} else {
				data->cell_doubles[row] = text.toDouble (&ok);
				if (ok) {
					data->cell_states[row] |= RKVarEditData::Valid;
				} else {
					data->invalid_fields.insert (row, text);
					data->cell_states[row] |= RKVarEditData::Invalid | RKVarEditData::UnsyncedInvalidState;
				}
			}
		}
	}
	cellChanged (row);
}

QString RKVariable::getLabeled (int row) const {
	if (data->value_labels) {
		QString otext = getText (row);
		if (data->value_labels->contains (otext)) {
			return (*(data->value_labels))[otext];
		}
	}
	return getText (row);
}

double *RKVariable::getNumeric (int from_row, int to_row) const {
	RK_TRACE (OBJECTS);
	if (to_row >= getLength ()) {
		RK_ASSERT (false);
		return 0;
	}
	RK_ASSERT (from_row <= to_row);

	// TODO: no, this is not good. Return a _copy_!
	// we simply return the whole array starting at the given offset for now. Change this, if the storage mechanism gets changed!
	return &(data->cell_doubles[from_row]);
}

void RKVariable::setNumeric (int from_row, int to_row, double *numdata) {
	RK_TRACE (OBJECTS);
	RK_ASSERT (to_row < getLength ());

	if (getDataType () == DataCharacter) {
		RK_ASSERT (false);		// asserting false to catch cases of this use for now. it's not really a problem, though
		int i = 0;
		for (int row=from_row; row <= to_row; ++row) {
			setText (row, QString::number (numdata[i++], 'g', MAX_PRECISION));
		}
	} else if (getDataType () == DataFactor) {
		int i = 0;
		for (int row=from_row; row <= to_row; ++row) {
			if (data->cell_states[row] & RKVarEditData::Invalid) data->cell_states[row] =  RKVarEditData::UnsyncedInvalidState;
			else data->cell_states[row] = 0;

			if (isnan (numdata[i]) || (!data->value_labels) || (!data->value_labels->contains (QString::number (numdata[i])))) {
				data->cell_states[row] |= RKVarEditData::NA;
			} else {
				data->cell_states[row] |= RKVarEditData::Valid;
				data->cell_doubles[row] = numdata[i];
			}
			++i;
		}
	} else {
		int i = 0;
		for (int row=from_row; row <= to_row; ++row) {
			if (data->cell_states[row] & RKVarEditData::Invalid) data->cell_states[row] = RKVarEditData::UnsyncedInvalidState;
			else data->cell_states[row] = 0;

			if (isnan (numdata[i])) {
				data->cell_states[row] |= RKVarEditData::NA;
			} else {
				data->cell_states[row] |= RKVarEditData::Valid;
				data->cell_doubles[row] = numdata[i];
			}
			++i;
		}
	}
	cellsChanged (from_row, to_row);
}

QString *RKVariable::getCharacter (int from_row, int to_row) const {
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

void RKVariable::setCharacter (int from_row, int to_row, QString *txtdata) {
	RK_TRACE (OBJECTS);
	RK_ASSERT (to_row < getLength ());
	
	if (getDataType () == DataCharacter) {
		int i=0;
		for (int row=from_row; row <= to_row; ++row) {
			if (data->cell_states[row] & RKVarEditData::Invalid) data->cell_states[row] = RKVarEditData::UnsyncedInvalidState;
			else data->cell_states[row] = 0;

			if (txtdata[i].isNull ()) data->cell_states[row] |= RKVarEditData::NA;
			else data->cell_states[row] |= RKVarEditData::Valid;

			data->cell_strings[row] = txtdata[i++];
		}
	} else {
		bool old_sync = data->immediate_sync;
		setSyncing (false);
		int i=0;
		for (int row=from_row; row <= to_row; ++row) {
			setText (row, txtdata[i++]);
		}
		if (old_sync) {
			syncDataToR ();
			setSyncing (true);
		}
		return;
	}
	cellsChanged (from_row, to_row);
}

void RKVariable::setUnknown (int from_row, int to_row) {
	RK_TRACE (OBJECTS);
	RK_ASSERT (to_row < getLength ());

	if ((from_row < 0)) from_row = 0;
	if ((to_row < 0)) to_row = data->allocated_length - 1;
		
	for (int row=from_row; row <= to_row; ++row) {
		data->cell_strings[row] = RKVarEditData::Unknown;
	}
}

RKVariable::Status RKVariable::cellStatus (int row) const {
	if (data->cell_states[row] == RKVarEditData::Unknown) return ValueUnknown;
	if (data->cell_states[row] & RKVarEditData::NA) return ValueUnused;
	if (data->cell_states[row] & RKVarEditData::Invalid) return ValueInvalid;
	return ValueValid;
}

void RKVariable::removeRows (int from_row, int to_row) {
	RK_TRACE (OBJECTS);

	QList<int> changed_invalids;
	int offset = (to_row - from_row) + 1;

	for (int row = from_row; row < getLength (); ++row) {
		if (data->invalid_fields.contains (row)) {
			QString inv = data->invalid_fields.take (row);
			changed_invalids.append (row);
			if (row > to_row) {
				changed_invalids.append (row - offset);
				data->invalid_fields.insert (row - offset, inv);
			}
		}
	}

	if (to_row < (data->allocated_length - 1)) {	// not the last rows
		if (data->cell_strings) {
			memMoveQStrings (&(data->cell_strings[from_row]), &(data->cell_strings[to_row+1]), (data->allocated_length - to_row - 1));
		} else {
			memmove (&(data->cell_doubles[from_row]), &(data->cell_doubles[to_row+1]), (data->allocated_length - to_row - 1) * sizeof (double));
		}
		memmove (&(data->cell_states[from_row]), &(data->cell_states[to_row+1]), (data->allocated_length - to_row - 1) * sizeof (int));
	}

	for (int row = (data->allocated_length - offset); row < data->allocated_length; ++row) {
		data->cell_states[row] = RKVarEditData::Unknown;
	}

	for (int i = 0; i < changed_invalids.size (); ++i) {
		writeInvalidField (changed_invalids[i], 0);
	}

	dimensions[0] -= offset;	
	downSize ();
}

void RKVariable::insertRows (int row, int count) {
	RK_TRACE (OBJECTS);
	int old_len = getLength ();
	extendToLength (getLength () + count);		// getLength is the new length after this!

	for (int i=old_len; i < getLength(); ++i) {
		data->cell_states[i] = RKVarEditData::NA;
	}

	QList<int> changed_invalids;
	for (int i = getLength () - count - 1; i >= row; --i) {
		if (data->invalid_fields.contains (i)) {
			QString dummy = data->invalid_fields.take (i);
			changed_invalids.append (i);
			changed_invalids.append (i + count);
			data->invalid_fields.insert (i + count, dummy);
		}
	}

	if (row >= getLength () && (count == 1)) {		// important special case
		if (data->cell_strings) data->cell_strings[row+count] = QString::null;
		if (data->cell_doubles) data->cell_doubles[row+count] = 0.0;
		data->cell_states[row+count] = RKVarEditData::NA;
	} else {
		if (data->cell_strings) {
			memMoveQStrings (&(data->cell_strings[row+count]), &(data->cell_strings[row]), (data->allocated_length - (row + count) - 1));
		}
		if (data->cell_doubles) memmove (&(data->cell_doubles[row+count]), &(data->cell_doubles[row]), (data->allocated_length - (row + count) - 1) * sizeof (double));
		memmove (&(data->cell_states[row+count]), &(data->cell_states[row]), (data->allocated_length - (row + count) - 1) * sizeof (int));
	}
	
	for (int i=row+count-1; i >= row; --i) {
		data->cell_states[i] = RKVarEditData::NA;
	}

	for (int i = 0; i < changed_invalids.size (); ++i) {
		writeInvalidField (changed_invalids[i], 0);
	}
}

RObject::ValueLabels RKVariable::getValueLabels () const {
	RK_ASSERT (data);

	if (!data->value_labels) return RObject::ValueLabels ();
	return (*(data->value_labels));
}

void RKVariable::setValueLabels (const ValueLabels& labels) {
	RK_TRACE (OBJECTS);
	RK_ASSERT (data);

	if (labels.isEmpty ()) {
		if (!data->value_labels) return;	// no change: was empty, is empty

		delete data->value_labels;
		data->value_labels = 0;
	} else {
		if (!(data->value_labels)) data->value_labels = new RObject::ValueLabels;
		else {
			if (*(data->value_labels) == labels) return;	// old and new lists are the same
		}
		*(data->value_labels) = labels;
	}

	writeValueLabels (0);
	RKGlobals::tracker ()->objectMetaChanged (this);

	// find out which values got valid / invalid and change those
	for (int i=0; i < getLength (); ++i) {
		if (cellStatus (i) == ValueInvalid) {
			if (labels.contains (getText (i))) {
				setText (i, getText (i));
			}
		} else {
			if (!(labels.contains (getText (i)))) {
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

void RKVariable::writeValueLabels (RCommandChain *chain) const {
	RK_TRACE (OBJECTS);
	RK_ASSERT (data);
	
	if (data->value_labels) {
		int i = 1;
		QString level_string = "c (";
		while (data->value_labels->contains (QString::number (i))) {
			level_string.append (rQuote ((*(data->value_labels))[QString::number (i)]));
			if (data->value_labels->contains (QString::number (++i))) {
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

QString RKVariable::getValueLabelString () const {
	RK_TRACE (OBJECTS);
	RK_ASSERT (data);

	if (data->value_labels) {
		int i = 1;
		QString level_string;
		while (data->value_labels->contains (QString::number (i))) {
			level_string.append ((*(data->value_labels))[QString::number (i)]);
			if (data->value_labels->contains (QString::number (++i))) {
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
	RK_ASSERT (data);

	ValueLabels new_labels;	
	QStringList list = string.split ("#,#");

	int i = 1;
	for (QStringList::const_iterator it = list.constBegin (); it != list.constEnd (); ++it) {
		new_labels.insert (QString::number (i), *it);
		++i;
	}
	setValueLabels (new_labels);
}

RKVariable::FormattingOptions RKVariable::getFormattingOptions () const {
	RK_TRACE (OBJECTS);
	RK_ASSERT (data);

	return data->formatting_options;
}

void RKVariable::setFormattingOptions (const FormattingOptions new_options) {
	RK_TRACE (OBJECTS);
	RK_ASSERT (data);

	if ((new_options.alignment == data->formatting_options.alignment) && (new_options.precision_mode == data->formatting_options.precision_mode) && (new_options.precision == data->formatting_options.precision)) return;

	data->formatting_options = new_options;
	setMetaProperty ("format", formattingOptionsToString (new_options));

	// also update display of all values:
	ChangeSet *set = new ChangeSet;
	set->from_index = 0;
	set->to_index = getLength () - 1;	
	RKGlobals::tracker ()->objectDataChanged (this, set);
}

QString RKVariable::getFormattingOptionsString () const {
	RK_TRACE (OBJECTS);
	RK_ASSERT (data);

	return getMetaProperty ("format");
}

void RKVariable::setFormattingOptionsString (const QString &string) {
	RK_TRACE (OBJECTS);
	RK_ASSERT (data);

	setFormattingOptions (parseFormattingOptionsString (string));
}

// static
QString RKVariable::formattingOptionsToString (const FormattingOptions& options) {
	RK_TRACE (OBJECTS);

	QString format_string;
	if (options.alignment != (int) FormattingOptions::AlignDefault) {
		format_string.append ("align:" + QString::number (options.alignment));
	}

	if (options.precision_mode != (int) FormattingOptions::PrecisionDefault) {
		if (!format_string.isEmpty ()) format_string.append ("#");
		format_string.append ("prec:");
		if (options.precision_mode == (int) FormattingOptions::PrecisionRequired) {
			format_string.append ("v");
		} else {
			format_string.append (QString::number (options.precision));
		}
	}

	return format_string;
}

// static
RKVariable::FormattingOptions RKVariable::parseFormattingOptionsString (const QString &string) {
	RK_TRACE (OBJECTS);

	FormattingOptions formatting_options;
	formatting_options.alignment = FormattingOptions::AlignDefault;
	formatting_options.precision_mode = FormattingOptions::PrecisionDefault;
	formatting_options.precision = 0;
	bool empty = true;

	QStringList list = string.split ("#", QString::SkipEmptyParts);
	QString option, parameter;
	for (QStringList::const_iterator it = list.constBegin (); it != list.constEnd (); ++it) {
		option = (*it).section (':', 0, 0);
		parameter = (*it).section (':', 1, 1);
		
		if (parameter.isEmpty ()) continue;
		
		if (option == "align") {
			int al = parameter.toInt ();
			if ((al >= (int) FormattingOptions::AlignDefault) && (al <= (int) FormattingOptions::AlignRight)) {
				empty = false;
				formatting_options.alignment = (FormattingOptions::Alignment) al;
			}
		} else if (option == "prec") {
			if (parameter == "d") {
				empty = false;
				formatting_options.precision_mode = FormattingOptions::PrecisionDefault;
			} else if (parameter == "v") {
				empty = false;
				formatting_options.precision_mode = FormattingOptions::PrecisionRequired;
			} else {
				int digits = parameter.toInt ();
				if ((digits >= 0) && (digits <= 15)) {
					empty = false;
					formatting_options.precision_mode = FormattingOptions::PrecisionFixed;
					formatting_options.precision = digits;
				}
			}
		} else {
			RK_ASSERT (false);
		}
	}
	
	return formatting_options;
}

RKVariable::CellAlign RKVariable::getAlignment () const {
	RK_ASSERT (data);
	
	if (data->formatting_options.alignment != FormattingOptions::AlignDefault) {
		if (data->formatting_options.alignment == FormattingOptions::AlignLeft) return AlignCellLeft;
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
