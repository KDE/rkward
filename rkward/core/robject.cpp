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
}

RObject::~RObject () {
}

QString RObject::getShortName () {
	return name;
}

QString RObject::getFullName () {
	QString pf = parent->getFullName ();
	if (pf != "") {
		return (pf + "[[\"" + RObject::name + "\"]]");
	} else {
		return RObject::name;
	}
}
