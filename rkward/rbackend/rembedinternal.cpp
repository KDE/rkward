/***************************************************************************
                          rembedinternal  -  description
                             -------------------
    begin                : Sun Jul 25 2004
    copyright            : (C) 2004, 2005, 2006, 2007, 2008, 2009, 2010 by Thomas Friedrichsmeier
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
RThread *RThread::this_pointer = 0;
RThread::RKReplStatus RThread::repl_status = { QByteArray (), 0, true, 0, 0, RThread::RKReplStatus::NoUserCommand, 0, false };
void* RThread::default_global_context = 0;

#include <qstring.h>
#include <QStringList>
#include <qtextcodec.h>
#include <klocale.h>

#include "../core/robject.h"
#include "../debug.h"

#include "rkrsupport.h"
#include "rinterface.h"
#include "rklocalesupport.h"
#include "rkpthreadsupport.h"
#include "rksignalsupport.h"
#include "../misc/rkcommonfunctions.h"

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

extern "C" {
#define R_INTERFACE_PTRS 1
// for R_CStackStart/Limit
#define CSTACK_DEFNS 1
// keep R from defining tons of aliases
#define R_NO_REMAP 1
// What the...? "Conflicting definitions" between stdint.h and Rinterface.h despite the #if in Rinterface.h
#define uintptr_t uintptr_t

// needed to detect CHARSXP encoding
#define IS_UTF8(x) (Rf_getCharCE(x) == CE_UTF8)
#define IS_LATIN1(x) (Rf_getCharCE(x) == CE_LATIN1)

#ifdef Q_WS_WIN
	// needed for R includes
#	define Win32
#endif

#include <Rdefines.h>
#include <R_ext/Rdynload.h>
#include <R_ext/eventloop.h>
#include <R_ext/Callbacks.h>
#include <R.h>
#include <Rinternals.h>
#include <R_ext/Parse.h>
#include <Rembedded.h>

#ifdef Q_WS_WIN
#	include <R_ext/RStartup.h>
#	include <R_ext/Utils.h>

	void RK_scheduleIntr () {
		UserBreak = 1;
	}

	void RK_doIntr () {
		RK_scheduleIntr ();
		R_CheckUserInterrupt ();
	}

	structRstart RK_R_Params;
#else
#	define RK_doIntr Rf_onintr
#	include <Rinterface.h>
#endif

// some functions we need that are not declared
extern void Rf_PrintWarnings (void);
extern void run_Rmainloop (void);
SEXP R_LastvalueSymbol;
#include <R_ext/eventloop.h>
}

#include "../rkglobals.h"
#include "rdata.h"

extern SEXP RKWard_RData_Tag;
SEXP parseCommand (const QString &command_qstring, RThread::RKWardRError *error);
SEXP runCommandInternalBase (SEXP pr, RThread::RKWardRError *error);

// ############## R Standard callback overrides BEGIN ####################
void RSuicide (const char* message) {
	RK_TRACE (RBACKEND);

	RBackendRequest request (true, RBackendRequest::BackendExit);
	request.params["message"] = QVariant (i18n ("The R engine has encountered a fatal error:\n%1").arg (message));
	RThread::this_pointer->handleRequest (&request);
	RThread::this_pointer->killed = true;
}

Rboolean RKToplevelStatementFinishedCallback (SEXP expr, SEXP value, Rboolean succeeded, Rboolean visible, void *) {
	RK_TRACE (RBACKEND);
	Q_UNUSED (expr);
	Q_UNUSED (value);
	Q_UNUSED (visible);

	if ((RThread::repl_status.eval_depth == 0) && (!RThread::repl_status.in_browser_context)) {		// Yes, toplevel-handlers _do_ get called in a browser context!
		RK_ASSERT (RThread::repl_status.user_command_status = RThread::RKReplStatus::UserCommandRunning);
		if (succeeded) {
			RThread::repl_status.user_command_successful_up_to = RThread::repl_status.user_command_parsed_up_to;
			if (RThread::repl_status.user_command_completely_transmitted) {
				RThread::repl_status.user_command_status = RThread::RKReplStatus::NoUserCommand;
				RThread::this_pointer->commandFinished ();
			} else RThread::repl_status.user_command_status = RThread::RKReplStatus::UserCommandTransmitted;
		} else {
			// well, this point of code is never reached with R up to 2.12.0. Instead failed user commands are handled in doError().
			RThread::repl_status.user_command_status = RThread::RKReplStatus::UserCommandFailed;
		}
	}
	
	return (Rboolean) true;
}

void RKInsertToplevelStatementFinishedCallback (void *) {
	RK_TRACE (RBACKEND);

	if (RThread::this_pointer->r_running) {
		int pos;
		Rf_addTaskCallback (&RKToplevelStatementFinishedCallback, 0, &RKInsertToplevelStatementFinishedCallback, "_rkward_main_callback", &pos);
	}
}

void RKTransmitNextUserCommandChunk (unsigned char* buf, int buflen) {
	RK_TRACE (RBACKEND);

	RK_ASSERT (RThread::repl_status.user_command_transmitted_up_to <= RThread::repl_status.user_command_buffer.length ());	// NOTE: QByteArray::length () does not count the trailing '\0'
	const char* current_buffer = RThread::repl_status.user_command_buffer.data ();
	current_buffer += RThread::repl_status.user_command_transmitted_up_to;	// Skip what we have already transmitted

	bool reached_eof = false;
	int pos = 0;
	while (pos < (buflen-1)) {
		buf[pos] = *current_buffer;
		if (*current_buffer == '\n') break;
		else if (*current_buffer == ';') break;
		else if (*current_buffer == '\0') {
			reached_eof = true;
			break;
		}
		++current_buffer;
		++pos;
	}
	RThread::repl_status.user_command_transmitted_up_to += (pos + 1);
	if (reached_eof) {
		buf[pos] = '\n';
		RThread::repl_status.user_command_completely_transmitted = true;
	}
	buf[++pos] = '\0';
}

int RReadConsole (const char* prompt, unsigned char* buf, int buflen, int hist) {
	RK_TRACE (RBACKEND);

	RK_ASSERT (buf && buflen);
	RK_ASSERT (RThread::repl_status.eval_depth >= 0);

	if (RThread::repl_status.in_browser_context) {		// previously we were in a browser context. Check, whether we've left that.
		if (RThread::default_global_context == R_GlobalContext) {
			RThread::repl_status.in_browser_context = false;
			RK_ASSERT (!hist);
		}
	}
	
	if ((!RThread::repl_status.in_browser_context) && (RThread::repl_status.eval_depth == 0)) {
		while (1) {
			if (RThread::repl_status.user_command_status == RThread::RKReplStatus::NoUserCommand) {
				RCommandProxy *command = RThread::this_pointer->fetchNextCommand ();
				if (!command) {
					return 0;	// jumps out of the event loop!
				}

				if (!(command->type & RCommand::User)) {
					RThread::this_pointer->runCommand (command);
					RThread::this_pointer->commandFinished ();
				} else {
					// so, we are about to transmit a new user command, which is quite a complex endeavour...
					/* Some words about running user commands:
					- User commands can only be run at the top level of execution, not in any sub-stacks. But then, they should never get there, in the first place.
					- Handling user commands is totally different from all other commands, and relies on R's "REPL" (read-evaluate-print-loop). This is a whole bunch of dedicated code, but there is no other way to achieve handling of commands as if they had been entered on a plain R console (incluing auto-printing, and toplevel handlers). Most importantly, since important symbols are not exported, such as R_Visible. Vice versa, it is not possible to treat all commands like user commands, esp. in substacks.

					Problems to deal with:
					- R_ReadConsole serves a lot of different functions, including reading in code, but also handling user input for readline() or browser(). This makes it necessary to carefully track the current status using "repl_status". You will find repl_status to be modified at a couple of different functions.
					- One difficulty lies in finding out, just when a command has finished (successfully or with an error). RKToplevelStatementFinishCallback(), and doError() handle the respective cases.
					NOTE; in R 2.12.0 and above, Rf_countContexts() might help to find out when we are back to square 1!
					*/
					RThread::repl_status.user_command_transmitted_up_to = 0;
					RThread::repl_status.user_command_completely_transmitted = false;
					RThread::repl_status.user_command_parsed_up_to = 0;
					RThread::repl_status.user_command_successful_up_to = 0;
					RThread::repl_status.user_command_buffer = RThread::this_pointer->current_locale_codec->fromUnicode (command->command);
					RKTransmitNextUserCommandChunk (buf, buflen);
					RThread::repl_status.user_command_status = RThread::RKReplStatus::UserCommandTransmitted;
					return 1;
				}
			} else if (RThread::repl_status.user_command_status == RThread::RKReplStatus::UserCommandTransmitted) {
				if (RThread::repl_status.user_command_completely_transmitted) {
					// fully transmitted, but R is still asking for more? This looks like an incomplete statement.
					// HOWEVER: It may also have been an empty statement such as " ", so let's check whether the prompt looks like a "continue" prompt
					bool incomplete = false;
					if (RThread::this_pointer->current_locale_codec->toUnicode (prompt) == RKRSupport::SEXPToString (Rf_GetOption (Rf_install ("continue"), R_BaseEnv))) {
						incomplete = true;
					}
					if (incomplete) RThread::this_pointer->current_command->status |= RCommand::Failed | RCommand::ErrorIncomplete;
					RThread::repl_status.user_command_status = RThread::RKReplStatus::ReplIterationKilled;
#warning TODO: use Rf_error(""), instead?
					RK_doIntr ();	// to discard the buffer
				} else {
					RKTransmitNextUserCommandChunk (buf, buflen);
					return 1;
				}
			} else if (RThread::repl_status.user_command_status == RThread::RKReplStatus::UserCommandSyntaxError) {
				RThread::this_pointer->current_command->status |= RCommand::Failed | RCommand::ErrorSyntax;
				RThread::repl_status.user_command_status = RThread::RKReplStatus::NoUserCommand;
				RThread::this_pointer->commandFinished ();
			} else if (RThread::repl_status.user_command_status == RThread::RKReplStatus::UserCommandRunning) {
				// This can mean three different things:
				// 1) User called readline ()
				// 2) User called browser ()
				// 3) R jumped us back to toplevel behind our backs.
				// Let's find out, which one it is.
				if (hist && (RThread::default_global_context != R_GlobalContext)) {
					break;	// this looks like a call to browser(). Will be handled below.
				}

				int n_frames = 0;
				RCommandProxy *dummy = RThread::this_pointer->runDirectCommand ("sys.nframe()", RCommand::GetIntVector);
				if ((dummy->getDataType () == RData::IntVector) && (dummy->getDataLength () == 1)) {
					n_frames = dummy->getIntVector ()[0];
				}
				// What the ??? Why does this simple version always return 0?
				//int n_frames = RKRSupport::SEXPToInt (RKRSupport::callSimpleFun0 (Rf_install ("sys.nframe"), R_GlobalEnv));
				if (n_frames < 1) {
					// No active frames? This can't be a call to readline(), then, so probably R jumped us back to toplevel, behind our backs.
					// For safety, let's reset and start over.
					RThread::this_pointer->current_command->status |= RCommand::Failed | RCommand::ErrorOther;
					RThread::repl_status.user_command_status = RThread::RKReplStatus::ReplIterationKilled;
					RK_doIntr ();	// to discard the buffer
				} else {
					// A call to readline(). Will be handled below
					break;
				}
			} else if (RThread::repl_status.user_command_status == RThread::RKReplStatus::UserCommandFailed) {
				RThread::this_pointer->current_command->status |= RCommand::Failed | RCommand::ErrorOther;
				RThread::repl_status.user_command_status = RThread::RKReplStatus::NoUserCommand;
				RThread::this_pointer->commandFinished ();
			} else {
				RK_ASSERT (RThread::repl_status.user_command_status == RThread::RKReplStatus::ReplIterationKilled);
				RThread::repl_status.user_command_status = RThread::RKReplStatus::NoUserCommand;
				RThread::this_pointer->commandFinished ();
			}
		}
	}

	// here, we handle readline() calls and such, i.e. not the regular prompt for code
	// browser() also takes us here.
	if (hist && (RThread::default_global_context != R_GlobalContext)) {
		// TODO: give browser() special handling!
		RThread::repl_status.in_browser_context = true;
	}

	RBackendRequest request (true, RBackendRequest::ReadLine);
	request.params["prompt"] = QVariant (prompt);
	request.params["cancelled"] = QVariant (false);

	RThread::this_pointer->handleRequest (&request);
	if (request.params["cancelled"].toBool ()) {
		if (RThread::this_pointer->current_command) RThread::this_pointer->current_command->status |= RCommand::Canceled;
		RK_doIntr();
		// threoretically, the above should have got us out of the loop, but for good measure:
		Rf_error ("cancelled");
		RK_ASSERT (false);	// should not reach this point.
	}

	QByteArray localres = RThread::this_pointer->current_locale_codec->fromUnicode (request.params["result"].toString ());
	// need to append a newline, here. TODO: theoretically, RReadConsole comes back for more, if \0 was encountered before \n.
	qstrncpy ((char *) buf, localres.left (buflen - 2).append ('\n').data (), buflen);
	return 1;
}

