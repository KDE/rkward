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
#define CHILD_GET_TYPE_COMMAND 2
#define LOAD_COMPLETE_COMMAND 3

#define WORKSPACE_LOAD_COMMAND 10

#include <qtimer.h>

#include <kio/netaccess.h>
#include <klocale.h>

#include "rkvariable.h"

#include "../rbackend/rinterface.h"
#include "../dataeditor/rkeditor.h"
#include "../rkeditormanager.h"
#include "rkmodificationtracker.h"

#include "../rkglobals.h"
#include "../rkward.h"

#include "../debug.h"

RObjectList::RObjectList () : RContainerObject (0, "") {
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

	RObject::rCommandDone (command);

	bool changed = false;

	if (command->getFlags () == UPDATE_LIST_COMMAND) {
		// first check, whether all known children still exist:
		checkRemovedChildren (command->getStringVector (), command->stringVectorLength ());
		
		// next, update the existing and/or new children
		num_children_updating = command->stringVectorLength ();
		// empty workspace?
		if (!num_children_updating) {
			update_chain = RKGlobals::rInterface ()->closeChain (update_chain);
			emit (updateComplete ());
			return;
		}
		for (int i = 0; i < command->stringVectorLength (); ++i) {
			QString cname = command->getStringVector ()[i];
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
		// TODO: handle special type like functions, etc.!
		if (command->getIntVector ()[0] == 1) {
			robj = new RContainerObject (pobj->parent, pobj->name);
		} else {
			robj = new RKVariable (pobj->parent, pobj->name);
		}
		RK_ASSERT (robj);
		pobj->parent->addChild (robj, pobj->name);
		delete pobj;
		pending_objects.remove (command);
		RKGlobals::tracker ()->addObject (robj, 0);
		
	} else if (command->getFlags () == WORKSPACE_LOAD_COMMAND) {
		KIO::NetAccess::removeTempFile (tmpfile);
	} else if (command->getFlags () == LOAD_COMPLETE_COMMAND) {
		RKGlobals::editorManager ()->restoreEditors ();
	}
	
	// TODO: signal change
}

void RObjectList::updateFromR () {
	RK_TRACE (OBJECTS);
	if (update_chain) {
		// gee, looks like another update is still on the way. lets schedule one for later:
		update_timer->start (UPDATE_DELAY_INTERVAL, true);
		return;
	}

	update_chain = RKGlobals::rInterface ()->startChain (0);

	RCommand *command = new RCommand ("ls (all.names=TRUE)", RCommand::App | RCommand::Sync | RCommand::GetStringVector, "", this, UPDATE_LIST_COMMAND);
	RKGlobals::rInterface ()->issueCommand (command, update_chain);
}

 void RObjectList::createFromR (RContainerObject *parent, const QString &cname) {
	RK_TRACE (OBJECTS);
 	PendingObject *obj = new PendingObject;
	obj->name = cname;
	obj->parent = parent;
	
	QString fullname = parent->makeChildName (cname);
	
	RCommand *command = new RCommand (".rk.get.type (" + fullname + ")", RCommand::App | RCommand::Sync | RCommand::GetIntVector, "", this, CHILD_GET_TYPE_COMMAND);
	pending_objects.insert (command, obj);
	RKGlobals::rInterface ()->issueCommand (command, update_chain);
}

void RObjectList::childUpdateComplete () {
	RK_TRACE (OBJECTS);
	RK_ASSERT (num_children_updating);
	if ((--num_children_updating) <= 0) {
		RK_TRACE (OBJECTS);

		RK_ASSERT (update_chain);
		update_chain = RKGlobals::rInterface ()->closeChain (update_chain);
		RK_ASSERT (!update_chain);

		emit (updateComplete ());
	}
}

void RObjectList::loadWorkspace (const KURL& url, bool merge) {
	RK_TRACE (OBJECTS);
	KIO::NetAccess::download (url, tmpfile, RKGlobals::rkApp ());
	
	RCommand *command;
	
	if (!merge) {
		command = new RCommand ("remove (list=ls (all.names=TRUE))", RCommand::App);
		RKGlobals::rInterface ()->issueCommand (command);
		current_url = url;
	}

	command = new RCommand ("load (\"" + url.path () + "\")", RCommand::App, "", this, WORKSPACE_LOAD_COMMAND);
	RKGlobals::rInterface ()->issueCommand (command);
	updateFromR ();
	
	// since the update is done in the update chain, this command is guaranteed to run after the update is complete
	command = new RCommand ("", RCommand::App | RCommand::EmptyCommand, "", this, LOAD_COMPLETE_COMMAND);
	RKGlobals::rInterface ()->issueCommand (command);
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

void RObjectList::removeChild (RObject *object) {
	RK_TRACE (OBJECTS);

	RObjectMap::iterator it = childmap.find (object->getShortName ());
	RK_ASSERT (it.data () == object);
	
	RCommand *command = new RCommand ("remove (" + object->getFullName () + ")", RCommand::App | RCommand::Sync);
	RKGlobals::rInterface ()->issueCommand (command, 0);
	
	childmap.remove (it);
	delete object;
}

RObject *RObjectList::findObject (const QString &full_name) {
	RK_TRACE (OBJECTS);
	
	// yeah, ok, this could be made more efficient relatively easily ...
	QString canonified = full_name;
	canonified = canonified.replace ("[\"", "$").replace ('[', "").replace ("\"]", "").replace (']', "");
	
	QStringList list = QStringList::split ('$', canonified);
	RContainerObject *cobject = this;
	RObject *object = 0;
	RObjectMap::iterator oit;
	for (QStringList::iterator it = list.begin (); it != list.end ();) {
		oit = cobject->childmap.find (*it);
		if (oit == cobject->childmap.end ()) {
			return 0;
		}
		object = oit.data ();
		++it;
		if (object->isContainer ()) {
			cobject = static_cast<RContainerObject *> (object);
	// found a non-container-object, although path is not finished
		} else {
			if (it != list.end ()) {
				object = 0;
				// end loop
				it = list.end ();
			}
		}
	}
	
	return object;
}

#include "robjectlist.moc"
