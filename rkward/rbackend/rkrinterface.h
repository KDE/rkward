/*
rkrinterface.h - This file is part of the RKWard project. Created: Fri Nov 1 2002
SPDX-FileCopyrightText: 2002-2024 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef RKRINTERFACE_H
#define RKRINTERFACE_H

#include <QFile>
#include <qobject.h>

#include "rcommand.h"

class RCommand;
class RKWardMainWindow;
class RKRBackend;
class RBackendRequest;
class RKRBackendProtocolFrontend;

/** This class provides the main interface to the R-processor.

    Note that since communication with R is asynchronous, there is no way to get
    R-output within the same function, the request is submitted. You have to
    provide a callback (possibly a lambda function), if you're interested in the output.

    For a detailed explanation see \ref UsingTheInterfaceToR .

    @see RCommand
    *@author Thomas Friedrichsmeier
*/

class RInterface : public QObject {
	Q_OBJECT
  public:
	static void create();
	/** destructor */
	~RInterface();

	static RInterface *instance() { return _instance; };

	/** issues the given command in the given chain */
	static void issueCommand(RCommand *command, RCommandChain *chain = nullptr);
	/** convenience function to create a new command and issue it. See documentation on RCommand::RCommand() and RInterface::issueCommand() */
	static void issueCommand(const QString &command, int type = 0, const QString &rk_equiv = QString(), RCommandChain *chain = nullptr);

	/** convenience function to call a function / lambda, once all commands issued in the given chain have finished (successfully or not). Internally, this works by queing an empty command. */
	template <typename T>
	static void whenAllFinished(QObject *receiver, T func, RCommandChain *chain = nullptr) {
		RCommand *c = new RCommand(QString(), RCommand::EmptyCommand | RCommand::Sync);
		c->whenFinished(receiver, func);
		issueCommand(c, chain);
	}

	/** opens a new command chain. Returns a pointer to the new chain. If you specify a parent, the new chain will be a sub-chain of that chain. */
	static RCommandChain *startChain(RCommandChain *parent = nullptr);
	/** closes the command chain. The chain (and even its parent, if it is already closed) may be deleted right afterwards! */
	static void closeChain(RCommandChain *chain);

	/** Ensures that the given command will not be executed, or, if it is already running, interrupts it. Note that commands marked RCommand::Sync can
	not be interrupted. */
	void cancelCommand(RCommand *command);
	/** Cancels the given command, unless it has already been submitted to the backend. Returns true, if command was cancelled, false otherwise. */
	bool softCancelCommand(RCommand *command);
	/** Cancels all running or outstanding commands. @See cancelCommand() */
	void cancelAll();

	/** Pauses process. The current command will continue to run, but no new command will be */
	void pauseProcessing(bool pause);

	/** returns the command currently running in the thread. Be careful when using the returned pointer! */
	RCommand *runningCommand() const { return (all_current_commands.isEmpty() ? nullptr : all_current_commands.last()); };

	enum RStatus {
		Busy,
		Idle,
		Starting,
		Dead
	};

	bool backendIsDead() const { return backend_dead; };
	/** implies backendIsDead() */
	bool backendFailedToStart() const { return (backend_dead && !backend_started); }
	bool backendIsIdle();
	static bool isNaReal(double value) { return na_real == value; };
	static bool isNaInt(int value) { return na_int == value; };
	struct BackendError {
		QString title;
		QString id;
		QString message;
		QString details;
	};
	BackendError backendError() const { return backend_error; };

  private:
	/** Calls RThread::flushOutput(), and takes care of adding the output to all applicable commands */
	void flushOutput(bool forced);
	/** pointer to the RThread */
	RKRBackend *r_thread;
	/** Used by the testing framework. see R function rk.record.commands(). */
	QFile command_logfile;
	enum {
		NotRecordingCommands,
		RecordingCommands,
		RecordingCommandsUnfiltered
	} command_logfile_mode;

	/** helper function to handle backend requests that (may) involve running additional R-"sub"-commands. TODO; This should probably be merged with processRBackendRequest.*/
	GenericRRequestResult processRCallRequest(const QString &call, const QVariant &args, RCommandChain *in_chain);
	/** helper function to handle backend requests that do not inolve running sub-commands. */
	void processRBackendRequest(RBackendRequest *request);

