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

#include "qobject.h"

#include "rthread.h"

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
	RInterface(RKwardApp *parent);
	~RInterface();
	
	void issueCommand (RCommand *command, RThread::CommandChain *chain=0) { r_thread->issueCommand (command, chain); };
	
	RThread::CommandChain *startChain (RThread::CommandChain *parent=0) { return r_thread->startChain (parent); };
	void closeChain (RThread::CommandChain *chain) { r_thread->closeChain (chain); };
private:
	RThread *r_thread;
	RKwardApp *app;
friend class RKwardApp;
	RKwatch *watch;
protected:
	void customEvent (QCustomEvent *e);
};

#endif
