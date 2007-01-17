
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

#include "rembedinternal.h"

// static
REmbedInternal *REmbedInternal::this_pointer = 0; 

extern "C" {
#define R_INTERFACE_PTRS 1

#include "Rdefines.h"
#include "R_ext/Rdynload.h"
#include "R_ext/eventloop.h"
#include "R_ext/Callbacks.h"
#include "R.h"
#include "Rinternals.h"
#include "Rinterface.h"
#include "Rdevices.h"
#include "Rversion.h"
#include "R_ext/Parse.h"

#include <dlfcn.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/types.h>
//#include <signal.h>
//#include <unistd.h>
#include <math.h>

#if (R_VERSION > R_Version(2, 2, 9))
#define R_2_3
#endif

#if (R_VERSION > R_Version(2, 3, 9))
#define R_2_4
#endif

#if (R_VERSION > R_Version(2, 4, 9))
#define R_2_5
#endif

#ifdef R_2_4
#define USE_R_REPLDLLDO1
#endif

#ifdef R_2_4
#include "Rembedded.h"
#else
extern void R_ReplDLLinit (void);
#endif

// some functions we need that are not declared
extern int Rf_initEmbeddedR(int argc, char **argv);
extern void Rf_PrintWarnings (void);
extern int R_interrupts_pending;
#ifdef R_2_3
extern int Rf_initialize_R(int ac, char **av);
extern void setup_Rmainloop(void); /* in main.c */
extern unsigned long R_CStackLimit;
#endif
#ifndef USE_R_REPLDLLDO1
extern Rboolean R_Visible;
#endif
#ifndef R_2_5
extern SEXP R_ParseVector(SEXP, int, ParseStatus*);
#endif
}

#include "../rkglobals.h"
#include "rdata.h"

#ifdef USE_R_REPLDLLDO1
const char *current_buffer = 0;
bool repldlldo1_wants_code = false;
bool repldll_buffer_transfer_finished = false;
int repldll_result = 0;		/* -2: error; -1: incomplete; 0: nothing, yet 1: ok 2: incomplete statement, while buffer not empty. Should not happen */
bool repldll_last_parse_successful = false;
#endif

// ############## R Standard callback overrides BEGIN ####################
void RSuicide (char* message) {
	RCallbackArgs args;
	args.type = RCallbackArgs::RSuicide;
	args.chars_a = &message;
	REmbedInternal::this_pointer->handleStandardCallback (&args);
	REmbedInternal::this_pointer->shutdown (true);
}

void RShowMessage (char* message) {
	RCallbackArgs args;
	args.type = RCallbackArgs::RShowMessage;
	args.chars_a = &message;
	REmbedInternal::this_pointer->handleStandardCallback (&args);
}

int RReadConsole (char* prompt, unsigned char* buf, int buflen, int hist) {
#ifdef USE_R_REPLDLLDO1
	// handle requests for new code
	if (repldlldo1_wants_code) {
		//Rprintf ("wants code, buffsize: %d\n", buflen);
		if (repldll_buffer_transfer_finished) {
			return 0;
		}

		int pos = 0;		// fgets emulation
		while (pos < (buflen-1)) {
			buf[pos] = *current_buffer;
			if (*current_buffer == '\0') {
				repldll_buffer_transfer_finished = true;
				break;
			}
			++current_buffer;
			++pos;
		}
		if (repldll_buffer_transfer_finished) buf[pos] = '\n';
		buf[++pos] = '\0';
		//Rprintf ("buffer now: '%s'\n", buf);
		return 1;
	}
#endif
	// here, we handle readline calls and such, i.e. not the regular prompt for code
	RCallbackArgs args;
	args.type = RCallbackArgs::RReadConsole;
	args.chars_a = &prompt;
	args.chars_b = (char **) (&buf);
	args.int_a = buflen;
	args.int_b = hist;		// actually, we ignore hist
	args.int_c = 1;			// not cancelled

	REmbedInternal::this_pointer->handleStandardCallback (&args);
// default implementation seems to return 1 on success, 0 on failure, contrary to some documentation. see unix/std-sys.c
	if (!(args.int_c)) {
		REmbedInternal::this_pointer->currentCommandWasCancelled ();
		Rf_onintr ();
	}
	if (buf && args.int_c) return 1;
	return 0;
}

