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
#include "rkmodificationtracker.h"

#define UPDATE_DIM_COMMAND 1

#include "../debug.h"

RKVariable::RKVariable (RContainerObject *parent, const QString &name) : RObject (parent, name) {
	RK_TRACE (OBJECTS);
// TODO: better check, wether it really is one
	RObject::type = Variable;
	var_type = Unknown;
}

RKVariable::~RKVariable () {
	RK_TRACE (OBJECTS);
}

QString RKVariable::getVarTypeString () {
	RK_TRACE (OBJECTS);
	return RObject::typeToText (var_type);
}

void RKVariable::setVarType (RObject::VarType new_type, bool sync) {
	RK_TRACE (OBJECTS);
	var_type = new_type;
	setMetaProperty ("type", QString ().setNum ((int) new_type), sync);
}


QString RKVariable::getTable () {
	RK_TRACE (OBJECTS);
	return parent->getFullName ();
}

void RKVariable::updateFromR () {
	RK_TRACE (OBJECTS);
	
	getMetaData (RKGlobals::rObjectList()->getUpdateCommandChain ());

	RCommand *command = new RCommand ("length (" + getFullName () + ")", RCommand::App | RCommand::Sync | RCommand::GetIntVector, "", this, UPDATE_DIM_COMMAND);
	RKGlobals::rInterface ()->issueCommand (command, RKGlobals::rObjectList()->getUpdateCommandChain ());
}

void RKVariable::rCommandDone (RCommand *command) {
	RK_TRACE (OBJECTS);

	RObject::rCommandDone (command);
	
	if (command->getFlags () == UPDATE_DIM_COMMAND) {
		if (command->intVectorLength () == 1) {
			length = command->getIntVector ()[0];
		} else {
			length = 1;
		}
		
		QString dummy = getMetaProperty ("type");
		int new_var_type = dummy.toInt ();
		var_type = (RObject::VarType) new_var_type;
		if (new_var_type != var_type) RKGlobals::tracker ()->objectMetaChanged (this, 0);

		parent->childUpdateComplete ();
	}
}
