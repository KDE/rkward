/***************************************************************************
                          rembedinternal  -  description
                             -------------------
    begin                : Sun Jul 25 2004
    copyright            : (C) 2004, 2005, 2006, 2007, 2008, 2009 by Thomas Friedrichsmeier
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

#include <qstring.h>
#include <QStringList>
#include <qtextcodec.h>
#include <klocale.h>
#include "../core/robject.h"
#include "../debug.h"

#include "rklocalesupport.h"
#include "rkpthreadsupport.h"
#include "rksignalsupport.h"

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#ifdef Q_WS_WIN
#	include <winsock.h>
#	undef ERROR	// windows define clashes with R define
#else
#include <dlfcn.h>
#include <sys/resource.h>
#include <sys/types.h>
#endif
#include <math.h>

#include "Rversion.h"

#if (R_VERSION > R_Version(2, 6, 9))
#define R_2_7
#endif

#ifndef R_2_7
#error R version 2.7.0 or later is needed to compile this version of RKWard
#endif

#if (R_VERSION > R_Version(2, 8, 9))
#define R_2_9
#endif

extern "C" {
#define R_INTERFACE_PTRS 1

// needed to detect CHARSXP encoding
#define IS_UTF8(x) (Rf_getCharCE(x) == CE_UTF8)
#define IS_LATIN1(x) (Rf_getCharCE(x) == CE_LATIN1)

#ifdef Q_WS_WIN
#	define Win32
#	include "R_ext/RStartup.h"
#	include "R_ext/Utils.h"

	extern int R_interrupts_pending;
#	warning Or is it UserBreak?
	void RK_scheduleIntr () {
		R_interrupts_pending = 1;
	}

	void RK_doIntr () {
		RK_scheduleIntr ();
		R_CheckUserInterrupt ();
	}

	structRstart RK_R_Params;
#else
#	define RK_doIntr Rf_onintr
#	include "Rinterface.h"
#endif
#include "Rdefines.h"
#include "R_ext/Rdynload.h"
#include "R_ext/eventloop.h"
#include "R_ext/Callbacks.h"
#include "R.h"
#include "Rinternals.h"
#include "R_ext/Parse.h"
#include "Rembedded.h"

// some functions we need that are not declared
extern void Rf_PrintWarnings (void);
extern int Rf_initialize_R(int ac, char **av);	// in embedded.h in R 2.9.0. TODO: check presence in R 2.7.0
extern void setup_Rmainloop(void);	// in embedded.h in R 2.9.0. TODO: check presence in R 2.7.0
extern uintptr_t R_CStackLimit;	// inRinterface.h in R 2.9.0. TODO: check presence in R 2.7.0
extern uintptr_t R_CStackStart;	// inRinterface.h in R 2.9.0. TODO: check presence in R 2.7.0
extern Rboolean R_Interactive;	// inRinterface.h in R 2.9.0. TODO: check presence in R 2.7.0
SEXP R_LastvalueSymbol;
#include "R_ext/eventloop.h"
}

#include "../rkglobals.h"
#include "rdata.h"

// Needed for the REPL
const char *current_buffer = 0;
bool repldlldo1_wants_code = false;
bool repldll_buffer_transfer_finished = false;
int repldll_result = 0;		/* -2: error; -1: incomplete; 0: nothing, yet 1: ok 2: incomplete statement, while buffer not empty. Should not happen */
bool repldll_last_parse_successful = false;


SEXP RKWard_RData_Tag;
QString *SEXPToStringList (SEXP from_exp, unsigned int *count);

// ############## R Standard callback overrides BEGIN ####################
void RSuicide (const char* message) {
	RK_TRACE (RBACKEND);

	RCallbackArgs args;
	args.type = RCallbackArgs::RBackendExit;
	args.params["message"] = QVariant (i18n ("The R engine has encountered a fatal error:\n%1").arg (message));
	REmbedInternal::this_pointer->handleStandardCallback (&args);
	REmbedInternal::this_pointer->shutdown (true);
	Rf_error ("Backend dead");	// this jumps us out of the REPL.
}

void RShowMessage (const char* message) {
	RK_TRACE (RBACKEND);

	RCallbackArgs args;
	args.type = RCallbackArgs::RShowMessage;
	args.params["message"] = QVariant (message);
	REmbedInternal::this_pointer->handleStandardCallback (&args);
}

