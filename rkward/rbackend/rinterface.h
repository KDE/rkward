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

/**
\page UsingTheInterfaceToR Using the Interface to R
\brief An Introduction to using the R-backend: Running commands in R and handling the results

This page tries to give you an introduction into using the RInterface and related classes. You'll learn how to submit a command for evaluation
by R and how you can get and handle the results. Note that is fairly low-level. You do not need to read this documentation in order to develop
plugins. You'll only need to know this in order to do C++-hacking in rkward.
The page is divided into several sections on special problems, but if you're new to rkward, you will probably want to read it as a whole instead of
jumping right to those sections.

\section UsingTheInterfaceToRTheSimpleCase The simple case: Fire and forget

In the most simple case, all you want to do is to send a command to be evaluated executed in the R backend, and you are not interested in what
happens (whether the command runs successfully, or what the output is). For this, all you need is the following
code:

\code
#include "rkglobals.h"
#include "rbackend/rinterface.h"

RKGlobals::rInter->issueCommand ("print (\"hello world!\")", RCommand::User);
\endcode

You will note, that actually there are two RInterface::issueCommand functions, this one is obviously the one taking a QString and several further
parameters as argument. It is actually quite similar to the other RInterface::issueCommand function which takes an RCommand as a paramter. This convenience class basically just creates an RCommand with the same parameters as the constructor of RCommand (RCommand::RCommand), and then submits this RCommand. We'll discuss what an RCommand really is further down below. For now a good enough explanations is, that it's simply a container for a command.

The first parameter here, is fairly obvious, the command-string the R backend should evaluate.

The second parameter (RCommand::User) is and integer, which can be a bit-wise or-able combination of the values in the RCommand::CommandTypes enum. We'll deal with the more specific values in that enum later. Here, were just using RCommand::User, which signifies, that the command came
directly from the user. This information is fairly important, as it affects the handling of the command at several places: Which color it will be given in
the R-command log (currently class RKWatch), whether the command can be cancelled (RCommand::User commands can be cancelled, but some other types of commands can not be cancelled), etc. See the documentation on RCommand::CommandTypes (not yet complete) for more information.

\section UsingTheInterfaceToRHandlingReturns A slightly more realistic example: Handling the result of an RCommand

Most of the time you don't just want to run a command, but you also want to know the result. Now, this is a tad bit more difficult than one might expect at first glance. The reason for this is, that the R backend runs in a separate thread. Hence, whenever you submit a command, it generally does not get executed right away - or at least you just don't know, when exactly it gets executed, and when the result is available. This is neccessary, so (expensive) commands running in the backend do not block operations in the GUI/frontend.

Ok, so how do you get informed, when you command was completed? Using RCommandReceiver. What you will want to do is inherit the class you
want to handle the results of RCommands from RCommandReceiver. When finished, the RCommand will be submitted to the (pure virtual) RCommandReceiver::rCommandDone function, which of course you'll have to implement in a meaningful way in your derived class.

The corresponding code would look something like this:

\code
#include "rkglobals.h"
#include "rbackend/rinterface.h"
#include "rbackend/rcommandreceiver.h"

class MyReceiver : public RCommandReceiver {
//...
protected:
/// receives finished RCommands and processes them
	void rCommandDone (RCommand *command);
//...
private:
/// does something by submitting an RCommand
	void someFunction ();
//...
};


void MyReceiver::someFunction () {
	RKGlobals::rInter->issueCommand ("print (1+1)", RCommand::App, "", this);
}

void MyReceiver::rCommandDone (RCommand *command) {
	if (command->successful ()) {
		qDebug ("Result was %s", command->output ()->utf8 ());
	}
}
\endcode

First thing to note is, that this time we're passing two additional parameters to RInterface::issueCommand. The first (or rather third) is really not used as of the time of this writing. What it's meant to become is a short descriptive string attached to the RCommand, that allows the user to make some sense of what this command is all about. The other (fourth) is a pointer to an RCommandReceiver that should be informed of the result. In this case, we'll handle the result in the same object that we issued the command from, so we use "this".

So next the RCommand created with RInterface::issueCommand goes on its way to/through the backend (we'll discuss what happens there further down below). Then later, when it has completed its journey, rCommandDone will be called with a pointer to the command as parameter. Now we can use this pointer to retrieve the information we need to know. RCommand::successful and some further simple functions give information about whether the command had any errors and what kind of errors (see RCommand for details). RCommand::output contains the output of the command as a QString (provided the command was successful and had any output). In this case that should be "[1] 2".

\section UsingTheInterfaceToRMultipleCommands Dealing with several RCommands in the same object

\section UsingTheInterfaceToRInternalHandling What happens with and RCommand internally?

\section UsingTheInterfaceToRCommandChains How to ensure commands get executed in the correct order: RCommandChain

\section UsingTheInterfaceToROutputOptions Sending results to the output and retrieving low-level data from the backend

\section UsingTheInterfaceToRFurtherReading Where to find more detailed information on these topics

\section ToBeContinued To be continued
This page is still incomplete!

@see RInterface
@see RCommand
@see RCommandReceiver
@see RCommandStack

*/

#endif
