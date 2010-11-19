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

#include "rkrbackendprotocol_backend.h"

#include "rkrbackend.h"

#include "../debug.h"

#include <QCoreApplication>
#include <QThread>
#ifndef Q_WS_WIN
#	include <signal.h>		// needed for pthread_kill
#	include <pthread.h>		// seems to be needed at least on FreeBSD
#endif

#ifdef RKWARD_THREADED
#	include "rkrbackendprotocol_frontend.h"
#else
#	include <QLocalSocket>
#	include <QMutex>
#	include "kcomponentdata.h"
#	include "kglobal.h"
#	include "rktransmitter.h"
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
				RK_DO (qDebug ("Waiting for R thread to finish up..."), RBACKEND, DL_INFO);
				RKRBackendProtocolBackend::interruptProcessing ();
				RKRBackend::this_pointer->kill ();
				wait (1000);
				if (isRunning ()) {
					RK_DO (qDebug ("Backend thread is still running. It will be killed, now."), RBACKEND, DL_WARNING);
					terminate ();
					yieldCurrentThread ();
					RK_ASSERT (false);
				}
			}
		};

		void publicmsleep (int delay) { msleep (delay); };

		void run () {
			RK_TRACE (RBACKEND);
			RKRBackendProtocolBackend::instance ()->r_thread_id = currentThreadId ();
			RKRBackend::this_pointer->run ();
		}

		/** On pthread systems this is the pthread_id of the backend thread. It is needed to send SIGINT to the R backend */
		Qt::HANDLE thread_id;
		static RKRBackendThread* instance;
	};
	RKRBackendThread* RKRBackendThread::instance = 0;
#else
#	include "rkbackendtransmitter.h"

#	include "ktemporaryfile.h"
#	include "krandom.h"
	int RK_Debug_Level = 2;
	int RK_Debug_Flags = ALL;
	QMutex RK_Debug_Mutex;
	KTemporaryFile* RK_Debug_File;

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

	int main(int argc, char *argv[]) {
		QCoreApplication app (argc, argv);
		KComponentData data ("rkward");
		KGlobal::locale ();		// to initialize it in the primary thread

		RK_Debug_File = new KTemporaryFile ();
		RK_Debug_File->setPrefix ("rkward.rbackend");
		RK_Debug_File->setAutoRemove (false);
		if (RK_Debug_File->open ()) qInstallMsgHandler (RKDebugMessageOutput);

		QString servername;
		QString data_dir;
		QStringList args = app.arguments ();
		for (int i = 1; i < args.count (); ++i) {
			if (args[i].startsWith ("--debug-level")) {
				RK_Debug_Level = args[i].section (' ', 1).toInt ();
			} else if (args[i].startsWith ("--server-name")) {
				servername = args[i].section (' ', 1);
			} else if (args[i].startsWith ("--data-dir")) {
#warning What about paths with spaces?!
				data_dir = args[i].section (' ', 1);
			} else {
				printf ("unkown argument %s", qPrintable (args[i]));
			}
		}
		if (servername.isEmpty ()) {
			printf ("no server to connect to\n");
			return 1;
		}

		// a simple security token to all the frontend to make sure that it is really talking to the backend process that it started in the local socket connection.
		// this token is sent both via stdout and the local socket connection. The frontend simply compares both values.
		QString token = KRandom::randomString (32);
		printf ("%s\n", token.toLocal8Bit ().data ());
		fflush (stdout);

		RKRBackendTransmitter transmitter (servername, token);
		RKRBackendProtocolBackend backend (data_dir);
		transmitter.start ();
		RKRBackend::this_pointer->run ();
		transmitter.quit ();
		transmitter.wait (5000);

		if (!RKRBackend::this_pointer->isKilled ()) RKRBackend::tryToDoEmergencySave ();
	}
#endif

RKRBackendProtocolBackend* RKRBackendProtocolBackend::_instance = 0;
RKRBackendProtocolBackend::RKRBackendProtocolBackend (const QString &storage_dir) {
	RK_TRACE (RBACKEND);

	_instance = this;
	new RKRBackend ();
#ifdef RKWARD_THREADED
	r_thread = new RKRBackendThread ();
	// NOTE: r_thread_id is obtained from within the thread
	RKRBackendThread::instance->start ();
#else
	r_thread = QThread::currentThread ();	// R thread == main thread
	r_thread_id = QThread::currentThreadId ();
#endif
	data_dir = storage_dir;
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

void RKRBackendProtocolBackend::interruptProcessing () {
	if (inRThread ()) {
		RKRBackend::scheduleInterrupt ();
	} else {
#ifdef Q_WS_WIN
		RKRBackend::scheduleInterrupt ();		// Thread-safe on windows?!
#else
		pthread_kill ((pthread_t) instance ()->r_thread_id, SIGUSR1);	// NOTE: SIGUSR1 relays to SIGINT
#endif
	}
}
