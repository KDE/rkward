/***************************************************************************
                          rkrbackendprotocol  -  description
                             -------------------
    begin                : Thu Nov 04 2010
    copyright            : (C) 2010, 2013 by Thomas Friedrichsmeier
    email                : thomas.friedrichsmeier@kdemail.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "rkrbackendprotocol_backend.h"

#include "rkrbackend.h"

#include "../debug.h"

#include <QCoreApplication>
#include <QThread>
#include <QLocalSocket>
#include <QMutex>

#include "rktransmitter.h"
#include <iostream>

#include "rkbackendtransmitter.h"
#include <QUuid>		// mis-used as a random-string generator
#include <QDir>
#include <QUrl>

#ifdef Q_OS_MACOS
#include <CoreFoundation/CoreFoundation.h>
#endif

	void RK_setupGettext (const QString &);
	QMutex RK_Debug_Mutex;

	void RKDebugMessageOutput (QtMsgType type, const QMessageLogContext &, const QString &msg) {
		RK_Debug_Mutex.lock ();
		if (type == QtFatalMsg) {
			fprintf (stderr, "%s\n", qPrintable (msg));
		}
		RK_Debug::debug_file->write (qPrintable (msg));
		RK_Debug::debug_file->write ("\n");
		RK_Debug::debug_file->flush ();
		RK_Debug_Mutex.unlock ();
	}

	// NOTE: This function serves no benefit over qDebug() in the backend. But provided for consistency with the frontend.
	// See the frontend version in main.cpp
	void RKDebug (int flags, int level, const char *fmt, ...) {
		Q_UNUSED (flags);
		Q_UNUSED (level);
		const int bufsize = 1024*8;
		char buffer[bufsize];

		va_list ap;
		va_start (ap, fmt);
		vsnprintf (buffer, bufsize-1, fmt, ap);
		va_end (ap);
		RKDebugMessageOutput (QtDebugMsg, QMessageLogContext (), buffer);
	}

	int main(int argc, char *argv[]) {
#ifdef Q_OS_MACOS
		CFBundleRef mainBundle = CFBundleGetMainBundle();
		if (mainBundle) {
			// get the application's Info Dictionary. For app bundles this would live in the bundle's Info.plist,
			// for regular executables it is obtained in another way.
			CFMutableDictionaryRef infoDict = (CFMutableDictionaryRef) CFBundleGetInfoDictionary(mainBundle);
			if (infoDict) {
				// Add or set the "LSUIElement" key with/to value "1". This can simply be a CFString.
				CFDictionarySetValue(infoDict, CFSTR("LSUIElement"), CFSTR("1"));
				// That's it. We're now considered as an "agent" by the window server, and thus will have
				// neither menubar nor presence in the Dock or App Switcher.
			}
		}
#endif
		QCoreApplication app (argc, argv);

		setvbuf (stdout, NULL, _IONBF, 0);
		setvbuf (stderr, NULL, _IONBF, 0);

		RK_Debug::RK_Debug_Flags = RBACKEND;
		if (RK_Debug::setupLogFile (QDir::tempPath () + "/rkward.rbackend")) qInstallMessageHandler (RKDebugMessageOutput);

		QString servername, rkd_server_name;
		QString data_dir, locale_dir;
		QStringList args = app.arguments ();
		for (int i = 1; i < args.count (); ++i) {
			if (args[i].startsWith (QLatin1String ("--debug-level"))) {
				RK_Debug::RK_Debug_Level = args[i].section ('=', 1).toInt ();
			} else if (args[i].startsWith (QLatin1String ("--server-name"))) {
				servername = QUrl::fromPercentEncoding (args[i].section ('=', 1).toUtf8 ());
			} else if (args[i].startsWith (QLatin1String ("--data-dir"))) {
				data_dir = QUrl::fromPercentEncoding (args[i].section ('=', 1).toUtf8 ());
			} else if (args[i].startsWith (QLatin1String ("--locale-dir"))) {
				locale_dir = QUrl::fromPercentEncoding (args[i].section ('=', 1).toUtf8 ());
			} else if (args[i].startsWith (QLatin1String ("--rkd-server-name"))) {
				rkd_server_name = QUrl::fromPercentEncoding (args[i].section ('=', 1).toUtf8 ());
			} else {
				printf ("unknown argument %s", qPrintable (args[i]));
			}
		}
		if (servername.isEmpty ()) {
			printf ("no server to connect to\n");
			return 1;
		}

		// a simple security token to send to the frontend to make sure that it is really talking to the backend process that it started in the local socket connection.
		// this token is sent both via stdout and the local socket connection. The frontend simply compares both values.
		QString token = QUuid::createUuid ().toString ();
		std::cout << token.toLocal8Bit ().data () << "\n";
		std::cout.flush ();

		RKRBackendTransmitter transmitter (servername, token);
		RKRBackendProtocolBackend backend (data_dir, rkd_server_name);
		transmitter.start ();
		RKRBackend::this_pointer->run (locale_dir);
		RK_DEBUG(RBACKEND, DL_DEBUG, "Main loop finished");

		QMetaObject::invokeMethod(&transmitter, "doExit", Qt::QueuedConnection);
		transmitter.wait (5000);

		if (!RKRBackend::this_pointer->isKilled ()) RKRBackend::tryToDoEmergencySave ();
		QMetaObject::invokeMethod(&app, "quit", Qt::QueuedConnection);
		exit(0);
	}

RKRBackendProtocolBackend* RKRBackendProtocolBackend::_instance = 0;
RKRBackendProtocolBackend::RKRBackendProtocolBackend (const QString &storage_dir, const QString &_rkd_server_name) {
	RK_TRACE (RBACKEND);

	_instance = this;
	new RKRBackend ();
	r_thread = QThread::currentThread ();	// R thread == main thread
#ifndef Q_OS_WIN
	r_thread_id = QThread::currentThreadId ();
#endif
	data_dir = storage_dir;
	rkd_server_name = _rkd_server_name;
}

RKRBackendProtocolBackend::~RKRBackendProtocolBackend () {
	RK_TRACE (RBACKEND);
}

void RKRBackendProtocolBackend::sendRequest (RBackendRequest *_request) {
	RK_TRACE (RBACKEND);

	RBackendRequest* request = _request;
	if (!request->synchronous) {
		request = _request->duplicate ();	// the instance we send to the frontend will remain in there, and be deleted, there
		_request->done = true;				// for aesthetics
	}
	RKRBackendEvent* event = new RKRBackendEvent (request);
	RK_ASSERT (request->type != RBackendRequest::Output);
	qApp->postEvent (RKRBackendTransmitter::instance (), event);
}

bool RKRBackendProtocolBackend::inRThread () {
	return (QThread::currentThread () == instance ()->r_thread);
}

void RKRBackendProtocolBackend::msleep (int delay) {
	static_cast<RKRBackendTransmitter*> (RKRBackendTransmitter::instance ())->publicmsleep (delay);
}

QString RKRBackendProtocolBackend::backendDebugFile () {
	return RK_Debug::debug_file->fileName ();
}
