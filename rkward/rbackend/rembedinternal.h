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
class RData;
class RCommand;
class QTextCodec;
/** This function converts a list of strings to a QStringList (locale aware), and returns the pointer. Needed to keep R and Qt includes separate. The strings can be deleted afterwards. Implementation is in rthread.cpp */
QString *stringsToStringList (char **strings, int count);
/** Function to delete an array of Qstring. Does delete [] strings, nothing more. But can not inline this in this class due to conflicting R and Qt includes. Implementation is in rthread.cpp */
void deleteQStringArray (QString *strings);

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
/** low-level running of a command.
@param command command to be run
@param error this will be set to a value in RKWardError depending on success/failure of the command
@param print_result whether the R_Visible flag should be set. If true, R will behave mostly as if in a regular console session. Otherwise values
will only be printed if called for expressedly with print ("...") or similar.
@param suppress_incomplete make sure never to run an incomplete command */
	void runCommandInternal (const QString &command, RKWardRError *error, bool print_result=false);
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

/** Flags used to classify output. */
//	static bool output_is_warning;
/** Flags used to classify output. */
//	static bool next_output_is_error;
	QTextCodec *current_locale_codec;
private:
	int r_version;
// can't declare this as part of the class, as it would confuse REmbed
//	SEXPREC *runCommandInternalBase (const char *command, bool *error);
};
 
#endif