#ifdef Q_WS_WIN
int RReadConsoleWin (const char* prompt, char* buf, int buflen, int hist) {
	return RReadConsole (prompt, (unsigned char*) buf, buflen, hist);
}
#endif

void RWriteConsoleEx (const char *buf, int buflen, int type) {
	RK_TRACE (RBACKEND);

	// output while nothing else is running (including handlers?) -> This may be a syntax error.
	if (RThread::repl_status.eval_depth == 0) {
		if (RThread::repl_status.user_command_status == RThread::RKReplStatus::UserCommandTransmitted) {
			// status UserCommandTransmitted might have been set from RKToplevelStatementFinishedHandler, too, in which case all is fine
			// (we're probably inside another task handler at this point, then)
			if (RThread::repl_status.user_command_parsed_up_to < RThread::repl_status.user_command_transmitted_up_to) { 
				RThread::repl_status.user_command_status = RThread::RKReplStatus::UserCommandSyntaxError;
			}
		} else if (RThread::repl_status.user_command_status == RThread::RKReplStatus::ReplIterationKilled) {
			// purge superflous newlines
			if (QString ("\n") == buf) return;
		} else {
			RK_ASSERT (RThread::repl_status.user_command_status != RThread::RKReplStatus::NoUserCommand);
		}
	}

	RThread::this_pointer->handleOutput (RThread::this_pointer->current_locale_codec->toUnicode (buf, buflen), buflen, type == 0 ? ROutput::Output : ROutput::Warning);
}

