/***************************************************************************
                          robject  -  description
                             -------------------
    begin                : Thu Aug 19 2004
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
#include "robject.h"

#include "rcontainerobject.h"

RObject::RObject (RContainerObject *parent, const QString &name) {
	RObject::parent = parent;
	RObject::name = name;
	type = 0;
	state = 0;
}

RObject::~RObject () {
}

QString RObject::getShortName () {
	return name;
}

QString RObject::getFullName () {
	return parent->makeChildName (RObject::name);
}

QString RObject::makeChildName (const QString &short_child_name) {
	return (getFullName () + "[[\"" + short_child_name + "\"]]");
}