	/** A list of all commands that have entered, and not yet left, the backend thread */
	QList<RCommand *> all_current_commands;
	/** NOTE: processing R events while waiting for the next command may, conceivably, lead to new requests, which may also wait for sub-commands! Thus we keep a simple stack of requests. */
	QList<RBackendRequest *> command_requests;
	RBackendRequest *currentCommandRequest() const { return (command_requests.isEmpty() ? nullptr : command_requests.last()); };
	void tryNextCommand();
	void doNextCommand(RCommand *command);
	RCommand *popPreviousCommand(int id);
	void handleCommandOut(RCommand *command);
	bool previously_idle;

	RCommandChain *openSubcommandChain(RCommand *parent_command);
	QList<RCommand *> current_commands_with_subcommands;
	void closeSubcommandChain(RCommand *parent_command);
	void reportFatalError();

	/** @see locked */
	enum LockType {
		User = 1 /**< locked on user request */
	};

	/** Used for locking the backend, meaning not further commands will be given to the backend. It is called, if the RThread is paused on User request.
	 * @see RInterface::pauseProcessing
	 * May be an OR'ed combination of several LockType s, but currently, there is only one LockType */
	int locked;

	BackendError backend_error;
	bool startup_phase2_error;
	void runStartupCommand(RCommand *command, RCommandChain *chain, std::function<void(RCommand *)> callback);
	RCommand *dummy_command_on_stack;
	friend class RKRBackendProtocolFrontend;
	bool backend_dead;
	bool backend_started;
	static double na_real;
	static int na_int;
	RKRBackendProtocolFrontend *backendprotocol;
	friend class RKWardMainWindow;
	friend class RCommand;

  protected:
	void handleRequest(RBackendRequest *request);
	static RInterface *_instance;
	void _issueCommand(RCommand *command, RCommandChain *chain = nullptr);
	/** constructor */
	RInterface();
  Q_SIGNALS:
	void backendWorkdirChanged();
	/** Note: status is actually RInterface::RStatus */
	void backendStatusChanged(int new_status);
};

