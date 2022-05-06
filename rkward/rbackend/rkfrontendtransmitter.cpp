/*
rkfrontendtransmitter - This file is part of RKWard (https://rkward.kde.org). Created: Thu Nov 04 2010
SPDX-FileCopyrightText: 2010-2019 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "rkfrontendtransmitter.h"

#include "rkrbackendprotocol_frontend.h"
#include "rkwarddevice/rkgraphicsdevice_frontendtransmitter.h"
#include "rksessionvars.h"
#include "../misc/rkcommonfunctions.h"
#include "../settings/rksettingsmodulegeneral.h"

#include <KLocalizedString>
#include <krandom.h>

#include <QCoreApplication>
#include <QProcess>
#include <QLocalServer>
#include <QLocalSocket>
#include <QDir>
#include <QStandardPaths>
#include <QElapsedTimer>

#include "../version.h"
#include "../debug.h"

QString findBackendAtPath (const QString &path) {
	QDir dir (path);
	dir.makeAbsolute ();
#ifdef Q_OS_WIN
	QString ret = dir.filePath ("rkward.rbackend.exe");
#else
	QString ret = dir.filePath ("rkward.rbackend");
#endif
	RK_DEBUG (RBACKEND, DL_DEBUG, "Looking for backend at %s", qPrintable (ret));
	QFileInfo fi (ret);
	if (fi.exists () && fi.isExecutable ()) return ret;
	return QString ();
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

QString localeDir () {
	// adapted from KCatalog::catalogLocaleDir()
	QString relpath = QStringLiteral ("%1/LC_MESSAGES/rkward.mo").arg (QLocale ().name ().section ('_', 0, 0));
	QString file = QStandardPaths::locate (QStandardPaths::GenericDataLocation, QStringLiteral ("locale/") + relpath);
	if (file.isEmpty ()) return QString ();
	return QFileInfo (file.left (file.size() - relpath.size ())).absolutePath ();
}

void RKFrontendTransmitter::run () {
	RK_TRACE (RBACKEND);

	// start server
	qsrand (QTime::currentTime ().msec ()); // Workaround for some versions of kcoreaddons (5.21.0 through at least 5.34.0). See https://phabricator.kde.org/D5966
	server = new QLocalServer (this);
	// we add a bit of randomness to the servername, as in general the servername must be unique
	// there could be conflicts with concurrent or with previous crashed rkward sessions.
	if (!server->listen ("rkward" + KRandom::randomString (8))) handleTransmissionError ("Failure to start frontend server: " + server->errorString ());
	connect (server, &QLocalServer::newConnection, this, &RKFrontendTransmitter::connectAndEnterLoop, Qt::QueuedConnection);
	// start backend
	backend = new QProcess (this);

	// Try to synchronize language selection in frontend and backend
	QStringList env = QProcess::systemEnvironment ();
	int index = env.indexOf (QRegExp("^LANGUAGE=.*", Qt::CaseInsensitive));
	if (index >= 0) env.removeAt (index);
	env.append ("LANGUAGE=" + QLocale ().name ().section ('_', 0, 0));
	backend->setEnvironment (env);

	QStringList args;
	args.append ("--debug-level=" + QString::number (RK_Debug::RK_Debug_Level));
	// NOTE: QProcess quotes its arguments, *but* properly passing all spaces and quotes through the R CMD wrapper, seems near(?) impossible on Windows. Instead, we use percent encoding, internally.
	args.append ("--server-name=" + server->fullServerName ().toUtf8 ().toPercentEncoding ());
	args.append ("--rkd-server-name=" + rkd_transmitter->serverName ().toUtf8 ().toPercentEncoding ());
	args.append ("--data-dir=" + RKSettingsModuleGeneral::filesPath ().toUtf8 ().toPercentEncoding ());
	args.append ("--locale-dir=" + localeDir ().toUtf8 ().toPercentEncoding ());
	connect (backend, static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), this, &RKFrontendTransmitter::backendExit);
	QString backend_executable = findBackendAtPath (QCoreApplication::applicationDirPath ());
#ifdef Q_OS_MACOS
	if (backend_executable.isEmpty ()) backend_executable = findBackendAtPath (QCoreApplication::applicationDirPath () + "../Resources"); // an appropriate location in a standalone app-bundle
#endif
	if (backend_executable.isEmpty ()) backend_executable = findBackendAtPath (QCoreApplication::applicationDirPath () + "/rbackend");	// for running directly from the build-dir
	if (backend_executable.isEmpty ()) backend_executable = findBackendAtPath (QCoreApplication::applicationDirPath () + "../lib/libexec");
#ifdef Q_OS_MACOS
	if (backend_executable.isEmpty ()) backend_executable = findBackendAtPath (QCoreApplication::applicationDirPath () + "/../../../rbackend");
	if (backend_executable.isEmpty ()) backend_executable = findBackendAtPath (QCoreApplication::applicationDirPath () + "/../Frameworks/libexec");  // For running from .dmg created by craft --package rkward
#endif
	if (backend_executable.isEmpty()) backend_executable = findBackendAtPath(RKWARD_BACKEND_PATH);
	if (backend_executable.isEmpty()) {
		handleTransmissionError(i18n("The backend executable could not be found. This is likely to be a problem with your installation."));
		exec();   // To actually show the transmission error
		return;
	}
	QString debugger = RKSettingsModuleGeneral::startupOption("backend-debugger").toString();
	args.prepend (RKCommonFunctions::windowsShellScriptSafeCommand (backend_executable));
	if (!debugger.isEmpty ()) {
		args = debugger.split (' ') + args;
	}
#ifdef Q_OS_MACOS
	// Resolving libR.dylib and friends is a pain on MacOS, and running through R CMD does not always seem to be enough.
	// (Apparently DYLIB_FALLBACK_LIBRARY_PATH is ignored on newer versions of MacOS). Safest best seems to be to start in the lib directory, itself.
	QProcess dummy;
	dummy.start(RKSessionVars::RBinary(), QStringList() << "--slave" << "--no-save" << "--no-init-file" << "-e" << "cat(R.home('lib'))");
	dummy.waitForFinished ();
	QString r_home = QString::fromLocal8Bit (dummy.readAllStandardOutput ());
	RK_DEBUG(RBACKEND, DL_INFO, "Setting working directory to %s", qPrintable (r_home));
	backend->setWorkingDirectory (r_home);
#endif
	args.prepend ("CMD");
	if (DL_DEBUG >= RK_Debug::RK_Debug_Level) {
		qDebug("%s", qPrintable(RKSessionVars::RBinary()));
		qDebug("%s", qPrintable(args.join("\n")));
	}
	backend->start(RKSessionVars::RBinary(), args, QIODevice::ReadOnly);

	if (!backend->waitForStarted()) {
		handleTransmissionError(i18n("The backend executable could not be started. Error message was: %1", backend->errorString()));
	} else {
		token = waitReadLine(backend, 5000).trimmed();
		backend->closeReadChannel(QProcess::StandardError);
		backend->closeReadChannel(QProcess::StandardOutput);
	}

	exec ();

	// It's ok to only give backend a short time to finish. We only get here, after QuitCommand has been handled by the backend
	backend->waitForFinished(1000);

	if (!connection) {
		RK_ASSERT (false);
		return;
	}
}

QString RKFrontendTransmitter::waitReadLine (QIODevice* con, int msecs) {
	RK_TRACE (RBACKEND);

	// NOTE: On Qt5+Windows, readyReady may actually come in char by char, so calling waitForReadyRead() does not guarantee we will
	//       see the full line, at all. But also, of course, we want to put some cap on trying. Using a time threshold for this.
	QElapsedTimer time;
	time.start();
	QByteArray ret;
	do {
		ret.append(con->readLine());
		if (ret.contains('\n')) break;
		con->waitForReadyRead(500);
	} while(time.elapsed() < msecs);
	return QString::fromLocal8Bit(ret);
}

void RKFrontendTransmitter::connectAndEnterLoop () {
	RK_TRACE (RBACKEND);
	RK_ASSERT (server->hasPendingConnections ());

	QLocalSocket *con = server->nextPendingConnection ();
	server->close ();

	// handshake
	QString token_c = waitReadLine(con, 1000).trimmed();
	if (token_c != token) handleTransmissionError (i18n ("Error during handshake with backend process. Expected token '%1', received token '%2'", token, token_c));
	QString version_c = waitReadLine(con, 1000).trimmed();
	if (version_c != RKWARD_VERSION) handleTransmissionError (i18n ("Version mismatch during handshake with backend process. Frontend is version '%1' while backend is '%2'.\nPlease fix your installation.", QString (RKWARD_VERSION), version_c));

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
		delete list;
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
	else if (token.isEmpty ()) handleTransmissionError (i18n ("The backend process failed to start with exit code %1, message: '%2'.", exitcode, QString::fromLocal8Bit(backend->readAllStandardError())));
	else handleTransmissionError (i18n ("Backend process has exited with code %1, message: '%2'.", exitcode, QString::fromLocal8Bit(backend->readAllStandardError())));
}

void RKFrontendTransmitter::writeRequest (RBackendRequest *request) {
	RK_TRACE (RBACKEND);

	transmitRequest (request);
	connection->flush ();
	delete request;
}

void RKFrontendTransmitter::handleTransmissionError (const QString &message) {
	RK_TRACE (RBACKEND);

	if (connection) connection->close ();
	RBackendRequest* req = new RBackendRequest (false, RBackendRequest::BackendExit);
	req->params["message"] = message;
	RKRBackendEvent* event = new RKRBackendEvent (req);
	qApp->postEvent (RKRBackendProtocolFrontend::instance (), event);

	exit ();
}

