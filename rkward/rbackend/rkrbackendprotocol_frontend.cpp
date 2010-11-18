/***************************************************************************
                          rkrbackendprotocol  -  description
                             -------------------
    begin                : Thu Nov 04 2010
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

#include "rkrbackendprotocol_frontend.h"

#include "rinterface.h"

#include <QThread>

#ifdef RKWARD_THREADED
#	include "rkrbackend.h"
#	include "rkrbackendprotocol_backend.h"
#else
#	include "rkfrontendtransmitter.h"
#	include <QCoreApplication>
#endif

#include "../debug.h"

#ifndef RKWARD_THREADED
#endif

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
#ifdef RKWARD_THREADED
	delete RKRBackendProtocolBackend::instance ();
#else
	RKFrontendTransmitter::instance ()->quit ();
	RKFrontendTransmitter::instance ()->wait (1000);
	delete RKFrontendTransmitter::instance ();
#endif
}

void RKRBackendProtocolFrontend::setupBackend (QVariantMap backend_params) {
	RK_TRACE (RBACKEND);

#ifdef RKWARD_THREADED
	new RKRBackendProtocolBackend ();
#else
	new RKFrontendTransmitter ();
#endif
}

void RKRBackendProtocolFrontend::setRequestCompleted (RBackendRequest *request) {
	RK_TRACE (RBACKEND);

	bool sync = request->synchronous;
	request->completed ();
	if (!sync) return;

#ifndef RKWARD_THREADED
	RKRBackendEvent* ev = new RKRBackendEvent (request);
	qApp->postEvent (RKFrontendTransmitter::instance (), ev);
#endif
	QThread::yieldCurrentThread ();
}

ROutputList RKRBackendProtocolFrontend::flushOutput (bool force) {
#ifdef RKWARD_THREADED
	return (RKRBackend::this_pointer->flushOutput (force));
#else
	return static_cast<RKFrontendTransmitter*> (RKFrontendTransmitter::instance ())->flushOutput (force);
#endif
}

void RKRBackendProtocolFrontend::interruptProcessing () {
	RK_TRACE (RBACKEND);

#ifdef RKWARD_THREADED
	RK_ASSERT (!RKRBackendProtocolBackend::inRThread ());
	RKRBackendProtocolBackend::interruptProcessing ();
#else
//	kill (SIGUSR1, pid_of_it);
#warning will not work on windows!
#endif
}

void RKRBackendProtocolFrontend::terminateBackend () {
	RK_TRACE (RBACKEND);

#ifdef RKWARD_THREADED
	RKRBackend::this_pointer->kill ();
#else
	// Backend process will terminate automatically, when the transmitter dies
#endif
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

