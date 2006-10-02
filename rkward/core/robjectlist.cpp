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

#define UPDATE_WORKSPACE_COMMAND 1

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

void RObjectList::updateFromR () {
	RK_TRACE (OBJECTS);
	if (update_chain) {
		// gee, looks like another update is still on the way. lets schedule one for later:
		update_timer->start (UPDATE_DELAY_INTERVAL, true);
		RK_DO (qDebug ("another object-list update is already running. Rescheduling a further update for later"), OBJECTS, DL_DEBUG);
		return;
	}

	emit (updateStarted ());
	update_chain = RKGlobals::rInterface ()->startChain (0);

	RCommand *command = new RCommand (".rk.get.environment.structure (as.environment (\".GlobalEnv\"))", RCommand::App | RCommand::Sync | RCommand::GetStructuredData, QString::null, this, ROBJECT_UDPATE_STRUCTURE_COMMAND);
	RKGlobals::rInterface ()->issueCommand (command, update_chain);
}

bool RObjectList::updateStructure (RData *new_data) {
	RK_TRACE (OBJECTS);
	RK_ASSERT (new_data->getDataType () == RData::StructureVector);

//	if (!RObject::updateStructure (new_data)) return false;		// this is the workspace object. nothing to update
	updateChildren (new_data);		// children are directly in the structure

	RK_ASSERT (update_chain);
	RKGlobals::rInterface ()->closeChain (update_chain);
	update_chain = 0;

	RK_DO (qDebug ("object list update complete"), OBJECTS, DL_DEBUG);
	emit (updateComplete ());

	return true;
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
