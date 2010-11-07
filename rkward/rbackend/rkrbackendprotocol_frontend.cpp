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
#ifdef RKWARD_THREADED
#	include <QThread>
#	include "rembedinternal.h"
#	include "rkrbackendprotocol_backend.h"
#endif

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
	delete RKRBackendProtocolBackend::instance ();
}

void RKRBackendProtocolFrontend::setupBackend (QVariantMap backend_params) {
	RK_TRACE (RBACKEND);

#ifdef RKWARD_THREADED
	new RKRBackendProtocolBackend ();
#else
#error
#endif
}

void RKRBackendProtocolFrontend::setRequestCompleted (RBackendRequest *request) {
	RK_TRACE (RBACKEND);

#ifdef RKWARD_THREADED
	bool sync = request->synchronous;
	request->completed ();
	if (sync) QThread::yieldCurrentThread ();
#else
#error
#endif
}

ROutputList RKRBackendProtocolFrontend::flushOutput (bool force) {
	RK_TRACE (RBACKEND);

#ifdef RKWARD_THREADED
	return (RKRBackend::this_pointer->flushOutput (force));
#else
#error
#endif
}

void RKRBackendProtocolFrontend::interruptProcessing () {
	RK_TRACE (RBACKEND);

#ifdef RKWARD_THREADED
	RK_ASSERT (!RKRBackendProtocolBackend::inRThread ());
	RKRBackendProtocolBackend::interruptProcessing ();
#else
	kill (SIGUSR1, pid_of_it);
#error
#endif
}

void RKRBackendProtocolFrontend::terminateBackend () {
	RK_TRACE (RBACKEND);

#ifdef RKWARD_THREADED
	RKRBackend::this_pointer->kill ();
#else
	kill (SIGUSR2, pid_of_it);
#error
#endif
}

#ifdef RKWARD_THREADED
void RKRBackendProtocolFrontend::customEvent (QEvent *e) {
	if (((int) e->type ()) == ((int) RKRBackendEvent::RKWardEvent)) {
		RKRBackendEvent *ev = static_cast<RKRBackendEvent*> (e);
		frontend->handleRequest (ev->data ());
	} else {
		RK_ASSERT (false);
		return;
	}
}
#endif
