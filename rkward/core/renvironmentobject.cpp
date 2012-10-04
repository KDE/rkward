/***************************************************************************
                          renvironmentobject  -  description
                             -------------------
    begin                : Wed Sep 27 2006
    copyright            : (C) 2006, 2009, 2010, 2011 by Thomas Friedrichsmeier
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

#include "renvironmentobject.h"

#include <kmessagebox.h>
#include <klocale.h>

#include "robjectlist.h"
#include "../rbackend/rinterface.h"
#include "../settings/rksettingsmoduleobjectbrowser.h"
#include "rkmodificationtracker.h"
#include "../rkglobals.h"

#include "../debug.h"

REnvironmentObject::REnvironmentObject (RContainerObject *parent, const QString &name) : RContainerObject (parent, name) {
	RK_TRACE (OBJECTS);

	namespace_envir = 0;
	type = Environment;
	if (parent == RObjectList::getObjectList ()) {
		type |= ToplevelEnv;
		if (name == ".GlobalEnv") {
			type |= GlobalEnv;
		} else if (name.contains (':')) {
			type |= PackageEnv;
		}
	}
}

REnvironmentObject::~REnvironmentObject () {
	RK_TRACE (OBJECTS);
	delete namespace_envir;
}

QString REnvironmentObject::packageName () const {
	RK_ASSERT (isType (PackageEnv));
	return name.section (':', 1);
}

QString REnvironmentObject::getFullName () const {
	RK_TRACE (OBJECTS);

	if (type & GlobalEnv) return name;	// .GlobalEnv
	if (type & ToplevelEnv) return ("as.environment (" + rQuote (name) + ")");
	if (isPackageNamespace ()) return ("asNamespace (" + rQuote (static_cast<REnvironmentObject*>(parent)->packageName ()) + ")");
	return parent->makeChildName (name, type & Misplaced);
}

QString REnvironmentObject::makeChildName (const QString &short_child_name, bool misplaced) const {
	RK_TRACE (OBJECTS);

	QString safe_name;
	bool irregular = false;
	if (irregularShortName (short_child_name)) {
		irregular = true;
		safe_name = rQuote (short_child_name);
	} else safe_name = short_child_name;

	if (type & GlobalEnv) {		// don't print as ".GlobalEnv$something" unless childname needs fixing
		if (irregular) return (getShortName () + "$" + safe_name);
		return (safe_name);
	}
	if (type & ToplevelEnv) {
/* Some items are placed outside of their native namespace. E.g. in package:boot item "motor". It can be retrieved using as.environment ("package:boot")$motor. This is extremly ugly. We need to give them (and only them) this special treatment. */
// TODO: hopefully one day operator "::" will work even in those cases. So check back later, and remove after a sufficient amount of backwards compatibility time
// NOTE: This appears to have been fixed in R 2.14.0, when all packages were forced to have namespaces.
		if ((type & PackageEnv) && (!misplaced)) return (packageName () + "::" + safe_name);
		return (getFullName () + '$' + safe_name);
	}
	if (isPackageNamespace ()) return (static_cast<REnvironmentObject*>(parent)->packageName () + ":::" + safe_name);
	return (getFullName () + '$' + safe_name);
}

QString REnvironmentObject::makeChildBaseName (const QString &short_child_name) const {
	RK_TRACE (OBJECTS);

	if (type & ToplevelEnv) return (short_child_name);
	if (isPackageNamespace ()) return (static_cast<REnvironmentObject*>(parent)->packageName () + ":::" + short_child_name);
	return (name + '$' + short_child_name);
}

void REnvironmentObject::writeMetaData (RCommandChain *chain) {
	RK_TRACE (OBJECTS);

	if (type & ToplevelEnv) return;
	RContainerObject::writeMetaData (chain);
}

