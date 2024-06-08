/*
rkrbackendprotocol - This file is part of RKWard (https://rkward.kde.org). Created: Thu Nov 04 2010
SPDX-FileCopyrightText: 2010-2024 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "rkrbackendprotocol_backend.h"

#include "rkrbackend.h"

#include "../debug.h"

#include <QCoreApplication>
#include <QThread>
#include <QLocalSocket>
#include <QMutex>
#include <QUuid>     // mis-used as a random-string generator
#include <QDir>
#include <QUrl>

#include <iostream>

#include "rkbackendtransmitter.h"
#include "rktransmitter.h"
#include "rkrapi.h"

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

#ifdef RK_DLOPEN_LIBRSO
	extern "C"
#	ifdef Q_OS_WIN
	__declspec(dllexport)
#	else
	__attribute__((__visibility__("default")))
#endif
	int do_main(int argc, char *argv[], void* libr_dll_handle, void* (*dlsym_fun)(void*, const char*)) {
#else
	int main(int argc, char *argv[]) {
#endif
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

		setvbuf(stdout, nullptr, _IONBF, 0);
		setvbuf(stderr, nullptr, _IONBF, 0);

		RK_Debug::RK_Debug_Flags = RBACKEND;
		if (RK_Debug::setupLogFile (QDir::tempPath () + "/rkward.rbackend")) qInstallMessageHandler (RKDebugMessageOutput);

		QString servername, rkd_server_name;
		QString data_dir, locale_dir;
		QStringList args = app.arguments();
		bool setup = false;
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
			} else if (args[i] == QLatin1String("--setup")) {
				setup = true;
			} else {
				printf ("unknown argument %s", qPrintable (args[i]));
			}
		}
		if (servername.isEmpty ()) {
			printf ("no server to connect to\n");
			return 1;
		}
		RK_DEBUG(RBACKEND, DL_DEBUG, "Qt version (runtime): %s", qVersion());
		RK_DEBUG(RBACKEND, DL_DEBUG, "Qt version (compile time): %s", QT_VERSION_STR);

		// a simple security token to send to the frontend to make sure that it is really talking to the backend process that it started in the local socket connection.
		// this token is sent both via stdout and the local socket connection. The frontend simply compares both values.
		QString token = QUuid::createUuid ().toString ();

#ifdef RK_DLOPEN_LIBRSO
		RFn::init(libr_dll_handle, dlsym_fun);
#endif

		RKRBackendTransmitter transmitter (servername, token);
		RKRBackendProtocolBackend::p_transmitter = &transmitter; // cppcheck-suppress danglingLifetime ; -> valid for the lifetime of the backend
		RKRBackendProtocolBackend backend (data_dir, rkd_server_name);
		transmitter.start ();
		RKRBackend::this_pointer->run(locale_dir, setup);
		// NOTE:: Since some unknown version of R (4.3.0 at the latest, but probably much earlier), run_Rmainloop() does not return, it will
		//        eventually exit, instead.
		RKRBackendProtocolBackend::doExit();
		return 0;
	}

	void RKRBackendProtocolBackend::doExit() {
		RK_DEBUG(RBACKEND, DL_DEBUG, "Main loop finished");

		QMetaObject::invokeMethod(p_transmitter, "doExit", Qt::QueuedConnection);
		p_transmitter->wait (5000);

		if (!RKRBackend::this_pointer->isKilled ()) RKRBackend::tryToDoEmergencySave ();
		QMetaObject::invokeMethod(qApp, "quit", Qt::QueuedConnection);
		exit(0);
	}

RKRBackendProtocolBackend* RKRBackendProtocolBackend::_instance = nullptr;
RKRBackendTransmitter* RKRBackendProtocolBackend::p_transmitter = nullptr;
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
	qApp->postEvent(p_transmitter, event);
}

bool RKRBackendProtocolBackend::inRThread () {
	return (QThread::currentThread () == instance ()->r_thread);
}

void RKRBackendProtocolBackend::msleep(int delay) {
	p_transmitter->publicmsleep(delay);
}

QString RKRBackendProtocolBackend::backendDebugFile () {
	return RK_Debug::debug_file->fileName ();
}
