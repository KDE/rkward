/***************************************************************************
                          rkrbackendprotocol  -  description
                             -------------------
    begin                : Thu Nov 04 2010
    copyright            : (C) 2010, 2013 by Thomas Friedrichsmeier
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

#include "rkrbackendprotocol_backend.h"

#include "rkrbackend.h"

#include "../debug.h"

#include <QCoreApplication>
#include <QThread>

#ifdef RKWARD_THREADED
#	include "rkrbackendprotocol_frontend.h"
#else
#	include <QLocalSocket>
#	include <QMutex>
#	include "rktransmitter.h"
#	include <stdio.h>
#endif

#ifdef RKWARD_THREADED
	class RKRBackendThread : public QThread {
	public:
		RKRBackendThread () {
#	ifdef Q_WS_WIN
			// we hope that on other platforms the default is reasonable
			setStackSize (0xa00000);	// 10MB as recommended by r_exts-manual
#	endif
			instance = this;
		};
		~RKRBackendThread () {};

		// called form the *other* thread, only
		void exitThread () {
			RK_TRACE (RBACKEND);
			if (isRunning ()) {
				RK_DEBUG (RBACKEND, DL_INFO, "Waiting for R thread to finish up...");
				RKRBackendProtocolBackend::interruptProcessing ();
				RKRBackend::this_pointer->kill ();
				wait (1000);
				if (isRunning ()) {
					RK_DEBUG (RBACKEND, DL_WARNING, "Backend thread is still running. It will be killed, now.");
					terminate ();
					yieldCurrentThread ();
					RK_ASSERT (false);
				}
			}
		};

		void publicmsleep (int delay) { msleep (delay); };

		void run () {
			RK_TRACE (RBACKEND);
#	ifndef Q_WS_WIN
			RKRBackendProtocolBackend::instance ()->r_thread_id = currentThreadId ();
#	endif
			RKRBackend::this_pointer->run ();
		}

		/** On pthread systems this is the pthread_id of the backend thread. It is needed to send SIGINT to the R backend */
		Qt::HANDLE thread_id;
		static RKRBackendThread* instance;
	};
	RKRBackendThread* RKRBackendThread::instance = 0;
#else
#	include "rkbackendtransmitter.h"
#	include <QUuid>		// mis-used as a random-string generator
#	include <QTemporaryFile>
#	include <QDir>

	extern "C" void RK_setupGettext (const char*);
	int RK_Debug_Level = 2;
	int RK_Debug_Flags = ALL;
	QMutex RK_Debug_Mutex;
	QTemporaryFile* RK_Debug_File;

	void RKDebugMessageOutput (QtMsgType type, const char *msg) {
		RK_Debug_Mutex.lock ();
		if (type == QtFatalMsg) {
			fprintf (stderr, "%s\n", msg);
		}
		RK_Debug_File->write (msg);
		RK_Debug_File->write ("\n");
		RK_Debug_File->flush ();
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
		RKDebugMessageOutput (QtDebugMsg, buffer);
	}

	int main(int argc, char *argv[]) {
		QCoreApplication app (argc, argv);

		setvbuf (stdout, NULL, _IONBF, 0);
		setvbuf (stderr, NULL, _IONBF, 0);

		RK_Debug_File = new QTemporaryFile (QDir::tempPath () + "/rkward.rbackend");
		RK_Debug_File->setAutoRemove (false);
		if (RK_Debug_File->open ()) qInstallMsgHandler (RKDebugMessageOutput);

		QString servername, rkd_server_name;
		QString data_dir, locale_dir;
		QStringList args = app.arguments ();
		for (int i = 1; i < args.count (); ++i) {
			if (args[i].startsWith ("--debug-level")) {
				RK_Debug_Level = args[i].section ('=', 1).toInt ();
			} else if (args[i].startsWith ("--server-name")) {
				servername = args[i].section ('=', 1);
			} else if (args[i].startsWith ("--data-dir")) {
#warning What about paths with spaces?!
				data_dir = args[i].section ('=', 1);
			} else if (args[i].startsWith ("--locale-dir")) {
				locale_dir = args[i].section ('=', 1);
			} else if (args[i].startsWith ("--rkd-server-name")) {
				rkd_server_name = args[i].section ('=', 1);
			} else {
				printf ("unkown argument %s", qPrintable (args[i]));
			}
		}
		if (servername.isEmpty ()) {
			printf ("no server to connect to\n");
			return 1;
		}

		// a simple security token to send to the frontend to make sure that it is really talking to the backend process that it started in the local socket connection.
		// this token is sent both via stdout and the local socket connection. The frontend simply compares both values.
		QString token = QUuid::createUuid ().toString ();
		printf ("%s\n", token.toLocal8Bit ().data ());
		fflush (stdout);

		RKRBackendTransmitter transmitter (servername, token);
		RKRBackendProtocolBackend backend (data_dir, rkd_server_name);
		transmitter.start ();
		RKRBackend::this_pointer->run (locale_dir);
		transmitter.quit ();
		transmitter.wait (5000);

		if (!RKRBackend::this_pointer->isKilled ()) RKRBackend::tryToDoEmergencySave ();
	}
#endif

RKRBackendProtocolBackend* RKRBackendProtocolBackend::_instance = 0;
RKRBackendProtocolBackend::RKRBackendProtocolBackend (const QString &storage_dir, const QString &_rkd_server_name) {
	RK_TRACE (RBACKEND);

	_instance = this;
	new RKRBackend ();
#ifdef RKWARD_THREADED
	r_thread = new RKRBackendThread ();
	// NOTE: r_thread_id is obtained from within the thread
	RKRBackendThread::instance->start ();
#else
	r_thread = QThread::currentThread ();	// R thread == main thread
#	ifndef Q_WS_WIN
	r_thread_id = QThread::currentThreadId ();
#	endif
#endif
	data_dir = storage_dir;
	rkd_server_name = _rkd_server_name;
}

RKRBackendProtocolBackend::~RKRBackendProtocolBackend () {
	RK_TRACE (RBACKEND);

#ifdef RKWARD_THREADED
	RKRBackendThread::instance->exitThread ();
	delete RKRBackendThread::instance;
#endif
}

void RKRBackendProtocolBackend::sendRequest (RBackendRequest *_request) {
	RK_TRACE (RBACKEND);

	RBackendRequest* request = _request;
	if (!request->synchronous) {
		request = _request->duplicate ();	// the instance we send to the frontend will remain in there, and be deleted, there
		_request->done = true;				// for aesthetics
	}
	RKRBackendEvent* event = new RKRBackendEvent (request);
#ifdef RKWARD_THREADED
	qApp->postEvent (RKRBackendProtocolFrontend::instance (), event);
#else
	RK_ASSERT (request->type != RBackendRequest::Output);
	qApp->postEvent (RKRBackendTransmitter::instance (), event);
#endif
}

bool RKRBackendProtocolBackend::inRThread () {
	return (QThread::currentThread () == instance ()->r_thread);
}

void RKRBackendProtocolBackend::msleep (int delay) {
#ifdef RKWARD_THREADED
	RKRBackendThread::instance->publicmsleep (delay);
#else
	static_cast<RKRBackendTransmitter*> (RKRBackendTransmitter::instance ())->publicmsleep (delay);
#endif
}

QString RKRBackendProtocolBackend::backendDebugFile () {
#ifdef RKWARD_THREADED
	return (QString ());
#else
	return RK_Debug_File->fileName ();
#endif
}
