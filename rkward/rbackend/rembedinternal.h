/***************************************************************************
                          rembedinternal  -  description
                             -------------------
    begin                : Sun Jul 25 2004
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

#ifndef R_EMBED_H
#define R_EMBED_H

/** This struct is an ugly hack that allows us to pass all R standard callbacks to the main thread and back using the same mechanism.
Basically, it contains enough space for all arguments and return values ever used. However, of course, it is inherently totally unsafe.
All rests on having the handling functions know exactly, how these variables are used. So be extremely careful with modifications!

Also, of course, this method of passing the arguments is somewhat wasteful, as most of the time we're alocating a lot more memory than needed.
However, since all R standard callbacks are used very infrequently (and ask for user interaction), this is not really an issue.

The bool done member is used to find out, when exactly the main thread has finished processing the callback. */
struct RCallbackArgs {
/** is main thread done with the callback, yet? Initialized to false inside the true handler: RThread::doStandardCallback () */
	bool done;

/** This enum specifies, what sort of callback this is */
	enum RCallbackType { RSuicide, RShowMessage, RReadConsole, RWriteConsole, RResetConsole, RFlushConsole, RClearerrConsole,
											RBusy, RCleanUp, RShowFiles, RChooseFile, /*REditFile,*/ REditFiles
	} type;

	char **chars_a;
	char **chars_b;
	char **chars_c;
	char **chars_d;
	int int_a;
	int int_b;
	int int_c;
	bool bool_a;
};

 /** The main purpose of separating this class from REmbed is that R- and Qt-includes don't go together well. Hence this class is Qt-agnostic while
	REmbed is essentially R-agnostic.
	
	@see REmbed

	*@author Thomas Friedrichsmeier
  */
  
class REmbedInternal {
public: 
/** constructor. You can't create an instance of this class do to pure virtual functions. Create an instance of REmbed instead. */
	REmbedInternal();
/** destructor */
	virtual ~REmbedInternal();

/** connect R standard callbacks */
	void connectCallbacks ();

/** Enum specifying types of errors that may occur while parsing/evaluation a command in R */
	enum RKWardRError {
		NoError=0,			/**< No error */
		Incomplete=1,		/**< The command is incomplete. Command was syntactically ok up to given point, but incomplete. It may or may not be semantically correct. */
		SyntaxError=2,		/**< Syntax error */
		OtherError=3		/**< Other error, usually a semantic error, e.g. object not found */
	};

/** clean shutdown of R. If suicidal, perform only the most basic shutdown operations */
	void shutdown (bool suicidal);
protected:
/** low-level initialization of R
@param r_home R_HOME-directory
@param argc Number of arguments as would be passed on the commandline to R
@param argv Arguments as would be passed on the commandline to R */
	bool startR (const char* r_home, int argc, char **argv);
/** low-level running of a command.
@param command char* of the command to be run
@param error this will be set to a value in RKWardError depending on success/failure or the command
@param print_result whether the R_Visible flag should be set. If true, R will behave mostly as if in a regular console session. Otherwise values
will only be printed if called for expressedly with print ("...") or similar. */
	void runCommandInternal (const char *command, RKWardRError *error, bool print_result=false);

/** basically a wrapper to runCommandInternal (). Tries to convert the result of the command to an array of char* after running the command. Since
this will not ever be done for user commands, the R_Visible flag will never be set.
@param command char* of the command to be run 
@param count length of array returned
@param error this will be set to a value in RKWardError depending on success/failure or the command
@returns an array of char* or 0 on failure
@see RCommand::GetStringVector */
	char **getCommandAsStringVector (const char *command, int *count, RKWardRError *error);
/** basically a wrapper to runCommandInternal (). Tries to convert the result of the command to an array of double after running the command. Since
this will not ever be done for user commands, the R_Visible flag will never be set.
@param command char* of the command to be run 
@param count length of array returned
@param error this will be set to a value in RKWardError depending on success/failure or the command
@returns an array of double or 0 on failure
@see RCommand::GetRealVector */
	double *getCommandAsRealVector (const char *command, int *count, RKWardRError *error);
/** basically a wrapper to runCommandInternal (). Tries to convert the result of the command to an array of int after running the command. Since
this will not ever be done for user commands, the R_Visible flag will never be set.
@param command char* of the command to be run 
@param count length of array returned
@param error this will be set to a value in RKWardError depending on success/failure or the command
@returns an array of int or 0 on failure
@see RCommand::GetIntVector */
	int *getCommandAsIntVector (const char *command, int *count, RKWardRError *error);
public:
/** call this periodically to make R's x11 windows process their events */
	static void processX11Events ();

/** The main callback from R to rkward. Since we need QStrings and stuff to handle the requests, this is only a pure virtual function. The real
implementation is in REmbed::handleSubstackCall () */
	virtual void handleSubstackCall (char **call, int call_length) = 0;
	//virtual char **handleGetValueCall (char **call, int call_length, int *reply_length) = 0;

/** This second callback handles R standard callbacks. The difference to the first one is, that these are typically required to finish within the same
functions. On the other hand, also, they don't require further computations in R, and hence no full-fledged substack.

Otherwise it is very similar to handleSubstackCall (), esp. in that is implemented in REmbed::handleStandardCallback () */
	virtual void handleStandardCallback (RCallbackArgs *args) = 0;

/** only one instance of this class may be around. This pointer keeps the reference to it, for interfacing to from C to C++ */
	static REmbedInternal *this_pointer;
private:
// can't declare this as part of the class, as it would confuse REmbed
//	SEXPREC *runCommandInternalBase (const char *command, bool *error);
};
 
#endif
