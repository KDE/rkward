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
#include <QStringList>
#include <QEvent>

#include "rcommand.h"
#include "rcommandstack.h"

#ifdef Q_WS_WIN
extern "C" {
	void RK_scheduleIntr();
}
#endif

/** This struct is used to pass the standard callbacks from R to the main thread (if needed; some are handled in the backend thread). Note that for the callbacks that need to be passed to the main
thread, we can be quite wasteful both in terms of cycles and memory, since these are usually
requests for user interaction. Hence we use a QVariantMap to accommodate all the different needed
parameters, easily, and in a readable way. */
struct RCallbackArgs {
/** is main thread done with the callback, yet? Initialized to false inside the true handler: RThread::doStandardCallback () */
	bool done;
/** type of the callback */
	enum RCallbackType {
		RBackendExit,
		RShowMessage,
		RShowFiles,
		RChooseFile,
		REditFiles,
		RReadLine
       } type;
/** All the parameters sent in either direction */
	QVariantMap params;
};

class QStringList;
class QTextCodec;
class RInterface;
struct RCallbackArgs;
struct ROutput;

/** this struct is used to pass on eval-requests (i.e. request for RKWard to do something, which may involve issuing further commands) from the
backend-thread to the main thread. Do not use outside the backend-classes. */
struct REvalRequest {
private:
friend class RInterface;
friend class RThread;
	QStringList call;
	RCommandChain *in_chain;
};

/** Simple event class to relay information from the RThread to the main thread. This is basically like QCustomEvent in Qt3*/
class RKRBackendEvent : public QEvent {
public:
	enum EventType {
		Base = QEvent::User + 1,
		RCommandIn,
		RCommandOut,
		RBusy,
		RIdle,
		RCommandOutput,
		RStarted,
		REvalRequest,
		RCallbackRequest,
		RStartupError
	};

	RKRBackendEvent (EventType type, void* data=0) : QEvent ((QEvent::Type) type) { _data = data; };
	RKRBackendEvent ();

	EventType etype () { return ((EventType) type ()); };
	void* data () { return _data; };
private:
	void* _data;
};

/** This class represents the thread the R backend is running in. So to speak, this is where the "eventloop" of R is running. The main thing happening
in this class, is that an infinite loop is running. Whenever there are commands to be executed, those get evaluated. Also, at regular intervals,
processing of X11-Events in R is triggered. The rest of the time the thread sleeps.

Actually, there are really two copies of the main loop: The regular one, and a second one which gets run when the R backend has requested some
task to be carried out (@see handleSubstackCall). In this case, we might have to run some further child commands in the backend, before we proceed with the commands in
the main queque. Some thing like:

- Run some RCommand s
	- R backend asks for some information / action
		- potentially some more RCommand s are needed to accomplish this request
			- (additional levels of substacks)
		- return the result
	- R backend request completed
- Run some more RCommand s

This subordinate/nested eventloop is done in handleSubstackCall ().

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

/** Pause output by placing it in a delay loop, until unpaused again */
	void pauseOutput (bool paused) { output_paused = paused; };
/** the internal counterpart to pauseOutput () */
	void waitIfOutputPaused ();

/** interrupt processing of the current command. This is much like the user pressing Ctrl+C in a terminal with R. This is probably the only non-portable function in RThread, but I can't see a good way around placing it here, or to make it portable. */
	void interruptProcessing (bool interrupt);

/** Enum specifying types of errors that may occur while parsing/evaluating a command in R */
	enum RKWardRError {
		NoError=0,			/**< No error */
		Incomplete=1,		/**< The command is incomplete. Command was syntactically ok up to given point, but incomplete. It may or may not be semantically correct. */
		SyntaxError=2,		/**< Syntax error */
		OtherError=3		/**< Other error, usually a semantic error, e.g. object not found */
	};

/** An enum describing whether initialization of the embedded R-process went well, or what errors occurred. */
	enum InitStatus {
		Ok=0,					/**< No error */
		LibLoadFail=1,		/**< Error while trying to load the rkward R package */
		SinkFail=2,			/**< Error while redirecting R's stdout and stderr to files to be read from rkward */
		OtherFail=4			/**< Other error while initializing the embedded R-process */
	}; // TODO: make obsolete!

/** initializes the R-backend. Returns an error-code that consists of a bit-wise or-conjunction of the RThread::InitStatus -enum, RThread::Ok on success.
Note that you should call initialize only once in a application */
	int initialize ();

	void enterEventLoop ();

/** clean shutdown of R.
@param suicidal if true, perform only the most basic shutdown operations */
	void shutdown (bool suicidal);
protected:
/** low-level initialization of R
@param argc Number of arguments as would be passed on the commandline to R
@param argv Arguments as would be passed on the commandline to R
@param stack_check C stack checking enabled */
	bool startR (int argc, char **argv, bool stack_check);

/** convenience low-level function for running a command, directly
@param command command to be runCommand
@returns true if command was run successfully, false in case of an error */
	bool runDirectCommand (const QString &command);