void RWriteConsole (char *buf, int buflen) {
/*	RCallbackArgs args;
	args.type = RCallbackArgs::RWriteConsole;
	args.chars_a = &buf;
	args.int_a = buflen;
	REmbedInternal::this_pointer->handleStandardCallback (&args); */
	REmbedInternal::this_pointer->handleOutput (buf, buflen);
}

void RResetConsole () {
// we leave this un-implemented on purpose! We simply don't want that sort of thing to be done.
}

void RFlushConsole () {
/* nope, we're not going to do the line below after all. Two reasons:
1) We'd still have to add mutex protection around this call (ok, doable of course)
2) I don't think we need it at all: We do our own flushing, and R rarely flushes, for obscure reasons, anyway.
In order to prevent R from doing silly things, we still override this function and leave it unimplemented on purpose. */
//	REmbedInternal::this_pointer->flushOutput ();
}

void RClearerrConsole () {
// we leave this un-implemented on purpose! We simply don't want that sort of thing to be done.
}

void RCleanUp (SA_TYPE saveact, int status, int RunLast) {
	if (saveact != SA_SUICIDE) {
		RCallbackArgs args;
		args.type = RCallbackArgs::RCleanUp;
		args.int_a = status;
		args.int_b = RunLast;
		REmbedInternal::this_pointer->handleStandardCallback (&args);

		if(saveact == SA_DEFAULT) saveact = SA_SAVE;
		if (saveact == SA_SAVE) {
				if (RunLast) R_dot_Last ();
				if( R_DirtyImage) R_SaveGlobalEnv ();
		} else {
				if (RunLast) R_dot_Last ();
		}

		// clean up temp directory
		char *tmpdir;
		if((tmpdir = getenv ("R_SESSION_TMPDIR"))) {
			char buf[1024];
			snprintf ((char *) buf, 1024, "rm -rf %s", tmpdir);
			R_system ((char *) buf);
		}

		REmbedInternal::this_pointer->shutdown (false);
	}
	/*else {
		we've already informed the user, and shut down the backend in RSuicide
	}*/
}

int RShowFiles (int nfile, char **file, char **headers, char *wtitle, Rboolean del, char *pager) {
	RCallbackArgs args;
	args.type = RCallbackArgs::RShowFiles;
	args.int_a = nfile;
	args.chars_a = file;
	args.chars_b = headers;		// what exactly are the "headers"?!?
	args.chars_c = &wtitle;
	args.chars_d = &pager;		// we ingnore the pager-parameter for now.
	args.int_b = del;

	REmbedInternal::this_pointer->handleStandardCallback (&args);

// default implementation seems to returns 1 on success, 0 on failure. see unix/std-sys.c
	return 1;
}

int RChooseFile (int isnew, char *buf, int len) {
	RCallbackArgs args;
	args.type = RCallbackArgs::RChooseFile;
	args.int_a = isnew;
	args.int_b = len;
	args.chars_a = &buf;

	REmbedInternal::this_pointer->handleStandardCallback (&args);

// return length of filename (strlen (buf))
	return args.int_c;
}

int REditFiles (int nfile, char **file, char **title, char *editor) {
	RCallbackArgs args;
	args.type = RCallbackArgs::REditFiles;
	args.int_a = nfile;
	args.chars_a = file;
	args.chars_b = title;
	args.chars_c = &editor;

	REmbedInternal::this_pointer->handleStandardCallback (&args);

// default implementation seems to return 1 if nfile <= 0, else 1. No idea, what for. see unix/std-sys.c
	return (nfile <= 0);
}

int REditFile (char *buf) {
	char *editor = "none";
	char *title = "";

// does not exist in standard R 2.1.0, so no idea what to return.
	return REditFiles (1, &buf, &title, editor);
}

