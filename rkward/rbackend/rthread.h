/***************************************************************************
                          rthread  -  description
                             -------------------
    begin                : Mon Aug 2 2004
    copyright            : (C) 2004 by Thomas Friedrichsmeier
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

#include "rcommand.h"
#include "rcommandstack.h"
#include "rembedinternal.h"

class RInterface;
struct RCallbackArgs;

#define RCOMMAND_IN_EVENT 10001
#define RCOMMAND_OUT_EVENT 10002
#define RBUSY_EVENT 10003
#define RIDLE_EVENT 10004
#define RSTARTED_EVENT 11001
#define R_EVAL_REQUEST_EVENT 12001
#define R_CALLBACK_REQUEST_EVENT 12002
// don't use the numbers following RSTARTUP_ERROR_EVENT, because an error code will be added!
#define RSTARTUP_ERROR_EVENT 13000

/** This class represents the thread the R backend is running in. So to speak, this is where the "eventloop" of R is running. The main thing happening
in this class, is that an infinite loop is running. Whenever there are commands to be executed, those get evaluated. Also, at regular intervals,
processing of X11-Events in R is triggered. The rest of the time the thread sleeps.

Actually, there are really two copies of the main loop: The regular one, and a second one which gets run when the R backend has requested some
task to be carried out. In this case, we might have to run some further child commands in the backend, before we proceed with the commands in
the main queque. Some thing like:

- Run some R commands
	- R backend asks for some information / action
		- potentially some more R commands are needed to accomplish this request
			- (additional levels of substacks)
		- return the result
	- R backend request completed
- Run some more R commands

This subordinate/nested eventloop is done in doSubstack ().

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
/** constructor. You need to specify a pointer to the RInterface, so the thread knows where to post its events. Only one RThread should ever be
created, and that happens in RInterface::RInterface (). */
	RThread ();
/** destructor */
	~RThread();

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
/** Unlocks the thread.  Also the thread may get locked when cancelling the currently running command. @see lock */
	void unlock (LockType reason) { locked -= (locked & reason); };
/** "Kills" the thread. Actually this just tells the thread that is is about to be terminated. Allows the thread to terminate gracefully */
	void kill () { killed = true; };

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
	void handleOutput (char *buf, int buf_length);

/** this function is public for technical reasons, only. Don't use except from REmbedInternal! Called from REmbedInternal when the R backend
signals a condition. @see REmbedInternal::handleCondition () */
//	void handleCondition (char **call, int call_length);

/** this function is public for technical reasons, only. Don't use except from REmbedInternal! Called from REmbedInternal when the R backend
reports an error. @see REmbedInternal::handleError () */
	void handleError (char **call, int call_length);

/** This function is public for technical reasons, only. Don't use except from REmbedInternal!

This is a sub-eventloop, being run when the backend request information from the frontend. See \ref RThread for a more detailed description
@see REmbedInternal::handleSubstackCall () */
	void handleSubstackCall (char **call, int call_length);

/** This function is public for technical reasons, only. Don't use except from REmbedInternal!

This is a minimal sub-eventloop, being run, when the backend requests simple information from the frontend. It differs from handleSubstack in two
points:
1) it does not create a full-fledged substack for additional R commands
2) it may return information via the args parameter immediately

@see REmbedInternal::handleStandardCallback () */
	void handleStandardCallback (RCallbackArgs *args);

/** The command currently being executed. This is used from RInterface::cancelCommand to find out, whether the command to be cancelled is
already/still running. */
	RCommand *current_command;
protected:
/** the main loop. See \ref RThread for a more detailed description */
	void run ();
private:
/** This is the function in which an RCommand actually gets processed. Basically it passes the command to REmbedInteranl::runCommandInternal () and sends RInterface some events about what is currently happening. */
	void doCommand (RCommand *command);
	
	int locked;
	bool killed;
};

#endif
