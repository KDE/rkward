/*
rkpseudoobjects - This file is part of the RKWard project. Created: Fri Mar 11 2011
SPDX-FileCopyrightText: 2011-2013 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
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
	explicit RSlotsPseudoObject(RObject *parent);
	~RSlotsPseudoObject();

	QString getFullName(int) const override;
	QString makeChildName(const QString &short_child_name, int) const override;
};

/**
This class represents the namespace environment of a loaded package. While the namespace environment exists in R, it is not
actually a logical child of the package environment, which is why we call it a pseudo object in RKWard (actually, perhaps we should
rather call it a "special" object, instead).

@author Thomas Friedrichsmeier
*/
class RKNamespaceObject : public REnvironmentObject {
  public:
	explicit RKNamespaceObject(REnvironmentObject *package, const QString &name = QString());
	~RKNamespaceObject();

	QString getFullName(int) const override;
	QString makeChildName(const QString &short_child_name, int) const override;
	QString namespaceName() const { return namespace_name; };

  private:
	QString namespace_name;
};

class RObjectList;
/**
This class represents the list of namespace environments which are loaded, but do not belong to a package on the search path.

(TODO: Actually, we should relax the assumption that objects can only be at one place in the hierarchy, and turn this into a list of
all namespace environments!)

It exists only once, as a direct child of the RObjectList.

@author Thomas Friedrichsmeier
*/
class RKOrphanNamespacesObject : public REnvironmentObject {
  public:
	explicit RKOrphanNamespacesObject(RObjectList *parent);
	~RKOrphanNamespacesObject();

	QString getFullName(int options) const override;
	QString makeChildName(const QString &short_child_name, int options) const override;
	QString getObjectDescription() const override;

	RKNamespaceObject *findOrphanNamespace(const QString &name) const;

	using REnvironmentObject::updateFromR;
	/** should not be called on this object. Reimplemented to raise an assert, and do nothing else. */
	void updateFromR(RCommandChain *chain) override;
	/** reimplemented from REnvironmentObject */
	void updateNamespacesFromR(RCommandChain *chain, const QStringList &current_symbols);
};

#endif
