/***************************************************************************
                          renvironmentobject  -  description
                             -------------------
    begin                : Wed Sep 27 2006
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

#include "renvironmentobject.h"

#include <kmessagebox.h>
#include <klocale.h>

#include "robjectlist.h"
#include "../rbackend/rinterface.h"
#include "../settings/rksettingsmoduleobjectbrowser.h"
#include "../rkglobals.h"

#include "../debug.h"

REnvironmentObject::REnvironmentObject (RContainerObject *parent, const QString &name) : RContainerObject (parent, name) {
	RK_TRACE (OBJECTS);

	type = Environment;
	if (parent == RObjectList::getObjectList ()) {
		type |= ToplevelEnv;
		if (name == ".GlobalEnv") {
			type |= GlobalEnv;
		} else if (name.contains (':')) {
			namespace_name = name.section (':', 1);
			type |= PackageEnv;
		}
	} else {
		//namespace_name = parent->makeChildName (name);	// not needed, will not be used
	}
}

REnvironmentObject::~REnvironmentObject () {
	RK_TRACE (OBJECTS);
}

QString REnvironmentObject::getFullName () const {
	RK_TRACE (OBJECTS);

	if (type & ToplevelEnv) return ("as.environment (\"" + name + "\")");
	return parent->makeChildName (name, type & Misplaced);
}

QString REnvironmentObject::makeChildName (const QString &short_child_name, bool misplaced) const {
	RK_TRACE (OBJECTS);

	if (type & GlobalEnv) return (short_child_name);
	if (type & ToplevelEnv) {
/* Some items are placed outside of their native namespace. E.g. in package:boot item "motor". It can be retrieved using as.environment ("package:boot")$motor. This is extremly ugly. We need to give them (and only them) this special treatment. */
// TODO: hopefully one day operator "::" will work even in those cases. So check back later, and remove after a sufficient amount of backwards compatibility time
		if ((type & PackageEnv) && (!misplaced)) return (namespace_name + "::" + RObject::rQuote (short_child_name));
		return (getFullName () + '$' + RObject::rQuote (short_child_name));
	}
	return (name + '$' + short_child_name);
}

QString REnvironmentObject::makeChildBaseName (const QString &short_child_name) const {
	RK_TRACE (OBJECTS);

	if (type & ToplevelEnv) {
		return (short_child_name);
	}
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
		if (RKSettingsModuleObjectBrowser::isPackageBlacklisted (namespace_name)) {
			KMessageBox::information (0, i18n ("The package '%1' (probably you just loaded it) is currently blacklisted for retrieving structure information. Practically this means, the objects in this package will not appear in the object browser, and there will be no object name completion or function argument hinting for objects in this package.\nPackages will typically be blacklisted, if they contain huge amount of data, that would take too long to load. To unlist the package, visit Settings->Configure RKWard->Workspace.", namespace_name), i18n("Package blacklisted"), "packageblacklist" + namespace_name);
			return;
		}
	}

	QString options;
	if (type & GlobalEnv) options = ", envlevel=-1";	// in the .GlobalEnv recurse one more level
	if (type & ToplevelEnv) options.append (", namespacename=" + rQuote (namespace_name));

	RCommand *command = new RCommand (".rk.get.structure (" + getFullName () + ", " + rQuote (getShortName ()) + options + ')', RCommand::App | RCommand::Sync | RCommand::GetStructuredData, QString::null, this, ROBJECT_UDPATE_STRUCTURE_COMMAND);
	RKGlobals::rInterface ()->issueCommand (command, chain);
}

bool REnvironmentObject::updateStructure (RData *new_data) {
	RK_TRACE (OBJECTS);
	RK_ASSERT (new_data->getDataType () == RData::StructureVector);
	RK_ASSERT (new_data->getDataLength () >= 5);

	if (!(type & ToplevelEnv)) {
		if (!RObject::updateStructure (new_data)) return false;
	}

	if (new_data->getDataLength () > 5) {
		RK_ASSERT (new_data->getDataLength () == 6);

		RData *children_sub = new_data->getStructureVector ()[5];
		RK_ASSERT (children_sub->getDataType () == RData::StructureVector);
		updateChildren (children_sub);
	} else {
		RK_ASSERT (false);
	}
	return true;
}

void REnvironmentObject::renameChild (RObject *object, const QString &new_name) {
	RK_TRACE (OBJECTS);

	if (type & GlobalEnv) {
		RContainerObject::renameChild (object, new_name);
	} else {
		RK_ASSERT (false);
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
