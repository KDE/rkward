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

#include "rcommand.h"

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

 /** The main purpose of separating this class from RThread is that R- and Qt-includes don't go together well. Hence this class is Qt-agnostic while
	RThread is essentially R-agnostic.
	
	@see RThread

	*@author Thomas Friedrichsmeier
*/
class REmbedInternal {
public: 
/** constructor. You can't create an instance of this class due to pure virtual functions. Create an instance of RThread instead. */
	REmbedInternal ();
/** destructor */
	virtual ~REmbedInternal ();

/** set up R standard callbacks */
	void setupCallbacks ();
/** connect R standard callbacks */
	void connectCallbacks ();

/** Enum specifying types of errors that may occur while parsing/evaluating a command in R */
	enum RKWardRError {
		NoError=0,			/**< No error */
		Incomplete=1,		/**< The command is incomplete. Command was syntactically ok up to given point, but incomplete. It may or may not be semantically correct. */
		SyntaxError=2,		/**< Syntax error */
		OtherError=3		/**< Other error, usually a semantic error, e.g. object not found */
	};

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

/** This gets called on normal R output (R_WriteConsole). Used to get at output. */
	virtual void handleOutput (const QString &output, int len, bool regular) = 0;

/** This gets called, when the console is flushed */
	virtual void flushOutput () = 0;

/** This gets called, when R reports warnings/messages. Used to get at warning-output. */
//	virtual void handleCondition (char **call, int call_length) = 0;

/** This gets called, when R reports an error (override of options ("error") in R). Used to get at error-output. */
	virtual void handleError (QString *call, int call_length) = 0;

/** The main callback from R to rkward. Since we need QStrings and stuff to handle the requests, this is only a pure virtual function. The real
implementation is in RThread::handleSubstackCall () */
	virtual void handleSubstackCall (QStringList &list) = 0;

/** This second callback handles R standard callbacks. The difference to the first one is, that these are typically required to finish within the same
function. On the other hand, also, they don't require further computations in R, and hence no full-fledged substack.

Otherwise it is very similar to handleSubstackCall (), esp. in that is implemented in RThread::handleStandardCallback ()
@see RCallbackArgs @see RCallbackType */
	virtual void handleStandardCallback (RCallbackArgs *args) = 0;

/** The command currently being executed. */
	RCommand *current_command;

	void runCommand (RCommand *command);

/** only one instance of this class may be around. This pointer keeps the reference to it, for interfacing to from C to C++ */
	static REmbedInternal *this_pointer;
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
protected:
/** thread is locked. No new commands will be executed. @see LockType @see lock @see unlock */
	int locked;
/** thread is killed. Should exit as soon as possible. @see kill */
	bool killed;
/** The internal storage for pauseOutput () */
	bool output_paused;
private:
	int r_version;
// can't declare this as part of the class, as it would confuse REmbed
//	SEXPREC *runCommandInternalBase (const char *command, bool *error);
};
 
#endif