/** For R callbacks that we want to disable, entirely */
void RDoNothing () {
	//RK_TRACE (RBACKEND);
}

void RCleanUp (SA_TYPE saveact, int status, int RunLast) {
	RK_TRACE (RBACKEND);

	if (saveact != SA_SUICIDE) {
		if (!RThread::this_pointer->isKilled ()) {
			RBackendRequest request (true, RBackendRequest::BackendExit);
			request.params["message"] = QVariant (i18n ("The R engine has shut down with status: %1").arg (status));
			RThread::this_pointer->handleRequest (&request);
		}

		if(saveact == SA_DEFAULT) saveact = SA_SAVE;
		if (saveact == SA_SAVE) {
				if (RunLast) R_dot_Last ();
				if (R_DirtyImage) R_SaveGlobalEnv ();
		} else {
				if (RunLast) R_dot_Last ();
		}
		R_CleanTempDir ();
	}
	RThread::this_pointer->killed = true;
	Rf_error ("Backend dead");	// this jumps us out of the REPL.
}

void RThread::tryToDoEmergencySave () {
	RK_TRACE (RBACKEND);

	// we're probably in a signal handler, and the stack base has changed.
	uintptr_t old_lim = R_CStackLimit;
	R_CStackLimit = (uintptr_t)-1;
	if (R_DirtyImage) R_SaveGlobalEnvToFile (RKCommonFunctions::getUseableRKWardSavefileName ("rkward_recover", ".RData").toLocal8Bit ());
	R_CStackLimit = old_lim;
}