// TODO: currently used on windows, only!
#warning implement in rinterface.cpp!
/* FROM R_ext/RStartup.h: "Return value here is expected to be 1 for Yes, -1 for No and 0 for Cancel:
   symbolic constants in graphapp.h" */
int RAskYesNoCancel (const char* message) {
	RK_TRACE (RBACKEND);

	RCallbackArgs args;
	args.type = RCallbackArgs::RShowMessage;
	args.params["message"] = QVariant (message);
	args.params["askync"] = QVariant (true);

	REmbedInternal::this_pointer->handleStandardCallback (&args);

	QString ret = args.params["result"].toString ();
	if (ret == "yes") return 1;
	if (ret == "no") return -1;
	return 0;
}

int RReadConsole (const char* prompt, unsigned char* buf, int buflen, int hist) {
	RK_TRACE (RBACKEND);

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

	// here, we handle readline() calls and such, i.e. not the regular prompt for code
	// browser() also takes us here.
	RCallbackArgs args;
	args.type = RCallbackArgs::RReadLine;
	args.params["prompt"] = QVariant (prompt);
	args.params["cancelled"] = QVariant (false);

	REmbedInternal::this_pointer->handleStandardCallback (&args);
// default implementation seems to return 1 on success, 0 on failure, contrary to some documentation. see unix/std-sys.c
	if (args.params["cancelled"].toBool ()) {
#warning TODO: this should be handled in rthread.cpp, instead
		REmbedInternal::this_pointer->currentCommandWasCancelled ();
		RK_doIntr();
		return 0;	// we should not ever get here, but still...
	}
	if (buf) {
		QByteArray localres = REmbedInternal::this_pointer->current_locale_codec->fromUnicode (args.params["result"].toString ());
		// need to append a newline, here. TODO: theoretically, RReadConsole comes back for more, if \0 was encountered before \n.
		qstrncpy ((char *) buf, localres.left (buflen - 2).append ('\n').data (), buflen);
		return 1;
	}
	return 0;
}

#ifdef Q_WS_WIN
int RReadConsoleWin (const char* prompt, char* buf, int buflen, int hist) {
	return RReadConsole (prompt, (unsigned char*) buf, buflen, hist);
}
#endif

void RWriteConsoleEx (const char *buf, int buflen, int type) {
	RK_TRACE (RBACKEND);

	REmbedInternal::this_pointer->handleOutput (REmbedInternal::this_pointer->current_locale_codec->toUnicode (buf, buflen), buflen, type == 0);
}

/** For R callbacks that we want to disable, entirely */
void RDoNothing () {
	RK_TRACE (RBACKEND);
}

void RCleanUp (SA_TYPE saveact, int status, int RunLast) {
	RK_TRACE (RBACKEND);

	if (saveact != SA_SUICIDE) {
		RCallbackArgs args;
		args.type = RCallbackArgs::RBackendExit;
		args.params["message"] = QVariant (i18n ("The R engine has shut down with status: %1").arg (status));
		REmbedInternal::this_pointer->handleStandardCallback (&args);

		if(saveact == SA_DEFAULT) saveact = SA_SAVE;
		if (saveact == SA_SAVE) {
				if (RunLast) R_dot_Last ();
				if (R_DirtyImage) R_SaveGlobalEnv ();
		} else {
				if (RunLast) R_dot_Last ();
		}

		REmbedInternal::this_pointer->shutdown (false);
	} else {
		REmbedInternal::this_pointer->shutdown (true);
	}
	Rf_error ("Backend dead");	// this jumps us out of the REPL.
}

QStringList charPArrayToQStringList (const char** chars, int count) {
	QStringList ret;
	for (int i = 0; i < count; ++i) {
		// do we need to do locale conversion, here?
		ret.append (chars[i]);
	}
	return ret;
}