void REnvironmentObject::updateFromR (RCommandChain *chain) {
	RK_TRACE (OBJECTS);
	if (type & PackageEnv) {
		if (RKSettingsModuleObjectBrowser::isPackageBlacklisted (packageName ())) {
			KMessageBox::information (0, i18n ("The package '%1' (probably you just loaded it) is currently blacklisted for retrieving structure information. Practically this means, the objects in this package will not appear in the object browser, and there will be no object name completion or function argument hinting for objects in this package.\nPackages will typically be blacklisted, if they contain huge amount of data, that would take too long to load. To unlist the package, visit Settings->Configure RKWard->Workspace.", packageName ()), i18n("Package blacklisted"), "packageblacklist" + packageName ());
			return;
		}
	}

	QString options;
	if (type & GlobalEnv) options = ", envlevel=-1";	// in the .GlobalEnv recurse one more level
	if (type & PackageEnv) options.append (", namespacename=" + rQuote (packageName ()));

	RCommand *command = new RCommand (".rk.get.structure (" + getFullName () + ", " + rQuote (getShortName ()) + options + ')', RCommand::App | RCommand::Sync | RCommand::GetStructuredData, QString::null, this, ROBJECT_UDPATE_STRUCTURE_COMMAND);
	RKGlobals::rInterface ()->issueCommand (command, chain);

	type |= Updating;
}

void REnvironmentObject::updateFromR (RCommandChain *chain, const QStringList &current_symbols) {
	RK_TRACE (OBJECTS);

	// only needed for the assert at the end
	int debug_baseline = 0;

	// which children are missing?
	for (int i = childmap.size () - 1; i >= 0; --i) {
		RObject *object = childmap[i];
		if (!current_symbols.contains (object->getShortName ())) {
			if (object->isPending () || (!(RKGlobals::tracker ()->removeObject (object, 0, true)))) debug_baseline++;
		}
	}

	// which ones are new in the list?
	for (int i = current_symbols.count () - 1; i >= 0; --i) {
		RObject *child = findChildByName (current_symbols[i]);
		if (!child) child = createPendingChild (current_symbols[i], i, false, false);
		if (child->isPending ()) {
			child->type -= RObject::Pending;	// HACK: Child is not actually pending: We've seen it!
			child->updateFromR (chain);
		}
	}

	RK_ASSERT ((debug_baseline + current_symbols.count ()) == childmap.size ());
}

bool REnvironmentObject::updateStructure (RData *new_data) {
	RK_TRACE (OBJECTS);
	RK_ASSERT (new_data->getDataType () == RData::StructureVector);
	RK_ASSERT (new_data->getDataLength () >= StorageSizeBasicInfo);

	if (!(type & ToplevelEnv)) {
		if (!RObject::updateStructure (new_data)) return false;
	}

	RData::RDataStorage new_data_data = new_data->structureVector ();
	if (new_data->getDataLength () > StorageSizeBasicInfo) {
		RData *children_sub = new_data_data.at (StoragePositionChildren);
		RK_ASSERT (children_sub->getDataType () == RData::StructureVector);
		updateChildren (children_sub);

		// a namespace to go with that?
		if (new_data->getDataLength () > (StorageSizeBasicInfo + 1)) {
			RK_ASSERT (new_data->getDataLength () == (StorageSizeBasicInfo + 2));
			updateNamespace (new_data_data.at (StoragePositionNamespace));
		} else updateNamespace (0);
	} else {
		RK_ASSERT (false);
	}
	return true;
}

void REnvironmentObject::updateNamespace (RData* new_data) {
	RK_TRACE (OBJECTS);

	if (!new_data) {
		if (namespace_envir) {
			RKGlobals::tracker ()->removeObject (namespace_envir, 0, true);
		}
		return;
	}

	RK_ASSERT (new_data->getDataType () == RData::StructureVector);
	bool added = false;
	if (!namespace_envir) {
		namespace_envir = new REnvironmentObject (this, "NAMESPACE");
		added = true;
		RKGlobals::tracker ()->lockUpdates (true);
	}
	namespace_envir->updateStructure (new_data->structureVector ().at (0));
	if (added) {
		RKGlobals::tracker ()->lockUpdates (false);

		int index = getObjectModelIndexOf (namespace_envir);
		REnvironmentObject *neo = namespace_envir;
		namespace_envir = 0;	// HACK: Must not be included in the count during the call to beginAddObject
		RKGlobals::tracker ()->beginAddObject (neo, this, index);
		namespace_envir = neo;
		RKGlobals::tracker ()->endAddObject (neo, this, index);
	}
}

QString REnvironmentObject::renameChildCommand (RObject *object, const QString &new_name) const {
	RK_TRACE (OBJECTS);

	return (makeChildName (new_name) + " <- " + object->getFullName () + '\n' + removeChildCommand (object));
}

QString REnvironmentObject::removeChildCommand (RObject *object) const {
	RK_TRACE (OBJECTS);

	return ("remove (" + object->getFullName () + ')');
}
