/***************************************************************************
                          rinterface.h  -  description
                             -------------------
    begin                : Fri Nov 1 2002
    copyright            : (C) 2002 by Thomas Friedrichsmeier
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

#ifndef RINTERFACE_H
#define RINTERFACE_H

#include <qobject.h>
#include <qmutex.h>

#include "rthread.h"

#define MUTEX_LOCK /*qDebug ("%d", ++RInterface::mutex_counter);*/ RInterface::mutex.lock ();
#define MUTEX_UNLOCK /*qDebug ("%d", --RInterface::mutex_counter);*/ RInterface::mutex.unlock ();

class RKwatch;
class RCommand;
class RKwardApp;

/** This class does the rather low-level interfacing to the R-processor. The
	interface can be used by submitting new commands with issueCommand () (see
	the RCommand-class). Your command will then be placed in a first in first
	out stack of commands to be executed. If you specified a receiver/slot in the
	constructor of the RCommand, you will be notified, when the command has
	finished.

	Note that since communication with R is asynchronous, there is no way to get
	R-output within the same function, the request is submitted. You have to
	provide a callback-slot, if you're interested in the output. (@see RCommand)
  *@author Thomas Friedrichsmeier
  */

class RInterface : public QObject {
	Q_OBJECT
public: 
	RInterface();
	~RInterface();

/** issues the given command in the given chain */
	void issueCommand (RCommand *command, RCommandChain *chain=0);
/** convenience function to create a new command and issue it. See documentation on RCommand::RCommand () and RInterface::issueCommand () */
	void issueCommand (const QString &command, int type = 0, const QString &rk_equiv = "", RCommandReceiver *receiver=0, int flags=0, RCommandChain *chain=0);

/** opens a new command chain. Returns a pointer to the new chain. If you specify a parent, the new chain will be a sub-chain of that chain. */
	RCommandChain *startChain (RCommandChain *parent=0);
/** closes the command chain returns pointer to parent chain */
	RCommandChain *closeChain (RCommandChain *chain);

/** Ensures that the given command will not be executed, or, if it is already running, interrupts it. Note that commands marked RCommand::Sync can
not be interrupted. */
	void cancelCommand (RCommand *command);

	static QMutex mutex;
	static int mutex_counter;
private:
	RThread *r_thread;
	RCommand *running_command_canceled;
	
	void processREvalRequest (REvalRequest *request);
//	void processRGetValueRequest (RGetValueRequest);
friend class RKwardApp;
	RKwatch *watch;
protected:
	void customEvent (QCustomEvent *e);
};

#endif
