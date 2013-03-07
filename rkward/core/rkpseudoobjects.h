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
 * - fix automatic updating of loadedNamespaces()
 * - override getObjectDescription()
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
	RKNamespaceObject (REnvironmentObject* package, const QString name = QString ());
	~RKNamespaceObject ();

	QString getFullName () const;
	QString makeChildName (const QString &short_child_name, bool misplaced=false) const;
	QString makeChildBaseName (const QString &short_child_name) const;
	QString namespaceName () const { return namespace_name; };
private:
	QString namespace_name;
};

class RObjectList;
/**
This class represents the list of namespace environments which are loaded, but do not belong to a package on the search path.

(TODO: Actually, we should relax the assumption that objects can only be at one place in the hierarchy, and turn this into a list of
all namespace environemnts!)

It exists only once, as a direct child of the RObjectList.

@author Thomas Friedrichsmeier
*/
class RKOrphanNamespacesObject : public REnvironmentObject {
public:
	RKOrphanNamespacesObject (RObjectList *parent);
	~RKOrphanNamespacesObject ();

	QString getFullName () const;
	QString makeChildName (const QString &short_child_name, bool misplaced=false) const;
	QString makeChildBaseName (const QString &short_child_name) const;
	QString getObjectDescription () const;

	RKNamespaceObject *findOrphanNamespace (const QString &name) const;

	/** should not be called on this object. Reimplemented to raise an assert, and do nothing else. */
	void updateFromR (RCommandChain *chain);
	/** reimplemented from REnvironmentObject */
	void updateFromR (RCommandChain *chain, const QStringList &current_symbols);
};

#endif