QStringList charPArrayToQStringList (const char** chars, int count) {
	QStringList ret;
	for (int i = 0; i < count; ++i) {
		// do we need to do locale conversion, here?
		ret.append (chars[i]);
	}
	return ret;
}

int RChooseFile (int isnew, char *buf, int len) {
	RK_TRACE (RBACKEND);

	RBackendRequest request (true, RBackendRequest::ChooseFile);
	request.params["new"] = QVariant ((bool) isnew);

	RThread::this_pointer->handleRequest (&request);

	QByteArray localres = RThread::this_pointer->current_locale_codec->fromUnicode (request.params["result"].toString ());
	qstrncpy ((char *) buf, localres.data (), len);

// return length of filename (strlen (buf))
	return (qMin (len - 1, localres.size ()));
}

/* There are about one million possible entry points to editing / showing files. We try to cover them all, using the
following bunch of functions (REditFilesHelper() and doShowEditFiles() are helpers, only) */

void REditFilesHelper (QStringList files, QStringList titles, QString wtitle, RBackendRequest::RCallbackType edit, bool delete_files) {
	RK_TRACE (RBACKEND);

	RK_ASSERT ((edit == RBackendRequest::ShowFiles) || (edit == RBackendRequest::EditFiles));
	RBackendRequest request (edit != RBackendRequest::ShowFiles, edit);		// editing is synchronous, showing is asynchronous
	if (edit == RBackendRequest::ShowFiles) {
		request.params["delete"] = QVariant (delete_files);
	}
	// see ?file.show() for what appears to be the intended meaning of these first three parameters
	// (which seem to be inconsistently named even in R itself...)
	request.params["files"] = QVariant (files);
	request.params["titles"] = QVariant (titles);
	request.params["wtitle"] = QVariant (wtitle);

	RThread::this_pointer->handleRequest (&request);
}