#ifdef USE_R_REPLDLLDO1
void RBusy (int busy) {
	// R_ReplDLLDo1 calls R_Busy (1) after reading in code (if needed), parsing it, and right before evaluating it.
	if (busy) {
		repldlldo1_wants_code = false;
		repldll_last_parse_successful = true;
	}
}
#endif

// ############## R Standard callback overrides END ####################

char *REmbedInternal::na_char_internal = new char;

REmbedInternal::REmbedInternal () {
}

void REmbedInternal::connectCallbacks () {
// R standard callback pointers.
// Rinterface.h thinks this can only ever be done on aqua, apparently. Here, we define it the other way around, i.e. #ifndef instead of #ifdef
// No, does not work -> undefined reference! -> TODO: nag R-devels
#ifndef HAVE_AQUA
	//extern int  (*ptr_R_EditFiles)(int, char **, char **, char *);
#endif

// connect R standard callback to our own functions. Important: Don't do so, before our own versions are ready to be used!
	R_Outputfile = NULL;
	R_Consolefile = NULL;
	ptr_R_Suicide = RSuicide;
	ptr_R_ShowMessage = RShowMessage;			// when exactly does this ever get used?
	ptr_R_ReadConsole = RReadConsole;
	ptr_R_WriteConsole = RWriteConsole;
	ptr_R_ResetConsole = RResetConsole;
	ptr_R_FlushConsole = RFlushConsole;
	ptr_R_ClearerrConsole = RClearerrConsole;
#ifdef USE_R_REPLDLLDO1
	ptr_R_Busy = RBusy;
#endif
	ptr_R_CleanUp = RCleanUp;			// unfortunately, it seems, we can't safely cancel quitting anymore, here!
	ptr_R_ShowFiles = RShowFiles;
	ptr_R_ChooseFile = RChooseFile;
	ptr_R_EditFile = REditFile;
//	ptr_R_EditFiles = REditFiles;		// undefined reference

// these two, we won't override
//	ptr_R_loadhistory = ... 	// we keep our own history
//	ptr_R_savehistory = ...	// we keep our own history
}

REmbedInternal::~REmbedInternal () {
}

void REmbedInternal::shutdown (bool suicidal) {
// Code-recipe below essentially copied from http://stat.ethz.ch/R-manual/R-devel/doc/manual/R-exts.html#Linking-GUIs-and-other-front_002dends-to-R
// modified quite a bit for our needs.
	char *tmpdir;

	if (!suicidal) {
		R_dot_Last ();
	}

	R_RunExitFinalizers();

	if((tmpdir = getenv("R_SESSION_TMPDIR"))) {
		char buf[1024];
		snprintf((char *)buf, 1024, "rm -rf %s", tmpdir);
		R_system((char *)buf);
	}

	/* close all the graphics devices */
	if (!suicidal) KillAllDevices ();
	fpu_setup ((Rboolean) FALSE);
}

static int timeout_counter = 0;

void REmbedInternal::processX11Events () {
/* what we do here is walk the list of objects, that have told R, they're listening for events.
We figure out which ones look for X11-events and tell those to do their stuff (regardless of whether events actually occurred) */
	extern InputHandler *R_InputHandlers;
	InputHandler *handler = R_InputHandlers;
	while (handler) {
		if (handler->activity == XActivity) {
			handler->handler ((void*) 0);
		}
		handler = handler->next;
	}

	/* I don't really understand what I'm doing here, but apparently this is necessary for Tcl-Tk windows to function properly. */
	R_PolledEvents ();
	
/* Maybe we also need to also call R_timeout_handler once in a while? Obviously this is extremely crude code! 
TODO: verify we really need this. */
	if (++timeout_counter > 100) {
//		extern void (* R_timeout_handler) ();	// already defined in Rinferface.h
		if (R_timeout_handler) R_timeout_handler ();
		timeout_counter = 0;
	}
}

