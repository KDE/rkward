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
#include <QMutex>
#include "rcommand.h"

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
#ifndef RKWARD_THREADED
		Output,		/**< A piece of output. Note: If the backend runs in a single process, output is handled in a pull fashion, instead of using requests. */
#endif
		OtherRequest		/**< Any other type of request. Note: which requests are in the enum, and which are not has mostly historical reasons. @see params */
	};

	RBackendRequest (bool synchronous, RCallbackType type);
	~RBackendRequest ();

	RCommandProxy *takeCommand () {
		RCommandProxy* ret = command;
		command = 0;
		return ret;
	}

	ROutputList *takeOutput () {
		ROutputList* ret = output;
		output = 0;
		return ret;
	}

/** Should this request be handled synchronously? False by default. */
	bool synchronous;
/** For synchronous requests, only: The frontend-thread will set this to true (using completed()), once the request has been "completed". Important: The backend thread MUST NOT touch a request after it has been sent, and before "done" has been set to true. */
	bool volatile done;
	RCallbackType type;
/** For synchronous requests, only: If the the frontend wants any commands to be executed, it will place the next one in this slot. The backend thread should keep executing commands (in a sub-eventloop) while this is non-zero. Also, the backend-thread may place here any command that has just finished. */
	RCommandProxy *command;
/** Any other parameters, esp. for RCallbackType::OtherRequest. Can be used in both directions. */
	QVariantMap params;
/** NOTE: only used for separate process backend. See RCallbackType::Output */
	ROutputList *output;
#ifndef RKWARD_THREADED
/** NOTE: this does @em not copy merge the "done" flag. Do that manually, @em after merging (and don't touch the request from the transmitter thread, after that). */
	void mergeReply (RBackendRequest *reply);
#endif
protected:
	friend class RKRBackendProtocolFrontend;
	friend class RKRBackendProtocolBackend;

	void completed () {
		if (!synchronous) delete this;
		else done = true;
	}

/** duplicates the request. NOTE: The command, and output, if any are @em taken from the original, and transferred to the copy, not really duplicated. */
	RBackendRequest *duplicate ();
};

#include <QEvent>
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

/** This is a reduced version of an RCommand, intended for use in the R backend. */
class RCommandProxy : public RData {
protected:
friend class RCommand;
friend class RKRBackend;
friend class RKRBackendSerializer;
friend class RBackendRequest;
	RCommandProxy ();
	~RCommandProxy ();
	RCommandProxy (const QString &command, int type);
public:		// all these are public for technical reasons, only.
	QString command;
	int type;
	int id;
	int status;
};

#ifndef RKWARD_THREADED
	/** functions for serialization / unserialization of communication between backend and frontend.
	NOTE: This could really be a namespace, instead of a class, but "friending" a class is simply easier... */
	class RKRBackendSerializer {
	public:
		static QByteArray serialize (const RBackendRequest &request);
		static RBackendRequest *unserialize (const QByteArray &buffer);

	private:
		static void serializeOutput (const ROutputList &list, QDataStream &stream);
		static void serializeData (const RData &data, QDataStream &stream);
		static void serializeProxy (const RCommandProxy &proxy, QDataStream &stream);
		static ROutputList* unserializeOutput (QDataStream &stream);
		static RData* unserializeData (QDataStream &stream);
		static RCommandProxy* unserializeProxy (QDataStream &stream);
	};
#endif

class RKROutputBuffer {
public:
	RKROutputBuffer ();
	virtual ~RKROutputBuffer ();

/** This gets called on normal R output (R_WriteConsole). Used to get at output. */
	void handleOutput (const QString &output, int len, ROutput::ROutputType type);

/** Flushes current output buffer. Meant to be called from RInterface::flushOutput, only.
@param forcibly: if true, will always flush the output. If false, will flush the output only if the mutex can be locked without waiting. */
	ROutputList flushOutput (bool forcibly=false);
protected:
/** Function to be called while waiting for downstream threads to catch up. Return false to make the buffer continue, immediately (e.g. to prevent lockups after a crash) */
	virtual bool doMSleep (int msecs) = 0;
private:
/** current output */
	ROutputList output_buffer;
/** Provides thread-safety for the output_buffer */
	QMutex output_buffer_mutex;
/** current length of output. If the backlog of output which has not yet been processed by the frontend becomes too long, output will be paused, automatically */
	int out_buf_len;
};

#endif
