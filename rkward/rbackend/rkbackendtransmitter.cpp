/*
rkbackendtransmitter - This file is part of RKWard (https://rkward.kde.org). Created: Thu Nov 18 2010
SPDX-FileCopyrightText: 2010-2013 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "rkbackendtransmitter.h"

#include "rkrbackend.h"

#include <QLocalSocket>

#include <iostream>

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
		RK_DEBUG (RBACKEND, DL_WARNING, "%d pending requests while exiting RKRBackendTransmitter", current_sync_requests.size ());
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

	int timeout = 0;
	do {
		RK_DEBUG(RBACKEND, DL_DEBUG, "Connecting to local socket %s", qPrintable(servername));
		std::cout << token.toLocal8Bit().data() << "\n";
		std::cout.flush();
	} while (!connection->waitForConnected(1000) && (++timeout < 20));
	if (timeout >= 20) handleTransmissionError("Could not connect: " + connection->errorString());

	// handshake
	RK_DEBUG(RBACKEND, DL_DEBUG, "Connection state: %d. Now Sending handshake", con->state());
	connection->write (token.toLocal8Bit ().data ());
	connection->write ("\n");
	connection->write (RKWARD_VERSION);
	connection->write ("\n");
	bool ok = connection->waitForBytesWritten ();
	RK_DEBUG(RBACKEND, DL_DEBUG, "Sending handshake complete, status: %s", ok ? "ok" : "fail");
	if (!ok) handleTransmissionError("Could not write connection handshake: " + connection->errorString());

	flushtimerid = startTimer (200);	// calls flushOutput(false), periodically. See timerEvent()

	exec ();
}

void RKRBackendTransmitter::doExit(){
	RK_TRACE (RBACKEND);
	auto con = connection;
	killTimer(flushtimerid);
	connection->waitForBytesWritten (1000);
	connection = nullptr; // See handleTransmissionError
	RK_DEBUG(RBACKEND, DL_DEBUG, "Aborting connection to frontend");
	con->abort();  // TODO: This never seems to complete!
	RK_DEBUG(RBACKEND, DL_DEBUG, "Done aborting connection to frontend");
	exit(0);
}

void RKRBackendTransmitter::writeRequest (RBackendRequest *request) {
	RK_TRACE (RBACKEND);

	if (request->type != RBackendRequest::Output) flushOutput (true);
	transmitRequest (request);
	connection->flush ();

	if (request->subcommandrequest) {
		current_sync_requests.append(request->subcommandrequest);
		RK_DEBUG(RBACKEND, DL_DEBUG, "Expecting replies for %d requests (added subrequest %p)", current_sync_requests.size(), request);
	}
	if (request->synchronous) {
		current_sync_requests.append(request);
		RK_DEBUG(RBACKEND, DL_DEBUG, "Expecting replies for %d requests (added %p)", current_sync_requests.size(), request);
	} else {
		delete request;
	}
}

void RKRBackendTransmitter::requestReceived (RBackendRequest* request) {
	RK_TRACE (RBACKEND);

	// first check for requests which originated in the frontend
	if (request->type == RBackendRequest::Interrupt) {
		RKRBackend::this_pointer->interruptCommand (request->params.value ("commandid", -1).toInt ());
	} else if (request->type == RBackendRequest::PriorityCommand) {
		RKRBackend::this_pointer->setPriorityCommand (request->takeCommand ());
	} else {    // requests which originated in the backend below this line
		if (current_sync_requests.isEmpty ()) {
			RK_ASSERT (false);
			return;
		}

		// "Synchronous" requests are not necessarily answered in the order they have been queued
		int id = request->id;
		RBackendRequest* current_sync_request = nullptr;
		for (int i = current_sync_requests.size () - 1; i >= 0; --i) {
			RBackendRequest *candidate = current_sync_requests[i];
			if (id == candidate->id) {
				current_sync_request = current_sync_requests.takeAt (i);
				break;
			}
		}
		RK_ASSERT (current_sync_request);
		if (current_sync_request->type == RBackendRequest::Output) {
			delete current_sync_request;	// this was just our internal request
		} else {
			current_sync_request->mergeReply (request);
			current_sync_request->done = true;
		}
		RK_DEBUG (RBACKEND, DL_DEBUG, "Expecting replies for %d requests (popped %p)", current_sync_requests.size (), current_sync_request);
	}
	delete request;
}

void RKRBackendTransmitter::flushOutput (bool force) {
	for (int i = 0; i < current_sync_requests.size(); ++i) {
		// Apparently, frontend isn't keeping up. Don't push the next piece of output, until it has processed the previous one!
		if (current_sync_requests.at(i)->type == RBackendRequest::RCallbackType::Output) return;
	}

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

	if (!connection) return;  // regular exit, or we did not even get to the point of setting up the connection.
	if (RKRBackend::this_pointer->isKilled()) return;
	RK_DEBUG (RBACKEND, DL_ERROR, "%s", qPrintable ("Transmission error " + message));
	RKRBackend::tryToDoEmergencySave ();
}

