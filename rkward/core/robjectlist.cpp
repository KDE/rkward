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
#define UPDATE_DELAY_INTERVAL 500

#define UPDATE_LIST_COMMAND 1
#define GET_TYPE_COMMAND 2

#include <qtimer.h>

#include "rkvariable.h"

#include "../rbackend/rinterface.h"

#include "../rkglobals.h"

#include "../debug.h"

RObjectList::RObjectList () : RContainerObject (0, "") {
	update_timer = new QTimer (this);
	
	connect (update_timer, SIGNAL (timeout ()), this, SLOT (timeout ()));
	
	update_timer->start (AUTO_UPDATE_INTERVAL, true);
	
	container_type = RObject::Workspace;
	
	command_chain = 0;
}


RObjectList::~RObjectList () {
}

void RObjectList::rCommandDone (RCommand *command) {
	bool changed = false;

	if (command->getFlags () == UPDATE_LIST_COMMAND) {
		num_children_updating = command->stringVectorLength ();
		for (int i = 0; i < num_children_updating; ++i) {
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
	if (command_chain) {
		// gee, looks like another update is still on the way. lets schedule one for later:
		update_timer->start (UPDATE_DELAY_INTERVAL, true);
		return;
	}

	command_chain = RKGlobals::rInterface ()->startChain (0);

	RCommand *command = new RCommand ("ls (all.names=TRUE)", RCommand::App | RCommand::Sync | RCommand::GetStringVector, "", this, UPDATE_LIST_COMMAND);
	RKGlobals::rInterface ()->issueCommand (command, command_chain);
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
	
	RCommand *command = new RCommand ("c (is.data.frame (" + fullname + "), is.matrix (" + fullname + "), is.array (" + fullname + "), is.list (" + fullname + "))", RCommand::App | RCommand::Sync | RCommand::GetIntVector, "", this, GET_TYPE_COMMAND);
	pending_objects.insert (command, obj);
	RKGlobals::rInterface ()->issueCommand (command, command_chain);
}

void RObjectList::childUpdateComplete () {
	RK_TRACE (APP);
	RK_ASSERT (num_children_updating);
	if ((--num_children_updating) <= 0) {
		RK_TRACE (APP);

		RK_ASSERT (command_chain);
		command_chain = RKGlobals::rInterface ()->closeChain (command_chain);
		RK_ASSERT (!command_chain);

	// TODO: check whether there really were any changes
		emit (updateComplete (true));
	}
}

void RObjectList::timeout () {
	updateFromR ();
}
#include "robjectlist.moc"
