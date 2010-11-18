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
#include <QProcess>
#include <QLocalServer>
#include <QLocalSocket>

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
	if (!server->listen ("rkward")) handleTransmissionError ("failure to start frontend server: " + server->errorString ());
	connect (server, SIGNAL (newConnection ()), this, SLOT (connectAndEnterLoop ()), Qt::QueuedConnection);

	// start backend
	backend = new QProcess (this);
	QStringList args;
	args.append ("--debug-level " + QString::number (RK_Debug_Level));
	args.append ("--server-name " + server->fullServerName ());
	args.append ("--data-dir " + RKSettingsModuleGeneral::filesPath ());
	backend->setProcessChannelMode (QProcess::MergedChannels);	// at least for now. Seems difficult to get interleaving right, without this.
	connect (backend, SIGNAL (readyReadStandardOutput ()), this, SLOT (newProcessOutput ()));
	connect (backend, SIGNAL (finished (int, QProcess::ExitStatus)), this, SLOT (backendExit (int)));
	QString backend_executable = KStandardDirs::findExe ("rkward.rbackend", QCoreApplication::applicationDirPath () + "/rbackend");
	if (backend_executable.isEmpty ()) backend_executable = KStandardDirs::findExe ("rkward.rbackend", QCoreApplication::applicationDirPath ());
	RK_ASSERT (!backend_executable.isEmpty ());
	backend->start (backend_executable, args, QIODevice::ReadOnly);

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

	setConnection (server->nextPendingConnection ());
	server->close ();
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
