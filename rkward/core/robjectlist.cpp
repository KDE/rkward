/***************************************************************************
                          robjectlist  -  description
                             -------------------
    begin                : Wed Aug 18 2004
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
#include "robjectlist.h"

#define AUTO_UPDATE_INTERVAL 10000

#define UPDATE_LIST_COMMAND 1
#define GET_TYPE_COMMAND 2

#include <qtimer.h>

#include "rkvariable.h"

#include "../rbackend/rinterface.h"
#include "rcommand.h"

#include "../rkglobals.h"

#include "../debug.h"

RObjectList::RObjectList () : RContainerObject (0, "") {
	update_timer = new QTimer (this);
	
	connect (update_timer, SIGNAL (timeout ()), this, SLOT (timeout ()));
	
	update_timer->start (AUTO_UPDATE_INTERVAL, true);
	
	container_type = RObject::Workspace;
}


RObjectList::~RObjectList () {
}

void RObjectList::gotRResult (RCommand *command) {
	bool changed = false;

	if (command->getFlags () == UPDATE_LIST_COMMAND) {
		for (int i = 0; i < command->stringVectorLength (); ++i) {
			QString cname = command->getStringVector ()[i];
			if (childmap.find (cname) != childmap.end ()) {
				childmap[cname]->updateFromR ();
			} else {
				createFromR (this, cname);
				changed = true;
			}
		}
	} else if (command->getFlags () == GET_TYPE_COMMAND) {
		bool container = false;
		
		if (command->intVectorLength () != 4) {
			RK_ASSERT (false);
		} else {
			if (command->getIntVector ()[0] + command->getIntVector ()[1] + command->getIntVector ()[2] + command->getIntVector ()[3]) {
				container = true;
			}
		}

		PendingObject *pobj = pending_objects[command];
		RObject *robj;
		if (container) {
			robj = new RContainerObject (pobj->parent, pobj->name);
		} else {
			robj = new RKVariable (pobj->parent, pobj->name);
		}
		pobj->parent->addChild (robj, pobj->name);
		delete pobj;
		pending_objects.remove (command);
	}
	
	// TODO: signal change
}

void RObjectList::updateFromR () {
	RCommand *command = new RCommand ("ls (all.names=TRUE)", RCommand::App | RCommand::Sync | RCommand::GetStringVector, "", this, SLOT (gotRResult (RCommand *)), UPDATE_LIST_COMMAND);
	RKGlobals::rInterface ()->issueCommand (command, 0);
}

 void RObjectList::createFromR (RContainerObject *parent, const QString &cname) {
 	PendingObject *obj = new PendingObject;
	obj->name = cname;
	obj->parent = parent;
	
	QString fullname;
	if (parent != this) {
		fullname = parent->getFullName () + "[[\"" + cname + "\"]]";
	} else {
		fullname = cname;
	}
	
	RCommand *command = new RCommand ("c (is.data.frame (" + fullname + "), is.matrix (" + fullname + "), is.array (" + fullname + "), is.list (" + fullname + "))", RCommand::App | RCommand::Sync | RCommand::GetIntVector, "", this, SLOT (gotRResult (RCommand *)), GET_TYPE_COMMAND);
	pending_objects.insert (command, obj);
	RKGlobals::rInterface ()->issueCommand (command, 0);
 }

void RObjectList::timeout () {
	updateFromR ();
}
#include "robjectlist.moc"
