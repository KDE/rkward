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

#include "../debug.h"

REnvironmentObject::REnvironmentObject (RContainerObject *parent, const QString &name) : RContainerObject (parent, name) {
	RK_TRACE (OBJECTS);

	type = Environment;
	if (name == ".GlobalEnv") {
		type |= GlobalEnv;
	}

	// TODO: determine namespace_name
	// TODO: determine if this is an environment var (or maybe this is done from the parent)
}

REnvironmentObject::~REnvironmentObject () {
	RK_TRACE (OBJECTS);
}

QString REnvironmentObject::getFullName () {
	RK_TRACE (OBJECTS);

	if (type & EnvironmentVar) return (parent->makeChildName (name));
	return ("as.environment (\"" + name + "\")");
}

QString REnvironmentObject::makeChildName (const QString &short_child_name) {
	RK_TRACE (OBJECTS);

	if (type & GlobalEnv) return (short_child_name);
	if (type & EnvironmentVar) return (name + "$" + short_child_name);
	return (namespace_name + "::" + short_child_name);
}

void REnvironmentObject::writeMetaData (RCommandChain *chain) {
	RK_TRACE (OBJECTS);

	if (type & EnvironmentVar) RContainerObject::writeMetaData (chain);
}

QString REnvironmentObject::listChildrenCommand () {
	RK_TRACE (OBJECTS);

	return ("ls (as.environment (" + getFullName () + ", all.names=TRUE)");
}

void REnvironmentObject::renameChild (RObject *object, const QString &new_name) {
	RK_TRACE (OBJECTS);

	if (type & GlobalEnv) {
		RObjectMap::iterator it = childmap.find (object->getShortName ());
		RK_ASSERT (it.data () == object);
		
		RCommand *command = new RCommand (makeChildName (new_name) + " <- " + object->getFullName ());
		RKGlobals::rInterface ()->issueCommand (command, 0);
		command = new RCommand ("remove (" + object->getFullName () + ")", RCommand::App | RCommand::Sync);
		RKGlobals::rInterface ()->issueCommand (command, 0);
		
		childmap.remove (it);
		childmap.insert (new_name, object);
	
		object->name = new_name;
	} else {
		RK_ASSERT (false);
	}
}

void REnvironmentObject::removeChild (RObject *object, bool removed_in_workspace) {
	RK_TRACE (OBJECTS);

	if ((type & GlobalEnv) || removed_in_workspace) {
		RObjectMap::iterator it = childmap.find (object->getShortName ());
		RK_ASSERT (it.data () == object);
		
		if (!removed_in_workspace) {
			RCommand *command = new RCommand ("remove (" + object->getFullName () + ")", RCommand::App | RCommand::Sync);
			RKGlobals::rInterface ()->issueCommand (command, 0);
		}
		
		childmap.remove (it);
		delete object;
	} else {
		RK_ASSERT (false);
	}
}
