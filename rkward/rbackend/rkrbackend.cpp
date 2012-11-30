/***************************************************************************
                          rkrbackend  -  description
                             -------------------
    begin                : Sun Jul 25 2004
    copyright            : (C) 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011 by Thomas Friedrichsmeier
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

#include "rkrbackend.h"

#ifdef Q_WS_WIN
#	include <windows.h>
#	include <winsock2.h>
#	include <locale.h>
#	undef ERROR 	// clashes with R
#	define Win32	// needed for R includes
#else
#	include <dlfcn.h>
#	include <sys/resource.h>
#	include <sys/types.h>
#endif

// statics
RKRBackend *RKRBackend::this_pointer = 0;
RKRBackend::RKReplStatus RKRBackend::repl_status = { QByteArray (), 0, true, 0, 0, RKRBackend::RKReplStatus::NoUserCommand, 0, RKRBackend::RKReplStatus::NotInBrowserContext, false };
void* RKRBackend::default_global_context = 0;

#include <qstring.h>
#include <QStringList>
#include <QThread>
#include <QDir>
#include <qtextcodec.h>

#include "../core/robject.h"
#include "../version.h"
#include "../debug.h"

#include "rkrsupport.h"
#include "rkstructuregetter.h"
#include "rklocalesupport.h"
#include "rkpthreadsupport.h"
#include "rksignalsupport.h"
#include "../misc/rkcommonfunctions.h"

#include <stdlib.h>
#include <string.h>
#include <stdint.h>

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

#include <Rdefines.h>
#include <R_ext/Rdynload.h>
#include <R_ext/eventloop.h>
#include <R_ext/Callbacks.h>
#include <R.h>
#include <Rversion.h>
#include <Rinternals.h>
#include <R_ext/Parse.h>
#include <Rembedded.h>

#ifdef Q_WS_WIN
#	include <R_ext/RStartup.h>
#	include <R_ext/Utils.h>

	structRstart RK_R_Params;

	// why oh why isn't Rinterface.h available on Windows?
	LibExtern void* R_GlobalContext;
	LibExtern uintptr_t R_CStackLimit;
	LibExtern void R_SaveGlobalEnvToFile(char*);
#else
#	include <Rinterface.h>
#endif

#ifndef Q_WS_WIN
#	include <signal.h>		// needed for pthread_kill
#	include <pthread.h>		// seems to be needed at least on FreeBSD
#	include <unistd.h>		// for non-blocking pipes
#	include <fcntl.h>
#endif

///// i18n
#if (defined ENABLE_NLS) && (!defined RKWARD_THREADED)		// ENABLE_NLS is from Rconfig.h
#	include <libintl.h>
#	define RK_MSG_DOMAIN "rkward"
	// define i18n to be compatible with the easiest usage in KDE
#	define i18n(msgid) QString::fromUtf8(dgettext(RK_MSG_DOMAIN, msgid))
	void RK_setupGettext (const char* locale_dir) {
		bindtextdomain (RK_MSG_DOMAIN, locale_dir);
		bind_textdomain_codeset (RK_MSG_DOMAIN, "UTF-8");
	}
#else
#	define i18n(msgid) QString(msgid)
	void RK_setupGettext (const char*) {};
#endif


///// interrupting R
void RK_scheduleIntr () {
	RK_DEBUG (RBACKEND, DL_DEBUG, "interrupt scheduled");
	RKRBackend::repl_status.interrupted = true;
#ifdef Q_WS_WIN
	UserBreak = 1;
#else
	RKSignalSupport::callOldSigIntHandler ();
#endif
}

void RK_doIntr () {
	RK_scheduleIntr ();
	R_CheckUserInterrupt ();
}

void RKRBackend::scheduleInterrupt () {
	if (RKRBackendProtocolBackend::inRThread ()) {
		RK_scheduleIntr ();
	} else {
#ifdef Q_WS_WIN
		RK_scheduleIntr ();		// Thread-safe on windows?!
#else
		pthread_kill ((pthread_t) RKRBackendProtocolBackend::instance ()->r_thread_id, SIGUSR1);	// NOTE: SIGUSR1 relays to SIGINT
#endif
	}
}

void RKRBackend::interruptCommand (int command_id) {
	RK_TRACE (RBACKEND);
	QMutexLocker lock (&all_current_commands_mutex);

	if (all_current_commands.isEmpty ()) return;
	if ((command_id == -1) || (all_current_commands.last ()->id == command_id)) {
		if (!too_late_to_interrupt) {
			RK_DEBUG (RBACKEND, DL_DEBUG, "scheduling interrupt for command id %d", command_id);
			scheduleInterrupt ();
		}
	} else {
		// if the command to cancel is *not* the topmost command, then do not interrupt, yet.
		foreach (RCommandProxy *candidate, all_current_commands) {
			if (candidate->id == command_id) {
				if (!current_commands_to_cancel.contains (candidate)) {
					RK_DEBUG (RBACKEND, DL_DEBUG, "scheduling delayed interrupt for command id %d", command_id);
					current_commands_to_cancel.append (candidate);
				}
			}
		}
	}
}

void clearPendingInterrupt_Worker (void *) {
	R_CheckUserInterrupt ();
}

void RKRBackend::clearPendingInterrupt () {
	RK_TRACE (RBACKEND);
	bool passed = R_ToplevelExec (clearPendingInterrupt_Worker, 0);
	if (!passed) RK_DEBUG (RBACKEND, DL_DEBUG, "pending interrupt cleared");
}

// some functions we need that are not declared
LibExtern void Rf_PrintWarnings (void);
LibExtern void run_Rmainloop (void);
#include <R_ext/eventloop.h>
}

#include "rdata.h"

extern SEXP RKWard_RData_Tag;
SEXP parseCommand (const QString &command_qstring, RKRBackend::RKWardRError *error);
SEXP runCommandInternalBase (SEXP pr, RKRBackend::RKWardRError *error);

// ############## R Standard callback overrides BEGIN ####################
Rboolean RKToplevelStatementFinishedCallback (SEXP expr, SEXP value, Rboolean succeeded, Rboolean visible, void *) {
	RK_TRACE (RBACKEND);
	Q_UNUSED (expr);
	Q_UNUSED (value);
	Q_UNUSED (visible);

	if ((RKRBackend::repl_status.eval_depth == 0) && (!RKRBackend::repl_status.browser_context)) {		// Yes, toplevel-handlers _do_ get called in a browser context!
		RK_ASSERT (RKRBackend::repl_status.user_command_status = RKRBackend::RKReplStatus::UserCommandRunning);
		if (succeeded) {
			RKRBackend::repl_status.user_command_successful_up_to = RKRBackend::repl_status.user_command_parsed_up_to;
			if (RKRBackend::repl_status.user_command_completely_transmitted) {
				RKRBackend::repl_status.user_command_status = RKRBackend::RKReplStatus::NoUserCommand;
				RKRBackend::this_pointer->commandFinished ();
			} else RKRBackend::repl_status.user_command_status = RKRBackend::RKReplStatus::UserCommandTransmitted;
		} else {
			// well, this point of code is never reached with R up to 2.12.0. Instead failed user commands are handled in doError().
			RKRBackend::repl_status.user_command_status = RKRBackend::RKReplStatus::UserCommandFailed;
		}
	}
	
	return (Rboolean) true;
}

void RKInsertToplevelStatementFinishedCallback (void *) {
	RK_TRACE (RBACKEND);

	if (RKRBackend::this_pointer->r_running) {
		int pos;
		Rf_addTaskCallback (&RKToplevelStatementFinishedCallback, 0, &RKInsertToplevelStatementFinishedCallback, "_rkward_main_callback", &pos);
	}
}

void RKTransmitNextUserCommandChunk (unsigned char* buf, int buflen) {
	RK_TRACE (RBACKEND);

	RK_ASSERT (RKRBackend::repl_status.user_command_transmitted_up_to <= RKRBackend::repl_status.user_command_buffer.length ());	// NOTE: QByteArray::length () does not count the trailing '\0'
	const char* current_buffer = RKRBackend::repl_status.user_command_buffer.data ();
	current_buffer += RKRBackend::repl_status.user_command_transmitted_up_to;	// Skip what we have already transmitted

	bool reached_eof = false;
	int pos = 0;
	const int max_pos = buflen - 2;	// one for the termination
	bool reached_newline = false;
	while (true) {
		buf[pos] = *current_buffer;
		if (*current_buffer == '\n') {
			reached_newline = true;
			break;
		} else if (*current_buffer == ';') break;
		else if (*current_buffer == '\0') {
			reached_eof = true;
			break;
		}
		if (pos >= max_pos) break;
		++current_buffer;
		++pos;
	}
	RKRBackend::repl_status.user_command_transmitted_up_to += (pos + 1);
	if (reached_eof) {
		buf[pos] = '\n';
		RKRBackend::repl_status.user_command_completely_transmitted = true;
	}
	buf[++pos] = '\0';

	if (reached_newline || reached_eof) {
		// Making this request synchronous is a bit painful. However, without this, it's extremely difficult to get correct interleaving of output and command lines
		RBackendRequest req (true, RBackendRequest::CommandLineIn);
		req.params["commandid"] = RKRBackend::this_pointer->current_command->id;
		RKRBackend::this_pointer->handleRequest (&req);
	}
}

int RReadConsole (const char* prompt, unsigned char* buf, int buflen, int hist) {
	RK_TRACE (RBACKEND);

	RK_ASSERT (buf && buflen);
	RK_ASSERT (RKRBackend::repl_status.eval_depth >= 0);

	if (RKRBackend::repl_status.browser_context) {		// previously we were in a browser context. Check, whether we've left that.
		if (RKRBackend::default_global_context == R_GlobalContext) {
			RKRBackend::repl_status.browser_context = RKRBackend::RKReplStatus::NotInBrowserContext;
			RKRBackend::this_pointer->handlePlainGenericRequest (QStringList ("endBrowserContext"), false);
		}
	}
	
	if ((!RKRBackend::repl_status.browser_context) && (RKRBackend::repl_status.eval_depth == 0)) {
		while (1) {
			if (RKRBackend::repl_status.user_command_status == RKRBackend::RKReplStatus::NoUserCommand) {
				RCommandProxy *command = RKRBackend::this_pointer->fetchNextCommand ();
				if (!command) {
#ifdef Q_OS_WINDOWS
					// Can't easily override R_CleanUp on Windows, so we're calling it manually, here, then force exit
					if (RKRBackend::this_pointer->killed == RKRBackend::ExitNow) RCleanUp (SA_NOSAVE, 0, 0);
					else RCleanUp (SA_SUICIDE, 1, 0);
					exit (0);
#endif
					return 0;	// jumps out of the event loop!
				}

				if (!(command->type & RCommand::User)) {
					RKRBackend::this_pointer->runCommand (command);
					RKRBackend::this_pointer->commandFinished ();
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
					RKRBackend::repl_status.user_command_transmitted_up_to = 0;
					RKRBackend::repl_status.user_command_completely_transmitted = false;
					RKRBackend::repl_status.user_command_parsed_up_to = 0;
					RKRBackend::repl_status.user_command_successful_up_to = 0;
					RKRBackend::repl_status.user_command_buffer = RKRBackend::this_pointer->current_locale_codec->fromUnicode (command->command);
					RKTransmitNextUserCommandChunk (buf, buflen);
					RKRBackend::repl_status.user_command_status = RKRBackend::RKReplStatus::UserCommandTransmitted;
					return 1;
				}
			} else if (RKRBackend::repl_status.user_command_status == RKRBackend::RKReplStatus::UserCommandTransmitted) {
				if (RKRBackend::repl_status.user_command_completely_transmitted) {
					// fully transmitted, but R is still asking for more? This looks like an incomplete statement.
					// HOWEVER: It may also have been an empty statement such as " ", so let's check whether the prompt looks like a "continue" prompt
					bool incomplete = false;
					if (RKRBackend::this_pointer->current_locale_codec->toUnicode (prompt) == RKRSupport::SEXPToString (Rf_GetOption (Rf_install ("continue"), R_BaseEnv))) {
						incomplete = true;
					}
					if (incomplete) RKRBackend::this_pointer->current_command->status |= RCommand::Failed | RCommand::ErrorIncomplete;
					RKRBackend::repl_status.user_command_status = RKRBackend::RKReplStatus::ReplIterationKilled;
					if (RKRBackend::RKRBackend::repl_status.user_command_parsed_up_to <= 0) RKRBackend::this_pointer->startOutputCapture ();	// HACK: No capture active, but commandFinished() will try to end one
					Rf_error ("");	// to discard the buffer
				} else {
					RKTransmitNextUserCommandChunk (buf, buflen);
					return 1;
				}
			} else if (RKRBackend::repl_status.user_command_status == RKRBackend::RKReplStatus::UserCommandSyntaxError) {
				RKRBackend::this_pointer->current_command->status |= RCommand::Failed | RCommand::ErrorSyntax;
				RKRBackend::repl_status.user_command_status = RKRBackend::RKReplStatus::NoUserCommand;
				if (RKRBackend::RKRBackend::repl_status.user_command_parsed_up_to <= 0) RKRBackend::this_pointer->startOutputCapture ();	// HACK: No capture active, but commandFinished() will try to end one
				RKRBackend::this_pointer->commandFinished ();
			} else if (RKRBackend::repl_status.user_command_status == RKRBackend::RKReplStatus::UserCommandRunning) {
				// This can mean three different things:
				// 1) User called readline ()
				// 2) User called browser ()
				// 3) R jumped us back to toplevel behind our backs.
				// Let's find out, which one it is.
				if (hist && (RKRBackend::default_global_context != R_GlobalContext)) {
					break;	// this looks like a call to browser(). Will be handled below.
				}

				int n_frames = 0;
				RCommandProxy *dummy = RKRBackend::this_pointer->runDirectCommand ("sys.nframe()", RCommand::GetIntVector);
				if ((dummy->getDataType () == RData::IntVector) && (dummy->getDataLength () == 1)) {
					n_frames = dummy->intVector ().at (0);
				}
				// What the ??? Why does this simple version always return 0?
				//int n_frames = RKRSupport::SEXPToInt (RKRSupport::callSimpleFun0 (Rf_install ("sys.nframe"), R_GlobalEnv));
				if (n_frames < 1) {
					// No active frames? This can't be a call to readline(), then, so probably R jumped us back to toplevel, behind our backs.
					// For safety, let's reset and start over.
					RKRBackend::this_pointer->current_command->status |= RCommand::Failed | RCommand::ErrorOther;
					RKRBackend::repl_status.user_command_status = RKRBackend::RKReplStatus::ReplIterationKilled;
					Rf_error("");	// to discard the buffer
				} else {
					// A call to readline(). Will be handled below
					break;
				}
			} else if (RKRBackend::repl_status.user_command_status == RKRBackend::RKReplStatus::UserCommandFailed) {
				RKRBackend::this_pointer->current_command->status |= RCommand::Failed | RCommand::ErrorOther;
				RKRBackend::repl_status.user_command_status = RKRBackend::RKReplStatus::NoUserCommand;
				RKRBackend::this_pointer->commandFinished ();
			} else {
				RK_ASSERT (RKRBackend::repl_status.user_command_status == RKRBackend::RKReplStatus::ReplIterationKilled);
				RKRBackend::repl_status.user_command_status = RKRBackend::RKReplStatus::NoUserCommand;
				RKRBackend::this_pointer->commandFinished ();
			}
		}
	}

	// here, we handle readline() calls and such, i.e. not the regular prompt for code
	// browser() also takes us here.
	QVariantMap params;
	RBackendRequest::RCallbackType request_type = RBackendRequest::ReadLine;
	params["prompt"] = QVariant (prompt);
	params["cancelled"] = QVariant (false);

	// add info for browser requests
	if (hist && (RKRBackend::default_global_context != R_GlobalContext)) {
		if (RKRBackend::repl_status.browser_context == RKRBackend::RKReplStatus::InBrowserContextPreventRecursion) {
			qstrncpy ((char *) buf, "n\n", buflen);	// skip this, by feeding the browser() a continue
			return 1;
		} else {
			RKRBackend::repl_status.browser_context = RKRBackend::RKReplStatus::InBrowserContextPreventRecursion;
			RCommandProxy *dummy = RKRBackend::this_pointer->runDirectCommand (".rk.callstack.info()", RCommand::GetStructuredData);

			request_type = RBackendRequest::Debugger;
			if ((dummy->getDataType () == RData::StructureVector) && (dummy->getDataLength () >= 4)) {
				RData::RDataStorage dummy_data = dummy->structureVector ();
				params["calls"] = QVariant (dummy_data.at (0)->stringVector ());
				params["funs"] = QVariant (dummy_data.at (1)->stringVector ());
				params["envs"] = QVariant (dummy_data.at (2)->stringVector ());
				params["locals"] = QVariant (dummy_data.at (3)->stringVector ());
				params["relsrclines"] = QVariant (dummy_data.at (4)->stringVector ());		// hacky: passing a QList<int> is not supported by QVariant
			} else {
				RK_ASSERT (false);
			}

			RKRBackend::repl_status.browser_context = RKRBackend::RKReplStatus::InBrowserContext;
		}

		RK_ASSERT (RKRBackend::repl_status.browser_context == RKRBackend::RKReplStatus::InBrowserContext);
	}

	RBackendRequest request (true, request_type);
	request.params = params;

	RKRBackend::this_pointer->handleRequest (&request);
	if (request.params["cancelled"].toBool ()) {
		if (RKRBackend::this_pointer->current_command) RKRBackend::this_pointer->current_command->status |= RCommand::Canceled;
		Rf_error ("cancelled");
		RK_ASSERT (false);	// should not reach this point.
	}

	QByteArray localres = RKRBackend::this_pointer->current_locale_codec->fromUnicode (request.params["result"].toString ());
	// need to append a newline, here. TODO: theoretically, RReadConsole comes back for more, if \0 was encountered before \n.
	qstrncpy ((char *) buf, localres.left (buflen - 2).append ('\n').data (), buflen);
	return 1;
}

#ifdef Q_WS_WIN
int RReadConsoleWin (const char* prompt, char* buf, int buflen, int hist) {
	return RReadConsole (prompt, (unsigned char*) buf, buflen, hist);
}
#endif

bool RKRBackend::fetchStdoutStderr (bool forcibly) {
#ifndef Q_OS_WIN
	if (!forcibly) {
		if (!stdout_stderr_mutex.tryLock ()) return false;
	} else {
		stdout_stderr_mutex.lock ();
	}

	// it seems, setting this only once is not always enough.
	fcntl (stdout_stderr_fd, F_SETFL, fcntl (stdout_stderr_fd, F_GETFL, 0) | O_NONBLOCK);
	char buffer[1024];
	while (true) {
		int bytes = read (stdout_stderr_fd, buffer, 1023);
		if (bytes <= 0) break;
		buffer[bytes] = '\0';
		// NOTE: we must not risk blocking inside handleOutput, while the stdout_stderr_mutex is locked!
		handleOutput (current_locale_codec->toUnicode (buffer, bytes), bytes, ROutput::Warning, false);
	}

	stdout_stderr_mutex.unlock ();
#endif
	return true;
}

void RWriteConsoleEx (const char *buf, int buflen, int type) {
	RK_TRACE (RBACKEND);

	// output while nothing else is running (including handlers?) -> This may be a syntax error.
	if ((RKRBackend::repl_status.eval_depth == 0) && (!RKRBackend::repl_status.browser_context) && (!RKRBackend::this_pointer->isKilled ())) {
		if (RKRBackend::repl_status.user_command_status == RKRBackend::RKReplStatus::UserCommandTransmitted) {
			// status UserCommandTransmitted might have been set from RKToplevelStatementFinishedHandler, too, in which case all is fine
			// (we're probably inside another task handler at this point, then)
			if (RKRBackend::repl_status.user_command_parsed_up_to < RKRBackend::repl_status.user_command_transmitted_up_to) { 
				RKRBackend::repl_status.user_command_status = RKRBackend::RKReplStatus::UserCommandSyntaxError;
			}
		} else if (RKRBackend::repl_status.user_command_status == RKRBackend::RKReplStatus::ReplIterationKilled) {
			// purge superflous newlines and empty output
			return;
		} else {
			RK_ASSERT (RKRBackend::repl_status.user_command_status != RKRBackend::RKReplStatus::NoUserCommand);
		}
	}

	if (RKRBackend::this_pointer->killed == RKRBackend::AlreadyDead) return;	// this check is mostly for fork()ed clients
	if (RKRBackend::repl_status.browser_context == RKRBackend::RKReplStatus::InBrowserContextPreventRecursion) return;
	RKRBackend::this_pointer->fetchStdoutStderr (true);
	RKRBackend::this_pointer->handleOutput (RKRBackend::this_pointer->current_locale_codec->toUnicode (buf, buflen), buflen, type == 0 ? ROutput::Output : ROutput::Warning);
}

/** For R callbacks that we want to disable, entirely */
void RDoNothing () {
	//RK_TRACE (RBACKEND);
}

