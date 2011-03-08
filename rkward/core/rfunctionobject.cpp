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
}

RFunctionObject::~RFunctionObject () {
	RK_TRACE (OBJECTS);
}

QString RFunctionObject::printArgs () const {
	RK_TRACE (OBJECTS);

	QString ret;
	for (int i = 0; i < argnames.size (); ++i) {
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
	RK_ASSERT (new_data->getDataLength () >= StorageSizeBasicInfo);
	RK_ASSERT (new_data->getDataType () == RData::StructureVector);

	if (!RObject::updateStructure (new_data)) return false;

	if (updateArguments (new_data)) RKGlobals::tracker ()->objectMetaChanged (this);

	return true;
}

bool RFunctionObject::updateArguments (RData *new_data) {
	RK_TRACE (OBJECTS);
	RK_ASSERT (new_data->getDataLength () == (StoragePositionFunValues + 1));
	RK_ASSERT (new_data->getDataType () == RData::StructureVector);

	RData *argnames_data = new_data->getStructureVector ()[StoragePositionFunArgs];
	RData *argvalues_data = new_data->getStructureVector ()[StoragePositionFunValues];

	RK_ASSERT (argnames_data->getDataType () == RData::StringVector);
	RK_ASSERT (argvalues_data->getDataType () == RData::StringVector);

	QStringList new_argnames = argnames_data->getStringVector ();
	QStringList new_argvalues = argvalues_data->getStringVector ();
	RK_ASSERT (new_argnames.size () == new_argvalues.size ());

	if ((new_argnames != argnames) || (new_argvalues != argvalues)) {
		argnames = new_argnames;
		argvalues = new_argvalues;
		return true;
	}
	return false;
}
