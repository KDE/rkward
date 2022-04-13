/*
rkpseudoobjects - This file is part of RKWard (https://rkward.kde.org). Created: Fri Mar 11 2011
SPDX-FileCopyrightText: 2011-2013 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "rkpseudoobjects.h"

#include <KLocalizedString>

#include "../debug.h"

RSlotsPseudoObject::RSlotsPseudoObject (RObject *parent) : RContainerObject (parent, "SLOTS") {
	RK_TRACE (OBJECTS);
	type |= PseudoObject;
	pseudo_object_types.insert (this, SlotsObject);
}

RSlotsPseudoObject::~RSlotsPseudoObject () {
	RK_TRACE (OBJECTS);
	pseudo_object_types.remove (this);
}

QString RSlotsPseudoObject::getFullName (int options) const {
	RK_TRACE (OBJECTS);

	return (".rk.get.slots (" + parent->getFullName (options) + ')');
}

QString RSlotsPseudoObject::makeChildName (const QString &short_child_name, bool, int options) const {
	RK_TRACE (OBJECTS);

	if (options & ExplicitSlotsExpansion) {
		return (QStringLiteral("slot(") + parent->getFullName (options) + QStringLiteral(", ") + rQuote(short_child_name) + ')');
	}
	QString safe_name = short_child_name;
	if (irregularShortName (safe_name)) safe_name = rQuote (short_child_name);
	return (parent->getFullName (options) + '@' + safe_name);
}

RKNamespaceObject::RKNamespaceObject (REnvironmentObject* package, const QString &name) : REnvironmentObject (package, name.isNull () ? "NAMESPACE" : name) {
	RK_TRACE (OBJECTS);
	type |= PseudoObject;
	pseudo_object_types.insert (this, NamespaceObject);
	if (name.isNull ()) namespace_name = package->packageName ();
	else namespace_name = name;
}

RKNamespaceObject::~RKNamespaceObject () {
	RK_TRACE (OBJECTS);
	pseudo_object_types.remove (this);
}

QString RKNamespaceObject::getFullName (int) const {
	RK_TRACE (OBJECTS);
	return ("asNamespace (" + rQuote (namespace_name) + ')');
}

QString RKNamespaceObject::makeChildName (const QString& short_child_name, bool, int) const {
	RK_TRACE (OBJECTS);
	QString safe_name = short_child_name;
	if (irregularShortName (safe_name)) safe_name = rQuote (short_child_name);
	return (namespace_name + ":::" + safe_name);
}

#include "robjectlist.h"
#include "rkmodificationtracker.h"
#include "../rkglobals.h"

RKOrphanNamespacesObject::RKOrphanNamespacesObject (RObjectList* parent) : REnvironmentObject (parent, i18nc ("Note: 'namespace' is a technical term, should not be translated", "Orphan Namespaces")) {
	RK_TRACE (OBJECTS);
	type |= PseudoObject;
	pseudo_object_types.insert (this, OrphanNamespacesObject);
}

RKOrphanNamespacesObject::~RKOrphanNamespacesObject () {
	RK_TRACE (OBJECTS);
	pseudo_object_types.remove (this);
}

QString RKOrphanNamespacesObject::getFullName (int) const {
	RK_TRACE (OBJECTS);
	return ("loadedNamespaces ()");
}

QString RKOrphanNamespacesObject::makeChildName (const QString& short_child_name, bool, int) const {
	RK_TRACE (OBJECTS);
	return ("asNamespace (" + rQuote (short_child_name) + ')');
}

void RKOrphanNamespacesObject::updateFromR (RCommandChain* chain) {
	RK_TRACE (OBJECTS);
	Q_UNUSED (chain);
	RK_ASSERT (false);
}

void RKOrphanNamespacesObject::updateFromR (RCommandChain* chain, const QStringList& current_symbols) {
	RK_TRACE (OBJECTS);
	Q_UNUSED (chain);	// because the namespace objects themselves are not updated, only added as incomplete objects

	// which former children are missing?
	for (int i = childmap.size () - 1; i >= 0; --i) {
		RObject *object = childmap[i];
		if (!current_symbols.contains (object->getShortName ())) {
			RKModificationTracker::instance()->removeObject (object, 0, true);
		}
	}

	// which ones are new in the list?
	for (int i = 0; i < current_symbols.size (); ++i) {
		if (!findOrphanNamespace (current_symbols[i])) {
			RKNamespaceObject *nso = new RKNamespaceObject (this, current_symbols[i]);
			nso->type |= Incomplete;
			RKModificationTracker::instance()->beginAddObject (nso, this, i);
			childmap.insert (i, nso);
			RKModificationTracker::instance()->endAddObject (nso, this, i);
		}
	}

	RK_ASSERT (current_symbols.size () == childmap.size ());
}

RKNamespaceObject* RKOrphanNamespacesObject::findOrphanNamespace (const QString& name) const {
	RK_TRACE (OBJECTS);

	for (int i = childmap.size () - 1; i >= 0; --i) {
		RObject *obj = childmap[i];
		if (obj->getShortName () == name) {
			RK_ASSERT (obj->isPackageNamespace ());
			return static_cast<RKNamespaceObject*> (obj);
		}
	}
	return 0;
}

QString RKOrphanNamespacesObject::getObjectDescription () const {
	RK_TRACE (OBJECTS);

	QString desc = REnvironmentObject::getObjectDescription ();
	desc.append (QString ("<p>%1</p>").arg (i18n ("This special object does not actually exist anywhere in R. It is used, here, to list namespaces which are loaded, but not attached to a package on the search path. These are typically 'imported' namespaces.")));
	return desc;
}
