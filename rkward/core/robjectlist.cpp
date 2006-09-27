/***************************************************************************
                          robjectlist  -  description
                             -------------------
    begin                : Wed Aug 18 2004
    copyright            : (C) 2004, 2006 by Thomas Friedrichsmeier
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
#define CHILD_GET_TYPE_COMMAND 2
#define LOAD_COMPLETE_COMMAND 3

#include <qtimer.h>
#include <qstringlist.h>

#include <klocale.h>

#include "rkvariable.h"
#include "rfunctionobject.h"

#include "../rbackend/rinterface.h"
#include "rkmodificationtracker.h"

#include "../rkglobals.h"

#include "../debug.h"

RObjectList::RObjectList () : RContainerObject (0, QString::null) {
	RK_TRACE (OBJECTS);
	update_timer = new QTimer (this);
	
	connect (update_timer, SIGNAL (timeout ()), this, SLOT (timeout ()));
	
	//update_timer->start (AUTO_UPDATE_INTERVAL, true);
	
	type = RObject::Workspace;
	
	update_chain = 0;
}


RObjectList::~RObjectList () {
	RK_TRACE (OBJECTS);
}

void RObjectList::rCommandDone (RCommand *command) {
	RK_TRACE (OBJECTS);

	bool changed = false;

	if (command->getFlags () == UPDATE_LIST_COMMAND) {
		// first check, whether all known children still exist:
		checkRemovedChildren (command->getStringVector (), command->stringVectorLength ());
		
		// next, update the existing and/or new children
		num_children_updating = command->stringVectorLength ();		// TODO: is this correct? Some children might have been removed!
		// empty workspace?
		if (!num_children_updating) {
			num_children_updating = 1;
			childUpdateComplete ();
			return;
		}
		for (int i = 0; i < command->stringVectorLength (); ++i) {
			QString cname = command->getStringVector ()[i];		// for easier typing
			/*if (cname == (".rk.meta")) {
				childUpdateComplete ();
				continue;
			}*/
			if (childmap.find (cname) != childmap.end ()) {
				childmap[cname]->updateFromR ();
			} else {
				createFromR (this, cname);
				changed = true;
			}
		}
	} else if (command->getFlags () == CHILD_GET_TYPE_COMMAND) {
		if (command->intVectorLength () != 1) {
			RK_ASSERT (false);
		}

		PendingObject *pobj = pending_objects[command];
		RObject *robj;
		// TODO: handle more special types!
		if (command->getIntVector ()[0] == 1) {
			robj = new RContainerObject (pobj->parent, pobj->name);
		} else if (command->getIntVector ()[0] == 2) {
			robj = new RFunctionObject (pobj->parent, pobj->name);
		} else {
			robj = new RKVariable (pobj->parent, pobj->name);
		}
		RK_ASSERT (robj);
		pobj->parent->addChild (robj, pobj->name);
		delete pobj;
		pending_objects.remove (command);
		RKGlobals::tracker ()->addObject (robj, 0);
	}
	
	// TODO: signal change
}

void RObjectList::updateFromR () {
	RK_TRACE (OBJECTS);
	if (update_chain) {
		// gee, looks like another update is still on the way. lets schedule one for later:
		update_timer->start (UPDATE_DELAY_INTERVAL, true);
		RK_DO (qDebug ("another object-list update is already running (%d children still updating). Rescheduling a further update for later", num_children_updating), OBJECTS, DL_DEBUG);
		return;
	}

	emit (updateStarted ());
	update_chain = RKGlobals::rInterface ()->startChain (0);

	RCommand *command = new RCommand ("ls (all.names=TRUE)", RCommand::App | RCommand::Sync | RCommand::GetStringVector, QString::null, this, UPDATE_LIST_COMMAND);
	RKGlobals::rInterface ()->issueCommand (command, update_chain);
}

 void RObjectList::createFromR (RContainerObject *parent, const QString &cname) {
	RK_TRACE (OBJECTS);
 	PendingObject *obj = new PendingObject;
	obj->name = cname;
	obj->parent = parent;
	
	QString fullname = parent->makeChildName (cname);

	RCommand *command = new RCommand (".rk.get.type (" + fullname + ")", RCommand::App | RCommand::Sync | RCommand::GetIntVector, QString::null, this, CHILD_GET_TYPE_COMMAND);
	pending_objects.insert (command, obj);
	RKGlobals::rInterface ()->issueCommand (command, update_chain);
}

void RObjectList::childUpdateComplete () {
	RK_TRACE (OBJECTS);
	RK_ASSERT (num_children_updating);
	if ((--num_children_updating) <= 0) {
		RK_TRACE (OBJECTS);

		RK_ASSERT (update_chain);
		RKGlobals::rInterface ()->closeChain (update_chain);
		update_chain = 0;

		RK_DO (qDebug ("object list update complete"), OBJECTS, DL_DEBUG);
		emit (updateComplete ());
	}
}

void RObjectList::timeout () {
	RK_TRACE (OBJECTS);
	updateFromR ();
}

void RObjectList::renameChild (RObject *object, const QString &new_name) {
	RK_TRACE (OBJECTS);

	RObjectMap::iterator it = childmap.find (object->getShortName ());
	RK_ASSERT (it.data () == object);
	
	RCommand *command = new RCommand (makeChildName (new_name) + " <- " + object->getFullName ());
	RKGlobals::rInterface ()->issueCommand (command, 0);
	command = new RCommand ("remove (" + object->getFullName () + ")", RCommand::App | RCommand::Sync);
	RKGlobals::rInterface ()->issueCommand (command, 0);
	
	childmap.remove (it);
	childmap.insert (new_name, object);

	object->name = new_name;
}

void RObjectList::removeChild (RObject *object, bool removed_in_workspace) {
	RK_TRACE (OBJECTS);

	RObjectMap::iterator it = childmap.find (object->getShortName ());
	RK_ASSERT (it.data () == object);
	
	if (!removed_in_workspace) {
		RCommand *command = new RCommand ("remove (" + object->getFullName () + ")", RCommand::App | RCommand::Sync);
		RKGlobals::rInterface ()->issueCommand (command, 0);
	}
	
	childmap.remove (it);
	delete object;
}

#include "robjectlist.moc"
