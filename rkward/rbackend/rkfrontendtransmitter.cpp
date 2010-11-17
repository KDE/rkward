/***************************************************************************
                          rkfrontendtransmitter  -  description
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

#include "rkfrontendtransmitter.h"

#include "rkrbackendprotocol_frontend.h"
#include "../misc/rkcommonfunctions.h"
#include "../settings/rksettingsmodulegeneral.h"

#include "kstandarddirs.h"
#include <QCoreApplication>

#include "../debug.h"

RKFrontendTransmitter* RKFrontendTransmitter::_instance = 0;
RKFrontendTransmitter::RKFrontendTransmitter () : QThread () {
	RK_TRACE (RBACKEND);

	connection = 0;

	RK_ASSERT (_instance == 0);
	_instance = this;

	moveToThread (this);
	start ();
}

RKFrontendTransmitter::~RKFrontendTransmitter () {
	RK_TRACE (RBACKEND);

	RK_ASSERT (!server->isListening ());
	delete connection;
}

void RKFrontendTransmitter::run () {
	RK_TRACE (RBACKEND);

	// start server
	server = new QLocalServer (this);
	if (!server->listen ("rkward")) handleTransmitError ("failure to start frontend server: " + server->errorString ());
	connect (server, SIGNAL (newConnection ()), this, SLOT (connectAndEnterLoop ()), Qt::QueuedConnection);

	// start backend
	backend = new QProcess (this);
	QStringList args;
	args.append ("--debug-level " + QString::number (RK_Debug_Level));
	args.append ("--server-name " + server->fullServerName ());
	args.append ("--data-dir " + RKSettingsModuleGeneral::filesPath ());
	backend->setProcessChannelMode (QProcess::MergedChannels);	// at least for now. Seems difficult to get interleaving right, without this.
	connect (backend, SIGNAL (readyReadStandardOutput ()), this, SLOT (newProcessOutput ()));
	connect (backend, SIGNAL (finished (int, QProcess::ExitStatus)), this, SLOT (backendExit (int, QProcess::ExitStatus)));
	QString backend_executable = KStandardDirs::findExe ("rkward.rbackend", QCoreApplication::applicationDirPath () + "/rbackend");
	if (backend_executable.isEmpty ()) backend_executable = KStandardDirs::findExe ("rkward.rbackend", QCoreApplication::applicationDirPath ());
	RK_ASSERT (!backend_executable.isEmpty ());
	backend->start (backend_executable, args, QIODevice::ReadOnly);

	exec ();
}

void RKFrontendTransmitter::connectAndEnterLoop () {
	RK_TRACE (RBACKEND);
	RK_ASSERT (server->hasPendingConnections ());

	connection = server->nextPendingConnection ();
	server->close ();

	connect (connection, SIGNAL (stateChanged (QLocalSocket::LocalSocketState)), this, SLOT (connectionStateChanged ()));
	connect (connection, SIGNAL (readyRead ()), this, SLOT (newConnectionData ()));
}

void RKFrontendTransmitter::newProcessOutput () {
	RK_TRACE (RBACKEND);
#warning TODO: fix interleaving
	QString output = QString::fromLocal8Bit (backend->readAll ());
	handleOutput (output, output.size (), ROutput::Warning);
}

void RKFrontendTransmitter::newConnectionData () {
	RK_TRACE (RBACKEND);

	if (!connection->canReadLine ()) return;

	QString line = QString::fromLocal8Bit (connection->readLine ());
	bool ok;
	int expected_length = line.toInt (&ok);
	if (!ok) handleTransmitError ("Protocol header error. Last connection error was: " + connection->errorString ());

	QByteArray receive_buffer;
	while (receive_buffer.length () < expected_length) {
		if (connection->bytesAvailable ()) {
			receive_buffer.append (connection->read (expected_length - receive_buffer.length ()));
		} else {
			connection->waitForReadyRead (1000);
			if (!connection->isOpen ()) {
				handleTransmitError ("Connection closed unexepctedly. Last error: " + connection->errorString ());
				return;
			}
		}
	}

	RBackendRequest *req = RKRBackendSerializer::unserialize (receive_buffer);
	if (req->type == RBackendRequest::Output) {
		ROutputList* list = req->output;
		for (int i = 0; i < list->size (); ++i) {
			ROutput *out = (*list)[i];
			handleOutput (out->output, out->output.length (), out->type);
			delete (out);
		}
		req->output = 0;
		RK_ASSERT (req->synchronous);
		writeRequest (req);	// to tell the backend, that we are keeping up. This also deletes the request.
		return;
	}
	RKRBackendEvent* event = new RKRBackendEvent (req);
	qApp->postEvent (RKRBackendProtocolFrontend::instance (), event);
}

void RKFrontendTransmitter::backendExit (int exitcode, QProcess::ExitStatus exitstatus) {
	RK_TRACE (RBACKEND);

	RBackendRequest* req = new RBackendRequest (false, RBackendRequest::BackendExit);
	RKRBackendEvent* event = new RKRBackendEvent (req);
	qApp->postEvent (RKRBackendProtocolFrontend::instance (), event);
}

void RKFrontendTransmitter::connectionStateChanged () {
	if (connection->state () != QLocalSocket::UnconnectedState) return;		// only interested in connection failure
	RK_TRACE (RBACKEND);

	RBackendRequest* req = new RBackendRequest (false, RBackendRequest::BackendExit);
	RKRBackendEvent* event = new RKRBackendEvent (req);
	qApp->postEvent (RKRBackendProtocolFrontend::instance (), event);
}

void RKFrontendTransmitter::writeRequest (RBackendRequest *request) {
	RK_TRACE (RBACKEND);

	QByteArray buffer = RKRBackendSerializer::serialize (*request);
	connection->write (QString::number (buffer.length ()).toLocal8Bit () + "\n");
	connection->write (buffer);
	connection->flush ();
	delete request;
}

void RKFrontendTransmitter::customEvent (QEvent *e) {
	if (((int) e->type ()) == ((int) RKRBackendEvent::RKWardEvent)) {
		RKRBackendEvent *ev = static_cast<RKRBackendEvent*> (e);
		writeRequest (ev->data ());
	} else {
		RK_ASSERT (false);
		return;
	}
}

void RKFrontendTransmitter::handleTransmitError (const QString &message) {
	RK_TRACE (RBACKEND);
	#warning: Show those errors to the user!

	qDebug ("%s", qPrintable (message));
}

#include "rkfrontendtransmitter.moc"
