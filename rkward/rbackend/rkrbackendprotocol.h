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

#ifndef RKRBACKENDPROTOCOL_H
#define RKRBACKENDPROTOCOL_H

#include <QVariantMap>
#include <QObject>

#include "rcommand.h"

#ifndef RKWARD_SPLIT_PROCESS
#	define RKWARD_THREADED
#endif

class RKRBackendProtocolFrontend;
class RKRBackendProtocolBackend;
class RKRBackend;
class RInterface;

class RBackendRequest {
public:
	enum RCallbackType {
		BackendExit,
		ShowMessage,
		ShowFiles,
		ChooseFile,
		EditFiles,
		ReadLine,
		CommandOut,
		Started,
		EvalRequest,
		CallbackRequest,
		HistoricalSubstackRequest,
		OtherRequest		/**< Any other type of request. Note: which requests are in the enum, and which are not has mostly historical reasons. @see params */
	};

	RBackendRequest (bool synchronous, RCallbackType type) {
		RBackendRequest::synchronous = synchronous;
		RBackendRequest::type = type;
		done = false;
		command = 0;
	}
	~RBackendRequest () {};

/** Should this request be handled synchronously? False by default. */
	bool synchronous;
/** For synchronous requests, only: The frontend-thread will set this to true (using completed()), once the request has been "completed". Important: The backend thread MUST NOT touch a request after it has been sent, and before "done" has been set to true. */
	bool done;
	RCallbackType type;
/** For synchronous requests, only: If the the frontend wants any commands to be executed, it will place the next one in this slot. The backend thread should keep executing commands (in a sub-eventloop) while this is non-zero. Also, the backend-thread may place here any command that has just finished. */
	RCommandProxy *command;
/** Any other parameters, esp. for RCallbackType::OtherRequest. Can be used in both directions. */
	QVariantMap params;
protected:
	friend class RKRBackendProtocolFrontend;
	friend class RKRBackendProtocolBackend;

	void completed () {
		if (!synchronous) delete this;
		else done = true;
	}

	RBackendRequest *duplicate () {
		RBackendRequest* ret = new RBackendRequest (synchronous, type);
		ret->done = done;
		ret->command = command;
		ret->params = params;
		return ret;
	}
};

#ifdef RKWARD_THREADED
#	include <QEvent>
	/** Simple event class to relay information from the RKRBackend to the main thread. This is basically like QCustomEvent in Qt3*/
	class RKRBackendEvent : public QEvent {
	public:
		enum EventType {
			RKWardEvent = QEvent::User + 1
		};
		RKRBackendEvent (RBackendRequest* data=0) : QEvent ((QEvent::Type) RKWardEvent) { _data = data; };
		RKRBackendEvent ();

		RBackendRequest* data () { return _data; };
	private:
		RBackendRequest* _data;
	};
#endif

class RKRBackendProtocolFrontend : public QObject {
public:
	RKRBackendProtocolFrontend (RInterface* parent);
	~RKRBackendProtocolFrontend ();

	static void setRequestCompleted (RBackendRequest *request);
	ROutputList flushOutput (bool force);
	void interruptProcessing ();
	void terminateBackend ();
	void setupBackend (QVariantMap backend_params);
	static RKRBackendProtocolFrontend* instance () { return _instance; };
protected:
#ifdef RKWARD_THREADED
/** needed to handle the QEvents, the R thread is sending (notifications on what's happening in the backend thread) */
	void customEvent (QEvent *e);
#endif
private:
	static RKRBackendProtocolFrontend* _instance;
	RInterface *frontend;
};

class RKRBackendProtocolBackend {
public:
	static bool inRThread ();
protected:
friend class RKRBackendProtocolFrontend;
friend class RKRBackend;
friend class RKRBackendThread;
	RKRBackendProtocolBackend ();
	~RKRBackendProtocolBackend ();

	void sendRequest (RBackendRequest *request);
	static void msleep (int delay);
	static void interruptProcessing ();
	static RKRBackendProtocolBackend* instance () { return _instance; };
private:
	static RKRBackendProtocolBackend* _instance;
};

#endif
