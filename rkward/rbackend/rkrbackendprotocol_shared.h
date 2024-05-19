/*
rkrbackendprotocol - This file is part of the RKWard project. Created: Thu Nov 04 2010
SPDX-FileCopyrightText: 2010-2020 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef RKRBACKENDPROTOCOL_SHARED_H
#define RKRBACKENDPROTOCOL_SHARED_H

#include <QVariantMap>
#include <QMutex>
#include "rcommand.h"

class RCommandProxy;

/** Class to represent an "event" sent between backend and frontend. This encapuslates all communication sent between the two processes (actually the graphics device uses a separate channel, but that's not the point, here).
 *
 *  - Most, but not all requests originate in the backend.
 *  - Some requests are "asynchronous" in the sense of fire and forget, i.e. the backend notifies the frontend of some condition, but does not wait for a reply.
 *  - Many requests are synchronous, in that they will block what the backend is currently doing, until the frontend posted the event back (with or without some kind of return value)). Notably, however, while waiting for a synchronous request to
 *  complete, the backend will still do event processing, which may include running further R commands (which may again involve sending requests to the frontend).
 *  - Some requests contain a "subcommandrequest". This allows the frontend to (optionally) send commands that will be executed _before_ the original/main request is completed.
 *  - All of this means that requests and their answers are not necessarily send/received in a sorted order.
 *  - In an attempt to streamline communication, the next command to process is generally sent as part of the answer to a request. This should probably be reconsidered.
 *  - The real mess, however, is that the whole mechanism has grown over time, and contains strange termms and ad-hoc additions. Parts of it should probably be re-designed from the ground up. Hopefully, these notes, help with that.
 */
class RBackendRequest {
public:
	enum RCallbackType {
		BackendExit,
		ShowMessage,
		ShowFiles,
		EditFiles,
		ReadLine,      // 4
		CommandOut,                  /**< Request the next command, and notify about the result of the previus. TODO split. */
		Started,
		RCallRequest,   // 7
		SetParamsFromBackend,
		Debugger,
		CommandLineIn,	/**< The next line of the current user command has been submitted in the backend. */
		Output,		/**< A piece of output. Note: If the backend runs in a single process, output is handled in a pull fashion, instead of using requests. */  //11
		Interrupt,	/**< Interrupt evaluation. This request type originates in the frontend, not the backend. */
		PriorityCommand, /**< Send a command to be run during R's event processing. This request type originates in the frontend, not the backend. */
		OutputStartedNotification, /**< Only used in the frontend: Notification that a new bit of output has arrived. Used to trigger flushing after a timeout. */
		OtherRequest		/**< Any other type of request. Note: which requests are in the enum, and which are not has mostly historical reasons. @see params */
	};

	RBackendRequest (bool synchronous, RCallbackType type);
	~RBackendRequest ();

	RCommandProxy *takeCommand () {
		RCommandProxy* ret = command;
		command = nullptr;
		return ret;
	}

	ROutputList *takeOutput () {
		ROutputList* ret = output;
		output = nullptr;
		return ret;
	}

	RBackendRequest *subcommandrequest;
/** Should this request be handled synchronously? False by default. */
	bool synchronous;
/** For synchronous requests, only: The frontend-thread will set this to true (using completed()), once the request has been "completed". Important: The backend thread MUST NOT touch a request after it has been sent, and before "done" has been set to true. */
	bool volatile done;
	int id;
	static int _id;
	RCallbackType type;
/** For synchronous requests, only: If the frontend wants any commands to be executed, it will place the next one in this slot. The backend thread should keep executing commands (in a sub-eventloop) while this is non-zero. Also, the backend-thread may place here any command that has just finished. */
	RCommandProxy *command;
/** Any other parameters, esp. for RCallbackType::OtherRequest. Can be used in both directions. */
	QVariantMap params;
	void setResult(const GenericRRequestResult &res);
	GenericRRequestResult getResult() const;
/** NOTE: only used for separate process backend. See RCallbackType::Output */
	ROutputList *output;
/** NOTE: this does @em not copy merge the "done" flag. Do that manually, @em after merging (and don't touch the request from the transmitter thread, after that). */
	void mergeReply (RBackendRequest *reply);
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
	explicit RKRBackendEvent(RBackendRequest* data=nullptr) : QEvent((QEvent::Type) RKWardEvent) { _data = data; };
	~RKRBackendEvent () {};

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
	RCommandProxy (const QString &command, int type);
public:		// all these are public for technical reasons, only.
	~RCommandProxy ();
	QString command;
	QString updates_object;
	int type;
	int id;
	int status;
	int has_been_run_up_to;
};

class RKROutputBuffer {
public:
	RKROutputBuffer ();
	virtual ~RKROutputBuffer ();

/** This gets called on normal R output (R_WriteConsole). Used to get at output.
    returns true, if a *new* piece of output started, i.e. the buffer was empty before this. */
	bool handleOutput (const QString &output, int len, ROutput::ROutputType type, bool allow_blocking=true);

	enum CaptureMode {
		RecordMessages = 1,
		RecordOutput = 2,
		SuppressMessages = 4,
		SuppressOutput = 8,
		NoNesting = 16
	};
	void pushOutputCapture (int capture_mode);
	QString popOutputCapture (bool highlighted);

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

	struct OutputCapture {
		ROutputList recorded;
		int mode;
	};
	QList<OutputCapture> output_captures;
};

namespace RKRSharedFunctionality {
	QString quote (const QString &string);
};

#endif
