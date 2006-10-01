/***************************************************************************
                          rfunctionobject  -  description
                             -------------------
    begin                : Wed Apr 26 2006
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

#include "rfunctionobject.h"

#include "../rbackend/rinterface.h"
#include "robjectlist.h"
#include "../rkglobals.h"
#include "../debug.h"

#define CLASSIFY_COMMAND 1
#define UPDATE_ARGS_COMMAND 2

RFunctionObject::RFunctionObject (RContainerObject *parent, const QString &name) : RObject (parent, name) {
	RK_TRACE (OBJECTS);
	type = Function;
}

RFunctionObject::~RFunctionObject () {
	RK_TRACE (OBJECTS);
}

void RFunctionObject::updateFromR () {
	RK_TRACE (OBJECTS);

// TODO: move classification / type mismatch-checking to RObject
	RCommand *command = new RCommand (".rk.classify (" + getFullName () + ")", RCommand::App | RCommand::Sync | RCommand::GetIntVector, QString::null, this, CLASSIFY_COMMAND);
	RKGlobals::rInterface ()->issueCommand (command, RKGlobals::rObjectList()->getUpdateCommandChain ());
}

void RFunctionObject::rCommandDone (RCommand *command) {
	RK_TRACE (OBJECTS);

	bool dummy;
	if (command->getFlags () == CLASSIFY_COMMAND) {
		if (!handleClassifyCommand (command, &dummy)) {
			return; // will be deleted!
		}

		RCommand *command = new RCommand ("c (as.character (names (formals (" + getFullName () +"))), as.character (formals (" +getFullName () + ")))", RCommand::App | RCommand::Sync | RCommand::GetStringVector, QString::null, this, UPDATE_ARGS_COMMAND);
		RKGlobals::rInterface ()->issueCommand (command, RKGlobals::rObjectList()->getUpdateCommandChain ());

	} else if (command->getFlags () == UPDATE_ARGS_COMMAND) {
		RK_ASSERT (command->getDataLength () % 2 == 0);

		function_args.clear ();
		for (unsigned int i = 0; i < command->getDataLength (); i += 2) {
			function_args.append (new FunctionArg (command->getStringVector ()[i], command->getStringVector ()[i+1]));
		}

		parent->childUpdateComplete ();
	}
}
