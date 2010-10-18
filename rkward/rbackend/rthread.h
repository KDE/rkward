/***************************************************************************
                          rthread  -  description
                             -------------------
    begin                : Mon Aug 2 2004
    copyright            : (C) 2004, 2006, 2007, 2009 by Thomas Friedrichsmeier
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
#ifndef RTHREAD_H
#define RTHREAD_H

#include <qthread.h>
#include <qstringlist.h>
#include <QList>
#include <QEvent>
#include <QDateTime>

#include "rcommand.h"
#include "rcommandstack.h"
#include "rembedinternal.h"

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
or when the backend needs information from the frontend. Also RThread and REmbedInternal are only separate for technical reasons (R-includes and Qt-includes clashing).

Only one RThread-object can be used in an application.
Don't use this class in RKWard directly. Unless you really want to modify the internal workings of the backend, you will want to look at RInterface and use the functions there.

@see RInterface
@see REmbedInternal

@author Thomas Friedrichsmeier
*/
class RThread : public QThread, public REmbedInternal {
public:
/** constructor. Only one RThread should ever be created, and that happens in RInterface::RInterface (). */
	RThread ();
/** destructor */
	~RThread ();

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
/** Pause output by placing it in a delay loop, until unpaused again */
	void pauseOutput (bool paused) { output_paused = paused; };
/** the internal counterpart to pauseOutput () */
	void waitIfOutputPaused ();

/** An enum describing whether initialization of the embedded R-process went well, or what errors occurred. */
	enum InitStatus {
		Ok=0,					/**< No error */
		LibLoadFail=1,		/**< Error while trying to load the rkward R package */
		SinkFail=2,			/**< Error while redirecting R's stdout and stderr to files to be read from rkward */
		OtherFail=4			/**< Other error while initializing the embedded R-process */
	};

/** initializes the R-backend. Returns an error-code that consists of a bit-wise or-conjunction of the RThread::InitStatus -enum, RThread::Ok on success.
Note that you should call initialize only once in a application */
	int initialize ();

/** this function is public for technical reasons, only. Don't use except from REmbedInternal! Called from REmbedInternal when the R backend
generates standard output. @see REmbedInternal::handleOutput () */
	void handleOutput (const QString &output, int buf_length, bool regular);

/** Flushes current output buffer. Lock the mutex before calling this function! It is called from both threads and is not re-entrant */
	void flushOutput ();

/** this function is public for technical reasons, only. Don't use except from REmbedInternal! Called from REmbedInternal when the R backend
signals a condition. @see REmbedInternal::handleCondition () */
//	void handleCondition (char **call, int call_length);

/** this function is public for technical reasons, only. Don't use except from REmbedInternal! Called from REmbedInternal when the R backend
reports an error. @see REmbedInternal::handleError () */
	void handleError (QString *call, int call_length);

/** This function is public for technical reasons, only. Don't use except from REmbedInternal!

This is a sub-eventloop, being run when the backend request information from the frontend. See \ref RThread for a more detailed description
@see REmbedInternal::handleSubstackCall () */
	void handleSubstackCall (QStringList &call);

/** This function is public for technical reasons, only. Don't use except from REmbedInternal!

This is a minimal sub-eventloop, being run, when the backend requests simple information from the frontend. It differs from handleSubstack in two
points:
1) it does not create a full-fledged substack for additional R commands
2) it may return information via the args parameter immediately

@see REmbedInternal::handleStandardCallback () */
	void handleStandardCallback (RCallbackArgs *args);

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

/** interrupt processing of the current command. This is much like the user pressing Ctrl+C in a terminal with R. This is probably the only non-portable function in RThread, but I can't see a good way around placing it here, or to make it portable. */
	void interruptProcessing (bool interrupt);

/** On pthread systems this is the pthread_id of the backend thread. It is needed to send SIGINT to the R backend */
	Qt::HANDLE thread_id;
protected:
/** the main loop. See \ref RThread for a more detailed description */
	void run ();
private:
/** This is the function in which an RCommand actually gets processed. Basically it passes the command to REmbedInteranl::runCommandInternal () and sends RInterface some events about what is currently happening. */
	void doCommand (RCommand *command);
	void notifyCommandDone (RCommand *command);
/** thread is locked. No new commands will be executed. @see LockType @see lock @see unlock */
	int locked;
/** thread is killed. Should exit as soon as possible. @see kill */
	bool killed;
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