void RCleanUp (SA_TYPE saveact, int status, int RunLast) {
	RK_TRACE (RBACKEND);
	Q_UNUSED (RunLast);		// R_dot_Last is called while "running" the QuitCommand

	if (RKRBackend::this_pointer->killed == RKRBackend::AlreadyDead) return;	// Nothing to clean up
	if (!RKRBackend::this_pointer->r_running) return;			// prevent recursion (if an error occurs, here, we get jumped to the console repl, again!)
	R_CheckUserInterrupt ();	// if there are any user interrupts pendinging, we want them handled *NOW*
	RKRBackend::this_pointer->r_running = false;

	// we could be in a signal handler, and the stack base may have changed.
	uintptr_t old_lim = R_CStackLimit;
	R_CStackLimit = (uintptr_t)-1;

	if ((status != 0) && (RKRBackend::this_pointer->killed != RKRBackend::ExitNow)) RKRBackend::this_pointer->killed = RKRBackend::EmergencySaveThenExit;

	if (RKRBackend::this_pointer->killed == RKRBackend::EmergencySaveThenExit) {
		if (R_DirtyImage) {
			QString filename;
			QDir dir (RKRBackendProtocolBackend::dataDir ());
			int i=0;
			while (true) {
				filename = "rkward_recover" + QString::number (i) + ".RData";
				if (!dir.exists (filename)) break;
				i++;
			}
			filename = dir.absoluteFilePath (filename);

			R_SaveGlobalEnvToFile (filename.toLocal8Bit ().data ());
			qDebug ("Created emergency save file in %s", qPrintable (filename));
		} else {
			qDebug ("Image not dirty while crashing. No emergency save created.");
		}
	}

	if (saveact != SA_SUICIDE) {
		if (!RKRBackend::this_pointer->isKilled ()) {
			RBackendRequest request (true, RBackendRequest::BackendExit);
			request.params["message"] = QVariant (i18n ("The R engine has shut down with status: %1").arg (status));
			RKRBackend::this_pointer->handleRequest (&request);
		}

		R_RunExitFinalizers ();
		Rf_KillAllDevices ();
		R_CleanTempDir ();
	}

	RKRBackend::this_pointer->killed = RKRBackend::AlreadyDead;	// just in case

	R_CStackLimit = old_lim;	// well, it should not matter any longer, but...
}

