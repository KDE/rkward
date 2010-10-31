/***************************************************************************
                          rembedinternal  -  description
                             -------------------
    begin                : Sun Jul 25 2004
    copyright            : (C) 2004, 2005, 2006, 2007, 2009 by Thomas Friedrichsmeier
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

#ifndef R_EMBED_H
#define R_EMBED_H

#include <stddef.h>

#include <QMap>
#include <QVariant>
#include <QThread>
#include <QMutex>
#include <QStringList>
#include <QEvent>

#include "rcommand.h"
#include "rcommandstack.h"

#ifdef Q_WS_WIN
extern "C" {
	void RK_scheduleIntr();
}
#endif

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

	RBackendRequest *duplicate () {
		RBackendRequest* ret = new RBackendRequest (synchronous, type);
		ret->done = done;
		ret->command = command;
		ret->params = params;
		return ret;
	}

	void completed () {
		if (!synchronous) delete this;
		else done = true;
	}

/** Should this request be handled synchronously? False by default. */
	bool synchronous;
/** For synchronous requests, only: The frontend-thread will set this to true (using completed()), once the request has been "completed". Important: The backend thread MUST NOT touch a request after it has been sent, and before "done" has been set to true. */
	bool done;
	RCallbackType type;
/** For synchronous requests, only: If the the frontend wants any commands to be executed, it will place the next one in this slot. The backend thread should keep executing commands (in a sub-eventloop) while this is non-zero. Also, the backend-thread may place here any command that has just finished. */
	RCommandProxy *command;
/** Any other parameters, esp. for RCallbackType::OtherRequest. Can be used in both directions. */
	QVariantMap params;
};

class QStringList;
class QTextCodec;
class RInterface;
struct ROutput;

/** Simple event class to relay information from the RThread to the main thread. This is basically like QCustomEvent in Qt3*/
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

/** This class represents the thread the R backend is running in. So to speak, this is where the "eventloop" of R is running. The main thing happening
in this class, is that it enters R's REPL (Read-evaluate-parse-loop). Whenever there are commands to be executed, those get evaluated. When there are not,
processing of X11-Events in R is triggered. The rest of the time the thread sleeps.

Actually, there is also a custom sub-eventloop, which gets called when the R backend has requested something. See handleRequest(). In this case, we might have to run some further child commands in the backend, before we proceed with the commands in the main queque. Some thing like:

- Run some RCommand s
	- R backend asks for some information / action
		- potentially some more RCommand s are needed to accomplish this request
			- (additional levels of substacks)
		- return the result
	- R backend request completed
- Run some more RCommand s

This subordinate/nested eventloop is done in handleRequest ().

A closely related class is RInterface: RThread communicates with RInterface by placing QCustomEvent s, when commands are done
or when the backend needs information from the frontend. For historical reasons, the definitions of RThread class-members are currently spread over different files.

Only one RThread-object can be used in an application.
Don't use this class in RKWard directly. Unless you really want to modify the internal workings of the backend, you will want to look at RInterface and use the functions there.

@see RInterface

@author Thomas Friedrichsmeier
*/
class RThread : public QThread {
public: 
/** constructor. Only one RThread should ever be created, and that happens in RInterface::RInterface (). */
	RThread ();
/** destructor */
	virtual ~RThread ();

/** interrupt processing of the current command. This is much like the user pressing Ctrl+C in a terminal with R. This is probably the only non-portable function in RThread, but I can't see a good way around placing it here, or to make it portable. */
	void interruptProcessing (bool interrupt);

/** Enum specifying types of errors that may occur while parsing/evaluating a command in R */
	enum RKWardRError {
		NoError=0,			/**< No error */
		Incomplete=1,		/**< The command is incomplete. Command was syntactically ok up to given point, but incomplete. It may or may not be semantically correct. */
		SyntaxError=2,		/**< Syntax error */
		OtherError=3		/**< Other error, usually a semantic error, e.g. object not found */
	};

/** initializes the R-backend. Emits an RCallbackType::Started-request (with any error messages) when done.
Note that you should call initialize only once in a application */
	void initialize ();

