/*
rkrbackendprotocol - This file is part of RKWard (https://rkward.kde.org). Created: Thu Nov 04 2010
SPDX-FileCopyrightText: 2010-2022 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "rkrbackendprotocol_frontend.h"

#include "rkrinterface.h"

#include <QThread>

#include "rkfrontendtransmitter.h"
#include <QCoreApplication>

#include "../debug.h"

RKRBackendProtocolFrontend* RKRBackendProtocolFrontend::_instance = nullptr;
RKRBackendProtocolFrontend::RKRBackendProtocolFrontend (RInterface* parent) : QObject (parent) {
	RK_TRACE (RBACKEND);

	RK_ASSERT (!_instance);
	frontend = parent;
	_instance = this;
}

RKRBackendProtocolFrontend::~RKRBackendProtocolFrontend () {
	RK_TRACE (RBACKEND);

	RK_ASSERT(_instance == this);
	terminateBackend ();
	RKFrontendTransmitter::instance ()->wait(1000);  // Wait for thread to catch the backend's exit request, and exit()
	QMetaObject::invokeMethod(RKFrontendTransmitter::instance(), &RKFrontendTransmitter::quit, Qt::QueuedConnection);  // tell it to quit, otherwise
	RKFrontendTransmitter::instance ()->wait(3000);  // Wait for thread to quit and clean up.
	qApp->processEvents(QEventLoop::AllEvents, 500); // Not strictly needed, but avoids some mem leaks on exit by handling all posted BackendExit events
	delete RKFrontendTransmitter::instance ();
	_instance = nullptr;
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

void RKRBackendProtocolFrontend::sendPriorityCommand (RCommandProxy* proxy) {
	RK_TRACE (RBACKEND);

	RBackendRequest *req = new RBackendRequest (false, RBackendRequest::PriorityCommand);
	req->command = proxy;
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
