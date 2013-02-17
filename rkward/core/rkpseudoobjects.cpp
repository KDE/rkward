/***************************************************************************
                          rkpseudoobjects  -  description
                             -------------------
    begin                : Fri Mar 11 2011
    copyright            : (C) 2011-2013 by Thomas Friedrichsmeier
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

#include "rkpseudoobjects.h"

#include "../debug.h"

RSlotsPseudoObject::RSlotsPseudoObject (RObject *parent) : RContainerObject (parent, "SLOTS") {
	RK_TRACE (OBJECTS);
	pseudo_object_types.insert (this, SlotsObject);
}

RSlotsPseudoObject::~RSlotsPseudoObject () {
	RK_TRACE (OBJECTS);
	pseudo_object_types.remove (this);
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

RKNamespaceObject::RKNamespaceObject (REnvironmentObject* package) : REnvironmentObject (package, "NAMESPACE") {
	RK_TRACE (OBJECTS);
	pseudo_object_types.insert (this, NamespaceObject);
}

RKNamespaceObject::~RKNamespaceObject () {
	RK_TRACE (OBJECTS);
	pseudo_object_types.remove (this);
}

QString RKNamespaceObject::getFullName () const {
	RK_TRACE (OBJECTS);
	return ("asNamespace (" + rQuote (static_cast<REnvironmentObject*>(parent)->packageName ()) + ")");
}

QString RKNamespaceObject::makeChildName (const QString& short_child_name, bool) const {
	RK_TRACE (OBJECTS);
	QString safe_name = short_child_name;
	if (irregularShortName (safe_name)) safe_name = rQuote (short_child_name);
	return (static_cast<REnvironmentObject*>(parent)->packageName () + ":::" + safe_name);
}

QString RKNamespaceObject::makeChildBaseName (const QString& short_child_name) const {
	RK_TRACE (OBJECTS);
	return (static_cast<REnvironmentObject*>(parent)->packageName () + ":::" + short_child_name);
}
