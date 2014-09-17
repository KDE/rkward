/***************************************************************************
                          rkfrontendtransmitter  -  description
                             -------------------
    begin                : Thu Nov 04 2010
    copyright            : (C) 2010-2014 by Thomas Friedrichsmeier
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
#include "rkwarddevice/rkgraphicsdevice_frontendtransmitter.h"
#include "../misc/rkcommonfunctions.h"
#include "../settings/rksettingsmodulegeneral.h"
#include "../rkglobals.h"

#include <klocale.h>
#include <krandom.h>
#include <kstandarddirs.h>
#include <QCoreApplication>
#include <QProcess>
#include <QLocalServer>
#include <QLocalSocket>
#include <QDir>

#include "../version.h"
#include "../debug.h"

QString findBackendAtPath (const QString &path) {
	QDir dir (path);
	dir.makeAbsolute ();
	QString ret;
#ifdef Q_WS_WIN
	if (QFileInfo (dir.filePath ("rkward.rbackend.exe")).isExecutable ()) ret = dir.filePath ("rkward.rbackend.exe");
#else
	if (QFileInfo (dir.filePath ("rkward.rbackend")).isExecutable ()) ret = dir.filePath ("rkward.rbackend");
#endif
	RK_DEBUG (RBACKEND, DL_DEBUG, "Looking for backend at %s: %s", qPrintable (dir.absolutePath ()), qPrintable (ret));
	return ret;
}

RKFrontendTransmitter::RKFrontendTransmitter () : RKAbstractTransmitter () {
	RK_TRACE (RBACKEND);

	rkd_transmitter = new RKGraphicsDeviceFrontendTransmitter ();
	start ();
}

RKFrontendTransmitter::~RKFrontendTransmitter () {
	RK_TRACE (RBACKEND);

	delete rkd_transmitter;
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

	// Try to synchronize language selection in frontend and backend
	QStringList env = QProcess::systemEnvironment ();
	int index = env.indexOf (QRegExp("^LANGUAGE=.*", Qt::CaseInsensitive));
	if (index >= 0) env.removeAt (index);
	env.append ("LANGUAGE=" + KGlobal::locale ()->language ());
	backend->setEnvironment (env);

	QStringList args;
	args.append ("--debug-level=" + QString::number (RK_Debug_Level));
	args.append ("--server-name=" + server->fullServerName ());
	args.append ("--rkd-server-name=" + rkd_transmitter->serverName ());
	args.append ("--data-dir=" + RKSettingsModuleGeneral::filesPath ());
	args.append ("--locale-dir=" + KGlobal::dirs()->findResourceDir ("locale", KGlobal::locale ()->language () + "/LC_MESSAGES/rkward.mo"));
	connect (backend, SIGNAL (finished (int, QProcess::ExitStatus)), this, SLOT (backendExit (int)));
	QString backend_executable = findBackendAtPath (QCoreApplication::applicationDirPath ());
	if (backend_executable.isEmpty ()) backend_executable = findBackendAtPath (QCoreApplication::applicationDirPath () + "/rbackend");	// for running directly from the build-dir
#ifdef Q_WS_MAC
        if (backend_executable.isEmpty ()) backend_executable = findBackendAtPath (QCoreApplication::applicationDirPath () + "/../../../rbackend");
#endif
	if (backend_executable.isEmpty ()) handleTransmissionError (i18n ("The backend executable could not be found. This is likely to be a problem with your installation."));
	QString debugger = RKGlobals::startup_options["backend-debugger"].toString ();
	if (!debugger.isEmpty ()) {
		args.prepend (backend_executable);
		QStringList l = debugger.split (' ');
		args = l.mid (1) + args;
		backend->start (l.first (), args, QIODevice::ReadOnly);
	} else {
		backend->start (backend_executable, args, QIODevice::ReadOnly);
	}

	// fetch security token
	if (!backend->canReadLine ()) backend->waitForReadyRead ();
	token = QString::fromLocal8Bit (backend->readLine ()).trimmed ();
	backend->closeReadChannel (QProcess::StandardError);
	backend->closeReadChannel (QProcess::StandardOutput);

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
	if (version_c != RKWARD_VERSION) handleTransmissionError (i18n ("Version mismatch during handshake with backend process. Frontend is version '%1' while backend is '%2'.\nPlease fix your installation.").arg (RKWARD_VERSION).arg (version_c));

	setConnection (con);
}

void RKFrontendTransmitter::requestReceived (RBackendRequest* request) {
	RK_TRACE (RBACKEND);

	if (request->type == RBackendRequest::Output) {
		ROutputList* list = request->output;
		for (int i = 0; i < list->size (); ++i) {
			ROutput *out = (*list)[i];

			if (handleOutput (out->output, out->output.length (), out->type)) {
				RKRBackendEvent* event = new RKRBackendEvent (new RBackendRequest (false, RBackendRequest::OutputStartedNotification));
				qApp->postEvent (RKRBackendProtocolFrontend::instance (), event);
			}

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

	if (!exitcode && token.isEmpty ()) handleTransmissionError (i18n ("The backend process could not be started. Please check your installation."));
	else if (token.isEmpty ()) handleTransmissionError (i18n ("The backend process failed to start with exit code %1.", exitcode));
	else handleTransmissionError (i18n ("Backend process has exited with code %1.", exitcode));
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
