/***************************************************************************
                          rfunctionobject  -  description
                             -------------------
    begin                : Wed Apr 26 2006
    copyright            : (C) 2006 by Thomas Friedrichsmeier
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

#include "rfunctionobject.h"

#include "../rbackend/rdata.h"
#include "rkmodificationtracker.h"
#include "../rkglobals.h"
#include "../debug.h"

RFunctionObject::RFunctionObject (RContainerObject *parent, const QString &name) : RObject (parent, name) {
	RK_TRACE (OBJECTS);
	type = Function;

	argcount = 0;
	argnames = 0;
	argvalues = 0;
}

RFunctionObject::~RFunctionObject () {
	RK_TRACE (OBJECTS);
}

QString RFunctionObject::printArgs () const {
	RK_TRACE (OBJECTS);

	QString ret;
	for (unsigned int i = 0; i < argcount; ++i) {
		if (i) ret.append (", ");
		ret.append (argnames[i]);
		if (!argvalues[i].isEmpty ()) {
			ret.append ("=");
			ret.append (argvalues[i]);
		}
	}
	return ret;
}

bool RFunctionObject::updateStructure (RData *new_data) {
	RK_TRACE (OBJECTS);
	RK_ASSERT (new_data->getDataLength () >= 5);
	RK_ASSERT (new_data->getDataType () == RData::StructureVector);

	if (!RObject::updateStructure (new_data)) return false;

	if (updateArguments (new_data)) RKGlobals::tracker ()->objectMetaChanged (this);

	return true;
}

bool RFunctionObject::updateArguments (RData *new_data) {
	RK_TRACE (OBJECTS);
	RK_ASSERT (new_data->getDataLength () == 7);
	RK_ASSERT (new_data->getDataType () == RData::StructureVector);

	RData *argnames_data = new_data->getStructureVector ()[5];
	RData *argvalues_data = new_data->getStructureVector ()[6];

	unsigned int new_arglen = argnames_data->getDataLength (); 
	RK_ASSERT (argnames_data->getDataType () == RData::StringVector);
	RK_ASSERT (argvalues_data->getDataType () == RData::StringVector);

	RK_ASSERT (new_arglen == argvalues_data->getDataLength ());
	QString *new_argnames = argnames_data->getStringVector ();
	QString *new_argvalues = argvalues_data->getStringVector ();
	argnames_data->detachData ();
	argvalues_data->detachData ();

	bool changed = false;
	if (new_arglen != argcount) {
		changed = true;
	} else {
		for (unsigned int i = 0; i < new_arglen; ++i) {
			if (argnames[i] != new_argnames[i]) {
				changed = true;
				break;
			}

			if (argvalues[i] != new_argvalues[i]) {
				changed = true;
				break;
			}
		}
	}

	argcount = new_arglen;
	delete [] argnames;
	delete [] argvalues;
	argnames = new_argnames;
	argvalues = new_argvalues;

	return changed;
}
