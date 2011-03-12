/***************************************************************************
                          rslotspseudoobject  -  description
                             -------------------
    begin                : Fri Mar 11 2011
    copyright            : (C) 2011 by Thomas Friedrichsmeier
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

#include "rslotspseudoobject.h"

#include "../debug.h"

RSlotsPseudoObject::RSlotsPseudoObject (RObject *parent, const QString &name) : RContainerObject (parent, name) {
	RK_TRACE (OBJECTS);
	RObject::name = "SLOTS";
	type |= RObject::PseudoObject;
}

RSlotsPseudoObject::~RSlotsPseudoObject () {
	RK_TRACE (OBJECTS);
}

QString RSlotsPseudoObject::getFullName () const {
	RK_TRACE (OBJECTS);

	return (".rk.get.slots (" + parent->getFullName () + ")");
}

QString RSlotsPseudoObject::makeChildName (const QString &short_child_name, bool) const {
	RK_TRACE (OBJECTS);

	QString safe_name = short_child_name;
	if (irregularShortName (safe_name)) safe_name = rQuote (short_child_name);
	return (parent->getFullName () + "@" + safe_name);
}