void RSuicide (const char* message) {
	RK_TRACE (RBACKEND);

	if (!RKRBackend::this_pointer->isKilled ()) {
		RBackendRequest request (true, RBackendRequest::BackendExit);
		request.params["message"] = QVariant (i18n ("The R engine has encountered a fatal error:\n%1").arg (message));
		RKRBackend::this_pointer->handleRequest (&request);
		RKRBackend::this_pointer->killed = RKRBackend::EmergencySaveThenExit;
		RCleanUp (SA_SUICIDE, 1, 0);
	} else {
		RK_ASSERT (false);
	}
}

void RKRBackend::tryToDoEmergencySave () {
	RK_TRACE (RBACKEND);

	if (RKRBackendProtocolBackend::inRThread ()) {
		// If we are in the correct thread, things are easy:
		RKRBackend::this_pointer->killed = RKRBackend::EmergencySaveThenExit;
		RCleanUp (SA_SUICIDE, 1, 0);
		RK_doIntr ();	// to jump out of the loop, if needed
	} else {
		// If we are in the wrong thread, things are a lot more tricky. We need to cause the R thread to exit, and wait for it to finish saving.
		// Fortunately, if we are in the wrong thread, that probably means, the R thread did *not* crash, and will thus still be functional
		this_pointer->killed = EmergencySaveThenExit;
		RK_scheduleIntr ();
		for (int i = 0; i < 100; ++i) {		// give it up to ten seconds to intterrupt and exit the loop
			if (!this_pointer->r_running) break;
			RKRBackendProtocolBackend::msleep (100);
		}
		if (!this_pointer->r_running) {
			for (int i = 0; i < 600; ++i) {		// give it up to sixty seconds to finish saving
				if (this_pointer->killed == AlreadyDead) return;	// finished
				RKRBackendProtocolBackend::msleep (100);
			}
		}
		RK_ASSERT (false);	// Too bad, but we seem to be stuck. No chance but to return (and crash)
	}
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

	RKRBackend::this_pointer->handleRequest (&request);

	QByteArray localres = RKRBackend::this_pointer->current_locale_codec->fromUnicode (request.params["result"].toString ());
	qstrncpy ((char *) buf, localres.data (), len);

// return length of filename (strlen (buf))
	return (qMin (len - 1, localres.size ()));
}

