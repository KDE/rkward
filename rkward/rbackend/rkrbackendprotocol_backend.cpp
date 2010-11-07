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

#include "rembedinternal.h"

#include "../debug.h"

#ifdef RKWARD_THREADED
#	ifndef Q_WS_WIN
#		include <signal.h>		// needed for pthread_kill
#		include <pthread.h>		// seems to be needed at least on FreeBSD
#	endif
#	include <QThread>
#	include <QApplication>
#	include "rkrbackendprotocol_frontend.h"
#else
#	include <unistd.h>
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
			thread_id = currentThreadId ();
			RKRBackend::this_pointer->run ();
		}

		/** On pthread systems this is the pthread_id of the backend thread. It is needed to send SIGINT to the R backend */
		Qt::HANDLE thread_id;
		static RKRBackendThread* instance;
	};
	RKRBackendThread* RKRBackendThread::instance = 0;
#else
#error
#endif

RKRBackendProtocolBackend* RKRBackendProtocolBackend::_instance = 0;
RKRBackendProtocolBackend::RKRBackendProtocolBackend () {
	RK_TRACE (RBACKEND);

	_instance = this;
	new RKRBackend ();
#ifdef RKWARD_THREADED
	new RKRBackendThread ();
	RKRBackendThread::instance->start ();
#else
#error
#endif
}

RKRBackendProtocolBackend::~RKRBackendProtocolBackend () {
	RK_TRACE (RBACKEND);

#ifdef RKWARD_THREADED
	RKRBackendThread::instance->exitThread ();
	delete RKRBackendThread::instance;
#else
#error
#endif
}

void RKRBackendProtocolBackend::sendRequest (RBackendRequest *_request) {
	RK_TRACE (RBACKEND);

#ifdef RKWARD_THREADED
	RBackendRequest* request = _request;
	if (!request->synchronous) {
		request = _request->duplicate ();	// the instance we send to the frontend will remain in there, and be deleted, there
		_request->done = true;				// for aesthetics
	}

	RKRBackendEvent* event = new RKRBackendEvent (request);
	qApp->postEvent (RKRBackendProtocolFrontend::instance (), event);
#else
#error
#endif
}

bool RKRBackendProtocolBackend::inRThread () {
#ifdef RKWARD_THREADED
	return (QThread::currentThread () == RKRBackendThread::instance);
#else
#error
#endif
}

void RKRBackendProtocolBackend::msleep (int delay) {
#	ifdef RKWARD_THREADED
	RKRBackendThread::instance->publicmsleep (delay);
#else
	usleep (delay * 1000);
#endif
}

void RKRBackendProtocolBackend::interruptProcessing () {
	if (inRThread ()) {
		RKRBackend::scheduleInterrupt ();
	} else {
#ifdef Q_WS_WIN
		RKRBackend::scheduleInterrupt ();
#else
#		ifdef RKWARD_THREADED
		pthread_kill ((pthread_t) RKRBackendThread::instance->thread_id, SIGUSR1);	// relays to SIGINT
#		else
#		error
#		endif
	}
#endif
}
