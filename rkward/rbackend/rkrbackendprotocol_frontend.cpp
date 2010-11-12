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
#	include <QLocalServer>
#	include <QLocalSocket>
#	include <QProcess>
#endif

#include "../debug.h"

#ifndef RKWARD_THREADED
	class RKFrontendTransmitter : public QThread, public QObject, public RKROutputBuffer {
		Q_OBJECT
		public:
			RKFrontendTransmitter () {
				RK_TRACE (RBACKEND);

				// start server
				connection = 0;
				if (!server.listen ("rkward")) handleTransmitError ("failure to start frontend server: " + server.errorString ());
				connect (&server, SIGNAL (newConnection ()), this, SLOT (connectAndEnterLoop ()));

				// start backend
				QStringList args;
				args.append ("--debug-level " + QString::number (RK_Debug_Level));
				args.append ("--server-name " + server.fullName ());
				backend.setProcessChannelMode (QProcess::MergedChannels);	// at least for now. Seems difficult to get interleaving right, without this.
				connect (&backend, SIGNAL (readyReadStandardOutput ()), this, SLOT (newProcessOutput ()));
				connect (&backend, SIGNAL (finished (int, QProcess::ExitStatus)), this, SLOT (backendExit (int, QProcess::ExitStatus)));
				backend.start ("rkward_backend", args, QIODevice::ReadOnly);
			};

			~RKFrontendTransmitter () {
				RK_TRACE (RBACKEND);

				RK_ASSERT (!server.isListening ());
				delete connection;
			};

			void run () {
				RK_ASSERT (connection);

				connection->moveToThread (this);
				connect (connection, SIGNAL (stateChanged (QLocalSocket::LocalSocketState)), this, SLOT (connectionStateChanged (QLocalSocket::LocalSocketState)));
				connect (connection, SIGNAL (readyRead ()), this, SLOT (newConnectionData ()));
				backend.moveToThread (this);

				exec ();
			}
		private slots:
			void connectAndEnterLoop () {
				RK_TRACE (RBACKEND);
				RK_ASSERT (server.hasPendingConnections ());

				connection = server.nextPendingConnection ();
				server.close ();

				start ();
			};

			void newProcessOutput () {
				#error
			};
			void newConnectionData () {
				#error
			};
			void backendExit (int exitcode, QProcess::ExitStatus exitstatus) {
				#error
			};
			void connectionStateChanged (QLocalSocket::LocalSocketState state) {
				if (state != QLocalSocket::UnconnectedState) return;		// only interested in connection failure
				#error

			};
		private:
			void handleTransmitError (const QString &message) {
				RK_TRACE (RBACKEND);
				#warning: Show those errors to the user!

				qDebug ("%s", qPrintable (message));
			}

			QProcess backend;
			QLocalServer server;
			QLocalSocket* connection;
	};
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