/**
\page UsingTheInterfaceToR Using the Interface to R
\brief An Introduction to using the R-backend: Running commands in R and handling the results

This page tries to give you an introduction into using the RInterface and related classes. You'll learn how to submit a command for evaluation
by R and how you can get and handle the results. Note that this is fairly low-level. You do not need to read this documentation in order to develop
plugins. You'll only need to know this in order to do C++-hacking in rkward.
The page is divided into several sections on special problems, but if you're new to rkward, you will probably want to read it as a whole instead of
jumping right to those sections.

\section UsingTheInterfaceToRTheSimpleCase The simple case: Fire and forget

In the most simple case, all you want to do is to send a command to be evaluated executed in the R backend, and you are not interested in what
happens (whether the command runs successfully, or what the output is). For this, all you need is the following
code:

\code

#include "rbackend/rinterface.h"

RInterface::issueCommand ("print (\"hello world!\")", RCommand::User);
\endcode

You will note, that actually there are two RInterface::issueCommand functions, this one is obviously the one taking a QString and several further
parameters as argument. It is actually quite similar to the other RInterface::issueCommand function which takes an RCommand as a parameter. This convenience class basically just creates an RCommand with the same parameters as the constructor of RCommand (RCommand::RCommand), and then submits this RCommand. We'll discuss what an RCommand really is further down below. For now a good enough explanations is, that it's simply a container for a command.

The first parameter here is fairly obvious, the command-string the R backend should evaluate.

The second parameter (RCommand::User) is an integer, which can be a bit-wise or-able combination of the values in the RCommand::CommandTypes enum. We'll deal with the more specific values in that enum later. Here, we're just using RCommand::User, which signifies, that the command came
directly from the user. This information is fairly important, as it affects the handling of the command at several places: Which color it will be given in
the R-command log (currently class RKwatch), whether the command can be cancelled (RCommand::User commands can be cancelled, but some other types of commands can not be cancelled), etc. See the documentation on RCommand::CommandTypes (not yet complete) for more information.

\section UsingTheInterfaceToRHandlingReturns A slightly more realistic example: Handling the result of an RCommand

Most of the time you don't just want to run a command, but you also want to know the result. Now, this is a tad bit more difficult than one might expect at first glance. The reason for this is that the R backend runs in a separate thread. Hence, whenever you submit a command, it generally does not get executed right away - or at least you just don't know, when exactly it gets executed, and when the result is available. This is necessary, so (expensive) commands running in the backend do not block operations in the GUI/frontend.

Ok, so how do you get informed, when your command was completed? Using RCommand::notifier(), or in most cases, the convience function RCommand::whenFinished(). Here, you can simply specify a function or
lambda expression that will be called. This function or expression @em may take an RCommand* as parameter. In some cases, all you want to know is when all commands that have been already queued up to now
have been run. In this case, you can use RInterface::whenAllFinished().

Example code:

\code

#include "rbackend/rinterface.h"

class MyReceiver : public QObject {
//...
private:
/// does something by submitting an RCommand
    void someFunction ();
//...
};


void MyReceiver::someFunction () {
    RCommand *c = new RCommand("print (1+1)", RCommand::App);
    c->whenFinished(this, [this](RCommand *command) {
        if (command->successful ()) {
            qDebug ("Result was %s", command->output ()->utf8 ());
        }
    });
    RInterface::issueCommand(c);
}

\endcode

So what happens is that the RCommand created with RInterface::issueCommand goes on its way to/through the backend (we'll discuss what happens there further down below). Then later, when it has completed its journey, the lambda expression will be called with a pointer to the command as parameter. Now we can use this pointer to retrieve the information we need to know. RCommand::successful and some further simple functions give information about whether the command had any errors and what kind of errors (see RCommand for details). RCommand::output contains the output of the command as a QString (provided the command was successful and had any output). In this case that should be "[1] 2".

\section UsingTheInterfaceToRMultipleCommands Dealing with several RCommands in the same object

// TODO: Adjust this section to RCommandNotifier / RCommand::whenFinished().

In some cases you don't just want to deal with a single RCommand in a callback, and then use some special handling for each of those commands. So the problem is, how to find out, which of your commands you're currently dealing with ,in the callback.

There are several ways to deal with this:

    - storing the RCommand::id () (each command is automatically assigned a unique id)
    - keeping the pointer (CAUTION: don't use that pointer except to compare it with the pointer of an incoming command. Commands get deleted when they are finished, and maybe (in the future) if they become obsolete etc. Hence the pointers you keep may be invalid!)


Now what about that RKGlobals::rObjectList()->getUpdateCommandChain ()? We'll talk about RCommandChain and what you need it for further down below. But first we'll have a look at how an RCommand is handled internally.

\section UsingTheInterfaceToRInternalHandling What happens with an RCommand internally?

So far we've discussed RInterface:issueCommand () and the callback when it has finished. But what happens in between?

First the RCommand is placed in a first-in-first-out stack. This stack is needed, since - as discussed - the commands get executed in a separate thread, so several command may get stacked up, before the first one gets run.

Then, in the backend thread (RThread) there is a loop running, which fetches those commands from the stack and executes them one by one. Whenever a command has been executed in this thread, it gets updated with information on any errors that occurred and of course also with the result of running the command. Next, a QCustomEvent is being posted. What this does is - rougly speaking -, transfer the pointer to the command back to the main thread in a safe way.

Whenever the main thread becomes active again, it will find that QCustomEvent and handle it in RInterface::customEvent.

The most important thing happening there, is a call to RCommand::finished (which cause the commandFinished()-signal to be emitted), and right after that the RCommand gets deleted.

\section UsingTheInterfaceToRThreadingIssues Threading issues

The above description sounds simple enough, but there may be some threading issues to keep in mind. Consider what needs to happen when RKVariable is trying to update the information on the corresponding object in R-space (see code example above):

- RKVariable will first run a command to determine the dimensionality of the object.

Now this is more significant than you may think, as RKVariable is a special kind of RObject, which only handles one-dimensional data. Hence, if you create an object in R with

\code
myobject <- c (1, 2, 3)
\endcode

In RKWard an RKVariable will be responsible for "myobject".

However, if next, you assign something different to myobject:

\code
myobject <- data.frame (x=c (1, 2, 3), y=c (2, 3, 4))
\endcode

the object "myobject" can no longer be handled by RKVariable, but instead by RContainerObject.

What this means practically is that if RKVariable finds out its corresponding object in R-space has more than a single dimension, the RKVariable will have to be deleted and an RContainerObject needs to be created instead.

So what's the problem?

Consider this hypothetical example:

- RKVariable for object "myobject" runs a command to determine the dimensionality of "myobject"
- Before that command has finished, the user assigns a data.frame to "myobject" (or deletes the object, or whatever)
- The command to determine the dimensionality gets run and returns "1 dimension". RKVariable will assume it knows how to handle the object, and tries to do something with "myobject" which is only applicable for one-dimensional objects (e.g. trying to get the data as a one-dimensional array)
- The user command assigning a data.frame gets run
- RKVariables command to get the data fails

Now, you may argue this does not sound all that likely, and probably it isn't, but the point to see is that there are cases in which the threaded nature of R access can pose some problems. More generally, that is the case, if you want a sequence of commands to run in exactly that order without being disturbed by intervening commands.

To cope with this, it is sometimes desirable to keep closer control over the order in which commands get run in the backend.

\section UsingTheInterfaceToRCommandChains How to ensure commands get executed in the correct order: RCommandChain

The way to do this is to use RCommandChain. Basically, when you want commands to be executed in a sequence being sure that no other commands intervene, you do this:

\code
    RCommandChain *chain = RInterface::startChain ();

    // create first command
    RInterface::issueCommand (first_command, chain);

    // wait for command to return, potentially allows further calls to RInterface::issueCommand () from other places in the code

    // create second command
    RInterface::issueCommand (second_command, chain);

    RInterface::closeChain (chain);
\endcode

Now the point is that you place both of your commands in a dedicated "chain", telling RKWard that those two commands will have to be run in direct succession. If between the first and the second command, another section of the code issues a different command, this command will never be run until all commands in the chain have been run and the chain has been marked as closed.

To illustrate, consider this series of events:

\code
1) RCommandChain *chain = RInterface::startChain ();
2) RInterface::issueCommand (first_command, chain);
3) RInterface::issueCommand (some_command);
4) RInterface::issueCommand (second_command, chain);
5) RInterface::issueCommand (some_command2);
6) RInterface::closeChain (chain);
\endcode

Now let's assume for a second, the R backend has been busy doing other stuff and has not executed any of the commands so far, then the execution stack will now look like this:

- top level
    - chain
        - first_command
        - second_command
        - chain is marked as closed: after executing second_command, we may proceed with the top level
    - some_command
    - some_command2

So the order of execution will be guaranteed to be first_command, second_command, some_command, some_command2, although they were issued ina different order. You can also open sub chains, using

\code
RCommandChain *sub_chain = RInterface::startChain (parent_chain);
\endcode

Remember to close chains when you placed all the commands you needed to. If you don't close the chain, RKWard will continue to wait for new commands in that chain, and never proceed with commands outside of the chain.

\section UsingTheInterfaceToROutputOptions Sending results to the output and retrieving low-level data from the backend

There are a few special type-modifiers you can specify when creating an RCommand (as part of the second parameter to RCommand::RCommand or RInterface::issueCommand), that determine what will be done with the result:

- RCommand::EmptyCommand
This one tells the backend, that the command does not really need to be executed, and does not contain anything. You'll rarely need this flag, but sometimes it is useful to submit an empty command simply to find out when it is finished. @note: RInterface::whenAllFinished() is a more readable alternative to this.

- RCommand::DirectToOutput
This is typically used in plugins: When you specify this modifier, the plain text result of this command (i.e. whatever R prints out when evaluating the command) will be added to the HTML output file.

- RCommand::GetIntVector, RCommand::GetStringVector, RCommand::GetRealVector
These are special modifiers helpful when transferring data from R to RKWard (used primarily in the editor classes and in conjunction with RCommand::Sync): They tell the backend to try to fetch the result as an array of int, char*, or double, respectively. For instance, if you know object "myobject" is an integer vector, you may get the data using

\code
auto command = new RCommand("myobject", RCommand::Sync | RCommand::GetIntVector);
command->whenFinished(this, [this](RCommand *command) { this->syncDataFromR(command->getIntVector(); });
RInterface::issueCommand(command);
\endcode

Assuming the data can in fact be converted to a vector of integers, you can then access the data using these members in RCommand:

- RCommand::intVectorLength (): size of int array
- RCommand::getIntVector (): a pointer to the int array. Warning: The array is owned by the RCommand and will be deleted together with the RCommand, so don't just store the pointer
- RCommand::detachIntVector (): If you do want to keep the array, use this call to transfer ownership to your class. You are now responsible for freeing up the data yourself.

Obviously, whenever trying to transfer data from the backend, this approach is highly superior to parsing the QString RCommand::output ().

\section UsingTheInterfaceToRFurtherReading Where to find more detailed information on these topics

The following classes contain (or should contain) further important documentation:

- \ref RInterface
- \ref RCommand
- \ref RCommandNotifier
- \ref RCommandStack

Even lower level API:

- \ref RRBackendProtcolFrontend
- \ref RBackend
- \ref RRBackendProtcolBackend

*/

#endif
