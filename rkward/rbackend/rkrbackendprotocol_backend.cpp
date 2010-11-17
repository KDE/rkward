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
	/** Private class used by the RKRBackendProtocol, in case of the backend running in a split process.
	This will be used as the secondary thread, and takes care of serializing, sending, receiving, and unserializing requests. */
	class RKRBackendTransmitter : public QThread {
	public:
		RKRBackendTransmitter (const QString &servername) {
			RK_TRACE (RBACKEND);

			current_request = 0;
			connection.connectToServer (servername);	// acutal connection will be done inside run()
		};

		~RKRBackendTransmitter () {
			RK_TRACE (RBACKEND);
			if (!current_requests.isEmpty ()) {
				RK_DO (qDebug ("%d pending requests when exiting RKRBackendTransmitter", current_requests.size ()), RBACKEND, DL_WARNING);
			}
		};

		void publicmsleep (int delay) { msleep (delay); };

		void run () {
			RK_TRACE (RBACKEND);

			if (!connection.waitForConnected ()) handleTransmitError ("Could not connect: %s");
#warning do handshake
			while (1) {
				flushOutput (false);
				request_mutex.lock ();
				while (!current_requests.isEmpty ()) {
					RBackendRequest* request = current_requests.takeFirst ();

					request_mutex.unlock ();
					flushOutput (true);
					handleRequestInternal (request);
					request_mutex.lock ();
				}
				request_mutex.unlock ();
				msleep (1);
			}
		};
		
		void postRequest (RBackendRequest *request) {
			QMutextLocker (request_mutex);

			RK_TRACE (RBACKEND);
			RK_ASSERT (request);
			current_requests.append (request);
		}

		void handleRequestInternal (RBackendRequest *request) {
			RK_TRACE (RBACKEND);
			RK_ASSERT (request);

			// send request
			QByteArray buffer = RKRBackendSerializer::serialize (*request);
			connection.write (QString::number (buffer.length ()).local8Bit ().data () + "\n");
			connection.write (buffer);
			while (connection.bytesToWrite ()) {
				if (!connection.waitForBytesWritten ()) handleTransmitError ("Could not connect: %s");
#warning, at this point we could check for an early reply to CommandOut requests
// currently, there is not as much concurrency between the processes as there could be. This is due to the fact, that the transmitter will always block until the result of the
// last command has been serialized and transmitted to the frontend. Instead, it could check for and fetch the next command, already (if available), to keep the backend going.
			}
			if (!request->synchronous) {
				delete request;			// async requests are posted as copy
				return;
			}

			// wait for reply
			// NOTE: currently, in the backend, we *never* expect a read without a synchronous request
			QByteArray receive_buffer;
			unsigned int expected_length = 0;
			bool got_header = false;
			while (1) {
				bool have_data = waitForReadyRead ();
				if (!connection.isOpen ()) {
					handleTransmitError ("Connection closed unexepctedly. Last error: %s");
					return;
				}
				if (!have_data) continue;
				if (!got_header) {
					if (!connection.canReadLine ()) continue;

					QString line = QString::fromLocal8Bit (connection.readLine ());
					bool ok;
					expected_length = line.toInt (&ok);
					if (!ok) handleTransmitError ("Protocol header error. Last connection error was: %s");
					got_header = true;
				}

				receive_buffer.append (connection.readAll (expected_length - receive_buffer.length ()));
				if (receive_buffer.length () >= expected_length () break;
			}

			RBackendRequest* reply = RBackendRequest::unserialize (fetchTransmission (true));
			request->mergeReply (reply);
			RK_ASSERT (reply->done);
			delete reply;
#warning Read up on whether volatile provides a good enough memory barrier at this point!
			request->done = true;	// must be the very last thing we do with the request!
		}

		/** fetch the next transmission.
		@param block Block until a transmission was received.
		@note @em If a transmission is available, this will always block until the transmission has been received in full. */
		QByteArray fetchTransmission (bool block) {
			QByteArray receive_buffer;
			unsigned int expected_length = 0;
			bool got_header = false;
			boo have_data = false;
			while (1) {
				bool have_data = have_data || waitForReadyRead (1);
				if (!connection.isOpen ()) {
					handleTransmitError ("Connection closed unexepctedly. Last error: %s");
					return receive_buffer;
				}
				if (!have_data) {
					if (!block) return receive_buffer;
					continue;
				}
				RK_TRACE (RBACKEND);
				if (!got_header) {
					if (!connection.canReadLine ()) continue;	// at this point we have received *something*, but not even a full header, yet.

					QString line = QString::fromLocal8Bit (connection.readLine ());
					bool ok;
					expected_length = line.toInt (&ok);
					if (!ok) handleTransmitError ("Protocol header error. Last connection error was: %s");
					got_header = true;
				}

				receive_buffer.append (connection.readAll (expected_length - receive_buffer.length ()));
				if (receive_buffer.length () >= expected_length) return receive_buffer;
			}
		}

		void flushOutput (bool force) {
			ROutputList out = RKRBackend::flushOutput (force);
			if (out.isEmpty ()) return;

			// output request would not strictly need to be synchronous. However, making them synchronous ensures that the frontend is keeping up with the output sent by the backend.
			RBackendRequest request (true, RBackendRequest::Output);
			request.output = new ROutputList (out);
			handleRequestInternal (&request);
		}

		void handleTransmitError (const char* message_template) {
			printf (message_template, qPrintable (connection.errorString ()));
		}

		QLocalSocket connection;
		QList<RBackendRequest *> current_requests;		// there *can* be multiple active requests (if the first ones are asynchronous)
		QMutex request_mutex;
	};

	int RK_Debug_Level = 2;
	int RK_Debug_Flags = ALL;
	QMutex RK_Debug_Mutex;

	void RKDebugMessageOutput (QtMsgType type, const char *msg) {
		RK_Debug_Mutex.lock ();
		if (type == QtFatalMsg) {
			fprintf (stderr, "%s\n", msg);
		}
		RKSettingsModuleDebug::debug_file->write (msg);
		RKSettingsModuleDebug::debug_file->write ("\n");
		RKSettingsModuleDebug::debug_file->flush ();
		RK_Debug_Mutex.unlock ();
	}

	int main(int argc, char *argv[]) {
		QCoreApplication app (argc, argv);
		KComponentData data ("rkward");
		KGlobal::locale ();		// to initialize it in the primary thread

		QString servername;
		QStringList args = app->arguments ();
		for (int i = 1; i < args.count (); ++i) {
			if (args[i].startsWith ("--debug-level")) {
				RK_Debug_Level = args.value (++i, QString ()).toInt ();
			} else if (args[i].startsWith ("--server-name")) {
				servername = args.value (++i, QString ());
			} else {
				printf ("unkown argument %s", qPrintable (args[i]));
			}
		}
		if (severname.isEmpty ()) {
			printf ("no server to connect to");
		}

		RKRBackendTransmitter transmitter (servername);
		RKRBackendProtocolBackend backend ();
		transmitter.start ();
		RKRBackend::this_pointer->run ();
	}
#error
#endif

RKRBackendProtocolBackend* RKRBackendProtocolBackend::_instance = 0;
RKRBackendProtocolBackend::RKRBackendProtocolBackend () {
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
#ifdef RKWARD_THREADED
	RKRBackendEvent* event = new RKRBackendEvent (request);
	qApp->postEvent (RKRBackendProtocolFrontend::instance (), event);
#else
	RBackendTransmitter::instance ()->postRequest (request);
#endif
}

bool RKRBackendProtocolBackend::inRThread () {
	return (QThread::currentThread () == instance ()->r_thread);
}

void RKRBackendProtocolBackend::msleep (int delay) {
#ifdef RKWARD_THREADED
	RKRBackendThread::instance->publicmsleep (delay);
#else
	RKRBackendTransmitter::instance->publicmsleep (delay);
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