/** This function is the R side wrapper around stringsToStringList */
QString *SEXPToStringList (SEXP from_exp, unsigned int *count) {
	char **strings = 0;
	
	// bad format? coerce the vector first
	if (TYPEOF (from_exp) != STRSXP) {
		SEXP strexp;
		PROTECT (strexp = coerceVector (from_exp, STRSXP));
		QString *list = SEXPToStringList (strexp, count);
		UNPROTECT (1);
		return list;
	}

	// format already good? Avoid coercion (and associated copying)
	*count = length (from_exp);
	strings = new char* [*count];
	unsigned int i = 0;
	for (; i < *count; ++i) {
		SEXP dummy = VECTOR_ELT (from_exp, i);

		if (TYPEOF (dummy) != CHARSXP) {
			strings[i] = strdup ("not defined");	// can this ever happen?
		} else {
			if (dummy == NA_STRING) {
				strings[i] = REmbedInternal::na_char_internal;
			} else {
				strings[i] = (char *) STRING_PTR (dummy);
			}
		}
	}
	QString *list = stringsToStringList (strings, i);
	delete [] strings;

	return list;
}

int *SEXPToIntArray (SEXP from_exp, unsigned int *count) {
	int *integers;

	// bad format? coerce the vector first
	if (TYPEOF (from_exp) != INTSXP) {
		SEXP intexp;
		PROTECT (intexp = coerceVector (from_exp, INTSXP));
		integers = SEXPToIntArray (intexp, count);
		UNPROTECT (1);
		return integers;
	}

	// format already good? Avoid coercion (and associated copying)
	*count = length (from_exp);
	integers = new int[*count];
	for (unsigned int i = 0; i < *count; ++i) {
		integers[i] = INTEGER (from_exp)[i];
		if (integers[i] == R_NaInt) integers[i] = INT_MIN;		// this has no effect for now, but if R ever chnages it's R_NaInt, then it will
	}
	return integers;
}

double *SEXPToRealArray (SEXP from_exp, unsigned int *count) {
	double *reals;

	// bad format? coerce the vector first
	if (TYPEOF (from_exp) != REALSXP) {
		SEXP realexp;
		PROTECT (realexp = coerceVector (from_exp, REALSXP));
		reals = SEXPToRealArray (realexp, count);
		UNPROTECT (1);
		return reals;
	}
	
	// format already good? Avoid coercion (and associated copying)
	*count = length (from_exp);
	reals = new double[*count];
	for (unsigned int i = 0; i < *count; ++i) {
		reals[i] = REAL (from_exp)[i];
		if (R_IsNaN (reals[i]) || R_IsNA (reals[i]) ) reals[i] = RKGlobals::na_double;
	}
	return reals;
}

RData *SEXPToRData (SEXP from_exp) {
	RData *data = new RData;

	unsigned int count;
	int type = TYPEOF (from_exp);
	switch (type) {
		case INTSXP:
			data->data = SEXPToIntArray (from_exp, &count);
			data->datatype = RData::IntVector;
			break;
		case REALSXP:
			data->data = SEXPToRealArray (from_exp, &count);
			data->datatype = RData::RealVector;
			break;
		case VECSXP:
			count = 0;
			count = length (from_exp);
			{
				RData **structure_array = new RData*[count];
				for (unsigned int i=0; i < count; ++i) {
					SEXP subexp = VECTOR_ELT (from_exp, i);
					//PROTECT (subexp);	// should already be protected as part of the parent from_exp
					structure_array[i] = SEXPToRData (subexp);
					//UNPROTECT (1);
				}
				data->data = structure_array;
			}
			data->datatype = RData::StructureVector;
			break;
/*		case NILSXP:
			data->data = 0;
			data->datatype = RData::NoData;
			count = 0;
			break; */
		case STRSXP:
		default:
			data->data = SEXPToStringList (from_exp, &count);
			data->datatype = RData::StringVector;
	}

	data->length = count;

	return data;
}

SEXP doError (SEXP call) {
	unsigned int count;
	QString *strings = SEXPToStringList (call, &count);
	REmbedInternal::this_pointer->handleError (strings, count);
	deleteQStringArray (strings);
	return R_NilValue;
}

