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
#include <qdir.h>

#include "../core/robjectlist.h"
#include "../core/rkmodificationtracker.h"
#include "../rkglobals.h"
#include "../settings/rksettingsmodulelogfiles.h"

#include "../debug.h"

RRequestHandler::RRequestHandler (int socket, QObject *parent, bool reject) : QSocket (parent) {
	RK_TRACE (RBACKEND);
	
	setSocket (socket);
	if (reject) {
		RK_DO (qDebug ("Rejecting TCP connection on socket %d, as one connection (hopefully with R) is already established!", socket), RBACKEND, DL_WARNING);
		close ();
	} else {
		connect (this, SIGNAL (readyRead ()), SLOT (readFromR ()));
		connect (this, SIGNAL (connectionClosed ()), SLOT(connectionTerminated ()));
	}
	eof_string = "\n#RKEND#\n";
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
		// for now a complete update is needed, in case new objects were added
		RKGlobals::rObjectList ()->updateFromR ();
		RKGlobals::tracker ()->objectDataChanged (obj, set);
	} else if (call == "checkconnection") {
		stream << "ok";
	} else if (call == "get.tempfile.name") {
		QString file_prefix = request.section ("\t", 1, 1);
		QString file_extension = request.section ("\t", 2, 2);
		QDir dir (RKSettingsModuleLogfiles::filesPath ());
		
		int i=0;
		while (dir.exists (file_prefix + QString::number (i) + file_extension)) {
			i++;
		}
		stream << dir.filePath (file_prefix + QString::number (i) + file_extension);
	} else {
		RK_DO (qDebug ("unrecognized request from R: %s", request.latin1 ()), RBACKEND, DL_DEBUG);
	}
	
	stream << eof_string;
}

void RRequestHandler::connectionTerminated () {
	RK_TRACE (RBACKEND);
	RK_DO (qDebug ("R Connection terminated"), RBACKEND, DL_DEBUG);
}

#include "rrequesthandler.moc"
