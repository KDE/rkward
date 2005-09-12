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

class REmbed;
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

Internally, this class is closely related to REmbed and REmbedInternal, which is where the "real work" is being done. RThread basically just takes care of delegating the work. Also related is RInterface: RThread communicates with RInterface by placing QCustomEvent s, when commands are done
or when the backend needs information from the frontend.

Unless you really want to modify the internal workings of the backend, you will want to look at RInterface and use the functions there.

@see RInterface
@see REmbed
@see REmbedInternal

@author Thomas Friedrichsmeier
*/
class RThread : public QThread {
public:
/** constructor. You need to specify a pointer to the RInterface, so the thread knows where to post its events. Only one RThread should ever be
created, and that happens in RInterface::RInterface (). */
	RThread (RInterface *parent);
/** destructor */
	~RThread();

/** Locks the thread. This is called by RInterface, when the currently running command is to be cancelled. It is used to make sure that the
backend thread does not proceed with further commands, before the main thread takes notice. @see unlock @see RInterface::cancelCommand */
	void lock () { locked=true; };
/** Unlocks the thread. The thread is initially locked so the main thread can check for some conditions before the backend thread may produce
more errors/crashes. Also the thread may get locked when cancelling the currently running command. @see lock */
	void unlock () { locked=false; };
/** "Kills" the thread. Actually this just tells the thread that is is about to be terminated. Allows the thread to terminate gracefully */
	void kill () { killed = true; };

/** this is a sub-eventloop, being run when the backend request information from the frontend. See \ref RThread for a more detailed description

@see REmbed::handleSubstackCall () */
	void doSubstack (char **call, int call_length);

/** this is a minimal sub-eventloop, being run, when the backend requests simple information from the frontend. It differs from doSubstack in two
points:
1) it does not create a full-fledged substack for additional R commands
2) it may return information via the args parameter immediately

@see REmbed::handleStandardCallback () */
	void doStandardCallback (RCallbackArgs *args);

/** The command currently being executed. This is used from RInterface::cancelCommand to find out, whether the command to be cancelled is
already/still running. */
	RCommand *current_command;
protected:
/** the main loop. See \ref RThread for a more detailed description */
	void run ();
private:
	RInterface *inter;
/** This is the function in which an RCommand actually gets processed. Basically it passes the command to REmbed::doCommand () and sends
RInterface some events about what is currently happening. */
	void doCommand (RCommand *command);
	REmbed *embeddedR;
	
	bool locked;
	bool killed;
};

#endif