/*
SEXP doCondition (SEXP call) {
	int count;
	char **strings = extractStrings (call, &count);
	REmbedInternal::this_pointer->handleCondition (strings, count);
	return R_NilValue;
} */

SEXP doSubstackCall (SEXP call) {
	unsigned int count;
	QString *strings = SEXPToStringList (call, &count);
	REmbedInternal::this_pointer->handleSubstackCall (strings, count);
	deleteQStringArray (strings);
	return R_NilValue;
}

bool REmbedInternal::startR (int argc, char** argv) {
#ifdef R_2_3
	Rf_initialize_R (argc, argv);
	R_CStackLimit = (unsigned long) -1;
	setup_Rmainloop ();
	RKGlobals::na_double = NA_REAL;
	R_ReplDLLinit ();
	return true;
#else
	bool ok = (Rf_initEmbeddedR (argc, argv) >= 0);
	RKGlobals::na_double = NA_REAL;
	R_ReplDLLinit ();
	return ok;
#endif
}

bool REmbedInternal::registerFunctions (const char *library_path) {
	DllInfo *info = R_getDllInfo (library_path);
	if (!info) return false;

	R_CallMethodDef callMethods [] = {
//		{ "rk.do.condition", (DL_FUNC) &doCondition, 1 },
		{ "rk.do.error", (DL_FUNC) &doError, 1 },
		{ "rk.do.command", (DL_FUNC) &doSubstackCall, 1 },
		{ 0, 0, 0 }
	};
	R_registerRoutines (info, NULL, callMethods, NULL, NULL);

	return true;
}

SEXP runCommandInternalBase (const char *command, REmbedInternal::RKWardRError *error) {
// some copying from RServe below
	int r_error = 0;
	ParseStatus status = PARSE_NULL;
	SEXP cv, pr, exp;

	PROTECT(cv=allocVector(STRSXP, 1));
	SET_VECTOR_ELT(cv, 0, mkChar(command));  

	// TODO: Maybe we can use R_ParseGeneral instead. Then we could find the exact character, where parsing fails. Nope: not exported API
#ifdef R_2_5
	pr=R_ParseVector(cv, -1, &status, NILSXP);
#else
	pr=R_ParseVector(cv, -1, &status);
#endif
	UNPROTECT(1);

	if ((!pr) || (TYPEOF (pr) == NILSXP)) {
		// got a null SEXP. This means parse was *not* ok, even if R_ParseVector told us otherwise
		if (status == PARSE_OK) {
			status = PARSE_ERROR;
			printf ("weird parse error\n");
		}
	}

	if (status != PARSE_OK) {
		if ((status == PARSE_INCOMPLETE) || (status == PARSE_EOF)) {
			*error = REmbedInternal::Incomplete;
		} else if (status == PARSE_ERROR) {
			//extern SEXP parseError (SEXP call, int linenum);
			//parseError (R_NilValue, 0);
			*error = REmbedInternal::SyntaxError;
		} else { // PARSE_NULL
			*error = REmbedInternal::OtherError;
		}
		exp = R_NilValue;
	} else {			// no error during parsing, let's try to evaluate the command
		PROTECT (pr);
		exp=R_NilValue;

		if (TYPEOF(pr)==EXPRSXP && LENGTH(pr)>0) {
			int bi=0;
			while (bi<LENGTH(pr)) {
				SEXP pxp=VECTOR_ELT(pr, bi);
				exp=R_tryEval(pxp, R_GlobalEnv, &r_error);
				bi++;
				if (r_error) {
					break;
				}
			}
		} else {
			exp=R_tryEval(pr, R_GlobalEnv, &r_error);
		}

		if (r_error) {
			*error = REmbedInternal::OtherError;
		} else {
			*error = REmbedInternal::NoError;
		}

		UNPROTECT(1); /* pr */
	}

	// for safety, let's protect exp for the two print calls below.
	// TODO: this is not good. It causes an additional PROTECT and UPROTECT. Need to (re-)move printing
	PROTECT (exp);
	/* Do NOT ask me why, but the line below is needed for warnings to be printed, while otherwise they would not be shown.
	Apparently we need to print at least something in order to achieve this. Whatever really happens in Rprintf () to have such an effect, I did not bother to find out. */
	Rprintf ("");

	Rf_PrintWarnings ();

	UNPROTECT (1);		// exp; We unprotect this, as most of the time the caller is not really interested in the result
	return exp;
}

