/***************************************************************************
                          rkrbackendprotocol  -  description
                             -------------------
    begin                : Thu Nov 04 2010
    copyright            : (C) 2010, 2011 by Thomas Friedrichsmeier
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

#include "rkrbackendprotocol_frontend.h"

#include "rinterface.h"

#include <QThread>

#include "rkfrontendtransmitter.h"
#include <QCoreApplication>

#include "../debug.h"

RKRBackendProtocolFrontend* RKRBackendProtocolFrontend::_instance = 0;
RKRBackendProtocolFrontend::RKRBackendProtocolFrontend (RInterface* parent) : QObject (parent) {
	RK_TRACE (RBACKEND);

	RK_ASSERT (!_instance);
	frontend = parent;
	_instance = this;
}

RKRBackendProtocolFrontend::~RKRBackendProtocolFrontend () {
	RK_TRACE (RBACKEND);

	terminateBackend ();
	RKFrontendTransmitter::instance ()->quit ();
	RKFrontendTransmitter::instance ()->wait (1000);
	delete RKFrontendTransmitter::instance ();
}

void RKRBackendProtocolFrontend::setupBackend () {
	RK_TRACE (RBACKEND);

	new RKFrontendTransmitter ();
}

void RKRBackendProtocolFrontend::setRequestCompleted (RBackendRequest *request) {
	RK_TRACE (RBACKEND);

	bool sync = request->synchronous;
	request->completed ();
	if (!sync) return;

	RKRBackendEvent* ev = new RKRBackendEvent (request);
	qApp->postEvent (RKFrontendTransmitter::instance (), ev);

	QThread::yieldCurrentThread ();
}

ROutputList RKRBackendProtocolFrontend::flushOutput (bool force) {
	return static_cast<RKFrontendTransmitter*> (RKFrontendTransmitter::instance ())->flushOutput (force);
}

void RKRBackendProtocolFrontend::interruptCommand (int command_id) {
	RK_TRACE (RBACKEND);

	RBackendRequest *req = new RBackendRequest (false, RBackendRequest::Interrupt);
	req->params.insert ("commandid", QVariant (command_id));
	qApp->postEvent (RKFrontendTransmitter::instance (), new RKRBackendEvent (req));
}

void RKRBackendProtocolFrontend::terminateBackend () {
	RK_TRACE (RBACKEND);

	// Backend process will terminate automatically, when the transmitter dies
}

void RKRBackendProtocolFrontend::customEvent (QEvent *e) {
	if (((int) e->type ()) == ((int) RKRBackendEvent::RKWardEvent)) {
		RKRBackendEvent *ev = static_cast<RKRBackendEvent*> (e);
		frontend->handleRequest (ev->data ());
	} else {
		RK_ASSERT (false);
		return;
	}
}
