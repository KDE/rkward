/***************************************************************************
                          rkvariable  -  description
                             -------------------
    begin                : Thu Aug 12 2004
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
#include "rkvariable.h"

#include "rcontainerobject.h"
#include "robjectlist.h"

#include "../rbackend/rinterface.h"
#include "../rkglobals.h"

#define FIND_META_COMMAND 1
#define UPDATE_DIM_COMMAND 2

#include "../debug.h"

RKVariable::RKVariable (RContainerObject *parent, const QString &name) : RObject (parent, name) {
	RK_TRACE (OBJECTS);
// TODO: better check, wether it really is one
	RObject::type |= Variable;
}

RKVariable::~RKVariable () {
	RK_TRACE (OBJECTS);
}

QString RKVariable::getTypeString () {
	RK_TRACE (OBJECTS);
	return type;
}

QString RKVariable::getTable () {
	RK_TRACE (OBJECTS);
	return parent->getFullName ();
}

void RKVariable::updateFromR () {
	RK_TRACE (OBJECTS);

	RCommand *command = new RCommand ("!is.null (attr (" + getFullName () + ", \".rk.meta\"))", RCommand::App | RCommand::Sync | RCommand::GetIntVector, "", this, FIND_META_COMMAND);
	RKGlobals::rInterface ()->issueCommand (command, RKGlobals::rObjectList()->getUpdateCommandChain ());
}

void RKVariable::rCommandDone (RCommand *command) {
	RK_TRACE (OBJECTS);
	RK_DO (qDebug ("command: %s", command->command ().latin1 ()), OBJECTS, DL_DEBUG);

	RObject::rCommandDone (command);
	
	if (command->getFlags () == FIND_META_COMMAND) {
		if ((command->intVectorLength () == 1) && command->getIntVector ()[0]) {
			RObject::type |= HasMetaObject;
			getMetaData (RKGlobals::rObjectList()->getUpdateCommandChain ());
		} else {
			RObject::type -= (RObject::type & HasMetaObject);
		}
		
		RCommand *ncommand = new RCommand ("length (" + getFullName () + ")", RCommand::App | RCommand::Sync | RCommand::GetIntVector, "", this, UPDATE_DIM_COMMAND);
		RKGlobals::rInterface ()->issueCommand (ncommand, RKGlobals::rObjectList()->getUpdateCommandChain ());

	} else if (command->getFlags () == UPDATE_DIM_COMMAND) {
		if (command->intVectorLength () == 1) {
			length = command->getIntVector ()[0];
		} else {
			length = 1;
		}
		parent->childUpdateComplete ();
	}
}