/* There are about one million possible entry points to editing / showing files. We try to cover them all, using the
following bunch of functions (REditFilesHelper() and doShowEditFiles() are helpers, only) */

void REditFilesHelper (QStringList files, QStringList titles, QString wtitle, RBackendRequest::RCallbackType edit, bool delete_files, bool prompt) {
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
	request.params["prompt"] = QVariant (prompt);

	RKRBackend::this_pointer->handleRequest (&request);
}

int REditFiles (int nfile, const char **file, const char **title, const char *wtitle) {
	RK_TRACE (RBACKEND);

	REditFilesHelper (charPArrayToQStringList (file, nfile), charPArrayToQStringList (title, nfile), wtitle, RBackendRequest::EditFiles, false, true);

// default implementation seems to return 1 if nfile <= 0, else 1. No idea, what for. see unix/std-sys.c
	return (nfile <= 0);
}

SEXP doShowEditFiles (SEXP files, SEXP titles, SEXP wtitle, SEXP del, SEXP prompt, RBackendRequest::RCallbackType edit) {
	RK_TRACE (RBACKEND);

	QStringList file_strings = RKRSupport::SEXPToStringList (files);
	QStringList title_strings = RKRSupport::SEXPToStringList (titles);
	QString wtitle_string = RKRSupport::SEXPToString (wtitle);
	bool del_files = RKRSupport::SEXPToInt (del, 0) != 0;
	bool do_prompt = RKRSupport::SEXPToInt (prompt, 0) != 0;

	RK_ASSERT (file_strings.size () == title_strings.size ());
	RK_ASSERT (file_strings.size () >= 1);

	REditFilesHelper (file_strings, title_strings, wtitle_string, edit, del_files, do_prompt);

	return (R_NilValue);
}

SEXP doEditFiles (SEXP files, SEXP titles, SEXP wtitle, SEXP prompt) {
	return (doShowEditFiles (files, titles, wtitle, R_NilValue, prompt, RBackendRequest::EditFiles));
}

int REditFile (const char *buf) {
	RK_TRACE (RBACKEND);

	const char *editor = "none";
	const char *title = "";

// does not exist in standard R 2.1.0, so no idea what to return.
	return REditFiles (1, const_cast<const char**> (&buf), &title, editor);
}

SEXP doShowFiles (SEXP files, SEXP titles, SEXP wtitle, SEXP delete_files, SEXP prompt) {
	return (doShowEditFiles (files, titles, wtitle, delete_files, prompt, RBackendRequest::ShowFiles));
}

int RShowFiles (int nfile, const char **file, const char **headers, const char *wtitle, Rboolean del, const char */* pager */) {
	RK_TRACE (RBACKEND);

	REditFilesHelper (charPArrayToQStringList (file, nfile), charPArrayToQStringList (headers, nfile), QString (wtitle), RBackendRequest::ShowFiles, (bool) del, true);

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

	RKRBackend::this_pointer->handleRequest (&request);
 
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

	if (RKRBackend::this_pointer->killed) return -1;	// HACK: At this point R asks whether to save the workspace. We have already handled that. So return -1 for "no"
	return doDialogHelper (i18n ("Question from the R backend"), message, "yes", "no", "cancel", true);
}