int RShowFiles (int nfile, const char **file, const char **headers, const char *wtitle, Rboolean del, const char *pager) {
	RK_TRACE (RBACKEND);

	RCallbackArgs args;
	args.type = RCallbackArgs::RShowFiles;
	args.params["files"] = QVariant (charPArrayToQStringList (file, nfile));
	args.params["titles"] = QVariant (charPArrayToQStringList (headers, nfile));
	args.params["wtitle"] = QVariant (wtitle);
	args.params["delete"] = QVariant (del);
	REmbedInternal::this_pointer->handleStandardCallback (&args);

// default implementation seems to returns 1 on success, 0 on failure. see unix/std-sys.c
	return 1;
}

int RChooseFile (int isnew, char *buf, int len) {
	RK_TRACE (RBACKEND);

	RCallbackArgs args;
	args.type = RCallbackArgs::RChooseFile;
	args.params["new"] = QVariant ((bool) isnew);

	REmbedInternal::this_pointer->handleStandardCallback (&args);

	QByteArray localres = REmbedInternal::this_pointer->current_locale_codec->fromUnicode (args.params["result"].toString ());
	qstrncpy ((char *) buf, localres.data (), len);

// return length of filename (strlen (buf))
	return (qMin (len - 1, localres.size ()));
}

void REditFilesHelper (QStringList files, QStringList titles, QString name_or_something) {
	RK_TRACE (RBACKEND);

	RCallbackArgs args;
	args.type = RCallbackArgs::REditFiles;
	args.params["files"] = QVariant (files);
	args.params["titles"] = QVariant (titles);
	// and whatever the name_or_something from REditFiles is for...
	REmbedInternal::this_pointer->handleStandardCallback (&args);
}

int REditFiles (int nfile, const char **file, const char **title, const char *editor) {
	RK_TRACE (RBACKEND);

	REditFilesHelper (charPArrayToQStringList (file, nfile), charPArrayToQStringList (title, nfile), editor);

// default implementation seems to return 1 if nfile <= 0, else 1. No idea, what for. see unix/std-sys.c
	return (nfile <= 0);
}

SEXP doEditFiles (SEXP files, SEXP titles, SEXP name) {
	RK_TRACE (RBACKEND);

	// this function would be much shorter, if SEXPToStringList would simply return a QStringList...
	unsigned int files_count, titles_count, name_count;
	QString *file_strings = SEXPToStringList (files, &files_count);
	QString *title_strings = SEXPToStringList (titles, &titles_count);
	QString *name_strings = SEXPToStringList (name, &name_count);

	RK_ASSERT (name_count <= 1);
	RK_ASSERT (files_count == titles_count);
	RK_ASSERT (files_count >= 1);

	files_count = titles_count = qMin (files_count, titles_count);

	QStringList files_list;
	QStringList titles_list;
	for (unsigned int i = 0; i < files_count; ++i) {
		files_list.append (file_strings[i]);
		titles_list.append (title_strings[i]);
	}
	QString name_string;
	if (name_count) name_string = name_strings[0];

	REditFilesHelper (files_list, titles_list, name_string);

	delete [] file_strings;
	delete [] title_strings;
	delete [] name_strings;

	return (R_NilValue);
}

int REditFile (const char *buf) {
	RK_TRACE (RBACKEND);

	const char *editor = "none";
	const char *title = "";

// does not exist in standard R 2.1.0, so no idea what to return.
	return REditFiles (1, const_cast<const char**> (&buf), &title, editor);
}

void RBusy (int busy) {
	RK_TRACE (RBACKEND);

	// R_ReplDLLDo1 calls R_Busy (1) after reading in code (if needed), parsing it, and right before evaluating it.
	if (busy) {
		repldlldo1_wants_code = false;
		repldll_last_parse_successful = true;
	}
}

// ############## R Standard callback overrides END ####################

char *REmbedInternal::na_char_internal = new char;

REmbedInternal::REmbedInternal () {
	RK_TRACE (RBACKEND);

	current_locale_codec = QTextCodec::codecForLocale ();
	r_running = false;
}

