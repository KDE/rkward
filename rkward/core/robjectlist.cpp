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
#define UPDATE_CLASS_COMMAND 2
#define UPDATE_TYPE_COMMAND 3
#define UPDATE_DIM_COMMAND 4

#include <qtimer.h>

#include "rbackend/rinterface.h"
#include "rcommand.h"

#include "rkglobals.h"

#include "debug.h"

RObjectList::RObjectList () : QObject () {
	update_timer = new QTimer (this);
	
	connect (update_timer, SIGNAL (timeout ()), this, SLOT (timeout ()));
	
	update_timer->start (AUTO_UPDATE_INTERVAL, true);
}


RObjectList::~RObjectList () {
}

void RObjectList::updateObject (char *name) {
	UpdatingObject *obj = new UpdatingObject;
	obj->name = name;
	obj->num_classes = 0;
	obj->num_dimensions = 0;
	obj->classname = 0;
	obj->dimension = 0;
	obj->type = 0;

	RCommand *command = new RCommand ("class (" + obj->name + ")", RCommand::App | RCommand::Sync | RCommand::GetStringVector, "", this, SLOT (receivedROutput (RCommand *)), UPDATE_CLASS_COMMAND);
	RKGlobals::rInterface ()->issueCommand (command, 0);
	update_map.insert (command, obj);

	command = new RCommand ("c (is.data.frame (" + obj->name + "), is.matrix (" + obj->name + "), is.array (" + obj->name + "), is.list (" + obj->name + "))", RCommand::App | RCommand::Sync | RCommand::GetIntVector, "", this, SLOT (receivedROutput (RCommand *)), UPDATE_TYPE_COMMAND);
	RKGlobals::rInterface ()->issueCommand (command, 0);
	update_map.insert (command, obj);
	
	command = new RCommand ("dim (" + obj->name + ")", RCommand::App | RCommand::Sync | RCommand::GetIntVector, "", this, SLOT (receivedROutput (RCommand *)), UPDATE_DIM_COMMAND);
	RKGlobals::rInterface ()->issueCommand (command, 0);
	update_map.insert (command, obj);	
}

void RObjectList::receivedROutput (RCommand *command) {
	if (command->getFlags () == UPDATE_LIST_COMMAND) {
		for (int i = 0; i < command->stringVectorLength (); ++i) {
			updateObject (command->getStringVector ()[i]);
		}
	} else if (command->getFlags () == UPDATE_CLASS_COMMAND) {

		UpdatingObject *obj = update_map[command];
		obj->classname = new QString [obj->num_classes = command->stringVectorLength ()];
		for (int cn=0; cn < obj->num_classes; ++cn) {
			obj->classname[cn] = command->getStringVector ()[cn];
		}
		update_map.remove (command);
	
	} else if (command->getFlags () == UPDATE_TYPE_COMMAND) {

		UpdatingObject *obj = update_map[command];
		if (command->intVectorLength () != 4) {
			RK_ASSERT (false);
		} else {
			if (command->getIntVector ()[0]) {
				obj->type = DataFrame | Matrix | Array | List;
			} else if (command->getIntVector ()[1]) {
				obj->type = Matrix | Array | List;
			} else if (command->getIntVector ()[2]) {
				obj->type = Array | List;
			} else if (command->getIntVector ()[3]) {
				obj->type = List;
			}
		}
		update_map.remove (command);

	} else if (command->getFlags () == UPDATE_DIM_COMMAND) {

		UpdatingObject *obj = update_map[command];
		obj->dimension = new int [obj->num_dimensions = command->intVectorLength ()];
		for (int d=0; d < obj->num_dimensions; ++d) {
			obj->dimension[d] = command->getIntVector ()[d];
		}
		update_map.remove (command);

		/// TODO: remove debug code from here!
		RK_DO (qDebug ("object name: %s", obj->name.latin1 ()), APP, DL_DEBUG);
		RK_DO (qDebug ("%s", "class names:"), APP, DL_DEBUG);
		for (int a = 0; a < obj->num_classes; ++a) {
			RK_DO (qDebug ("%d: %s", a, obj->classname[a].latin1 ()), APP, DL_DEBUG);
		}
		RK_DO (qDebug ("%s", "dimensions:"), APP, DL_DEBUG);
		for (int a = 0; a < obj->num_dimensions; ++a) {
			RK_DO (qDebug ("%d: %d", a, obj->dimension[a]), APP, DL_DEBUG);
		}
		RK_DO (qDebug ("%s %d", "type:", obj->type), APP, DL_DEBUG);
		delete obj;
	}
}

void RObjectList::updateList () {
	RCommand *command = new RCommand ("ls (all.names=TRUE)", RCommand::App | RCommand::Sync | RCommand::GetStringVector, "", this, SLOT (receivedROutput (RCommand *)), UPDATE_LIST_COMMAND);
	RKGlobals::rInterface ()->issueCommand (command, 0);
}

void RObjectList::timeout () {
	updateList ();
}
#include "robjectlist.moc"