void RBusy (int busy) {
	RK_TRACE (RBACKEND);

	// R_ReplIteration calls R_Busy (1) after reading in code (if needed), successfully parsing it, and right before evaluating it.
	if (busy) {
		if (RKRBackend::repl_status.user_command_status == RKRBackend::RKReplStatus::UserCommandTransmitted) {
			if (RKRBackend::this_pointer->current_command->type & RCommand::CCCommand) {
				QByteArray chunk = RKRBackend::repl_status.user_command_buffer.mid (RKRBackend::repl_status.user_command_parsed_up_to, RKRBackend::repl_status.user_command_transmitted_up_to - RKRBackend::repl_status.user_command_parsed_up_to);
				RKRBackend::this_pointer->printCommand (RKRBackend::this_pointer->current_locale_codec->toUnicode (chunk));
			}
			if (RKRBackend::this_pointer->current_command->type & RCommand::CCOutput) {
				// flush any previous output caputre and start a new one
				if (RKRBackend::repl_status.user_command_successful_up_to > 0) RKRBackend::this_pointer->printAndClearCapturedMessages (false);
				RKRBackend::this_pointer->startOutputCapture ();
			}
			RKRBackend::repl_status.user_command_parsed_up_to = RKRBackend::repl_status.user_command_transmitted_up_to;
			RKRBackend::repl_status.user_command_status = RKRBackend::RKReplStatus::UserCommandRunning;
		}
	}
}

// ############## R Standard callback overrides END ####################

// NOTE: stdout_stderr_mutex is recursive to support fork()s, better
RKRBackend::RKRBackend () : stdout_stderr_mutex (QMutex::Recursive) {
	RK_TRACE (RBACKEND);

	current_locale_codec = QTextCodec::codecForLocale ();
	r_running = false;

	current_command = 0;

	RK_ASSERT (this_pointer == 0);
	this_pointer = this;
}

#ifdef Q_WS_WIN
void RKRBackend::setupCallbacks () {
	RK_TRACE (RBACKEND);

	R_setStartTime();
	R_DefParams(&RK_R_Params);

// IMPORTANT: see also the #ifndef QS_WS_WIN-portion!
	RK_R_Params.rhome = get_R_HOME ();
	RK_R_Params.home = getRUser ();
	RK_R_Params.CharacterMode = RGui;
	RK_R_Params.ShowMessage = RShowMessage;
	RK_R_Params.ReadConsole = RReadConsoleWin;
	RK_R_Params.WriteConsoleEx = RWriteConsoleEx;
	RK_R_Params.WriteConsole = 0;
	RK_R_Params.CallBack = RDoNothing;
	RK_R_Params.YesNoCancel = RAskYesNoCancel;
	RK_R_Params.Busy = RBusy;

	// TODO: callback mechanism(s) for ChosseFile, ShowFiles, EditFiles
	// TODO: also for RSuicide (Less important, obviously, since this should not be triggered, in normal operation).
	// NOTE: For RCleanUp see RReadConsole RCleanup? 

	RK_R_Params.R_Quiet = (Rboolean) 0;
	RK_R_Params.R_Interactive = (Rboolean) 1;
}

void RKRBackend::connectCallbacks () {
	RK_TRACE (RBACKEND);
	R_SetParams(&RK_R_Params);
}
#else
void RKRBackend::setupCallbacks () {
	RK_TRACE (RBACKEND);
}