#ifdef Q_WS_WIN
void REmbedInternal::setupCallbacks () {
	RK_TRACE (RBACKEND);

	R_setStartTime();
	R_DefParams(&RK_R_Params);

// IMPORTANT: see also the #ifndef QS_WS_WIN-portion!
	RK_R_Params.rhome = get_R_HOME ();
	RK_R_Params.home = getRUser ();
	RK_R_Params.CharacterMode = LinkDLL;
	RK_R_Params.ShowMessage = RShowMessage;
	RK_R_Params.ReadConsole = RReadConsoleWin;
	RK_R_Params.WriteConsoleEx = RWriteConsoleEx;
	RK_R_Params.WriteConsole = 0;
	RK_R_Params.CallBack = RDoNothing;
	RK_R_Params.YesNoCancel = RAskYesNoCancel;
	RK_R_Params.Busy = RBusy;

	// TODO: callback mechanism(s) for ChosseFile, ShowFiles, EditFiles
	// TODO: also for RSuicide / RCleanup? (Less important, obviously, since those should not be triggered, in normal operation).

	RK_R_Params.R_Quiet = (Rboolean) 0;
	RK_R_Params.R_Interactive = (Rboolean) 1;
}

void REmbedInternal::connectCallbacks () {
	RK_TRACE (RBACKEND);
	R_SetParams(&RK_R_Params);
}
#else
void REmbedInternal::setupCallbacks () {
	RK_TRACE (RBACKEND);
}

void REmbedInternal::connectCallbacks () {
	RK_TRACE (RBACKEND);

// IMPORTANT: see also the #ifdef QS_WS_WIN-portion!
// connect R standard callback to our own functions. Important: Don't do so, before our own versions are ready to be used!
	R_Outputfile = NULL;
	R_Consolefile = NULL;
	ptr_R_Suicide = RSuicide;
	ptr_R_ShowMessage = RShowMessage;		// rarely used in R on unix
	ptr_R_ReadConsole = RReadConsole;
	ptr_R_WriteConsoleEx = RWriteConsoleEx;
	ptr_R_WriteConsole = 0;
	ptr_R_ResetConsole = RDoNothing;
	ptr_R_FlushConsole = RDoNothing;
	ptr_R_ClearerrConsole = RDoNothing;
	ptr_R_Busy = RBusy;
	ptr_R_CleanUp = RCleanUp;			// unfortunately, it seems, we can't safely cancel quitting anymore, here!
	ptr_R_ShowFiles = RShowFiles;
	ptr_R_ChooseFile = RChooseFile;
// TODO: R devels disabled this for some reason. We set it anyway...
	ptr_R_EditFile = REditFile;
//	ptr_R_EditFiles = REditFiles;		// undefined reference

// these two, we won't override
//	ptr_R_loadhistory = ... 	// we keep our own history
//	ptr_R_savehistory = ...	// we keep our own history
}
#endif

REmbedInternal::~REmbedInternal () {
	RK_TRACE (RBACKEND);
}

void REmbedInternal::shutdown (bool suicidal) {
	RK_TRACE (RBACKEND);

	if (!r_running) return;		// already shut down
	r_running = false;

// Code-recipe below essentially copied from http://stat.ethz.ch/R-manual/R-devel/doc/manual/R-exts.html#Linking-GUIs-and-other-front_ends-to-R
// modified quite a bit for our needs.

	if (!suicidal) {
		R_dot_Last ();
	}

	R_RunExitFinalizers();

	R_CleanTempDir ();

	/* close all the graphics devices */
	if (!suicidal) Rf_KillAllDevices ();
#ifndef Q_WS_WIN
	fpu_setup ((Rboolean) FALSE);
#endif
}

#if 0
static int timeout_counter = 0;
#endif

void processX11EventsWorker (void *) {
// this basically copied from R's unix/sys-std.c (Rstd_ReadConsole)
	for (;;) {
		fd_set *what;
		what = R_checkActivityEx(R_wait_usec > 0 ? R_wait_usec : 50, 1, RK_doIntr);
		if (what == NULL) break;
		R_runHandlers(R_InputHandlers, what);
	}

#if 0
// TODO: The remainder of this function had been commented out since R 2.3.x and is not in Rstd_ReadConsole. Do we still need this?
	/* I don't really understand what I'm doing here, but apparently this is necessary for Tcl-Tk windows to function properly. */
	R_PolledEvents ();
	
/* Maybe we also need to also call R_timeout_handler once in a while? Obviously this is extremely crude code! 
TODO: verify we really need this. */
	if (++timeout_counter > 100) {
//		extern void (* R_timeout_handler) ();	// already defined in Rinferface.h
		if (R_timeout_handler) R_timeout_handler ();
		timeout_counter = 0;
	}
#endif
}