#ifndef USE_R_REPLDLLDO1
/* Basically a safe version of Rf_PrintValue, as yes, Rf_PrintValue may lead to an error and long_jump->crash!
For example in help (function, htmlhelp=TRUE), when no HTML-help is installed!
SEXP exp should be PROTECTed prior to calling this function.
//TODO: I don't think it's meant to be this way. Maybe nag the R-devels about it one day. 
//TODO: this is not entirely correct. See PrintValueEnv (), which is what Repl_Console uses (but is hidden)
*/
void tryPrintValue (SEXP exp, REmbedInternal::RKWardRError *error) {
	int ierror = 0;
	SEXP tryprint, e;

// Basically, we call 'print (expression)' (but inside a tryEval)
	tryprint = Rf_findFun (Rf_install ("print"),  R_GlobalEnv);
	PROTECT (tryprint);
	e = allocVector (LANGSXP, 2);
	PROTECT (e);
	SETCAR (e, tryprint);
	SETCAR (CDR (e), exp);
	R_tryEval (e, R_GlobalEnv, &ierror);
	UNPROTECT (2);	/* e, tryprint */

	if (ierror) {
		*error = REmbedInternal::OtherError;
	} else {
		*error = REmbedInternal::NoError;
	}
}
#endif

#ifdef USE_R_REPLDLLDO1
void runUserCommandInternal (void *) {
/* R_ReplDLLdo1 return codes:
-1: EOF
1: normal prompt
2: continuation prompt (parse incomplete) */
	do {
		//Rprintf ("iteration status: %d\n", repldll_result);
		repldll_result = -2;
		repldlldo1_wants_code = true;
		repldll_last_parse_successful = false;
	} while (((repldll_result = R_ReplDLLdo1 ()) == 2) && (!repldll_buffer_transfer_finished));	// keep iterating while the statement is incomplete, and we still have more in the buffer to transfer
	//Rprintf ("iteration complete, status: %d\n", repldll_result);
	PROTECT (R_LastvalueSymbol);		// why do we need this? No idea, but R_ToplevelExec tries to unprotect something
}
#endif