void RKRBackend::connectCallbacks () {
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

RKRBackend::~RKRBackend () {
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

void RKRBackend::processX11Events () {
	// do not trace
	if (!this_pointer->r_running) return;
	if (this_pointer->isKilled ()) return;

	RKRBackend::repl_status.eval_depth++;
// In case an error (or user interrupt) is caught inside processX11EventsWorker, we don't want to long-jump out.
	R_ToplevelExec (processX11EventsWorker, 0);
	RKRBackend::repl_status.eval_depth--;
}

LibExtern int R_interrupts_pending;
SEXP doError (SEXP call) {
	RK_TRACE (RBACKEND);

	if ((RKRBackend::repl_status.eval_depth == 0) && (!RKRBackend::repl_status.browser_context) && (!RKRBackend::this_pointer->isKilled ()) && (RKRBackend::repl_status.user_command_status != RKRBackend::RKReplStatus::ReplIterationKilled)) {
		RKRBackend::repl_status.user_command_status = RKRBackend::RKReplStatus::UserCommandFailed;
	}
	if (RKRBackend::repl_status.interrupted) {
		// it is unlikely, but possible, that an interrupt signal was received, but the current command failed for some other reason, before processing was acutally interrupted. In this case, R_interrupts_pending if not yet cleared.
		// NOTE: if R_interrupts_pending stops being exported one day, we might be able to use R_CheckUserInterrupt() inside an R_ToplevelExec() to find out, whether an interrupt was still pending.
		if (!R_interrupts_pending) {
			RKRBackend::repl_status.interrupted = false;
			if (RKRBackend::repl_status.user_command_status != RKRBackend::RKReplStatus::ReplIterationKilled) {	// was interrupted only to step out of the repl iteration
				QMutexLocker lock (&(RKRBackend::this_pointer->all_current_commands_mutex));
				foreach (RCommandProxy *command, RKRBackend::this_pointer->all_current_commands) command->status |= RCommand::Canceled;
				RK_DEBUG (RBACKEND, DL_DEBUG, "interrupted");
			}
		}
	} else if (RKRBackend::repl_status.user_command_status != RKRBackend::RKReplStatus::ReplIterationKilled) {
		QString string = RKRSupport::SEXPToString (call);
		RKRBackend::this_pointer->handleOutput (string, string.length (), ROutput::Error);
		RK_DEBUG (RBACKEND, DL_DEBUG, "error '%s'", qPrintable (string));
	}
	return R_NilValue;
}

SEXP doSubstackCall (SEXP call) {
	RK_TRACE (RBACKEND);

	R_CheckUserInterrupt ();

	QStringList list = RKRSupport::SEXPToStringList (call);

	// handle symbol updates inline
	if (list.count () == 2) {		// schedule symbol update for later
		if (list[0] == "ws") {
			// always keep in mind: No current command can happen for tcl/tk events.
			if ((!RKRBackend::this_pointer->current_command) || (RKRBackend::this_pointer->current_command->type & RCommand::ObjectListUpdate) || (!(RKRBackend::this_pointer->current_command->type & RCommand::Sync))) {		// ignore Sync commands that are not flagged as ObjectListUpdate
				if (!RKRBackend::this_pointer->changed_symbol_names.contains (list[1])) RKRBackend::this_pointer->changed_symbol_names.append (list[1]);
			}
			return R_NilValue;
		}
	}

	RKRBackend::this_pointer->handleHistoricalSubstackRequest (list);

	return R_NilValue;
}

SEXP doPlainGenericRequest (SEXP call, SEXP synchronous) {
	RK_TRACE (RBACKEND);

	R_CheckUserInterrupt ();

	QStringList ret = RKRBackend::this_pointer->handlePlainGenericRequest (RKRSupport::SEXPToStringList (call), RKRSupport::SEXPToInt (synchronous));
	return RKRSupport::StringListToSEXP (ret);
}

void R_CheckStackWrapper (void *) {
	R_CheckStack ();
}

SEXP doUpdateLocale () {
	RK_TRACE (RBACKEND);

	RK_DEBUG (RBACKEND, DL_WARNING, "Changing locale");
	RKRBackend::this_pointer->current_locale_codec = RKGetCurrentLocaleCodec ();
	RK_DEBUG (RBACKEND, DL_WARNING, "New locale codec is %s", RKRBackend::this_pointer->current_locale_codec->name ().data ());

	return R_NilValue;
}

// returns the MIME-name of the current locale encoding (from Qt)
SEXP doLocaleName () {
	RK_TRACE (RBACKEND);

	RK_ASSERT (RKRBackend::this_pointer->current_locale_codec);
	SEXP res = Rf_allocVector(STRSXP, 1);
	PROTECT (res);
	SET_STRING_ELT (res, 0, Rf_mkChar (RKRBackend::this_pointer->current_locale_codec->name ().data ()));
	UNPROTECT (1);
	return res;
}

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

bool RKRBackend::startR () {
	RK_TRACE (RBACKEND);

	setupCallbacks ();

	RKSignalSupport::saveDefaultSignalHandlers ();

	too_late_to_interrupt = false;
	r_running = true;
	int argc = 3;
	char* argv[3] = { qstrdup ("--slave"), qstrdup ("--no-save"), qstrdup ("--no-restore") };
	Rf_initialize_R (argc, argv);

	// in R on windows the stack limits detection seems to work out of the box for threads
#ifndef Q_WS_WIN
	{
		char dummy;
		size_t stacksize;
		void *stackstart;
		RKGetCurrentThreadStackLimits (&stacksize, &stackstart, &dummy);
		R_CStackStart = (uintptr_t) stackstart;
		R_CStackLimit = stacksize;
	}
#endif

#ifdef Q_WS_WIN
	R_set_command_line_arguments(argc, argv);
	FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
#endif

#ifndef Q_OS_WIN
	// re-direct stdout / stderr to a pipe, so we can read output from system() calls
	int pfd[2];
	pipe (pfd);
	dup2 (pfd[1], STDOUT_FILENO);
	dup2 (pfd[1], STDERR_FILENO);		// forward both to a single channel to avoid interleaving hell, for now.
	close (pfd[1]);
	stdout_stderr_fd = pfd[0];
#endif

	setup_Rmainloop ();

#ifndef Q_WS_WIN
	// safety check: If we are beyond the stack boundaries already, we better disable stack checking
	// this has to come *after* the first setup_Rmainloop ()!
	Rboolean stack_ok = R_ToplevelExec (R_CheckStackWrapper, (void *) 0);
	if (!stack_ok) {
		RK_DEBUG (RBACKEND, DL_WARNING, "R_CheckStack() failed during initialization. Will disable stack checking and try to re-initialize.");
		RK_DEBUG (RBACKEND, DL_WARNING, "Whether or not things work after this, *please* submit a bug report.");
		R_CStackStart = (uintptr_t) -1;
		R_CStackLimit = (uintptr_t) -1;
		setup_Rmainloop ();
	}
#endif

#ifndef Q_WS_WIN
	// on windows, set in connectCallbacks() for technical reasons
	R_Interactive = (Rboolean) TRUE;
#else
	setlocale (LC_NUMERIC, "C");	// something appears to mess with the locale on Windows. R will not work correctly without LC_NUMERIC=C
#endif

	RBackendRequest req (false, RBackendRequest::SetParamsFromBackend);
	req.params["na_real"] = NA_REAL;	// may not be initialized before setup_Rmainloop!
	req.params["na_int"] = NA_INTEGER;
	handleRequest (&req);

	RKWard_RData_Tag = Rf_install ("RKWard_RData_Tag");

	RKSignalSupport::installSignalProxies ();	// for the crash signals
	RKSignalSupport::installSigIntAndUsrHandlers (RK_scheduleIntr);

// register our functions
	R_CallMethodDef callMethods [] = {
		{ "rk.do.error", (DL_FUNC) &doError, 1 },
		{ "rk.do.command", (DL_FUNC) &doSubstackCall, 1 },
		{ "rk.do.generic.request", (DL_FUNC) &doPlainGenericRequest, 2 },
		{ "rk.get.structure", (DL_FUNC) &doGetStructure, 4 },
		{ "rk.get.structure.global", (DL_FUNC) &doGetGlobalEnvStructure, 3 },
		{ "rk.copy.no.eval", (DL_FUNC) &doCopyNoEval, 3 },
		{ "rk.edit.files", (DL_FUNC) &doEditFiles, 4 },
		{ "rk.show.files", (DL_FUNC) &doShowFiles, 5 },
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
		r_version = dummy->intVector ().at (0);
	} else {
		RK_ASSERT (false);
		r_version = 0;
	}

	return true;
}

#ifndef Q_OS_WIN
static bool backend_was_forked = false;
void prepareFork () {
	RK_TRACE (RBACKEND);
	if (!RKRBackendProtocolBackend::inRThread ()) return;

	// we need to make sure that the transmitter thread does not hold a lock on the mutex!
	RKRBackend::this_pointer->stdout_stderr_mutex.lock ();
}

void completeForkMaster () {
	RK_TRACE (RBACKEND);
	if (!RKRBackendProtocolBackend::inRThread ()) return;

	RKRBackend::this_pointer->stdout_stderr_mutex.unlock ();

	if (backend_was_forked) return;
	backend_was_forked = true;

	// Block SIGCHLD in the main thread from now on. I don't fully understand, why, but otherwise, these signals
	// interrupt the select() call in the fork()ing code of library(parallel)
	sigset_t new_set;
	sigemptyset (&new_set);
	sigaddset (&new_set, SIGCHLD);
	pthread_sigmask (SIG_BLOCK, &new_set, NULL);

//	This was used to show a warning message. Unfortunately, however, forks also occur on every popen (i.e. in system(..., intern=TRUE).
//	RKRBackend::this_pointer->handlePlainGenericRequest (QStringList ("forkNotification"), false);
	RK_DEBUG (RBACKEND, DL_WARNING, "Backend process forked (for the first time, this session)");
//	NOTE: perhaps we can heuristically differentiate from popen by checking sys.calls() for something with "fork" in it. 
//	esp., in case we discover adverse side-effects of blocking SIGCHLD, we should attempt this
}

void completeForkChild () {
	RKRBackendProtocolBackend::instance ()->r_thread_id = QThread::currentThreadId();
	RKRBackend::this_pointer->killed = RKRBackend::AlreadyDead;	// Not quite accurate, but disables all communication with the frontend
}
#endif

void RKRBackend::enterEventLoop () {
	RK_TRACE (RBACKEND);

#ifndef Q_OS_WIN
	pthread_atfork (prepareFork, completeForkMaster, completeForkChild);
#endif

	run_Rmainloop ();
	// NOTE: Do NOT run Rf_endEmbeddedR(). It does more that we want. We rely on RCleanup, instead.
}

SEXP parseCommand (const QString &command_qstring, RKRBackend::RKWardRError *error) {
	RK_TRACE (RBACKEND);

	ParseStatus status = PARSE_NULL;
	SEXP cv, pr;

	QByteArray localc = RKRBackend::this_pointer->current_locale_codec->fromUnicode (command_qstring);		// needed so the string below does not go out of scope
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
			*error = RKRBackend::Incomplete;
		} else if (status == PARSE_ERROR) {
			//extern SEXP parseError (SEXP call, int linenum);
			//parseError (R_NilValue, 0);
			*error = RKRBackend::SyntaxError;
		} else { // PARSE_NULL
			*error = RKRBackend::OtherError;
		}
		pr = R_NilValue;
	}

	return pr;
}

