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

#define WORKSPACE_LOAD_COMMAND 3

#include <qtimer.h>

#include <kio/netaccess.h>

#include "rkvariable.h"

#include "../rbackend/rinterface.h"
#include "../dataeditor/rkeditor.h"
#include "../rkeditormanager.h"

#include "../rkglobals.h"
#include "../rkward.h"

#include "../debug.h"

RObjectList::RObjectList () : RContainerObject (0, "") {
	update_timer = new QTimer (this);
	
	connect (update_timer, SIGNAL (timeout ()), this, SLOT (timeout ()));
	
	//update_timer->start (AUTO_UPDATE_INTERVAL, true);
	
	type = RObject::Workspace;
	
	update_chain = 0;
}


RObjectList::~RObjectList () {
}

void RObjectList::rCommandDone (RCommand *command) {
	bool changed = false;

	if (command->getFlags () == UPDATE_LIST_COMMAND) {
		num_children_updating = command->stringVectorLength ();
		// empty workspace?
		if (!num_children_updating) {
			update_chain = RKGlobals::rInterface ()->closeChain (update_chain);
			emit (updateComplete (true));
			return;
		}
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
		RK_ASSERT (robj);
		pobj->parent->addChild (robj, pobj->name);
		delete pobj;
		pending_objects.remove (command);
		
	} else if (command->getFlags () == WORKSPACE_LOAD_COMMAND) {
		KIO::NetAccess::removeTempFile (tmpfile);
	}
	
	// TODO: signal change
}

void RObjectList::updateFromR () {
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
 	PendingObject *obj = new PendingObject;
	obj->name = cname;
	obj->parent = parent;
	
	QString fullname = parent->makeChildName (cname);
	
	RCommand *command = new RCommand ("c (is.data.frame (" + fullname + "), is.matrix (" + fullname + "), is.array (" + fullname + "), is.list (" + fullname + "))", RCommand::App | RCommand::Sync | RCommand::GetIntVector, "", this, GET_TYPE_COMMAND);
	pending_objects.insert (command, obj);
	RKGlobals::rInterface ()->issueCommand (command, update_chain);
}

void RObjectList::childUpdateComplete () {
	RK_TRACE (APP);
	RK_ASSERT (num_children_updating);
	if ((--num_children_updating) <= 0) {
		RK_TRACE (APP);

		RK_ASSERT (update_chain);
		update_chain = RKGlobals::rInterface ()->closeChain (update_chain);
		RK_ASSERT (!update_chain);

	// TODO: check whether there really were any changes
		emit (updateComplete (true));
	}
}

void RObjectList::saveWorkspace (const KURL& url) {
	RCommandChain *chain = RKGlobals::rInterface ()->startChain (0);
	
	RKGlobals::editorManager ()->syncAllToR (chain);
	RKGlobals::rInterface ()->issueCommand (new RCommand ("save.image (\"" + url.path () + "\")", RCommand::App), chain);
	
	RKGlobals::rInterface ()->startChain (chain);
}

void RObjectList::loadWorkspace (const KURL& url) {
	KIO::NetAccess::download (url, tmpfile, RKGlobals::rkApp ());
	
	RCommand *command = new RCommand ("load (\"" + url.path () + "\")", RCommand::App, "", this, WORKSPACE_LOAD_COMMAND);
	RKGlobals::rInterface ()->issueCommand (command);
	updateFromR ();
}

void RObjectList::timeout () {
	updateFromR ();
}
#include "robjectlist.moc"