void REmbedInternal::processX11Events () {
	// do not trace
	if (!this_pointer->r_running) return;

// In case an error (or user interrupt) is caught inside processX11EventsWorker, we don't want to long-jump out.
	R_ToplevelExec (processX11EventsWorker, 0);
}

QString *SEXPToStringList (SEXP from_exp, unsigned int *count) {
	RK_TRACE (RBACKEND);

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
	QString *list = new QString[*count];
	unsigned int i = 0;
	for (; i < *count; ++i) {
#ifdef R_2_9
		SEXP dummy = STRING_ELT (from_exp, i);
#else
		SEXP dummy = VECTOR_ELT (from_exp, i);
#endif

		if (TYPEOF (dummy) != CHARSXP) {
			list[i] = QString ("not defined");	// can this ever happen?
		} else {
			if (dummy == NA_STRING) {
				list[i] = QString::null;
			} else {
				if (IS_UTF8 (dummy)) {
					list[i] = QString::fromUtf8 ((char *) STRING_PTR (dummy));
				} else if (IS_LATIN1 (dummy)) {
					list[i] = QString::fromLatin1 ((char *) STRING_PTR (dummy));
				} else {
					list[i] = REmbedInternal::this_pointer->current_locale_codec->toUnicode ((char *) STRING_PTR (dummy));
				}
			}
		}
	}

	return list;
}

int *SEXPToIntArray (SEXP from_exp, unsigned int *count) {
	RK_TRACE (RBACKEND);

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
	RK_TRACE (RBACKEND);

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
	RK_TRACE (RBACKEND);

	RData *data = new RData;

	unsigned int count;
	int type = TYPEOF (from_exp);
	switch (type) {
		case LGLSXP:
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
		case EXTPTRSXP:
			if (R_ExternalPtrTag (from_exp) == RKWard_RData_Tag) {		// our very own data
				delete data;
				data = (RData*) R_ExternalPtrAddr (from_exp);
				R_ClearExternalPtr (from_exp);
				count = data->length;
				break;
			}
		case STRSXP:
		default:
			data->data = SEXPToStringList (from_exp, &count);
			data->datatype = RData::StringVector;
	}

	data->length = count;

	return data;
}

SEXP doError (SEXP call) {
	RK_TRACE (RBACKEND);

	unsigned int count;
	QString *strings = SEXPToStringList (call, &count);
	REmbedInternal::this_pointer->handleError (strings, count);
	delete [] strings;
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
	RK_TRACE (RBACKEND);

	unsigned int count;
	QString *strings = SEXPToStringList (call, &count);
	QStringList list;
	for (unsigned int i = 0; i < count; ++i) {
		list.append (strings[i]);
	}
	REmbedInternal::this_pointer->handleSubstackCall (list);
	delete [] strings;
	return R_NilValue;
}

void R_CheckStackWrapper (void *) {
	R_CheckStack ();
}

SEXP doUpdateLocale () {
	RK_TRACE (RBACKEND);

	RK_DO (qDebug ("Changing locale"), RBACKEND, DL_WARNING);
	REmbedInternal::this_pointer->current_locale_codec = RKGetCurrentLocaleCodec ();
	RK_DO (qDebug ("New locale codec is %s", REmbedInternal::this_pointer->current_locale_codec->name ().data ()), RBACKEND, DL_WARNING);

	return R_NilValue;
}

#include "rkstructuregetter.cpp"

SEXP doGetStructure (SEXP toplevel, SEXP name, SEXP envlevel, SEXP namespacename) {
	RK_TRACE (RBACKEND);

	RKStructureGetter getter (false);
	RData *ret = getter.getStructure (toplevel, name, envlevel, namespacename);
	return R_MakeExternalPtr (ret, RKWard_RData_Tag, R_NilValue);
}

