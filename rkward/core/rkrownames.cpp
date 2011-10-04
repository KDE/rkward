/***************************************************************************
                          rkrownames  -  description
                             -------------------
    begin                : Tue Mar 21 2010
    copyright            : (C) 2010 by Thomas Friedrichsmeier
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

#include "rkrownames.h"

#include <limits.h>

#include <klocale.h>

#include "rcontainerobject.h"
#include "../rkglobals.h"
#include "../rbackend/rinterface.h"
#include "rkmodificationtracker.h"

#include "../debug.h"

RKRowNames::RKRowNames (RContainerObject *parent) : RKVariable (parent, QString ()) {
	RK_TRACE (OBJECTS);

	type = Variable | NonVisibleObject;

	setDataType (RObject::DataCharacter);
	check_duplicates = true;
	is_sequential_up_to_row = -1;

	name = i18n ("row names");
}

RKRowNames::~RKRowNames () {
	RK_TRACE (OBJECTS);
}

void RKRowNames::beginEdit () {
	RK_TRACE (OBJECTS);

	bool just_initialized = (data == 0);
	RKVariable::beginEdit ();

	if (just_initialized) {
		RK_ASSERT (data);
		for (int i = 0; i < getLength (); ++i) {
			data->cell_strings[i] = QString::number (i+1);
			data->cell_states[i] = RKVarEditData::Valid;
		}
		is_sequential_up_to_row = getLength () - 1;
	}
}

QString RKRowNames::getFullName () const {
//	RK_TRACE (OBJECTS);

	return ("row.names (" + parent->getFullName () + ")");
}

void RKRowNames::writeData (int from_row, int to_row, RCommandChain *chain) {
	RK_TRACE (OBJECTS);

	if (isSequential ()) {
		RKGlobals::rInterface ()->issueCommand (getFullName () + " <- NULL", RCommand::App | RCommand::Sync, QString::null, 0,0, chain);
	} else {
		// unfortunately, we always need to write the whole data, as row.names<- does not support indexing.
		QString data_string = "c (";
		for (int row = 0; row < getLength (); ++row) {
			// TODO: use getCharacter and direct setting of vectors.
			data_string.append (getRText (row));
			if (row != (getLength () - 1)) {
				data_string.append (", ");
			}
		}
		data_string.append (")");
		RKGlobals::rInterface ()->issueCommand (getFullName () + " <- " + data_string, RCommand::App | RCommand::Sync, QString::null, 0, 0, chain);
	}

	ChangeSet *set = new ChangeSet;
	set->from_index = from_row;
	set->to_index = to_row;
	RKGlobals::tracker ()->objectDataChanged (this, set);
}

void RKRowNames::setText (int row, const QString &text) {
	RK_TRACE (OBJECTS);

	lockSyncing (true);

	data->cell_strings[row] = QString ();	// don't get in the way of duplicate checking!
	QString real_text = text;
	if (real_text.isEmpty ()) {
		if (isSequential ()) {
			real_text = QString::number (row + 1);
		} else {
			real_text = i18n ("new.row");		// empty row names are forbidden
		}
	}

	bool was_sequential_row = false;
	if (is_sequential_up_to_row >= (row - 1)) {
		if (real_text == QString::number (row + 1)) {
			if (makeUnique (&real_text, true)) {
				is_sequential_up_to_row = qMax (row, is_sequential_up_to_row);
				was_sequential_row = true;
				if (is_sequential_up_to_row == row) {
					// even more sequential numbers after this?
					for (int i = row + 1; i < getLength (); ++i) {
						if (data->cell_strings[i] != QString::number (i + 1)) break;
						is_sequential_up_to_row = i;
					}
				}
			}
		}
	}

	if (!was_sequential_row) {
		makeUnique (&real_text, false);
		is_sequential_up_to_row = qMin (row - 1, is_sequential_up_to_row);
	}
	RKVariable::setText (row, real_text);

	lockSyncing (false);
}

bool RKRowNames::makeUnique (QString *text, bool non_sequentials_only) {
	RK_TRACE (OBJECTS);

	if (!check_duplicates) return true;

	bool from_index = 0;
	if (non_sequentials_only) from_index = is_sequential_up_to_row + 1;

	QString dummy = *text;
	for (long i = 0; i < INT_MAX; ++i) {
		// check whether the text is unique on this iteration
		bool is_unique = true;
		for (int row = from_index; row <= getLength (); ++row) {
			if (dummy == data->cell_strings[row]) {
				is_unique = false;
				break;
			}
		}
		if (is_unique) {
			if (i == 0) {
				return true;	// was unique on first attempt
			} else {
				*text = dummy;
				return false;
			}
		}

		// try adjusted text on next iteration
		dummy = *text + '.' + QString::number (i);
	}

	RK_ASSERT (false);
	return false;
}

void RKRowNames::insertRows (int row, int count) {
	RK_TRACE (OBJECTS);

	lockSyncing (true);

	bool was_sequential = isSequential ();
	RKVariable::insertRows (row, count);

	if (was_sequential) {	// length just increased
		is_sequential_up_to_row += count;
		for (int i = row; i < getLength (); ++i) {
			data->cell_strings[i] = QString::number (i + 1);
			data->cell_states[i] = RKVarEditData::Valid;	// was set to NA by RKVariable::insertRows
		}
	} else {
		is_sequential_up_to_row = qMin (is_sequential_up_to_row, row - 1);
		for (int i = row; i < row + count; ++i) {
			setText (i, QString ());
		}
	}
	// always need to update. If sequential, rows have just changed. If non-sequential, data needs to be written to backend
	cellsChanged (row, getLength () - 1);

	lockSyncing (false);
}

void RKRowNames::removeRows (int from_row, int to_row) {
	RK_TRACE (OBJECTS);

	lockSyncing (true);

	bool was_sequential = isSequential ();
	RKVariable::removeRows (from_row, to_row);

	if (was_sequential) {	// length just decreased
		is_sequential_up_to_row -= (to_row - from_row + 1);
		for (int i = from_row; i < getLength (); ++i) {
			data->cell_strings[i] = QString::number (i + 1);
		}
	} else {
		is_sequential_up_to_row = qMin (is_sequential_up_to_row, from_row - 1);
	}

	// always need to update. If sequential, rows have just changed. If non-sequential, data needs to be written to backend
	cellsChanged (from_row, getLength () - 1);

	lockSyncing (false);
}

void RKRowNames::setCharacterFromR (int from_row, int to_row, const QStringList &data) {
	RK_TRACE (OBJECTS);

	is_sequential_up_to_row = -1;
	check_duplicates = false;
	RKVariable::setCharacterFromR (from_row, to_row, data);
	check_duplicates = true;
}