/** convenience low-level function for running a command, directly. Use this overload, if you want to handle a return value.
@param command command to be runCommand
@param datatype the data type that should be (attempted to be) returned
@returns a pointer to the RCommand-instance that was created and used, internally. You can query this pointer for status and data. Be sure to delete it, when done. */
	RCommand *runDirectCommand (const QString &command, RCommand::CommandTypes datatype); 
public:
/** call this periodically to make R's x11 windows process their events */
	static void processX11Events ();

/** convenience struct for event passing */
	struct ROutputContainer {
		/** the actual output fragment */
		ROutput *output;
		/** the corresponding command */
		RCommand *command;
	};
/** current output */
	ROutput *current_output;
/** current length of output. Used so we can flush every once in a while, if output becomes too long */
	int out_buf_len;

/** This gets called on normal R output (R_WriteConsole). Used to get at output. */
	void handleOutput (const QString &output, int len, bool regular);

/** Flushes current output buffer. Lock the mutex before calling this function! It is called from both threads and is not re-entrant */
	void flushOutput ();

/** This gets called, when R reports an error (override of options ("error") in R). Used to get at error-output.
This function is public for technical reasons, only. Don't use except from R-backend code!
reports an error. */
	void handleError (QString *call, int call_length);

/** This is a sub-eventloop, being run when the backend request information from the frontend. See \ref RThread for a more detailed description */
	void handleSubstackCall (QStringList &call);

/** This is a minimal sub-eventloop, being run, when the backend requests simple information from the frontend. It differs from handleSubstack in two
points:
1) it does not create a full-fledged substack for additional R commands
2) it may return information via the args parameter immediately */
	void handleStandardCallback (RCallbackArgs *args);

/** The command currently being executed. */
	RCommand *current_command;

	void runCommand (RCommand *command);

/** only one instance of this class may be around. This pointer keeps the reference to it, for interfacing to from C to C++ */
	static RThread *this_pointer;
	static char *na_char_internal;
	static void tryToDoEmergencySave ();
	bool r_running;
/** Check whether the runtime version of R is at least the given version. Valid only *after* startR() has been called! */
	bool RRuntimeIsVersion (int major, int minor, int revision) {
		return (r_version >= (1000 * major + 10 * minor + revision));
	}

/** @see lock (), @see unlock ()*/
	enum LockType {
		User=1,		/**< locked on user request */
		Cancel=2,	/**< locked to safely cancel a running command */
		Startup=4	/**< locked on startup */
	};

/** Locks the thread. This is called by RInterface, when the currently running command is to be cancelled. It is used to make sure that the
backend thread does not proceed with further commands, before the main thread takes notice. Also it is called, if the RThread is paused on User request. Further, the thread is initially locked so the main thread can check for some conditions before the backend thread may produce
more errors/crashes. @see unlock @see RInterface::cancelCommand @see RInterface::pauseProcessing
@param reason As there are several reasons to lock the thread, and more than one reason may be in place at a given time, a reason needs to be specified for both lock () and unlock (). Only if all "reasons are unlocked ()", processing continues. */
	void lock (LockType reason) { locked |= reason; };
/** Unlocks the thread.  Also the thread may get locked when canceling the currently running command. @see lock */
	void unlock (LockType reason) { locked -= (locked & reason); };
/** "Kills" the thread. Actually this just tells the thread that is is about to be terminated. Allows the thread to terminate gracefully */
	void kill () { killed = true; };
	bool isKilled () { return killed; };

	QTextCodec *current_locale_codec;

	struct RKReplStatus {
		QByteArray user_command_buffer;
		int user_command_transmitted_up_to;
		bool user_command_completely_transmitted;
		int user_command_successful_up_to;
		enum {
			NoUserCommand,
			UserCommandTransmitted,
			UserCommandSyntaxError,
			UserCommandRunning
		} user_command_status;
		int eval_depth;		// Number (depth) of non-user commands currently running. User commands can only run at depth 0
	};
	static RKReplStatus repl_status;
protected:
/** thread is locked. No new commands will be executed. @see LockType @see lock @see unlock */
	int locked;
/** thread is killed. Should exit as soon as possible. @see kill */
	bool killed;
/** On pthread systems this is the pthread_id of the backend thread. It is needed to send SIGINT to the R backend */
	Qt::HANDLE thread_id;
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
/** This is the function in which an RCommand actually gets processed. Basically it passes the command to runCommand () and sends RInterface some events about what is currently happening. */
	void doCommand (RCommand *command);
	void notifyCommandDone (RCommand *command);
/** The internal storage for pauseOutput () */
	bool output_paused;

/** A copy of the names of the toplevel environments (as returned by "search ()"). */
	QStringList toplevel_env_names;
/** A copy of the names of the toplevel symbols in the .GlobalEnv. */
	QStringList global_env_toplevel_names;
/** A list of symbols that have been assigned new values during the current command */
	QStringList changed_symbol_names;
/** check wether the object list / global environment / individual symbols have changed, and updates them, if needed */
	void checkObjectUpdatesNeeded (bool check_list);
	QList<RCommand*> all_current_commands;
};
 
#endif