/** copy a symbol without touching it (esp. not forcing any promises) */
SEXP doCopyNoEval (SEXP name, SEXP fromenv, SEXP toenv) {
	RK_TRACE (RBACKEND);

	if(!isString (name) || length (name) != 1) error ("name is not a single string");
	if(!isEnvironment (fromenv)) error ("fromenv is not an environment");
	if(!isEnvironment (toenv)) error ("toenv is not an environment");
	defineVar (Rf_install (CHAR (STRING_ELT (name, 0))), findVar (Rf_install (CHAR (STRING_ELT (name, 0))), fromenv), toenv);
	return (R_NilValue);
}

bool REmbedInternal::startR (int argc, char** argv, bool stack_check) {
	RK_TRACE (RBACKEND);

	setupCallbacks ();

	RKSignalSupport::saveDefaultSigSegvHandler ();

	r_running = true;
	Rf_initialize_R (argc, argv);

	if (stack_check) {
		char dummy;
		size_t stacksize;
		void *stackstart;
		RKGetCurrentThreadStackLimits (&stacksize, &stackstart, &dummy);
		R_CStackStart = (uintptr_t) stackstart;
		R_CStackLimit = stacksize;
	} else {
		R_CStackStart = (uintptr_t) -1;
		R_CStackLimit = (uintptr_t) -1;
	}

#ifdef Q_WS_WIN
	R_set_command_line_arguments(argc, argv);
	FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
#endif

	setup_Rmainloop ();

	if (stack_check) {
		// safety check: If we are beyond the stack boundaries already, we better disable stack checking
		// this has to come *after* the first setup_Rmainloop ()!
		Rboolean stack_ok = R_ToplevelExec (R_CheckStackWrapper, (void *) 0);
		if (!stack_ok) {
			RK_DO (qDebug ("R_CheckStack() failed during initialization. Will disable stack checking and try to re-initialize."), RBACKEND, DL_WARNING);
			RK_DO (qDebug ("If this does not work, try the --disable-stack-check command line option, *and* submit a bug report."), RBACKEND, DL_WARNING);
			R_CStackStart = (uintptr_t) -1;
			R_CStackLimit = (uintptr_t) -1;
			setup_Rmainloop ();
		}
	}

	R_Interactive = (Rboolean) TRUE;

	RKGlobals::na_double = NA_REAL;
	R_ReplDLLinit ();

	RKWard_RData_Tag = Rf_install ("RKWard_RData_Tag");
	R_LastvalueSymbol = Rf_install (".Last.value");

	RKSignalSupport::installSigSegvProxy ();

// register our functions
	R_CallMethodDef callMethods [] = {
//		{ "rk.do.condition", (DL_FUNC) &doCondition, 1 },
		{ "rk.do.error", (DL_FUNC) &doError, 1 },
		{ "rk.do.command", (DL_FUNC) &doSubstackCall, 1 },
		{ "rk.update.locale", (DL_FUNC) &doUpdateLocale, 0 },
		{ "rk.get.structure", (DL_FUNC) &doGetStructure, 4 },
		{ "rk.copy.no.eval", (DL_FUNC) &doCopyNoEval, 3 },
		{ "rk.edit.files", (DL_FUNC) &doEditFiles, 3 },
		{ 0, 0, 0 }
	};
	R_registerRoutines (R_getEmbeddingDllInfo(), NULL, callMethods, NULL, NULL);

	return true;
}

SEXP parseCommand (const QString &command_qstring, REmbedInternal::RKWardRError *error) {
	RK_TRACE (RBACKEND);

	ParseStatus status = PARSE_NULL;
	SEXP cv, pr;

	QByteArray localc = REmbedInternal::this_pointer->current_locale_codec->fromUnicode (command_qstring);		// needed so the string below does not go out of scope
	const char *command = localc.data ();

	PROTECT(cv=allocVector(STRSXP, 1));
#ifdef R_2_9
	SET_STRING_ELT(cv, 0, mkChar(command));
#else
	SET_VECTOR_ELT(cv, 0, mkChar(command));
#endif

	// TODO: Maybe we can use R_ParseGeneral instead. Then we could find the exact character, where parsing fails. Nope: not exported API
	pr=R_ParseVector(cv, -1, &status, R_NilValue);
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
		pr = R_NilValue;
	}

	return pr;
}

