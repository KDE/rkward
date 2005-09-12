
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

#include <stdlib.h>
#include <string.h>

#include <math.h>

#include "../rkglobals.h"

/// ############## R Standard callback overrides BEGIN ####################
void RSuicide (char* message) {
// TODO
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
	RCallbackArgs args;
	args.type = RCallbackArgs::RWriteConsole;
	args.chars_a = &buf;
	args.int_a = buflen;
	REmbedInternal::this_pointer->handleStandardCallback (&args);
}

void RResetConsole () {
// we leave this un-implemented on purpose! We simply don't want that sort of thing to be done.
}

void RFlushConsole () {
// we leave this un-implemented on purpose! We simply don't want that sort of thing to be done.
}

void RClearerrConsole () {
// we leave this un-implemented on purpose! We simply don't want that sort of thing to be done.
}

void RBusy (int which) {
//TODO
}

void RCleanUp (SA_TYPE saveact, int status, int RunLast) {
// TODO
}

int RShowFiles (int nfile, char **file, char **headers, char *wtitle, Rboolean del, char *pager) {
// TODO
// default implementation seems to returns 1 on success, 0 on failure. see unix/std-sys.c
	return 1;
}

int RChooseFile (int isnew, char *buf, int len) {
// TODO
// return length of filename (strlen (buf))
	return 0;
}

int REditFile (char *buf) {
// TODO
// does not exist in standard R 2.1.0, so no idea what to return.
	return 0;
}

int REditFiles (int nfile, char **file, char **title, char *editor) {
//TODO
// default impelementation seems to return 1 if nfile <= 0, else 1. No idea, what for. see unix/std-sys.c
	return (nfile <= 0);
}
/// ############## R Standard callback overrides END ####################


REmbedInternal::REmbedInternal() {
	RKGlobals::empty_char = strdup ("");
	RKGlobals::unknown_char = strdup ("?");
	RKGlobals::na_double = NA_REAL;
}

void REmbedInternal::connectCallbacks () {
// R standard callback pointers.
// Rinterface.h thinks this can only ever be done on aqua, apparently. Here, we define it the other way around, i.e. #ifndef instead of #ifdef
#ifndef HAVE_AQUA
	extern int  (*ptr_R_EditFiles)(int, char **, char **, char *);
#endif

// connect R standard callback to our own functions. Important: Don't do so, before our own versions are ready to be used!
//	ptr_R_Suicide = RSuicide;
	ptr_R_ShowMessage = RShowMessage;			// when exactly does this ever get used?
	ptr_R_ReadConsole = RReadConsole;
	ptr_R_WriteConsole = RWriteConsole;				// when exactly does this ever get used? Apparently it does not get called if a sink is in place, but otherwise it is! Probably we can get rid of our sink, then, and provide "real-time" output! More '!'s!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	ptr_R_ResetConsole = RResetConsole;
	ptr_R_FlushConsole = RFlushConsole;
	ptr_R_ClearerrConsole = RClearerrConsole;
//	ptr_R_Busy = RBusy;
//	ptr_R_CleanUp = RCleanUp;
//	ptr_R_ShowFiles = RShowFiles;
//	ptr_R_ChooseFile = RChooseFile;
//	ptr_R_EditFile = REditFile;
//	ptr_R_EditFiles = REditFiles;

// these two, we won't override
//	ptr_R_loadhistory = ... 	// we keep our own history
//	ptr_R_savehistory = ...	// we keep our own history
}

REmbedInternal::~REmbedInternal (){
}

void REmbedInternal::shutdown () {
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
/*
SEXP getValueCall (SEXP call) {
	int count;
	char **strings = extractStrings (call, &count);
	int reply_length;
	REmbedInternal::this_pointer->handleGetValueCall (strings, count, &reply_length);
	return call;
}*/

SEXP doSubstackCall (SEXP call) {
	int count;
	char **strings = extractStrings (call, &count);
	REmbedInternal::this_pointer->handleSubstackCall (strings, count);
	return R_NilValue;
}

bool REmbedInternal::startR (const char* r_home, int argc, char** argv) {
	extern int Rf_initEmbeddedR(int argc, char **argv);
	setenv("R_HOME", r_home, 1);
	if (Rf_initEmbeddedR(argc, argv) < 0) {
		return false;
	}

// let's hope R internals never change...
	typedef void *HINSTANCE;
	extern int addDLL (char *path, char *name, HINSTANCE *handle);
	addDLL (strdup ("rkward_pseudo_dll_pseudo_path"), strdup ("rkward_pseudo_dll"), 0);
	DllInfo *info = R_getDllInfo ("rkward_pseudo_dll_pseudo_path");
	
	R_CallMethodDef callMethods [] = {
		//{ "rk.get.value", (DL_FUNC) &getValueCall, 1 },
		{ "rk.do.command", (DL_FUNC) &doSubstackCall, 1 },
		{ 0, 0, 0 }
	};
	R_registerRoutines (info, NULL, callMethods, NULL, NULL);
	
	return true;
}

SEXP runCommandInternalBase (const char *command, REmbedInternal::RKWardRError *error) {
// heavy copying from RServe below
	extern SEXP R_ParseVector(SEXP, int, int*);

	int maxParts=1;
	int r_error = 0;
	int status;
	int *stat = &status;
	const char *c = command;
	SEXP cv, pr, exp;

	while (*c) {
		if (*c=='\n' || *c==';') maxParts++;
		c++;
	}

	PROTECT(cv=allocVector(STRSXP, 1));
	SET_VECTOR_ELT(cv, 0, mkChar(command));  

	while (maxParts>0) {
		pr=R_ParseVector(cv, maxParts, stat);
		// 2=incomplete; 4=eof
		if (status!=2 && status!=4) {
			break;
		}
		maxParts--;
	}
	UNPROTECT(1);

	if (status != 1) {
		if ((status == 2) || (status == 4)) {
			*error = REmbedInternal::Incomplete;
		} else {
			*error = REmbedInternal::SyntaxError;
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

		UNPROTECT(1); /* pr */

		if (r_error) {
			*error = REmbedInternal::OtherError;
		} else {
			*error = REmbedInternal::NoError;
		}
	}

	return exp;
}

void REmbedInternal::runCommandInternal (const char *command, RKWardRError *error, bool print_result) {
	if (!print_result) {
		runCommandInternalBase (command, error);
	} else {
		extern Rboolean R_Visible;
		R_Visible = (Rboolean) 1;

		SEXP exp;
		PROTECT (exp = runCommandInternalBase (command, error));
		if (R_Visible) {
			if (*error == NoError) Rf_PrintValue (exp);
		}
		UNPROTECT (1);
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

} // extern "C"
