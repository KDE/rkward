/***************************************************************************
                          rrequesthandler  -  description
                             -------------------
    begin                : Fri Sep 3 2004
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
#include "rrequesthandler.h"

#include <qtextstream.h>

#include "../core/robjectlist.h"
#include "../core/rkmodificationtracker.h"
#include "../rkglobals.h"

#include "../debug.h"

RRequestHandler::RRequestHandler (int socket, QObject *parent) : QSocket (parent) {
	RK_TRACE (RBACKEND);
	connect (this, SIGNAL (readyRead ()), SLOT (readFromR ()));
	connect (this, SIGNAL (connectionClosed ()), SLOT(connectionTerminated ()));
	setSocket (socket);
}

RRequestHandler::~RRequestHandler () {
	RK_TRACE (RBACKEND);
}

void RRequestHandler::readFromR () {
	RK_TRACE (RBACKEND);
	
	QTextStream stream (this);
	QString request;
	
	if (canReadLine ()) {			// request must be single line for now!
		request = stream.readLine ();
		
		RK_DO (qDebug ("read from R: %s", request.latin1 ()), RBACKEND, DL_DEBUG);
		RK_ASSERT (!canReadLine ());
	} else {
		return;
	}
	
	QString call = request.section ("\t", 0, 0);
	
	if (call == "sync") {
		QString object_name = request.section ("\t", 1, 1);
		RObject *obj = RKGlobals::rObjectList ()->findObject (object_name);
		if (!obj) {
			RK_DO (qDebug ("sync requested for unknown object: %s", object_name.latin1 ()), RBACKEND, DL_WARNING);
			return;
		}
		
// TODO: make proper ChangeSet!
		RObject::ChangeSet *set = new RObject::ChangeSet;
		set->from_index = -1;
		set->to_index = -1;
		RKGlobals::tracker ()->objectMetaChanged (obj);
		RKGlobals::tracker ()->objectDataChanged (obj, set);
	} else {
		RK_DO (qDebug ("unrecognized request from R: %s", request.latin1 ()), RBACKEND, DL_DEBUG);
	}
	
	stream << "#RKEND#" << endl;
}

void RRequestHandler::connectionTerminated () {
	RK_TRACE (RBACKEND);
	RK_DO (qDebug ("R Connection terminated"), RBACKEND, DL_DEBUG);
}

#include "rrequesthandler.moc"