void REmbedInternal::runCommandInternal (const char *command, RKWardRError *error, bool print_result) {
	if (!print_result) {
		runCommandInternalBase (command, error);
	} else {		// run a user command
#ifdef USE_R_REPLDLLDO1
/* Using R_ReplDLLdo1 () is a pain, but it seems to be the only entry point for evaluating a command as if it had been entered on a plain R console (with auto-printing if not invisible, etc.). Esp. since R_Visible is no longer exported in R 2.5.0, as it seems as of today (2007-01-17).

Problems to deal with:
- R_ReplDLLdo1 () may do a jump on an error. Hence we need R_ToplevelExec (public sind R 2.4.0)
	- this is why runUserCommandInternal needs to be a separate function
- R_ReplDLLdo1 () expects to receive the code input via R_ReadConsole. The same R_ReadConsole that commands like readline () or browser () will use to get their input.
	- hence we need some state variables to figure out, when a call to R_ReadConsole originates directly from R_ReplDLLdo1 (), or some R statement. R_Busy () is our friend, here.
- R_ReplDLLdo1 () will only ever evaluate one statement, even if several statements have already been transfered to the buffer. In fact, it will even return once after each ';' or '\n', even if the statement is not complete, but more is already in the buffer
	- Hence we need two loops around R_ReplDLLdo1 (): one to make sure it continues reading until a statement is actually complete, another to continue if there is a second (complete or incomplete) statement in the command
	- Also, in case the command was too long to fit inside the buffer at once (repldll_buffer_transfer_finished)
- Some more state variables are used for figuring out, which type of error occurred, if any, since we don't get any decent return value

This is the logic spread out over the following section, runUserCommandInternal (), and RReadConsole (). */

		R_ReplDLLinit ();		// resets the parse buffer (things might be left over from a previous incomplete parse)
		bool prev_iteration_was_incomplete = false;

		current_buffer = command;
		repldll_buffer_transfer_finished = false;
		Rboolean ok = (Rboolean) 1;	// set to false, if there is a jump during the R_ToplevelExec (i.e.. some sort of error)

		repldll_result = 0;
		while ((ok != FALSE) && ((!repldll_buffer_transfer_finished) || (repldll_result != -1))) {
			// we always need to iterate until the parse returned an EOF AND we have no more code in the buffer to supply.
			// However, if this happens right after we last received an INCOMPLETE, this means the parse really was incomplete.
			// Otherwise, there's simply nothing more to parse.
			prev_iteration_was_incomplete = (repldll_result == 2);
			ok = R_ToplevelExec (runUserCommandInternal, 0);
		}
		if (ok == FALSE) {
			if (repldll_last_parse_successful) {
				*error = REmbedInternal::OtherError;
			} else {
				*error = REmbedInternal::SyntaxError;
			}
		} else {
			if (prev_iteration_was_incomplete) {
				*error = REmbedInternal::Incomplete;
			} else {
				*error = REmbedInternal::NoError;
			}
		}
		repldlldo1_wants_code = false;		// make sure we don't get confused in RReadConsole

#else

		R_Visible = (Rboolean) 0;

		SEXP exp;
		PROTECT (exp = runCommandInternalBase (command, error));
/*		char dummy[100];
		sprintf (dummy, "type: %d", TYPEOF (exp));
		Rprintf (dummy, 100); */
		if (*error == NoError) {
			SET_SYMVALUE (R_LastvalueSymbol, exp);
			if (R_Visible) {
				tryPrintValue (exp, error);
			}
		}
		UNPROTECT (1); /* exp */

		/* See the comment in the corresponding code in runCommandInternalBase. And yes, apparently, we need this at both places! */
		Rprintf ("");

		Rf_PrintWarnings ();
#endif
	}
}

QString *REmbedInternal::getCommandAsStringVector (const char *command, uint *count, RKWardRError *error) {	
	SEXP exp;
	QString *list = 0;
	
	PROTECT (exp = runCommandInternalBase (command, error));
	
	if (*error == NoError) {
		list = SEXPToStringList (exp, count);
	}
	
	UNPROTECT (1); // exp
	
	if (*error != NoError) {
		*count = 0;
		return 0;
	}
	return list;
}

double *REmbedInternal::getCommandAsRealVector (const char *command, uint *count, RKWardRError *error) {
	SEXP exp;
	double *reals = 0;
	
	PROTECT (exp = runCommandInternalBase (command, error));
	
	if (*error == NoError) {
		reals = SEXPToRealArray (exp, count);
	}
	
	UNPROTECT (1); // exp
	
	if (*error != NoError) {
		*count = 0;
		return 0;
	}
	return reals;
}

int *REmbedInternal::getCommandAsIntVector (const char *command, uint *count, RKWardRError *error) {
	SEXP exp;
	int *integers = 0;
	
	PROTECT (exp = runCommandInternalBase (command, error));
	
	if (*error == NoError) {
		integers = SEXPToIntArray (exp, count);
	}
	
	UNPROTECT (1); // exp
	
	if (*error != NoError) {
		*count = 0;
		return 0;
	}
	return integers;
}

RData *REmbedInternal::getCommandAsRData (const char *command, RKWardRError *error) {
	SEXP exp;
	RData *data = 0;
	
	PROTECT (exp = runCommandInternalBase (command, error));
	
	if (*error == NoError) {
		data = SEXPToRData (exp);
	}
	
	UNPROTECT (1); // exp
	
	return data;
}

