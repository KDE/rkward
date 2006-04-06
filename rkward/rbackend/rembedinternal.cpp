
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

#include "R_ext/Rdynload.h"
#include "R_ext/eventloop.h"
#include "R.h"
#include "Rinternals.h"
#include "Rinterface.h"
#include "Rdevices.h"
#include "R_ext/Parse.h"

#include <stdlib.h>
#include <string.h>
//#include <sys/types.h>
//#include <signal.h>
//#include <unistd.h>
#include <math.h>

// some functions we need that are not declared
extern int Rf_initEmbeddedR(int argc, char **argv);
typedef void *HINSTANCE;
extern int addDLL (char *path, char *name, HINSTANCE *handle);
extern SEXP R_ParseVector(SEXP, int, ParseStatus*);
extern void Rf_PrintWarnings (void);
extern int R_CollectWarnings;
extern int R_interrupts_pending;
extern Rboolean R_Visible;
}

#include "../rkglobals.h"

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
	RCallbackArgs args;
	args.type = RCallbackArgs::RReadConsole;
	args.chars_a = &prompt;
	args.chars_b = (char **) (&buf);
	args.int_a = buflen;
	args.int_b = hist;		// actually, we ignore hist

	REmbedInternal::this_pointer->handleStandardCallback (&args);
// default implementation seems to return 1 on success, 0 on failure, contrary to some documentation. see unix/std-sys.c
	if (buf)	return 1;
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

/*
void RBusy (int which) {
// I guess this is not any better (actually worse) than rkward's own busy indikator
} */

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

// ############## R Standard callback overrides END ####################

REmbedInternal::REmbedInternal() {
	RKGlobals::empty_char = strdup ("");
	RKGlobals::unknown_char = strdup ("?");
	RKGlobals::na_double = NA_REAL;
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
//	ptr_R_Busy = RBusy;				// probably we don't have any use for this
	ptr_R_CleanUp = RCleanUp;			// unfortunately, it seems, we can't safely cancel quitting anymore, here!
	ptr_R_ShowFiles = RShowFiles;
	ptr_R_ChooseFile = RChooseFile;
	ptr_R_EditFile = REditFile;
//	ptr_R_EditFiles = REditFiles;		// undefined reference

// these two, we won't override
//	ptr_R_loadhistory = ... 	// we keep our own history
//	ptr_R_savehistory = ...	// we keep our own history
}

REmbedInternal::~REmbedInternal (){
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
	fpu_setup (FALSE);
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

char **extractStrings (SEXP from_exp, int *count) {
	char **strings = 0;
	
	SEXP strexp;
	PROTECT (strexp = coerceVector (from_exp, STRSXP));
	*count = length (strexp);
	strings = new char* [length (strexp)];
	for (int i = 0; i < *count; ++i) {
		SEXP dummy = VECTOR_ELT (strexp, i);
// TODO: can we avoid this string copying by protecting strexp?
		if (TYPEOF (dummy) != CHARSXP) {
			strings[i] = strdup ("not defined");	// can this ever happen?
		} else {
			if (dummy == NA_STRING) {
				strings[i] = RKGlobals::empty_char;
			} else {
				strings[i] = strdup ((char *) STRING_PTR (dummy));
			}
		}
	}
	UNPROTECT (1);	// strexp
	
	return strings;
}

void deleteStrings (char **strings, int count) {
	for (int i= (count-1); i >=0; --i) {
		DELETE_STRING (strings[i]);
	}
	delete [] strings;
}

SEXP doError (SEXP call) {
	extern int R_ShowErrorMessages;
	if (R_ShowErrorMessages) {
		int count;
		char **strings = extractStrings (call, &count);
		REmbedInternal::this_pointer->handleError (strings, count);
		deleteStrings (strings, count);
	}
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
	int count;
	char **strings = extractStrings (call, &count);
	REmbedInternal::this_pointer->handleSubstackCall (strings, count);
	deleteStrings (strings, count);
	return R_NilValue;
}

bool REmbedInternal::startR (int argc, char** argv) {
	if (Rf_initEmbeddedR(argc, argv) < 0) {
		return false;
	}

// let's hope R internals never change...
	addDLL (strdup ("rkward_pseudo_dll_pseudo_path"), strdup ("rkward_pseudo_dll"), 0);
	DllInfo *info = R_getDllInfo ("rkward_pseudo_dll_pseudo_path");
	
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
// heavy copying from RServe below
	int maxParts=1;
	int r_error = 0;
	ParseStatus status = PARSE_NULL;
	const char *c = command;
	SEXP cv, pr, exp;

	while (*c) {
		if (*c=='\n' || *c==';') maxParts++;
		c++;
	}

	PROTECT(cv=allocVector(STRSXP, 1));
	SET_VECTOR_ELT(cv, 0, mkChar(command));  

	// TODO: Maybe we can use R_ParseGeneral instead. Then we could find the exact character, where parsing fails
	while (maxParts>0) {
		pr=R_ParseVector(cv, maxParts, &status);
		// 2=incomplete; 4=eof
		if (status!=PARSE_INCOMPLETE && status!=PARSE_EOF) {
			break;
		}
		maxParts--;
	}
	UNPROTECT(1);

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
	}

	/* Do NOT ask me why, but the line below is needed for warnings to be printed, while otherwise they would not be shown.
	Apparently we need to print at least something in order to achieve this. Whatever really happens in Rprintf () to have such an effect, I did not bother to find out. */
	Rprintf ("");

	if (R_CollectWarnings) {
		Rf_PrintWarnings ();
	}

//	SET_SYMVALUE(R_LastvalueSymbol, exp);
	UNPROTECT(1); /* pr */

	return exp;
}

