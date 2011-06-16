/***************************************************************************
                          rkbackendtransmitter  -  description
                             -------------------
    begin                : Thu Nov 18 2010
    copyright            : (C) 2010 by Thomas Friedrichsmeier
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

#include "rkbackendtransmitter.h"

#include "rkrbackend.h"

#include <QLocalSocket>

#include "../version.h"
#include "../debug.h"

RKRBackendTransmitter::RKRBackendTransmitter (const QString &servername, const QString &token) {
	RK_TRACE (RBACKEND);

	RKRBackendTransmitter::servername = servername;
	RKRBackendTransmitter::token = token;
}

RKRBackendTransmitter::~RKRBackendTransmitter () {
	RK_TRACE (RBACKEND);
	if (!current_sync_requests.isEmpty ()) {
		RK_DO (qDebug ("%d pending requests while exiting RKRBackendTransmitter", current_sync_requests.size ()), RBACKEND, DL_WARNING);
	}

	if (!connection) return;

	// To prevent closing the process before the frontend has had a chance to see the QuitCommand
	if (connection->bytesToWrite ()) connection->waitForBytesWritten (1000);
	msleep (1000);
}

void RKRBackendTransmitter::timerEvent (QTimerEvent *) {
	// do not trace
	flushOutput (false);
}

void RKRBackendTransmitter::run () {
	RK_TRACE (RBACKEND);

	QLocalSocket* con = new QLocalSocket (this);
	con->connectToServer (servername);
	setConnection (con);
	
	if (!connection->waitForConnected ()) handleTransmissionError ("Could not connect: " + connection->errorString ());
	// handshake
	connection->write (token.toLocal8Bit ().data ());
	connection->write ("\n");
	connection->write (RKWARD_VERSION);
	connection->write ("\n");
	connection->waitForBytesWritten ();

	startTimer (200);	// calls flushOutput(false), periodically. See timerEvent()

	exec ();
}

void RKRBackendTransmitter::writeRequest (RBackendRequest *request) {
	RK_TRACE (RBACKEND);

	if (request->type != RBackendRequest::Output) flushOutput (true);
	transmitRequest (request);
	connection->flush ();

	if (request->synchronous) {
		current_sync_requests.append (request);
		RK_DO (qDebug ("Expecting replies for %d requests (added %p)", current_sync_requests.size (), request), RBACKEND, DL_DEBUG);
	} else {
		delete request;
	}
}

void RKRBackendTransmitter::requestReceived (RBackendRequest* request) {
	RK_TRACE (RBACKEND);

	// first check for requests which originated in the frontend
	if (request->type == RBackendRequest::Interrupt) {
		RKRBackend::this_pointer->interruptCommand (request->params.value ("commandid", -1).toInt ());
	// requests which originated in the backend below this line
	} else {
		if (current_sync_requests.isEmpty ()) {
			RK_ASSERT (false);
			return;
		}

		RBackendRequest* current_sync_request = current_sync_requests.takeFirst ();
		if (current_sync_request->type == RBackendRequest::Output) {
			delete current_sync_request;	// this was just our internal request
		} else {
			current_sync_request->mergeReply (request);
			current_sync_request->done = true;
		}
		RK_DO (qDebug ("Expecting replies for %d requests (popped %p)", current_sync_requests.size (), current_sync_request), RBACKEND, DL_DEBUG);
	}
	delete request;
}

void RKRBackendTransmitter::flushOutput (bool force) {
	if (!current_sync_requests.isEmpty ()) return;

	RKRBackend::this_pointer->fetchStdoutStderr (force);
	ROutputList out = RKRBackend::this_pointer->flushOutput (force);
	if (out.isEmpty ()) return;

	RK_TRACE (RBACKEND);
	// output request would not strictly need to be synchronous. However, making them synchronous ensures that the frontend is keeping up with the output sent by the backend.
	RBackendRequest* request = new RBackendRequest (true, RBackendRequest::Output);
	request->output = new ROutputList (out);
	writeRequest (request);
}

void RKRBackendTransmitter::handleTransmissionError (const QString &message) {
	RK_TRACE (RBACKEND);

	RK_DO (qDebug ("%s", qPrintable ("Transmission error " + message)), RBACKEND, DL_ERROR);
	RKRBackend::tryToDoEmergencySave ();
}

#include "rkbackendtransmitter.moc"