SEXP runCommandInternalBase (SEXP pr, RKRBackend::RKWardRError *error) {
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
		*error = RKRBackend::OtherError;
	} else {
		*error = RKRBackend::NoError;
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

bool RKRBackend::runDirectCommand (const QString &command) {
	RK_TRACE (RBACKEND);

	RCommandProxy c (command, RCommand::App | RCommand::Sync | RCommand::Internal);
	runCommand (&c);
	return ((c.status & RCommand::WasTried) && !(c.status & RCommand::Failed));
}

RCommandProxy *RKRBackend::runDirectCommand (const QString &command, RCommand::CommandTypes datatype) {
	RK_TRACE (RBACKEND);
	RK_ASSERT ((datatype >= RCommand::GetIntVector) && (datatype <= RCommand::GetStructuredData));

	RCommandProxy *c = new RCommandProxy (command, RCommand::App | RCommand::Sync | RCommand::Internal | datatype);
	runCommand (c);
	return c;
}

void RKRBackend::runCommand (RCommandProxy *command) {
	RK_TRACE (RBACKEND);
	RK_ASSERT (command);

	RKWardRError error = NoError;

	int ctype = command->type;	// easier typing

	// running user commands is quite different from all other commands and should have been handled by RReadConsole
	RK_ASSERT (!(ctype & RCommand::User));

	if (ctype & RCommand::CCCommand) printCommand (command->command);
	if (ctype & RCommand::CCOutput) startOutputCapture ();

	if (ctype & RCommand::QuitCommand) {
		R_dot_Last ();		// should run while communication with frontend is still possible
		RBackendRequest req (true, RBackendRequest::BackendExit);
		req.params["regular"] = QVariant (true);
		handleRequest (&req);
		killed = ExitNow;
	} else if (!(ctype & RCommand::EmptyCommand)) {
		repl_status.eval_depth++;
		SEXP parsed = parseCommand (command->command, &error);
		if (error == NoError) {
			SEXP exp;
			PROTECT (exp = runCommandInternalBase (parsed, &error));
			if (error == NoError) {
				if (ctype & RCommand::GetStringVector) {
					command->setData (RKRSupport::SEXPToStringList (exp));
				} else if (ctype & RCommand::GetRealVector) {
					command->setData (RKRSupport::SEXPToRealArray (exp));
				} else if (ctype & RCommand::GetIntVector) {
					command->setData (RKRSupport::SEXPToIntArray (exp));
				} else if (ctype & RCommand::GetStructuredData) {
					RData *dummy = RKRSupport::SEXPToRData (exp);
					command->swallowData (*dummy);
					delete dummy;
				}
			}
			UNPROTECT (1); // exp
		}
		repl_status.eval_depth--;
	}

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

// On Windows, using runDirectCommand (".rk.cat.output ...") is not safe during some places where we call this, e.g. in RBusy.
// Not a problem on Linux with R 2.13.0, though
void RKRBackend::catToOutputFile (const QString &out) {
	RK_TRACE (RBACKEND);

	if (output_file.isEmpty ()) {
		RK_ASSERT (false);
		return;
	}
	QFile f (output_file);
	if (!f.open (QIODevice::WriteOnly | QIODevice::Append)) {
		RK_ASSERT (false);
		return;
	}
	f.write (current_locale_codec->fromUnicode (out));
	f.close ();
}

void RKRBackend::printCommand (const QString &command) {
	RK_TRACE (RBACKEND);

	QStringList params ("highlightRCode");
	params.append (command);
	QString highlighted = handlePlainGenericRequest (params, true).value (0);
	catToOutputFile (highlighted);
}

void RKRBackend::startOutputCapture () {
	RK_TRACE (RBACKEND);

	// TODO: One of those days, we need to revisit output handling. This request could be perfectly async, but unfortunately, in that case, output chunks can sneak in front of it.
	handlePlainGenericRequest (QStringList ("recordOutput"), true);
}

void RKRBackend::printAndClearCapturedMessages (bool with_header) {
	RK_TRACE (RBACKEND);

	QStringList params ("recordOutput");
	params.append ("end");
	QString out = handlePlainGenericRequest (params, true).value (0);

	if (out.isEmpty ()) return;
	if (with_header) out.prepend ("<h2>Messages, warnings, or errors:</h2>\n");
	catToOutputFile (out);
}

void RKRBackend::run (const QString &locale_dir) {
	RK_TRACE (RBACKEND);
	killed = NotKilled;
	previous_command = 0;

	initialize (QFile::encodeName (locale_dir));

	enterEventLoop ();
}

void RKRBackend::commandFinished (bool check_object_updates_needed) {
	RK_TRACE (RBACKEND);
	RK_DEBUG (RBACKEND, DL_DEBUG, "done running command");

	{
		QMutexLocker lock (&all_current_commands_mutex);
		too_late_to_interrupt = true;
	}
	clearPendingInterrupt ();	// Mutex must be unlocked for this!

	if (current_command->type & RCommand::CCOutput) printAndClearCapturedMessages (current_command->type & RCommand::Plugin);
	current_command->status -= (current_command->status & RCommand::Running);
	current_command->status |= RCommand::WasTried;

	if (current_command->type & RCommand::User) {
		RK_ASSERT (repl_status.eval_depth == 0);

		// This method may look a bit over-complex, but remember that repl_status.user_command_successful_up_to works on an *encoded* buffer
		QByteArray remainder_encoded = repl_status.user_command_buffer.mid (repl_status.user_command_successful_up_to);
		QString remainder = current_locale_codec->toUnicode (remainder_encoded);
		current_command->has_been_run_up_to = current_command->command.length () - remainder.length ();
	}

	if (check_object_updates_needed || (current_command->type & RCommand::ObjectListUpdate)) {
		checkObjectUpdatesNeeded (current_command->type & (RCommand::User | RCommand::ObjectListUpdate));
	}

	previous_command = current_command;

	{
		QMutexLocker lock (&all_current_commands_mutex);
		all_current_commands.pop_back();
		if (!all_current_commands.isEmpty ()) current_command = all_current_commands.last ();
		too_late_to_interrupt = false;
	}
}

RCommandProxy* RKRBackend::handleRequest (RBackendRequest *request, bool mayHandleSubstack) {
	RK_TRACE (RBACKEND);
	RK_ASSERT (request);

	RKRBackendProtocolBackend::instance ()->sendRequest (request);

	if ((!request->synchronous) && (!isKilled ())) {
		RK_ASSERT (mayHandleSubstack);	// i.e. not called from fetchNextCommand
		return 0;
	}

	int i = 0;
	while (!request->done) {
		if (killed) return 0;
		// NOTE: processX11Events() may, conceivably, lead to new requests, which may also wait for sub-commands!
		processX11Events ();
		// NOTE: sleeping and waking up again can be surprisingly CPU-intensive (yes: more than the event processing, above. I have profiled it).
		// However, we really don't want to introduce too much delay, either.
		// Thus, the logic is this: If there was no reply within 2 seconds, then probably we're waiting for a user event, and can afford some more
		// latency (not too much, though, as we still need to process events).
		if (!request->done) RKRBackendProtocolBackend::msleep (++i < 200 ? 10 : 50);
	}

	RCommandProxy* command = request->takeCommand ();
	if (!command) return 0;

	{
		QMutexLocker lock (&all_current_commands_mutex);
		all_current_commands.append (command);
		current_command = command;
	}

	if (!mayHandleSubstack) return command;
	
	while (command) {
		runCommand (command);
		commandFinished (false);

		command = fetchNextCommand ();
	};

	{
		QMutexLocker lock (&all_current_commands_mutex);
		if (current_commands_to_cancel.contains (current_command)) {
			RK_DEBUG (RBACKEND, DL_DEBUG, "will now interrupt parent command");
			current_commands_to_cancel.removeAll (current_command);
			scheduleInterrupt ();
		}
	}

	return 0;
}

RCommandProxy* RKRBackend::fetchNextCommand () {
	RK_TRACE (RBACKEND);

	RBackendRequest req (!isKilled (), RBackendRequest::CommandOut);	// when killed, we do *not* actually wait for the reply, before the request is deleted.
	req.command = previous_command;
	previous_command = 0;

	return (handleRequest (&req, false));
}

void RKRBackend::handleHistoricalSubstackRequest (const QStringList &list) {
	RK_TRACE (RBACKEND);

	RBackendRequest request (true, RBackendRequest::HistoricalSubstackRequest);
	request.params["call"] = list;
	handleRequest (&request);
}

QStringList RKRBackend::handlePlainGenericRequest (const QStringList &parameters, bool synchronous) {
	RK_TRACE (RBACKEND);

	RBackendRequest request (synchronous, RBackendRequest::PlainGenericRequest);
	if (parameters.value (0) == "getSessionInfo") {
		QStringList dummy = parameters;
		dummy.append (RKRBackendProtocolBackend::backendDebugFile ());
		dummy.append (R_MAJOR "." R_MINOR " " R_STATUS " (" R_YEAR "-" R_MONTH "-" R_DAY " r" R_SVN_REVISION ")");
		request.params["call"] = dummy;
	} else if (parameters.value (0) == "set.output.file") {
		output_file = parameters.value (1);
		if (parameters.length () > 2) {
			RK_ASSERT (parameters.value (2) == "SILENT");
			return QStringList ();		// For automated testing. The frontend should not be notified, here
		}
		request.params["call"] = parameters;
	} else {
		request.params["call"] = parameters;
	}
	handleRequest (&request);
	return request.params.value ("return").toStringList ();
}

void RKRBackend::initialize (const char *locale_dir) {
	RK_TRACE (RBACKEND);

	// in RInterface::RInterface() we have created a fake RCommand to capture all the output/errors during startup. Fetch it
	repl_status.eval_depth++;
	fetchNextCommand ();

	startR ();

	bool lib_load_fail = false;
	bool sink_fail = false;
	if (!runDirectCommand ("library (\"rkward\")\n")) lib_load_fail = true;
	RK_setupGettext (locale_dir);	// must happen *after* package loading, since R will re-set it
	if (!runDirectCommand (QString ("stopifnot(.rk.app.version==\"%1\")\n").arg (RKWARD_VERSION))) lib_load_fail = true;
	if (!runDirectCommand (".rk.fix.assignments ()\n")) sink_fail = true;

// error/output sink and help browser
	if (!runDirectCommand ("options (error=quote (.rk.do.error ()))\n")) sink_fail = true;

	QString error_messages;
	if (lib_load_fail) {
		error_messages.append (i18n ("</p>\t- The 'rkward' R-library either could not be loaded at all, or not in the correct version. This may lead to all sorts of errors, from single missing features to complete failure to function. The most likely cause is that the last installation did not place all files in the correct place. However, in some cases, left-overs from a previous installation that was not cleanly removed may be the cause.</p>\
		<p><b>You should quit RKWard, now, and fix your installation</b>. For help with that, see <a href=\"http://p.sf.net/rkward/compiling\">http://p.sf.net/rkward/compiling</a>.</p>\n"));
	}
	if (sink_fail) {
		error_messages.append (i18n ("<p>\t-There was a problem setting up the communication with R. Most likely this indicates a broken installation.</p>\
		<p><b>You should quit RKWard, now, and fix your installation</b>. For help with that, see <a href=\"http://p.sf.net/rkward/compiling\">http://p.sf.net/rkward/compiling</a>.</p></p>\n"));
	}

	RBackendRequest req (true, RBackendRequest::Started);
	req.params["message"] = QVariant (error_messages);
	// blocks until RKWard is set to go (esp, it has displayed startup error messages, etc.)
	// in fact, a number of sub-commands are run while handling this request!
	handleRequest (&req);

	commandFinished ();		// the fake startup command
	repl_status.eval_depth--;
}

void RKRBackend::checkObjectUpdatesNeeded (bool check_list) {
	RK_TRACE (RBACKEND);
	if (killed) return;

	/* NOTE: We're keeping separate lists of the items on the search path, and the toplevel symbols in .GlobalEnv here.
	This info is also present in RObjectList (and it's children). However: a) in a less convenient form, b) in the other thread. To avoid locking, and other complexity, keeping separate lists seems an ok solution. Keep in mind that only the names of only the toplevel objects are kept, here, so the memory overhead should be minimal */

	bool search_update_needed = false;
	bool globalenv_update_needed = false;

	if (check_list) {	
	// TODO: avoid parsing this over and over again
		RK_DEBUG (RBACKEND, DL_TRACE, "checkObjectUpdatesNeeded: getting search list");
		RCommandProxy *dummy = runDirectCommand ("search ()\n", RCommand::GetStringVector);
		if (dummy->stringVector () != toplevel_env_names) search_update_needed = true;
		if (search_update_needed) toplevel_env_names = dummy->stringVector ();
		delete dummy;
	
	// TODO: avoid parsing this over and over again
		RK_DEBUG (RBACKEND, DL_TRACE, "checkObjectUpdatesNeeded: getting globalenv symbols");
		dummy = runDirectCommand ("ls (globalenv (), all.names=TRUE)\n", RCommand::GetStringVector);
		QStringList new_globalenv_toplevel_names = dummy->stringVector ();
		if (new_globalenv_toplevel_names.count () != global_env_toplevel_names.count ()) {
			globalenv_update_needed = true;
		} else {
			for (int i = 0; i < new_globalenv_toplevel_names.count (); ++i) {
				// order is not important in the symbol list
				if (!global_env_toplevel_names.contains (new_globalenv_toplevel_names[i])) {
					globalenv_update_needed = true;
					break;
				}
			}
		}
		if (globalenv_update_needed) global_env_toplevel_names = new_globalenv_toplevel_names;
		delete dummy;
	
		if (search_update_needed) {	// this includes an update of the globalenv, even if not needed
			QStringList call = toplevel_env_names;
			call.prepend ("syncenvs");	// should be faster than the reverse
			handleHistoricalSubstackRequest (call);
		} 
		if (globalenv_update_needed) {
			QStringList call = global_env_toplevel_names;
			call.prepend ("syncglobal");	// should be faster than the reverse
			handleHistoricalSubstackRequest (call);
		}
	}

	if (search_update_needed || globalenv_update_needed) {
		RK_DEBUG (RBACKEND, DL_TRACE, "checkObjectUpdatesNeeded: updating watches");
		runDirectCommand (".rk.watch.globalenv ()\n");
	}

	if (!changed_symbol_names.isEmpty ()) {
		QStringList call = changed_symbol_names;
		call.prepend (QString ("sync"));	// should be faster than reverse
		handleHistoricalSubstackRequest (call);
		changed_symbol_names.clear ();
	}
}

bool RKRBackend::doMSleep (int msecs) {
	// do not trace!
	if (isKilled ()) return false;
	RKRBackendProtocolBackend::msleep (msecs);
	return true;
}