/* Basically a safe version of Rf_PrintValue, as yes, Rf_PrintValue may lead to an error and long_jump->crash!
For example in help (function, htmlhelp=TRUE), when no HTML-help is installed!
SEXP exp should be PROTECTed prior to calling this function.
//TODO: I don't think it's meant to be this way. Maybe nag the R-devels about it one day. */
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

void REmbedInternal::runCommandInternal (const char *command, RKWardRError *error, bool print_result) {
	if (!print_result) {
		runCommandInternalBase (command, error);
	} else {
		R_Visible = (Rboolean) 1;

		SEXP exp;
		PROTECT (exp = runCommandInternalBase (command, error));
/*		char dummy[100];
		sprintf (dummy, "type: %d", TYPEOF (exp));
		Rprintf (dummy, 100); */
		if (R_Visible) {
			if (*error == NoError) {
				tryPrintValue (exp, error);
			}
		}
		UNPROTECT (1); /* exp */

		/* See the comment in the corresponding code in runCommandInternalBase. And yes, apparently, we need this at both places! */
		Rprintf ("");

		if (R_CollectWarnings) {
			Rf_PrintWarnings ();
		}
	} 
}

char **REmbedInternal::getCommandAsStringVector (const char *command, int *count, RKWardRError *error) {	
	SEXP exp;
	char **strings = 0;
	
	PROTECT (exp = runCommandInternalBase (command, error));
	
	if (*error == NoError) {
		strings = extractStrings (exp, count);
	}
	
	UNPROTECT (1); // exp
	
	if (*error != NoError) {
		*count = 0;
		return 0;
	}
	return strings;
}

double *REmbedInternal::getCommandAsRealVector (const char *command, int *count, RKWardRError *error) {
	SEXP exp;
	double *reals = 0;
	
	PROTECT (exp = runCommandInternalBase (command, error));
	
	if (*error == NoError) {
		SEXP realexp;
		PROTECT (realexp = coerceVector (exp, REALSXP));
		*count = length (realexp);
		reals = new double[*count];
		for (int i = 0; i < *count; ++i) {
				reals[i] = REAL (realexp)[i];
				if (R_IsNaN (reals[i]) || R_IsNA (reals[i]) ) reals[i] = RKGlobals::na_double;
		}
		UNPROTECT (1);	// realexp
	}
	
	UNPROTECT (1); // exp
	
	if (*error != NoError) {
		*count = 0;
		return 0;
	}
	return reals;
}

int *REmbedInternal::getCommandAsIntVector (const char *command, int *count, RKWardRError *error) {
	SEXP exp;
	int *integers = 0;
	
	PROTECT (exp = runCommandInternalBase (command, error));
	
	if (*error == NoError) {
		SEXP intexp;
		PROTECT (intexp = coerceVector (exp, INTSXP));
		*count = length (intexp);
		integers = new int[*count];
		for (int i = 0; i < *count; ++i) {
				integers[i] = INTEGER (intexp)[i];
		}
		UNPROTECT (1);	// intexp
	}
	
	UNPROTECT (1); // exp
	
	if (*error != NoError) {
		*count = 0;
		return 0;
	}
	return integers;
}


