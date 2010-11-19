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

#include "klocale.h"
#include "krandom.h"
#include "kstandarddirs.h"
#include <QCoreApplication>
#include <QProcess>
#include <QLocalServer>
#include <QLocalSocket>

#include "../version.h"
#include "../debug.h"

RKFrontendTransmitter::RKFrontendTransmitter () : RKAbstractTransmitter () {
	RK_TRACE (RBACKEND);

	start ();
}

RKFrontendTransmitter::~RKFrontendTransmitter () {
	RK_TRACE (RBACKEND);

	RK_ASSERT (!server->isListening ());
}

void RKFrontendTransmitter::run () {
	RK_TRACE (RBACKEND);

	// start server
	server = new QLocalServer (this);
	// we add a bit of randomness to the servername, as in general the servername must be unique
	// there could be conflicts with concurrent or with previous crashed rkward sessions.
	if (!server->listen ("rkward" + KRandom::randomString (8))) handleTransmissionError ("Failure to start frontend server: " + server->errorString ());
	connect (server, SIGNAL (newConnection ()), this, SLOT (connectAndEnterLoop ()), Qt::QueuedConnection);

	// start backend
	backend = new QProcess (this);
	QStringList args;
	args.append ("--debug-level " + QString::number (RK_Debug_Level));
	args.append ("--server-name " + server->fullServerName ());
	args.append ("--data-dir " + RKSettingsModuleGeneral::filesPath ());
	backend->setProcessChannelMode (QProcess::MergedChannels);	// at least for now. Seems difficult to get interleaving right, without this.
	connect (backend, SIGNAL (finished (int, QProcess::ExitStatus)), this, SLOT (backendExit (int)));
	QString backend_executable = KStandardDirs::findExe ("rkward.rbackend", QCoreApplication::applicationDirPath () + "/rbackend");
	if (backend_executable.isEmpty ()) backend_executable = KStandardDirs::findExe ("rkward.rbackend", QCoreApplication::applicationDirPath ());
	RK_ASSERT (!backend_executable.isEmpty ());
	backend->start (backend_executable, args, QIODevice::ReadOnly);

	// fetch security token
	if (!backend->canReadLine ()) backend->waitForReadyRead ();
	token = QString::fromLocal8Bit (backend->readLine ());
	token.chop (1);

	connect (backend, SIGNAL (readyReadStandardOutput ()), this, SLOT (newProcessOutput ()));
	if (backend->bytesAvailable ()) newProcessOutput ();

	exec ();

	if (!connection) {
		RK_ASSERT (false);
		return;
	}

	connection->close ();
	backend->waitForFinished ();
}

void RKFrontendTransmitter::connectAndEnterLoop () {
	RK_TRACE (RBACKEND);
	RK_ASSERT (server->hasPendingConnections ());

	QLocalSocket *con = server->nextPendingConnection ();
	server->close ();

	// handshake
	if (!con->canReadLine ()) con->waitForReadyRead (1000);
	QString token_c = QString::fromLocal8Bit (con->readLine ());
	token_c.chop (1);
	if (token_c != token) handleTransmissionError (i18n ("Error during handshake with backend process. Expected token '%1', received token '%2'").arg (token).arg (token_c));
	if (!con->canReadLine ()) con->waitForReadyRead (1000);
	QString version_c = QString::fromLocal8Bit (con->readLine ());
	version_c.chop (1);
	if (version_c != VERSION) handleTransmissionError (i18n ("Version mismatch during handshake with backend process. Frontend is version '%1' while backend is '%2'.\nPlease fix your installation.").arg (VERSION).arg (version_c));

	setConnection (con);
}

void RKFrontendTransmitter::newProcessOutput () {
	RK_TRACE (RBACKEND);
#warning TODO: fix interleaving
	QString output = QString::fromLocal8Bit (backend->readAll ());
	handleOutput (output, output.size (), ROutput::Warning);
}

void RKFrontendTransmitter::requestReceived(RBackendRequest* request) {
	RK_TRACE (RBACKEND);

	if (request->type == RBackendRequest::Output) {
		ROutputList* list = request->output;
		for (int i = 0; i < list->size (); ++i) {
			ROutput *out = (*list)[i];
			handleOutput (out->output, out->output.length (), out->type);
			delete (out);
		}
		request->output = 0;
		RK_ASSERT (request->synchronous);
		writeRequest (request);	// to tell the backend, that we are keeping up. Also deletes the request.
		return;
	}
	RKRBackendEvent* event = new RKRBackendEvent (request);
	qApp->postEvent (RKRBackendProtocolFrontend::instance (), event);
}

void RKFrontendTransmitter::backendExit (int exitcode) {
	RK_TRACE (RBACKEND);

	handleTransmissionError ("Backend process has exited with code " + QString::number (exitcode));
}

void RKFrontendTransmitter::writeRequest (RBackendRequest *request) {
	RK_TRACE (RBACKEND);

	transmitRequest (request);
	connection->flush ();
	delete request;
}

void RKFrontendTransmitter::handleTransmissionError (const QString &message) {
	RK_TRACE (RBACKEND);

	RBackendRequest* req = new RBackendRequest (false, RBackendRequest::BackendExit);
	req->params["message"] = message;
	RKRBackendEvent* event = new RKRBackendEvent (req);
	qApp->postEvent (RKRBackendProtocolFrontend::instance (), event);

	exit ();
}

#include "rkfrontendtransmitter.moc"
