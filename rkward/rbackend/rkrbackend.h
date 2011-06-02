/***************************************************************************
                          rkrbackend  -  description
                             -------------------
    begin                : Sun Jul 25 2004
    copyright            : (C) 2004, 2005, 2006, 2007, 2009, 2010, 2011 by Thomas Friedrichsmeier
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
#include <QMutex>
#include <QStringList>
#include <QEvent>

#include "rcommand.h"
#include "rcommandstack.h"
#include "rkrbackendprotocol_backend.h"

#ifdef Q_WS_WIN
extern "C" {
	void RK_scheduleIntr();
}
#endif

class QStringList;
class QTextCodec;
class RInterface;
struct ROutput;

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

A closely related class is RInterface: RKRBackend communicates with RInterface by placing QCustomEvent s, when commands are done
or when the backend needs information from the frontend. For historical reasons, the definitions of RKRBackend class-members are currently spread over different files.

Only one RKRBackend-object can be used in an application.
Don't use this class in RKWard directly. Unless you really want to modify the internal workings of the backend, you will want to look at RInterface and use the functions there.

@see RInterface

@author Thomas Friedrichsmeier
*/
class RKRBackend : public RKROutputBuffer {
public: 
/** constructor. Only one RKRBackend should ever be created, and that happens in RInterface::RInterface (). */
	RKRBackend ();
/** destructor */
	virtual ~RKRBackend ();

/** interrupt processing of the current command. This is much like the user pressing Ctrl+C in a terminal with R. This is probably the only non-portable function in RKRBackend, but I can't see a good way around placing it here, or to make it portable. */
	static void interruptProcessing (bool interrupt);

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
/** low-level initialization of R */
	bool startR ();
/** reimplemented from RKROutputBuffer */
	bool doMSleep (int msecs);
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

	void handleRequest (RBackendRequest *request) { handleRequest (request, true); };
/** A relic of history. In contrast to handlePlainGenericRequest(), these requests support running sub-commands. However, the remaining requests which are currently handled this way
should probably be converted to dedicated RKRBackendRequest's in the future. See also handlePlainGenericRequest(). */
	void handleHistoricalSubstackRequest (const QStringList &list);
/** Sends a request to the frontend and returns the result (an empty QStringList in case of asynchronous requests). Note that this function has considerable overlap with
handleHistoricalSubstackRequest(). Exactly which requests get handled by which function is somewhat arbitrary, ATM. However, request that do not need sub-commands to be run, should generally be converted to use handlePlainGenericRequest(). (And probably all historicalSubstackRequests should be replaced!) */
	QStringList handlePlainGenericRequest (const QStringList &parameters, bool synchronous);
	RCommandProxy* fetchNextCommand ();

/** The command currently being executed. */
	RCommandProxy *current_command;
	QList<RCommandProxy*> all_current_commands;

	void runCommand (RCommandProxy *command);

/** only one instance of this class may be around. This pointer keeps the reference to it, for interfacing to from C to C++ */
	static RKRBackend *this_pointer;
	static void tryToDoEmergencySave ();
	bool r_running;
/** Check whether the runtime version of R is at least the given version. Valid only *after* startR() has been called! */
	bool RRuntimeIsVersion (int major, int minor, int revision) {
		return (r_version >= (1000 * major + 10 * minor + revision));
	}

/** backend is killed. Should exit as soon as possible. @see kill */
	enum KillType {
		NotKilled = 0,
		ExitNow = 1,
		EmergencySaveThenExit = 2,
		AlreadyDead = 3
	} killed;
/** "Kills" the backend. Actually this just tells the thread that is is about to be terminated. Allows the thread to terminate gracefully */
	void kill () { killed = ExitNow; };
	bool isKilled () { return (killed != NotKilled); };

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
/** A list of symbols that have been assigned new values during the current command */
	QStringList changed_symbol_names;
/** the main loop. See \ref RKRBackend for a more detailed description */
	void run ();
	static void scheduleInterrupt ();

	void startOutputCapture ();
	void printAndClearCapturedMessages (bool with_header);
	void printCommand (const QString &command);
	void catToOutputFile (const QString &out);

	QMutex all_current_commands_mutex;
	QList<RCommandProxy*> current_commands_to_cancel;
	bool too_late_to_interrupt;
	void interruptCommand (int command_id);

/** check stdout and stderr for new output (from sub-processes). Since this function is called from both threads, it is protected by a mutex.
 *  @param forcibly: if false, and the other thread currently has a lock on the mutex, do nothing, and return false.
 *  @returns: true, if output was actually fetched (or no output was available), false, if the function gave up on a locked mutex. */
	bool fetchStdoutStderr (bool forcibly, bool allow_blocking);
private:
	void clearPendingInterrupt ();
protected:
	RCommandProxy* handleRequest (RBackendRequest *request, bool mayHandleSubstack);
private:
	QMutex stdout_stderr_mutex;
	int stdout_stderr_fd;
/** set up R standard callbacks */
	void setupCallbacks ();
/** connect R standard callbacks */
	void connectCallbacks ();

	int r_version;
	QString output_file;
/** A copy of the names of the toplevel environments (as returned by "search ()"). */
	QStringList toplevel_env_names;
/** A copy of the names of the toplevel symbols in the .GlobalEnv. */
	QStringList global_env_toplevel_names;
/** check wether the object list / global environment / individual symbols have changed, and updates them, if needed */
	void checkObjectUpdatesNeeded (bool check_list);

	/** The previously executed command. Only non-zero until a new command has been requested. */
	RCommandProxy *previous_command;
};
 
#endif