int REditFiles (int nfile, const char **file, const char **title, const char *wtitle) {
	RK_TRACE (RBACKEND);

	REditFilesHelper (charPArrayToQStringList (file, nfile), charPArrayToQStringList (title, nfile), wtitle, RBackendRequest::EditFiles, false);

// default implementation seems to return 1 if nfile <= 0, else 1. No idea, what for. see unix/std-sys.c
	return (nfile <= 0);
}

SEXP doShowEditFiles (SEXP files, SEXP titles, SEXP wtitle, SEXP del, RBackendRequest::RCallbackType edit) {
	RK_TRACE (RBACKEND);

	// this function would be much shorter, if SEXPToStringList would simply return a QStringList...
	unsigned int files_count, titles_count;
	QString *file_strings = RKRSupport::SEXPToStringList (files, &files_count);
	QString *title_strings = RKRSupport::SEXPToStringList (titles, &titles_count);
	QString wtitle_string = RKRSupport::SEXPToString (wtitle);
	bool del_files = RKRSupport::SEXPToInt (del, 0) != 0;

	RK_ASSERT (files_count == titles_count);
	RK_ASSERT (files_count >= 1);

	files_count = titles_count = qMin (files_count, titles_count);

	QStringList files_list;
	QStringList titles_list;
	for (unsigned int i = 0; i < files_count; ++i) {
		files_list.append (file_strings[i]);
		titles_list.append (title_strings[i]);
	}

	REditFilesHelper (files_list, titles_list, wtitle_string, edit, del_files);

	delete [] file_strings;
	delete [] title_strings;

	return (R_NilValue);
}

SEXP doEditFiles (SEXP files, SEXP titles, SEXP wtitle) {
	return (doShowEditFiles (files, titles, wtitle, R_NilValue, RBackendRequest::EditFiles));
}

int REditFile (const char *buf) {
	RK_TRACE (RBACKEND);

	const char *editor = "none";
	const char *title = "";

// does not exist in standard R 2.1.0, so no idea what to return.
	return REditFiles (1, const_cast<const char**> (&buf), &title, editor);
}

SEXP doShowFiles (SEXP files, SEXP titles, SEXP wtitle, SEXP delete_files) {
	return (doShowEditFiles (files, titles, wtitle, delete_files, RBackendRequest::ShowFiles));
}

int RShowFiles (int nfile, const char **file, const char **headers, const char *wtitle, Rboolean del, const char */* pager */) {
	RK_TRACE (RBACKEND);

	REditFilesHelper (charPArrayToQStringList (file, nfile), charPArrayToQStringList (headers, nfile), QString (wtitle), RBackendRequest::ShowFiles, (bool) del);

// default implementation seems to returns 1 on success, 0 on failure. see unix/std-sys.c
	return 1;
}

/* FROM R_ext/RStartup.h: "Return value here is expected to be 1 for Yes, -1 for No and 0 for Cancel:
   symbolic constants in graphapp.h" */
int doDialogHelper (QString caption, QString message, QString button_yes, QString button_no, QString button_cancel, bool wait) {
	RK_TRACE (RBACKEND);

	RBackendRequest request (wait, RBackendRequest::ShowMessage);
	request.params["caption"] = QVariant (caption);
	request.params["message"] = QVariant (message);
	request.params["button_yes"] = QVariant (button_yes);
	request.params["button_no"] = QVariant (button_no);
	request.params["button_cancel"] = QVariant (button_cancel);

	RThread::this_pointer->handleRequest (&request);
 
	if (wait) {
		QString ret = request.params["result"].toString ();
		if (ret == "yes") return 1;
		if (ret == "no") return -1;
	}
	return 0;
}

SEXP doDialog (SEXP caption, SEXP message, SEXP button_yes, SEXP button_no, SEXP button_cancel, SEXP wait) {
	RK_TRACE (RBACKEND);

	int result = doDialogHelper (RKRSupport::SEXPToString (caption), RKRSupport::SEXPToString (message), RKRSupport::SEXPToString (button_yes), RKRSupport::SEXPToString (button_no), RKRSupport::SEXPToString (button_cancel), RKRSupport::SEXPToInt (wait));

	SEXP ret = Rf_allocVector(INTSXP, 1);
	INTEGER (ret)[0] = result;
	return ret;
}

void RShowMessage (const char* message) {
	RK_TRACE (RBACKEND);

	doDialogHelper (i18n ("Message from the R backend"), message, "ok", QString (), QString (), true);
}

// TODO: currently used on windows, only!
int RAskYesNoCancel (const char* message) {
	RK_TRACE (RBACKEND);

	return doDialogHelper (i18n ("Question from the R backend"), message, "yes", "no", "cancel", true);
}

