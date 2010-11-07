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

#ifndef RKRBACKENDPROTOCOL_SHARED_H
#define RKRBACKENDPROTOCOL_SHARED_H

#ifndef RKWARD_SPLIT_PROCESS
#	define RKWARD_THREADED
#endif

#include <QVariantMap>

class RCommandProxy;

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
		SetParamsFromBackend,
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

#include "rcommand.h"

/** This is a reduced version of an RCommand, intended for use in the R backend. */
class RCommandProxy : public RData {
protected:
friend class RCommand;
friend class RKRBackend;
	RCommandProxy ();
	~RCommandProxy ();
	RCommandProxy (const QString &command, int type);
public:		// all these are public for technical reasons, only.
	QString command;
	int type;
	int id;
	int status;
};

#endif