SEXP runCommandInternalBase (SEXP pr, REmbedInternal::RKWardRError *error) {
	RK_TRACE (RBACKEND);

	SEXP exp;
	int r_error = 0;

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

	// for safety, let's protect exp for the two print calls below.
	// TODO: this is not good. It causes an additional PROTECT and UPROTECT. Need to (re-)move printing
	PROTECT (exp);
	/* Do NOT ask me why, but the line below is needed for warnings to be printed, while otherwise they would not be shown.
	Apparently we need to print at least something in order to achieve this. Whatever really happens in Rprintf () to have such an effect, I did not bother to find out. */
	Rprintf ((char *) "");

	Rf_PrintWarnings ();

	UNPROTECT (1);		// exp; We unprotect this, as most of the time the caller is not really interested in the result
	return exp;
}

#if 0
// This is currently unused, but might come in handy, again.
/* Basically a safe version of Rf_PrintValue, as yes, Rf_PrintValue may lead to an error and long_jump->crash!
For example in help (function, htmlhelp=TRUE), when no HTML-help is installed!
SEXP exp should be PROTECTed prior to calling this function.
//TODO: I don't think it's meant to be this way. Maybe nag the R-devels about it one day. 
//TODO: this is not entirely correct. See PrintValueEnv (), which is what Repl_Console uses (but is hidden)
*/
void tryPrintValue (SEXP exp, REmbedInternal::RKWardRError *error) {
	RK_TRACE (RBACKEND);

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

void runUserCommandInternal (void *) {
	RK_TRACE (RBACKEND);

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

void REmbedInternal::runCommandInternal (const QString &command_qstring, RKWardRError *error, bool print_result) {
	RK_TRACE (RBACKEND);

	connectCallbacks ();		// sorry, but we will not play nicely with additional frontends trying to override our callbacks. (Unless they start their own R event loop, then they should be fine)

	*error = NoError;
	if (!print_result) {
		SEXP parsed = parseCommand (command_qstring, error);
		if (*error == NoError) runCommandInternalBase (parsed, error);
	} else {		// run a user command
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

This is the logic spread out over the following section, runUserCommandInternal (), and RReadConsole (). 

NOTE from Deepayan Sarkar: Another possible simplification (which may not be worth doing
ultimately): you distinguish between two types of calls to
R_ReadConsole based on R_busy calls, but you may be able to use the
second 'hist' argument. I didn't look too carefully, but it seems like
hist == 1 iff R wants a parse-able input.
*/

		R_ReplDLLinit ();		// resets the parse buffer (things might be left over from a previous incomplete parse)
		bool prev_iteration_was_incomplete = false;

		QByteArray localc = current_locale_codec->fromUnicode (command_qstring);		// needed so the string below does not go out of scope
		current_buffer = localc.data ();

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
	}
}

QString *REmbedInternal::getCommandAsStringVector (const QString &command, uint *count, RKWardRError *error) {	
	RK_TRACE (RBACKEND);

	SEXP exp;
	QString *list = 0;

	*error = NoError;
	SEXP parsed = parseCommand (command, error);
	if (*error == NoError) PROTECT (exp = runCommandInternalBase (parsed, error));
	
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

double *REmbedInternal::getCommandAsRealVector (const QString &command, uint *count, RKWardRError *error) {
	RK_TRACE (RBACKEND);

	SEXP exp;
	double *reals = 0;
	
	*error = NoError;
	SEXP parsed = parseCommand (command, error);
	if (*error == NoError) PROTECT (exp = runCommandInternalBase (parsed, error));
	
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

int *REmbedInternal::getCommandAsIntVector (const QString &command, uint *count, RKWardRError *error) {
	RK_TRACE (RBACKEND);

	SEXP exp;
	int *integers = 0;
	
	*error = NoError;
	SEXP parsed = parseCommand (command, error);
	if (*error == NoError) PROTECT (exp = runCommandInternalBase (parsed, error));
	
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

RData *REmbedInternal::getCommandAsRData (const QString &command, RKWardRError *error) {
	RK_TRACE (RBACKEND);

	SEXP exp;
	RData *data = 0;
	
	*error = NoError;
	SEXP parsed = parseCommand (command, error);
	if (*error == NoError) PROTECT (exp = runCommandInternalBase (parsed, error));
	
	if (*error == NoError) {
		data = SEXPToRData (exp);
	}
	
	UNPROTECT (1); // exp
	
	return data;
}