void RBusy (int busy) {
	RK_TRACE (RBACKEND);

	// R_ReplIteration calls R_Busy (1) after reading in code (if needed), successfully parsing it, and right before evaluating it.
	if (busy) {
		if (RThread::repl_status.user_command_status == RThread::RKReplStatus::UserCommandTransmitted) {
			RThread::repl_status.user_command_parsed_up_to = RThread::repl_status.user_command_transmitted_up_to;
			RThread::repl_status.user_command_status = RThread::RKReplStatus::UserCommandRunning;
		}
	}
}

// ############## R Standard callback overrides END ####################

char *RThread::na_char_internal = new char;

RThread::RThread () {
	RK_TRACE (RBACKEND);

	current_locale_codec = QTextCodec::codecForLocale ();
	r_running = false;

	current_command = 0;

	RK_ASSERT (this_pointer == 0);
	this_pointer = this;
	out_buf_len = 0;

#ifdef Q_WS_WIN
	// we hope that on other platforms the default is reasonable
	setStackSize (0xa00000);	// 10MB as recommended by r_exts-manual
#endif
}

#ifdef Q_WS_WIN
void RThread::setupCallbacks () {
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

void RThread::connectCallbacks () {
	RK_TRACE (RBACKEND);
	R_SetParams(&RK_R_Params);
}
#else
void RThread::setupCallbacks () {
	RK_TRACE (RBACKEND);
}

void RThread::connectCallbacks () {
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

RThread::~RThread () {
	RK_TRACE (RBACKEND);
}

#if 0
static int timeout_counter = 0;
#endif

void processX11EventsWorker (void *) {
// this basically copied from R's unix/sys-std.c (Rstd_ReadConsole)
#ifndef Q_WS_WIN
	for (;;) {
		fd_set *what;
		what = R_checkActivityEx(R_wait_usec > 0 ? R_wait_usec : 50, 1, RK_doIntr);
		R_runHandlers(R_InputHandlers, what);
		if (what == NULL) break;
	}
	/* This seems to be needed to make Rcmdr react to events. Has this always been the case? It was commented out for a long time, without anybody noticing. */
	R_PolledEvents ();
#else
#warning TODO: correct?
	R_ProcessEvents();
#endif

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

void RThread::processX11Events () {
	// do not trace
	if (!this_pointer->r_running) return;

	RThread::repl_status.eval_depth++;
// In case an error (or user interrupt) is caught inside processX11EventsWorker, we don't want to long-jump out.
	R_ToplevelExec (processX11EventsWorker, 0);
	RThread::repl_status.eval_depth--;
}

SEXP doError (SEXP call) {
	RK_TRACE (RBACKEND);

	if (RThread::this_pointer->repl_status.eval_depth == 0) {
		RThread::this_pointer->repl_status.user_command_status = RThread::RKReplStatus::UserCommandFailed;
	}
	QString string = RKRSupport::SEXPToString (call);
	RThread::this_pointer->handleOutput (string, string.length (), ROutput::Error);
	RK_DO (qDebug ("error '%s'", qPrintable (string)), RBACKEND, DL_DEBUG);
	return R_NilValue;
}

SEXP doSubstackCall (SEXP call) {
	RK_TRACE (RBACKEND);

	unsigned int count;
	QString *strings = RKRSupport::SEXPToStringList (call, &count);
	QStringList list;
	for (unsigned int i = 0; i < count; ++i) {
		list.append (strings[i]);
	}
	delete [] strings;

	// handle symbol updates inline
	if (list.count () == 2) {		// schedule symbol update for later
		if (list[0] == "ws") {
			// always keep in mind: No current command can happen for tcl/tk events.
			if ((!RThread::this_pointer->current_command) || (RThread::this_pointer->current_command->type & RCommand::ObjectListUpdate) || (!(RThread::this_pointer->current_command->type & RCommand::Sync))) {		// ignore Sync commands that are not flagged as ObjectListUpdate
				if (!RThread::this_pointer->changed_symbol_names.contains (list[1])) RThread::this_pointer->changed_symbol_names.append (list[1]);
			}
			return R_NilValue;
		}
	}

#warning TODO: extend this by sychronity parameter
	RThread::this_pointer->handleHistoricalSubstackRequest (list);

	return R_NilValue;
}

void R_CheckStackWrapper (void *) {
	R_CheckStack ();
}

SEXP doUpdateLocale () {
	RK_TRACE (RBACKEND);

	RK_DO (qDebug ("Changing locale"), RBACKEND, DL_WARNING);
	RThread::this_pointer->current_locale_codec = RKGetCurrentLocaleCodec ();
	RK_DO (qDebug ("New locale codec is %s", RThread::this_pointer->current_locale_codec->name ().data ()), RBACKEND, DL_WARNING);

	return R_NilValue;
}

// returns the MIME-name of the current locale encoding (from Qt)
SEXP doLocaleName () {
	RK_TRACE (RBACKEND);

	RK_ASSERT (RThread::this_pointer->current_locale_codec);
	SEXP res = Rf_allocVector(STRSXP, 1);
	PROTECT (res);
	SET_STRING_ELT (res, 0, Rf_mkChar (RThread::this_pointer->current_locale_codec->name ().data ()));
	UNPROTECT (1);
	return res;
}

#include "rkstructuregetter.cpp"

SEXP doGetStructure (SEXP toplevel, SEXP name, SEXP envlevel, SEXP namespacename) {
	RK_TRACE (RBACKEND);

	RKStructureGetter getter (false);
	RData *ret = getter.getStructure (toplevel, name, envlevel, namespacename);
	return R_MakeExternalPtr (ret, RKWard_RData_Tag, R_NilValue);
}

SEXP doGetGlobalEnvStructure (SEXP name, SEXP envlevel, SEXP namespacename) {
	RK_TRACE (RBACKEND);

	return doGetStructure (Rf_findVar (Rf_install (CHAR (STRING_ELT (name, 0))), R_GlobalEnv), name, envlevel, namespacename);
}

/** copy a symbol without touching it (esp. not forcing any promises) */
SEXP doCopyNoEval (SEXP name, SEXP fromenv, SEXP toenv) {
	RK_TRACE (RBACKEND);

	if(!Rf_isString (name) || Rf_length (name) != 1) Rf_error ("name is not a single string");
	if(!Rf_isEnvironment (fromenv)) Rf_error ("fromenv is not an environment");
	if(!Rf_isEnvironment (toenv)) Rf_error ("toenv is not an environment");
	Rf_defineVar (Rf_install (CHAR (STRING_ELT (name, 0))), Rf_findVar (Rf_install (CHAR (STRING_ELT (name, 0))), fromenv), toenv);
	return (R_NilValue);
}

bool RThread::startR (int argc, char** argv, bool stack_check) {
	RK_TRACE (RBACKEND);

	setupCallbacks ();

	RKSignalSupport::saveDefaultSignalHandlers ();

	r_running = true;
	Rf_initialize_R (argc, argv);

#ifndef Q_WS_WIN
	// in R on windows the stack limits detection seems to work out of the box for threads
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
#endif

#ifdef Q_WS_WIN
	R_set_command_line_arguments(argc, argv);
	FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
#endif

	setup_Rmainloop ();

#ifndef Q_WS_WIN
	// in R on windows the stack limits detection seems to work out of the box for threads
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
#endif

#ifndef Q_WS_WIN
	// on windows, set in connectCallbacks() for technical reasons
	R_Interactive = (Rboolean) TRUE;
#endif

	RKGlobals::na_double = NA_REAL;

	RKWard_RData_Tag = Rf_install ("RKWard_RData_Tag");
	R_LastvalueSymbol = Rf_install (".Last.value");

	RKSignalSupport::installSignalProxies ();

// register our functions
	R_CallMethodDef callMethods [] = {
//		{ "rk.do.condition", (DL_FUNC) &doCondition, 1 },
		{ "rk.do.error", (DL_FUNC) &doError, 1 },
		{ "rk.do.command", (DL_FUNC) &doSubstackCall, 1 },
		{ "rk.get.structure", (DL_FUNC) &doGetStructure, 4 },
		{ "rk.get.structure.global", (DL_FUNC) &doGetGlobalEnvStructure, 3 },
		{ "rk.copy.no.eval", (DL_FUNC) &doCopyNoEval, 3 },
		{ "rk.edit.files", (DL_FUNC) &doEditFiles, 3 },
		{ "rk.show.files", (DL_FUNC) &doShowFiles, 4 },
		{ "rk.dialog", (DL_FUNC) &doDialog, 6 },
		{ "rk.update.locale", (DL_FUNC) &doUpdateLocale, 0 },
		{ "rk.locale.name", (DL_FUNC) &doLocaleName, 0 },
		{ 0, 0, 0 }
	};
	R_registerRoutines (R_getEmbeddingDllInfo(), NULL, callMethods, NULL, NULL);

	connectCallbacks();
	RKInsertToplevelStatementFinishedCallback (0);
	default_global_context = R_GlobalContext;

	// get info on R runtime version
	RCommandProxy *dummy = runDirectCommand ("as.numeric (R.version$major) * 1000 + as.numeric (R.version$minor) * 10", RCommand::GetIntVector);
	if ((dummy->getDataType () == RData::IntVector) && (dummy->getDataLength () == 1)) {
		r_version = dummy->getIntVector ()[0];
	} else {
		RK_ASSERT (false);
		r_version = 0;
	}

	return true;
}

void RThread::enterEventLoop () {
	RK_TRACE (RBACKEND);

	run_Rmainloop ();
	Rf_endEmbeddedR (0);
}

SEXP parseCommand (const QString &command_qstring, RThread::RKWardRError *error) {
	RK_TRACE (RBACKEND);

	ParseStatus status = PARSE_NULL;
	SEXP cv, pr;

	QByteArray localc = RThread::this_pointer->current_locale_codec->fromUnicode (command_qstring);		// needed so the string below does not go out of scope
	const char *command = localc.data ();

	PROTECT(cv=Rf_allocVector(STRSXP, 1));
	SET_STRING_ELT(cv, 0, Rf_mkChar(command));

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
			*error = RThread::Incomplete;
		} else if (status == PARSE_ERROR) {
			//extern SEXP parseError (SEXP call, int linenum);
			//parseError (R_NilValue, 0);
			*error = RThread::SyntaxError;
		} else { // PARSE_NULL
			*error = RThread::OtherError;
		}
		pr = R_NilValue;
	}

	return pr;
}

SEXP runCommandInternalBase (SEXP pr, RThread::RKWardRError *error) {
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
		*error = RThread::OtherError;
	} else {
		*error = RThread::NoError;
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

bool RThread::runDirectCommand (const QString &command) {
	RK_TRACE (RBACKEND);

	RCommandProxy c (command, RCommand::App | RCommand::Sync | RCommand::Internal);
	runCommand (&c);
	return ((c.status & RCommand::WasTried) && !(c.status & RCommand::Failed));
}

RCommandProxy *RThread::runDirectCommand (const QString &command, RCommand::CommandTypes datatype) {
	RK_TRACE (RBACKEND);
	RK_ASSERT ((datatype >= RCommand::GetIntVector) && (datatype <= RCommand::GetStructuredData));

	RCommandProxy *c = new RCommandProxy (command, RCommand::App | RCommand::Sync | RCommand::Internal | datatype);
	runCommand (c);
	return c;
}

void RThread::runCommand (RCommandProxy *command) {
	RK_TRACE (RBACKEND);
	RK_ASSERT (command);

	RKWardRError error = NoError;

	int ctype = command->type;	// easier typing
	RData retdata;

	// running user commands is quite different from all other commands and should have been handled by RReadConsole
	RK_ASSERT (!(ctype & RCommand::User));

	if (ctype & RCommand::DirectToOutput) runDirectCommand (".rk.capture.messages()");

	if (ctype & RCommand::QuitCommand) {
		killed = true;
	} else if (!(ctype & RCommand::EmptyCommand)) {
		repl_status.eval_depth++;
		SEXP parsed = parseCommand (command->command, &error);
		if (error == NoError) {
			SEXP exp;
			PROTECT (exp = runCommandInternalBase (parsed, &error));
			if (error == NoError) {
				if (ctype & RCommand::GetStringVector) {
					retdata.datatype = RData::StringVector;
					retdata.data = RKRSupport::SEXPToStringList (exp, &(retdata.length));
				} else if (ctype & RCommand::GetRealVector) {
					retdata.datatype = RData::RealVector;
					retdata.data = RKRSupport::SEXPToRealArray (exp, &(retdata.length));
				} else if (ctype & RCommand::GetIntVector) {
					retdata.datatype = RData::IntVector;
					retdata.data = RKRSupport::SEXPToIntArray (exp, &(retdata.length));
				} else if (ctype & RCommand::GetStructuredData) {
					RData *dummy = RKRSupport::SEXPToRData (exp);
					retdata.setData (*dummy);
					delete dummy;
				}
			}
			UNPROTECT (1); // exp
		}
		repl_status.eval_depth--;
	}

	if (ctype & RCommand::DirectToOutput) runDirectCommand (".rk.print.captured.messages()");
	if (!(ctype & RCommand::Internal)) {
		if (!RInterface::backendIsLocked () || killed) processX11Events ();
	}

	command->setData (retdata);
	// common error/status handling
	if (error != NoError) {
		command->status |= RCommand::WasTried | RCommand::Failed;
		if (error == Incomplete) {
			command->status |= RCommand::ErrorIncomplete;
		} else if (error == SyntaxError) {
			command->status |= RCommand::ErrorSyntax;
		} else if (!(command->status & RCommand::Canceled)) {
			command->status |= RCommand::ErrorOther;
		}
	} else {
		command->status |= RCommand::WasTried;
	}
}
