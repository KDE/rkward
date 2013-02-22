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
#ifndef RKPSEUDOOBJECTS_H
#define RKPSEUDOOBJECTS_H

#include "rcontainerobject.h"
#include "renvironmentobject.h"

/** TODO:
 * - implement OrphanNamepacesObject
 * - override getObjectDescription()
 * - namespace objects should keep track of their namespace name, themselves
 * - internally store namespace objects by name, not attached to a package
 * */

/**
This class represents a "pseudo" list of the S4 slots of the corresponding parent object. In R, no such object exists.

@author Thomas Friedrichsmeier
*/
class RSlotsPseudoObject : public RContainerObject {
public:
	RSlotsPseudoObject (RObject *parent);
	~RSlotsPseudoObject ();

	QString getFullName () const;
	QString makeChildName (const QString &short_child_name, bool misplaced=false) const;
};

/**
This class represents the namespace environment of a loaded package. While the namespace environment exists in R, it is not
actually a logical child of the package environment, which is why we call it a pseudo object in RKWard (actually, perhaps we should
rather call it a "special" object, instead).

@author Thomas Friedrichsmeier
*/
class RKNamespaceObject : public REnvironmentObject {
public:
	RKNamespaceObject (REnvironmentObject* package);
	~RKNamespaceObject ();

	QString getFullName () const;
	QString makeChildName (const QString &short_child_name, bool misplaced=false) const;
	QString makeChildBaseName (const QString &short_child_name) const;
};

#endif