	void enterEventLoop ();
protected:
/** low-level initialization of R
@param argc Number of arguments as would be passed on the commandline to R
@param argv Arguments as would be passed on the commandline to R
@param stack_check C stack checking enabled */
	bool startR (int argc, char **argv, bool stack_check);
public:
/** convenience low-level function for running a command, directly
@param command command to be runCommand
@returns true if command was run successfully, false in case of an error */
	bool runDirectCommand (const QString &command);
/** convenience low-level function for running a command, directly. Use this overload, if you want to handle a return value.
@param command command to be runCommand
@param datatype the data type that should be (attempted to be) returned
@returns a pointer to the RCommandProxy-instance that was created and used, internally. You can query this pointer for status and data. Be sure to delete it, when done. */
	RCommandProxy *runDirectCommand (const QString &command, RCommand::CommandTypes datatype); 
/** call this periodically to make R's x11 windows process their events */
	static void processX11Events ();

/** This gets called on normal R output (R_WriteConsole). Used to get at output. */
	void handleOutput (const QString &output, int len, ROutput::ROutputType type);

/** Flushes current output buffer. Meant to be called from RInterface::flushOutput, only.
@param forcibly: if true, will always flush the output. If false, will flush the output only if the mutex can be locked without waiting. */
	ROutputList flushOutput (bool forcibly=false);

	void handleRequest (RBackendRequest *request) { handleRequest (request, true); };
/** A relic of history. Eventually most of these will be replaced by dedicated RBackendRequests. */
	void handleHistoricalSubstackRequest (const QStringList &list);
	RCommandProxy* fetchNextCommand ();

/** The command currently being executed. */
	RCommandProxy *current_command;
	QList<RCommandProxy*> all_current_commands;

	void runCommand (RCommandProxy *command);

/** only one instance of this class may be around. This pointer keeps the reference to it, for interfacing to from C to C++ */
	static RThread *this_pointer;
	static char *na_char_internal;
	static void tryToDoEmergencySave ();
	bool r_running;
/** Check whether the runtime version of R is at least the given version. Valid only *after* startR() has been called! */
	bool RRuntimeIsVersion (int major, int minor, int revision) {
		return (r_version >= (1000 * major + 10 * minor + revision));
	}

/** "Kills" the thread. Actually this just tells the thread that is is about to be terminated. Allows the thread to terminate gracefully */
	void kill () { killed = true; };
	bool isKilled () { return killed; };

	QTextCodec *current_locale_codec;

	struct RKReplStatus {
		QByteArray user_command_buffer;
		int user_command_transmitted_up_to;
		bool user_command_completely_transmitted;
		int user_command_parsed_up_to;
		int user_command_successful_up_to;
		enum {
			NoUserCommand,
			UserCommandTransmitted,
			UserCommandSyntaxError,
			UserCommandRunning,
			UserCommandFailed,
			ReplIterationKilled
		} user_command_status;
		int eval_depth;		// Number (depth) of non-user commands currently running. User commands can only run at depth 0
		bool in_browser_context;
		bool interrupted;
	};
	static RKReplStatus repl_status;
/** holds a copy of the default R_GlobalContext. Needed to find out, when a browser context has been left. */
	static void *default_global_context;

	void commandFinished (bool check_object_updates_needed=true);
/** thread is killed. Should exit as soon as possible. @see kill */
	bool killed;
/** A list of symbols that have been assigned new values during the current command */
	QStringList changed_symbol_names;
protected:
/** On pthread systems this is the pthread_id of the backend thread. It is needed to send SIGINT to the R backend */
	Qt::HANDLE thread_id;
/** If the length of the current output buffer is too long, this will pause any further output until the main thread has had a chance to catch up. */
	void waitIfOutputBufferExceeded ();
	RCommandProxy* handleRequest (RBackendRequest *request, bool mayHandleSubstack);
private:
/** set up R standard callbacks */
	void setupCallbacks ();
/** connect R standard callbacks */
	void connectCallbacks ();

	int r_version;
protected:
/** the main loop. See \ref RThread for a more detailed description */
	void run ();
private:  
/** A copy of the names of the toplevel environments (as returned by "search ()"). */
	QStringList toplevel_env_names;
/** A copy of the names of the toplevel symbols in the .GlobalEnv. */
	QStringList global_env_toplevel_names;
/** check wether the object list / global environment / individual symbols have changed, and updates them, if needed */
	void checkObjectUpdatesNeeded (bool check_list);

	/** current output */
	ROutputList output_buffer;
/** Provides thread-safety for the output_buffer */
	QMutex output_buffer_mutex;
/** current length of output. If the backlog of output which has not yet been processed by the frontend becomes too long, output will be paused, automatically */
	int out_buf_len;

	/** The previously executed command. Only non-zero until a new command has been requested. */
	RCommandProxy *previous_command;
};
 
#endif
