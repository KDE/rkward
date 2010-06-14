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
}

RKRowNames::~RKRowNames () {
	RK_TRACE (OBJECTS);
}

QString RKRowNames::getFullName () const {
//	RK_TRACE (OBJECTS);

	return ("row.names (" + parent->getFullName () + ")");
}

void RKRowNames::writeData (int from_row, int to_row, RCommandChain *chain) {
	RK_TRACE (OBJECTS);

	if (is_sequential_up_to_row == getLength ()) {
		RKGlobals::rInterface ()->issueCommand (getFullName () + " <- NULL", RCommand::App | RCommand::Sync, QString::null, 0,0, chain);

		ChangeSet *set = new ChangeSet;
		set->from_index = from_row;
		set->to_index = to_row;
		RKGlobals::tracker ()->objectDataChanged (this, set);
	} else {
		RKVariable::writeData (from_row, to_row, chain);
	}
}

void RKRowNames::setText (int row, const QString &text) {
	RK_TRACE (OBJECTS);

	RKVariable::setText (row, QString());	// don't get in the way of duplicate checking!
	QString real_text = text;
	if (real_text.isEmpty ()) real_text = i18n ("new.row");		// empty row names are forbidden

	bool is_sequential_number = false;
	if (is_sequential_up_to_row >= (row - 1)) {
		bool ok;
		long num = text.toInt(&ok);
		if (ok && (QString::number (num) == text)) {
			is_sequential_number = true;
			if (makeUnique (&real_text, true)) {
				is_sequential_up_to_row = row;
			}
		}
	}

	makeUnique (&real_text, false);

	RKVariable::setText (row, real_text);
}

bool RKRowNames::makeUnique (QString *text, bool non_sequentials_only) {
	RK_TRACE (OBJECTS);

	bool from_index = 0;
	if (non_sequentials_only) from_index = is_sequential_up_to_row + 1;

	QString dummy = *text;
	for (long i = 0; i < INT_MAX; ++i) {
		// check whether the text is unique on this iteration
		bool is_unique = true;
		for (int row = from_index; row <= getLength (); ++i) {
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
		QString dummy = *text + '.' + QString::number (i);
	}

	RK_ASSERT (false);
	return false;
}

void RKRowNames::setCharacterFromR (int from_row, int to_row, QString *data) {
	RK_TRACE (OBJECTS);

	check_duplicates = false;
	RKVariable::setCharacterFromR (from_row, to_row, data);
	check_duplicates = true;
}
