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
#include "robjectlist.h"
#include "../rbackend/rinterface.h"
#include "../rkglobals.h"

#include "../debug.h"

REnvironmentObject::REnvironmentObject (RContainerObject *parent, const QString &name) : RContainerObject (parent, name) {
	RK_TRACE (OBJECTS);

	type = Environment;
	if (parent == RObjectList::getObjectList ()) {
		type |= ToplevelEnv;
	}

	// TODO: determine namespace_name
	// TODO: determine if this is an environment var (or maybe this is done from the parent)
}

REnvironmentObject::~REnvironmentObject () {
	RK_TRACE (OBJECTS);
}

QString REnvironmentObject::getFullName () {
	RK_TRACE (OBJECTS);

	if (type & ToplevelEnv) return ("as.environment (\"" + name + "\")");
	return parent->makeChildName (name, type & Misplaced);
}

QString REnvironmentObject::makeChildName (const QString &short_child_name, bool misplaced) {
	RK_TRACE (OBJECTS);

	if (type & GlobalEnv) return (short_child_name);
	if (type & ToplevelEnv) {
/* Some items are placed outside of their native namespace. E.g. in package:boot item "motor". It can be retrieved using as.environment ("package:boot")$motor. This is extremly ugly. We need to give them (and only them) this special treatment. */
		if (misplaced) return (getFullName () + "$" + RObject::rQuote (short_child_name));
		return (namespace_name + "::" + RObject::rQuote (short_child_name));
	}
	return (name + "$" + short_child_name);
}

void REnvironmentObject::writeMetaData (RCommandChain *chain) {
	RK_TRACE (OBJECTS);

	if (type & ToplevelEnv) return;
	RContainerObject::writeMetaData (chain);
}

void REnvironmentObject::updateFromR () {
	RK_TRACE (OBJECTS);
	QString options;
	if (type & GlobalEnv) options = ", envleve=-1";	// in the .GlobalEnv recurse one more level
	if (type & ToplevelEnv) options.append (", namespacename=" + rQuote (namespace_name));

	RCommand *command = new RCommand (".rk.get.structure (" + getFullName () + ", " + rQuote (getShortName ()) + options + ")", RCommand::App | RCommand::Sync | RCommand::GetStructuredData, QString::null, this, ROBJECT_UDPATE_STRUCTURE_COMMAND);
	RKGlobals::rInterface ()->issueCommand (command, RObjectList::getObjectList ()->getUpdateCommandChain ());
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

void REnvironmentObject::removeChild (RObject *object, bool removed_in_workspace) {
	RK_TRACE (OBJECTS);

	if ((type & GlobalEnv) || removed_in_workspace) {
		RContainerObject::removeChild (object, removed_in_workspace);
	} else {
		RK_ASSERT (false);
	}
}

QString REnvironmentObject::renameChildCommand (RObject *object, const QString &new_name) {
	RK_TRACE (OBJECTS);

	return (makeChildName (new_name) + " <- " + object->getFullName () + "\n" + removeChildCommand (object));
}

QString REnvironmentObject::removeChildCommand (RObject *object) {
	RK_TRACE (OBJECTS);

	return ("remove (" + object->getFullName () + ")");
}
