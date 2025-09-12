/*
rkrbackend - This file is part of RKWard (https://rkward.kde.org). Created: Sun Jul 25 2004
SPDX-FileCopyrightText: 2004-2025 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "rkrbackend.h"

#ifdef Q_OS_WIN
#	include <winsock2.h>
#	include <windows.h>
#	undef ERROR  // clashes with R
#	define Win32 // needed for R includes
#else
#	include <dlfcn.h>
#	include <sys/resource.h>
#	include <sys/types.h>
#endif

// statics
RKRBackend *RKRBackend::this_pointer = nullptr;
RKRBackend::RKReplStatus RKRBackend::repl_status = {QByteArray(), 0, true, 0, 0, RKRBackend::RKReplStatus::NoUserCommand, 0, RKRBackend::RKReplStatus::NotInBrowserContext, false};
void *RKRBackend::default_global_context = nullptr;

#include <QDir>
#include <QString>
#include <QStringList>
#include <QThread>

#include "../core/robject.h"
#include "../debug.h"
#include "../misc/rkcommonfunctions.h"
#include "../version.h"
#include "rkreventloop.h"
#include "rkrsupport.h"
#include "rksignalsupport.h"
#include "rkstructuregetter.h"

#include <locale.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "rkrapi.h"

#ifdef Q_OS_WIN
structRstart RK_R_Params;
#endif

#ifndef Q_OS_WIN
#	include <signal.h>  // needed for pthread_kill
#	include <pthread.h> // seems to be needed at least on FreeBSD
#	include <unistd.h>  // for non-blocking pipes
#	include <fcntl.h>
#endif

///// i18n
#include <KLocalizedString>
#define RK_MSG_DOMAIN "rkward"
static void RK_setupGettext(const QString &locale_dir) {
	KLocalizedString::setApplicationDomain(RK_MSG_DOMAIN);
	if (!locale_dir.isEmpty()) {
		KLocalizedString::addDomainLocaleDir(RK_MSG_DOMAIN, locale_dir);
	}
}

///// interrupting R
/// schedule an interrupt to happen at the next R_CheckUserInterrupt() (i.e. inside R code)
void RK_scheduleIntr() {
	// NOTE: signal handler code. Must not produce debug output, here (could cause mutex deadlocK)
	RKRBackend::repl_status.interrupted = true;
	RKRBackend::this_pointer->awaiting_sigint = false;
#ifdef Q_OS_WIN
	ROb(UserBreak) = 1;
#else
	RKSignalSupport::callOldSigIntHandler();
#endif
}

/// force an immediate intterupt (unless R_interrupts_suspended
void RK_doIntr() {
	RK_scheduleIntr();
	RFn::R_CheckUserInterrupt();
}

/// thread-safe function to trigger an R interrupt, as if Ctrl+C had been hit in the R console
void RKRBackend::scheduleInterrupt() {
	if (RKRBackendProtocolBackend::inRThread()) {
		RK_scheduleIntr();
	} else {
		RK_DEBUG(RBACKEND, DL_DEBUG, "Interrupt scheduled from outside R thread");
#ifdef Q_OS_WIN
		RK_scheduleIntr(); // Thread-safe on windows?!
#else
		pthread_kill((pthread_t)RKRBackendProtocolBackend::instance()->r_thread_id, SIGUSR1); // NOTE: SIGUSR1 relays to SIGINT
#endif
	}
}

void RKRBackend::interruptCommand(int command_id) {
	RK_TRACE(RBACKEND);
	RK_ASSERT(command_id >= 0); // -1 would signify broken request
	RK_ASSERT(!RKRBackendProtocolBackend::inRThread());
	RK_DEBUG(RBACKEND, DL_DEBUG, "Received interrupt request for command id %d", command_id);
	QMutexLocker lock(&command_flow_mutex);

	/** A request to interrupt a command may happen at very different points in the code:
	 *  0) before it was even sent from the frontend -> trivially handled in the frontend, backend not involved
	 *  1) after it was sent from the frontend, but before we properly started handling it in the backend (deserializing the request, pushing it onto
	 *     all_current_commands, feeding it into the REPL/eval) -> handled in beginAllowInterruptCommand()
	 *  2) when it is properly running in the backend -> this is the typical case, we worry about, "immediate interrupt", below
	 *  2b) when it is properly running in the backend, but there is also an active sub-command, which we should allow to complete
	 *      -> see commands_to_cancel_deferred and handleDeferredInterrupts()
	 *  3) after it has finished running in the backend, but the frontend wasn't aware of that, yet TODO: fix resource leak
	 *
	 * The procedure to interrupt the current command (case 2, above), is also quite complex. Some sources of complication:
	 * 1) Requests to interrupt will originate outside the R thread.
	 * 2) We *have to* relay interruption with signals, otherwise most system calls inside R will not be interrupted (on Unix).
	 *    Note that use of any mutex is essentially a no-go in signal handler code.
	 * 3) Once the signal has arrived, R will essentially set a flag, and trigger an error at the next R_CheckUserInterrupt().
	 *    When exactly that gets hit is hard to tell, and when it does it will do the usual longjmp error handling.
	 * 4) We also need to do some pre- and post-processing around running commands, which we want to run, even in case of an interrupt
	 * 5) The usual painful dichotomy between "user commands" and command going through Rf_eval
	 *
	 * The strategy then is to keep track of what portion of a command may be interrupted (begin/endAllowInterruptCommand()), and make
	 * sure all steps between 1) and 3) are fully handled inisde such blocks. I.e. when we do send a signal, we need to keep track,
	 * whether it has actually arrived and got handled, before leaving endAllowInterruptCommand(). */
	if (current_command && (current_command->id == command_id) && (current_command->interruptible_stage)) {
		RK_DEBUG(RBACKEND, DL_DEBUG, "scheduling immediate interrupt for command id %d", command_id);
		awaiting_sigint = true;
		scheduleInterrupt();
	} else {
		RK_DEBUG(RBACKEND, DL_DEBUG, "scheduling delayed interrupt for command id %d", command_id);
		commands_to_cancel_deferred.append(command_id);
	}
}

void RKRBackend::beginAllowInterruptCommand(RCommandProxy *command) {
	if (command->type & RCommand::Internal) return;
	RK_TRACE(RBACKEND);

	bool intr = false;
	{
		QMutexLocker m(&command_flow_mutex);
		RK_ASSERT(!command->interruptible_stage);
		command->interruptible_stage = true;
		// see handleDeferredInterrupts(), but avoiding one mutex lock/unlock
		intr = commands_to_cancel_deferred.removeAll(command->id);
	}
	if (intr) {
		RK_DEBUG(RBACKEND, DL_DEBUG, "triggering deferred interrupt for command %d", command->id);
		// do not call while mutex is locked!
		RK_doIntr();
	}
}

void RKRBackend::endAllowInterruptCommand(RCommandProxy *command) {
	if (command->type & RCommand::Internal) return;
	RK_TRACE(RBACKEND);

	bool wait_interrupt;
	{
		QMutexLocker m(&command_flow_mutex);
		RK_ASSERT(command->interruptible_stage);
		command->interruptible_stage = false;
		wait_interrupt = awaiting_sigint;
	}
	// wait for any pending interrupt to happen
	while (wait_interrupt) {
		// do not call while mutex is locked!
		RFn::R_CheckUserInterrupt();
		RKRBackendProtocolBackend::msleep(10); // TODO: timeout after a while? In case interrupts are blocked, or something?
		{
			QMutexLocker m(&command_flow_mutex);
			wait_interrupt = awaiting_sigint;
		}
	}
}

void RKRBackend::handleDeferredInterrupts() {
	RK_TRACE(RBACKEND);
	bool intr = false;
	{
		QMutexLocker m(&command_flow_mutex);
		if (current_command && current_command->interruptible_stage) {
			intr = commands_to_cancel_deferred.removeAll(current_command->id);
		}
	}
	if (intr) {
		RK_DEBUG(RBACKEND, DL_DEBUG, "triggering deferred interrupt for command %d", current_command->id);
		// do not call while mutex is locked!
		RK_doIntr();
	}
}

static void markLastWarningAsErrorMessage() {
	const auto msg = u"error:"_s; // "error:" is just a placeholder. The desired effect is to promote latest "Warning" output to Error-level (in RInterface)
	RKRBackend::this_pointer->handleOutput(msg, msg.length(), ROutput::Error);
}

#include "rdata.h"

extern SEXP RKWard_RData_Tag;

// ############## R Standard callback overrides BEGIN ####################
static void RKTransmitNextUserCommandChunk(unsigned char *buf, int buflen) {
	RK_TRACE(RBACKEND);

	RK_ASSERT(RKRBackend::repl_status.user_command_transmitted_up_to <= RKRBackend::repl_status.user_command_buffer.length()); // NOTE: QByteArray::length () does not count the trailing '\0'
	const char *current_buffer = RKRBackend::repl_status.user_command_buffer.data();
	current_buffer += RKRBackend::repl_status.user_command_transmitted_up_to; // Skip what we have already transmitted

	bool reached_eof = false;
	int pos = 0;
	const int max_pos = buflen - 2; // one for the terming newline
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
	RKRBackend::repl_status.user_command_status = RKRBackend::RKReplStatus::UserCommandTransmitted;
	RK_DEBUG(RBACKEND, DL_DEBUG, "transmitted chunk %s", buf);

	if (reached_newline || reached_eof) {
		// feed a marker into the output stream that a source line of the command shall be interleaved into the output, at this point (if running in the console)
		RKRBackend::this_pointer->fetchStdoutStderr(true);
		// NOTE: Output literal is not used, but 0-length "output" would be discarded
		RKRBackend::this_pointer->handleOutput(u"\n"_s, 1, ROutput::CommandLineIn, false);
	}
}

void RBusy(int);

static void replCommandFinished() {
	RKRBackend::repl_status.user_command_status = RKRBackend::RKReplStatus::NoUserCommand;
	RKRBackend::this_pointer->commandFinished();
	RKRBackend::this_pointer->handleDeferredInterrupts();
}

int RReadConsole(const char *prompt, unsigned char *buf, int buflen, int hist) {
	RK_TRACE(RBACKEND);

	RK_ASSERT(buf && buflen);
	RK_ASSERT(RKRBackend::repl_status.eval_depth >= 0);

	if (RKRBackend::repl_status.browser_context) { // previously we were in a browser context. Check, whether we've left that.
		if (RKRBackend::default_global_context == ROb(R_GlobalContext)) {
			RKRBackend::repl_status.browser_context = RKRBackend::RKReplStatus::NotInBrowserContext;
			RKRBackend::this_pointer->doRCallRequest(QStringLiteral("endBrowserContext"), QVariant(), RKRBackend::Asynchronous);
		}
	}

	if ((!RKRBackend::repl_status.browser_context) && (RKRBackend::repl_status.eval_depth == 0)) {
		while (true) {
			if (RKRBackend::this_pointer->isKilled() || (RKRBackend::repl_status.user_command_status == RKRBackend::RKReplStatus::NoUserCommand)) {
				RCommandProxy *command = RKRBackend::this_pointer->fetchNextCommand();
				if (!command) {
					RK_DEBUG(RBACKEND, DL_DEBUG, "returning from REPL");
					return 0; // jumps out of the event loop!
				}

				if (!(command->type & RCommand::User)) {
					RKRBackend::this_pointer->runCommand(command);
					RKRBackend::this_pointer->commandFinished();
					RKRBackend::this_pointer->handleDeferredInterrupts();
				} else {
					// so, we are about to transmit a new user command, which is quite a complex endeavor...
					/* Some words about running user commands:
					- User commands can only be run at the top level of execution, not in any sub-stacks. But then, they should never get there, in the first place.
					- Handling user commands is totally different from all other commands, and relies on R's "REPL" (read-evaluate-print-loop). This is a whole bunch of dedicated code, but there is no other way to achieve handling of commands as if they had been entered on a plain R console (including auto-printing, and toplevel handlers). Most importantly, since important symbols are not exported, such as R_Visible. Vice versa, it is not possible to treat all commands like user commands, esp. in substacks.

					Problems to deal with:
					- R_ReadConsole serves a lot of different functions, including reading in code, but also handling user input for readline() or browser(). This makes it necessary to carefully track the current status using "repl_status". You will find repl_status to be modified at a couple of different functions. Sorry for the resulting spaghetti code!
					- One difficulty lies in finding out, just when a command has finished (successfully or with an error).
					See below (RKRBackend::repl_status.user_command_status == RKRBackend::RKReplStatus::UserCommandRunning)
					for this, and and RKUserCommandErrorDetection for finding out, if an error occured. */
					RKRBackend::repl_status.user_command_transmitted_up_to = 0;
					RKRBackend::repl_status.user_command_completely_transmitted = false;
					RKRBackend::repl_status.user_command_parsed_up_to = 0;
					RKRBackend::repl_status.user_command_successful_up_to = 0;
					RKRBackend::repl_status.user_command_buffer = RKTextCodec::toNative(command->command);
					RKTransmitNextUserCommandChunk(buf, buflen);
					return 1;
				}
			} else if (RKRBackend::repl_status.user_command_status == RKRBackend::RKReplStatus::UserCommandTransmitted) {
				if (RKRBackend::repl_status.user_command_completely_transmitted) {
					// fully transmitted, but R is still asking for more, without signalling R_Busy? This looks like an incomplete statement.
					// HOWEVER: It may also have been a (trailing) empty statement such as "\n", so let's check whether the prompt looks like a "continue" prompt
					RBusy(1); // Mark command as "running"
					if (RKTextCodec::fromNative(prompt) == RKRSupport::SEXPToString(RFn::Rf_GetOption(RFn::Rf_install("continue"), ROb(R_BaseEnv)))) {
						RKRBackend::this_pointer->current_command->status |= RCommand::Failed | RCommand::ErrorIncomplete;
						RKRBackend::repl_status.user_command_status = RKRBackend::RKReplStatus::ReplIterationKilled;
						RFn::Rf_error("%s", ""); // to discard the buffer
					} // else: continue in next iteration at UserCommandRunning
				} else {
					RKTransmitNextUserCommandChunk(buf, buflen);
					return 1;
				}
			} else if (RKRBackend::repl_status.user_command_status == RKRBackend::RKReplStatus::UserCommandRunning) {
				// This can mean three different things:
				// 1) User called readline ()
				// 2) User called browser ()
				// 3) The user command has finished (successfully or not)
				// Let's find out, which one it is.
				if (hist && (RKRBackend::default_global_context != ROb(R_GlobalContext))) {
					break; // this looks like a call to browser(). Will be handled below.
				}

				int n_frames = 0;
				RCommandProxy *dummy = RKRBackend::this_pointer->runDirectCommand(QStringLiteral("sys.nframe()"), RCommand::GetIntVector);
				if ((dummy->getDataType() == RData::IntVector) && (dummy->getDataLength() == 1)) {
					n_frames = dummy->intVector().at(0);
				}
				// What the ??? Why does this simple version always return 0?
				// int n_frames = RKRSupport::SEXPToInt (RKRSupport::callSimpleFun0 (RFn::Rf_install ("sys.nframe"), ROb(R_GlobalEnv);
				if (n_frames < 1) {
					// No active frames? This can't be a call to readline(), so the previous command must have finished.
					if (RKRBackend::repl_status.user_command_completely_transmitted) {
						replCommandFinished();
					} else RKRBackend::repl_status.user_command_status = RKRBackend::RKReplStatus::UserCommandTransmitted;
				} else {
					// A call to readline(). Will be handled below
					break;
				}
			} else if (RKRBackend::repl_status.user_command_status == RKRBackend::RKReplStatus::UserCommandFailed) {
				if (!(RKRBackend::this_pointer->current_command->status & RCommand::ErrorSyntax)) {
					RKRBackend::this_pointer->current_command->status |= RCommand::Failed | RCommand::ErrorOther;
				}
				replCommandFinished();
			} else {
				RK_ASSERT(RKRBackend::repl_status.user_command_status == RKRBackend::RKReplStatus::ReplIterationKilled);
				replCommandFinished();
			}
		}
	}

	// here, we handle readline() calls and such, i.e. not the regular prompt for code
	// browser() also takes us here.
	QVariantMap params;
	RBackendRequest::RCallbackType request_type = RBackendRequest::ReadLine;
	params[QStringLiteral("prompt")] = QVariant(RKTextCodec::fromNative(prompt));
	params[QStringLiteral("cancelled")] = QVariant(false);

	// add info for browser requests
	if (hist && (RKRBackend::default_global_context != ROb(R_GlobalContext))) {
		if (RKRBackend::repl_status.browser_context == RKRBackend::RKReplStatus::InBrowserContextPreventRecursion) {
			qstrncpy((char *)buf, "n\n", buflen); // skip this, by feeding the browser() a continue
			return 1;
		} else {
			RKRBackend::repl_status.browser_context = RKRBackend::RKReplStatus::InBrowserContextPreventRecursion;
			RCommandProxy *dummy = RKRBackend::this_pointer->runDirectCommand(QStringLiteral(".rk.callstack.info()"), RCommand::GetStructuredData);

			request_type = RBackendRequest::Debugger;
			if ((dummy->getDataType() == RData::StructureVector) && (dummy->getDataLength() >= 4)) {
				RData::RDataStorage dummy_data = dummy->structureVector();
				params[QStringLiteral("calls")] = QVariant(dummy_data.at(0)->stringVector());
				params[QStringLiteral("funs")] = QVariant(dummy_data.at(1)->stringVector());
				params[QStringLiteral("envs")] = QVariant(dummy_data.at(2)->stringVector());
				params[QStringLiteral("locals")] = QVariant(dummy_data.at(3)->stringVector());
				params[QStringLiteral("relsrclines")] = QVariant(dummy_data.at(4)->stringVector()); // hacky: passing a QList<int> is not supported by QVariant
			} else {
				RK_ASSERT(false);
			}

			RKRBackend::repl_status.browser_context = RKRBackend::RKReplStatus::InBrowserContext;
		}

		RK_ASSERT(RKRBackend::repl_status.browser_context == RKRBackend::RKReplStatus::InBrowserContext);
	}

	RBackendRequest request(true, request_type);
	request.params = params;

	RKRBackend::this_pointer->handleRequest(&request);
	if (request.params[QStringLiteral("cancelled")].toBool()) {
		if (RKRBackend::this_pointer->current_command) RKRBackend::this_pointer->current_command->status |= RCommand::Canceled;
		RFn::Rf_error("cancelled"); // TODO: probably a memleak, as d'tors (request, params) won't be called
		RK_ASSERT(false);           // should not reach this point.
	}

	QByteArray localres = RKTextCodec::toNative(request.params[QStringLiteral("result")].toString());
	// need to append a newline, here. TODO: theoretically, RReadConsole comes back for more, if \0 was encountered before \n.
	qstrncpy((char *)buf, localres.left(buflen - 2).append('\n').data(), buflen);
	return 1;
}

bool RKRBackend::fetchStdoutStderr(bool forcibly) {
#ifndef Q_OS_WIN
	if (killed) return false;
	if (!forcibly) {
		if (!stdout_stderr_mutex.tryLock()) return false;
	} else {
		stdout_stderr_mutex.lock();
	}
	if (stdout_stderr_fd < 0) {
		stdout_stderr_mutex.unlock();
		return false;
	}

	// it seems, setting this only once is not always enough.
	fcntl(stdout_stderr_fd, F_SETFL, fcntl(stdout_stderr_fd, F_GETFL, 0) | O_NONBLOCK);
	char buffer[1024];
	while (true) {
		int bytes = read(stdout_stderr_fd, buffer, 1023);
		if (bytes <= 0) break;
		buffer[bytes] = '\0';
		// NOTE: we must not risk blocking inside handleOutput, while the stdout_stderr_mutex is locked!
		handleOutput(RKTextCodec::fromNative(buffer), bytes, ROutput::Warning, false);
	}

	stdout_stderr_mutex.unlock();
#endif
	return true;
}

void RWriteConsoleEx(const char *buf, int buflen, int type) {
	RK_TRACE(RBACKEND);
	RK_DEBUG(RBACKEND, DL_DEBUG, "raw output type %d, size %d: %s", type, buflen, buf);

	// output while nothing else is running (including handlers?) -> This may be a syntax error.
	if ((RKRBackend::repl_status.eval_depth == 0) && (!RKRBackend::repl_status.browser_context) && (!RKRBackend::this_pointer->isKilled())) {
		if (RKRBackend::repl_status.user_command_status == RKRBackend::RKReplStatus::UserCommandTransmitted) {
			if (RKRBackend::repl_status.user_command_parsed_up_to < RKRBackend::repl_status.user_command_transmitted_up_to && !RKRBackend::repl_status.interrupted) {
				RKRBackend::this_pointer->current_command->status |= RCommand::Failed | RCommand::ErrorSyntax;
				RKRBackend::repl_status.user_command_status = RKRBackend::RKReplStatus::UserCommandFailed;
			}
		} else if (RKRBackend::repl_status.user_command_status == RKRBackend::RKReplStatus::ReplIterationKilled) {
			// purge superfluous newlines and empty output
			return;
		} else {
			RK_ASSERT(RKRBackend::repl_status.user_command_status != RKRBackend::RKReplStatus::NoUserCommand);
		}
	}

	if (RKRBackend::this_pointer->killed == RKRBackend::AlreadyDead) return; // this check is mostly for fork()ed clients
	if (RKRBackend::repl_status.browser_context == RKRBackend::RKReplStatus::InBrowserContextPreventRecursion) return;
	RKRBackend::this_pointer->fetchStdoutStderr(true);

	QString utf8 = RKTextCodec::fromNative(buf);
	RKRBackend::this_pointer->handleOutput(utf8, utf8.length(), type == 0 ? ROutput::Output : ROutput::Warning);
}

/** For R callbacks that we want to disable, entirely */
void RDoNothing() {
	// RK_TRACE (RBACKEND);
}

void RCleanUp(SA_TYPE saveact, int status, int RunLast) {
	RK_TRACE(RBACKEND);
	Q_UNUSED(RunLast); // R_dot_Last is called while "running" the QuitCommand

	if (RKRBackend::this_pointer->killed == RKRBackend::AlreadyDead) return; // Nothing to clean up
	if (!RKRBackend::this_pointer->r_running) return;                        // prevent recursion (if an error occurs, here, we get jumped to the console repl, again!)
	RFn::R_CheckUserInterrupt();                                             // if there are any user interrupts pending, we want them handled *NOW*
	RKRBackend::this_pointer->r_running = false;

	// we could be in a signal handler, and the stack base may have changed.
	uintptr_t old_lim = ROb(R_CStackLimit);
	ROb(R_CStackLimit) = (uintptr_t)-1;

	if ((status != 0) && (RKRBackend::this_pointer->killed != RKRBackend::ExitNow)) RKRBackend::this_pointer->killed = RKRBackend::EmergencySaveThenExit;

	if (RKRBackend::this_pointer->killed == RKRBackend::EmergencySaveThenExit) {
		if (ROb(R_DirtyImage)) {
			QString filename;
			QDir dir(RKRBackendProtocolBackend::dataDir());
			int i = 0;
			while (true) {
				filename = u"rkward_recover"_s + QString::number(i) + u".RData"_s;
				if (!dir.exists(filename)) break;
				i++;
			}
			filename = dir.absoluteFilePath(filename);

			RFn::R_SaveGlobalEnvToFile(filename.toLocal8Bit().data());
			RK_DEBUG(RBACKEND, DL_WARNING, "Created emergency save file in %s", qPrintable(filename));
		} else {
			RK_DEBUG(RBACKEND, DL_WARNING, "Image not dirty while crashing. No emergency save created.");
		}
	}

	if (saveact != SA_SUICIDE) {
		if (!RKRBackend::this_pointer->isKilled()) {
			RBackendRequest request(true, RBackendRequest::BackendExit);
			request.params[QStringLiteral("message")] = QVariant(i18n("The R engine has shut down with status: %1", status));
			RKRBackend::this_pointer->handleRequest(&request);
		}
		RK_DEBUG(RBACKEND, DL_DEBUG, "Cleaning up");
		RFn::R_RunExitFinalizers();
		RFn::Rf_KillAllDevices();
		RFn::R_CleanTempDir();
	}

	RKRBackend::this_pointer->killed = RKRBackend::AlreadyDead; // just in case

	ROb(R_CStackLimit) = old_lim; // well, it should not matter any longer, but...
	RK_DEBUG(RBACKEND, DL_DEBUG, "Cleanup finished");
	RKRBackendProtocolBackend::doExit();
}

void RSuicide(const char *message) {
	RK_TRACE(RBACKEND);

	if (!RKRBackend::this_pointer->isKilled()) {
		RBackendRequest request(true, RBackendRequest::BackendExit);
		request.params[u"message"_s] = QVariant(i18n("The R engine has encountered a fatal error:\n%1", RKTextCodec::fromNative(message)));
		RKRBackend::this_pointer->handleRequest(&request);
		RKRBackend::this_pointer->killed = RKRBackend::EmergencySaveThenExit;
		RCleanUp(SA_SUICIDE, 1, 0);
	} else {
		RK_ASSERT(false);
	}
}

void RKRBackend::tryToDoEmergencySave() {
	RK_TRACE(RBACKEND);

	if (RKRBackendProtocolBackend::inRThread()) {
		// If we are in the correct thread, things are easy:
		RKRBackend::this_pointer->killed = RKRBackend::EmergencySaveThenExit;
		RCleanUp(SA_SUICIDE, 1, 0);
		RK_doIntr(); // to jump out of the loop, if needed
	} else {
		// If we are in the wrong thread, things are a lot more tricky. We need to cause the R thread to exit, and wait for it to finish saving.
		// Fortunately, if we are in the wrong thread, that probably means, the R thread did *not* crash, and will thus still be functional
		this_pointer->killed = EmergencySaveThenExit;
		return;
		RK_scheduleIntr();
		for (int i = 0; i < 100; ++i) { // give it up to ten seconds to interrupt and exit the loop
			if (!this_pointer->r_running) break;
			RKRBackendProtocolBackend::msleep(100);
		}
		if (!this_pointer->r_running) {
			for (int i = 0; i < 600; ++i) {                      // give it up to sixty seconds to finish saving
				if (this_pointer->killed == AlreadyDead) return; // finished
				RKRBackendProtocolBackend::msleep(100);
			}
		}
		RK_ASSERT(false); // Too bad, but we seem to be stuck. No chance but to return (and crash)
	}
}

static QStringList charPArrayToQStringList(const char **chars, int count) {
	QStringList ret;
	for (int i = 0; i < count; ++i) {
		// do we need to do locale conversion, here?
		ret.append(RKTextCodec::fromNative(chars[i]));
	}
	return ret;
}

int RChooseFile(int isnew, char *buf, int len) {
	RK_TRACE(RBACKEND);

	QStringList params;
	params << QString() /* caption */ << QString() /* initial */ << QStringLiteral("*") /* filter */ << (isnew ? u"newfile"_s : u"file"_s);
	auto res = RKRBackend::this_pointer->doRCallRequest(QStringLiteral("choosefile"), params, RKRBackend::Synchronous);

	QByteArray localres = RKTextCodec::toNative(res.ret.toString());
	qstrncpy((char *)buf, localres.data(), len);

	// return length of filename (strlen(buf))
	return (qMin(len - 1, localres.size()));
}

/** This class tries to encapsulate the following mess in a (more) readable way:

There is no direct way to detect, if a user command (running directly in the REPL) has failed.
RResetConsole() is always called in that case, but there is one further use in R's do_edit() (the C function
invoked _unless_ options("editor") is set to a function - which it usually is in our case).

To handle that rare case as good as possible, we try to keep track of which calls happen in which order.
The interesting bit is in rresetconsole_called(). */
struct RKUserCommandErrorDetection {
	bool reset_signals_error = true;
	void rbusy_called(int busy) {
		if (busy) reset_signals_error = true;
		RK_DEBUG(RBACKEND, DL_DEBUG, "RBusy %d", busy);
	}
	void reditfiles_called() {
		reset_signals_error = false;
	}
	void rresetconsole_called() {
		RK_DEBUG(RBACKEND, DL_WARNING, "reset console called, repl status %d, reset_signals_error %d, wasintr %d, awaiting_sigint %d", RKRBackend::repl_status.user_command_status, reset_signals_error, RKRBackend::repl_status.interrupted, RKRBackend::this_pointer->awaiting_sigint);
		if (!reset_signals_error) {
			reset_signals_error = true;
			return;
		}

		if (RKRBackend::repl_status.user_command_status == RKRBackend::RKReplStatus::ReplIterationKilled) return;

		if ((RKRBackend::repl_status.eval_depth == 0) && (!RKRBackend::repl_status.browser_context) && (!RKRBackend::this_pointer->isKilled()) && (RKRBackend::repl_status.user_command_status != RKRBackend::RKReplStatus::NoUserCommand)) {
			RKRBackend::repl_status.user_command_status = RKRBackend::RKReplStatus::UserCommandFailed;
		}

		if (!RKRBackend::repl_status.interrupted) {
			markLastWarningAsErrorMessage();
			RK_DEBUG(RBACKEND, DL_DEBUG, "error in user command");
		}
	}
};
static RKUserCommandErrorDetection user_command_error_detect;

/* There are about one million possible entry points to editing / showing files. We try to cover them all, using the
following bunch of functions (REditFilesHelper() and doShowEditFiles() are helpers, only) */

void REditFilesHelper(const QStringList &files, const QStringList &titles, const QString &wtitle, RBackendRequest::RCallbackType edit, bool delete_files, bool prompt) {
	RK_TRACE(RBACKEND);

	RK_ASSERT((edit == RBackendRequest::ShowFiles) || (edit == RBackendRequest::EditFiles));
	RBackendRequest request(edit != RBackendRequest::ShowFiles, edit); // editing is synchronous, showing is asynchronous
	if (edit == RBackendRequest::ShowFiles) {
		request.params[QStringLiteral("delete")] = QVariant(delete_files);
	}
	// see ?file.show() for what appears to be the intended meaning of these first three parameters
	// (which seem to be inconsistently named even in R itself...)
	request.params[QStringLiteral("files")] = QVariant(files);
	request.params[QStringLiteral("titles")] = QVariant(titles);
	request.params[QStringLiteral("wtitle")] = QVariant(wtitle);
	request.params[QStringLiteral("prompt")] = QVariant(prompt);

	RKRBackend::this_pointer->handleRequest(&request);
}

// Called from R's edit(editor=NULL)->do_edit()
int REditFiles(int nfile, const char **file, const char **title, const char *wtitle) {
	RK_TRACE(RBACKEND);

	user_command_error_detect.reditfiles_called();
	REditFilesHelper(charPArrayToQStringList(file, nfile), charPArrayToQStringList(title, nfile), RKTextCodec::fromNative(wtitle), RBackendRequest::EditFiles, false, true);

	// default implementation seems to return 1 if nfile <= 0, else 1. No idea, what for. see unix/std-sys.c
	return (nfile <= 0);
}

// Called from rk.edit()
SEXP doShowEditFiles(SEXP files, SEXP titles, SEXP wtitle, SEXP del, SEXP prompt, RBackendRequest::RCallbackType edit) {
	RK_TRACE(RBACKEND);

	QStringList file_strings = RKRSupport::SEXPToStringList(files);
	QStringList title_strings = RKRSupport::SEXPToStringList(titles);
	QString wtitle_string = RKRSupport::SEXPToString(wtitle);
	bool del_files = RKRSupport::SEXPToInt(del, 0) != 0;
	bool do_prompt = RKRSupport::SEXPToInt(prompt, 0) != 0;

	RK_ASSERT(file_strings.size() == title_strings.size());
	RK_ASSERT(file_strings.size() >= 1);

	REditFilesHelper(file_strings, title_strings, wtitle_string, edit, del_files, do_prompt);

	return (ROb(R_NilValue));
}

SEXP doEditFiles(SEXP files, SEXP titles, SEXP wtitle, SEXP prompt) {
	return (doShowEditFiles(files, titles, wtitle, ROb(R_NilValue), prompt, RBackendRequest::EditFiles));
}

int REditFile(const char *buf) {
	RK_TRACE(RBACKEND);

	const char *editor = "none";
	const char *title = "";

	// does not exist in standard R 2.1.0, so no idea what to return.
	return REditFiles(1, const_cast<const char **>(&buf), &title, editor);
}

SEXP doShowFiles(SEXP files, SEXP titles, SEXP wtitle, SEXP delete_files, SEXP prompt) {
	return (doShowEditFiles(files, titles, wtitle, delete_files, prompt, RBackendRequest::ShowFiles));
}

int RShowFiles(int nfile, const char **file, const char **headers, const char *wtitle, Rboolean del, const char * /* pager */) {
	RK_TRACE(RBACKEND);

	REditFilesHelper(charPArrayToQStringList(file, nfile), charPArrayToQStringList(headers, nfile), RKTextCodec::fromNative(wtitle), RBackendRequest::ShowFiles, (bool)del, true);

	// default implementation seems to returns 1 on success, 0 on failure. see unix/std-sys.c
	return 1;
}

/* FROM R_ext/RStartup.h: "Return value here is expected to be 1 for Yes, -1 for No and 0 for Cancel:
   symbolic constants in graphapp.h" */
int doDialogHelper(const QString &caption, const QString &message, const QString &button_yes, const QString &button_no, const QString &button_cancel, const QString &default_button, bool wait) {
	RK_TRACE(RBACKEND);

	RBackendRequest request(wait, RBackendRequest::ShowMessage);
	request.params[QStringLiteral("caption")] = QVariant(caption);
	request.params[QStringLiteral("message")] = QVariant(message);
	request.params[QStringLiteral("button_yes")] = QVariant(button_yes);
	request.params[QStringLiteral("button_no")] = QVariant(button_no);
	request.params[QStringLiteral("button_cancel")] = QVariant(button_cancel);
	request.params[QStringLiteral("default")] = QVariant(default_button);

	RKRBackend::this_pointer->handleRequest(&request);

	if (wait) {
		QString ret = request.params[QStringLiteral("result")].toString();
		if (ret == QLatin1String("yes")) return 1;
		if (ret == QLatin1String("no")) return -1;
	}
	return 0;
}

SEXP doDialog(SEXP caption, SEXP message, SEXP button_yes, SEXP button_no, SEXP button_cancel, SEXP default_button, SEXP wait) {
	RK_TRACE(RBACKEND);

	int result = doDialogHelper(RKRSupport::SEXPToString(caption), RKRSupport::SEXPToString(message), RKRSupport::SEXPToString(button_yes), RKRSupport::SEXPToString(button_no), RKRSupport::SEXPToString(button_cancel), RKRSupport::SEXPToString(default_button), RKRSupport::SEXPToInt(wait));

	SEXP ret = RFn::Rf_allocVector(INTSXP, 1);
	RFn::INTEGER(ret)[0] = result;
	return ret;
}

void RShowMessage(const char *message) {
	RK_TRACE(RBACKEND);

	doDialogHelper(i18n("Message from the R backend"), RKTextCodec::fromNative(message), u"ok"_s, QString(), QString(), u"ok"_s, true);
}

// TODO: currently used on windows, only!
int RAskYesNoCancel(const char *message) {
	RK_TRACE(RBACKEND);

	if (RKRBackend::this_pointer->killed) return -1; // HACK: At this point R asks whether to save the workspace. We have already handled that. So return -1 for "no"
	return doDialogHelper(i18n("Question from the R backend"), RKTextCodec::fromNative(message), u"yes"_s, u"no"_s, u"cancel"_s, u"yes"_s, true);
}

void RBusy(int busy) {
	RK_TRACE(RBACKEND);
	user_command_error_detect.rbusy_called(busy);

	// R_ReplIteration calls R_Busy (1) after reading in code (if needed), successfully parsing it, and right before evaluating it.
	if (busy) {
		if (RKRBackend::repl_status.user_command_status == RKRBackend::RKReplStatus::UserCommandTransmitted) {
			if (RKRBackend::this_pointer->current_command->type & RCommand::CCOutput) {
				// flush any previous output capture and start a new one
				if (RKRBackend::repl_status.user_command_successful_up_to > 0) RKRBackend::this_pointer->printAndClearCapturedMessages(false);
				RKRBackend::this_pointer->startOutputCapture();
			}
			if (RKRBackend::this_pointer->current_command->type & RCommand::CCCommand) {
				QByteArray chunk = RKRBackend::repl_status.user_command_buffer.mid(RKRBackend::repl_status.user_command_parsed_up_to, RKRBackend::repl_status.user_command_transmitted_up_to - RKRBackend::repl_status.user_command_parsed_up_to);
				RKRBackend::this_pointer->printCommand(RKTextCodec::fromNative(chunk.data()));
			}
			RKRBackend::repl_status.user_command_parsed_up_to = RKRBackend::repl_status.user_command_transmitted_up_to;
			RKRBackend::repl_status.user_command_status = RKRBackend::RKReplStatus::UserCommandRunning;
			RKRBackend::this_pointer->beginAllowInterruptCommand(RKRBackend::this_pointer->current_command);
		}
	} else if (RKRBackend::repl_status.user_command_status != RKRBackend::RKReplStatus::NoUserCommand && RKRBackend::repl_status.eval_depth == 0) {
		if (RKRBackend::this_pointer->current_command->interruptible_stage) {
			RKRBackend::this_pointer->endAllowInterruptCommand(RKRBackend::this_pointer->current_command);
		}
	}
}

void RResetConsole() {
	RK_TRACE(RBACKEND);
	user_command_error_detect.rresetconsole_called();
}

// ############## R Standard callback overrides END ####################

// NOTE: stdout_stderr_mutex is recursive to support fork()s, better
RKRBackend::RKRBackend() : stdout_stderr_mutex() {
	RK_TRACE(RBACKEND);

	RK_ASSERT(this_pointer == nullptr);
	this_pointer = this;

	RKTextCodec::reinit();
	r_running = false;

	current_command = nullptr;
	pending_priority_command = nullptr;
	stdout_stderr_fd = -1;
}

#ifdef Q_OS_WIN
void RKRBackend::setupCallbacks() {
	RK_TRACE(RBACKEND);

	RFn::R_setStartTime();
	RFn::R_DefParamsEx(&RK_R_Params, /* RSTART_VERSION */ 1);

	// IMPORTANT: see also the #ifndef QS_WS_WIN-portion!
	RK_R_Params.rhome = RFn::get_R_HOME();
	RK_R_Params.home = RFn::getRUser();
	RK_R_Params.CharacterMode = RGui;
	RK_R_Params.ShowMessage = RShowMessage;
	RK_R_Params.ReadConsole = RReadConsole;
	RK_R_Params.WriteConsoleEx = RWriteConsoleEx;
	RK_R_Params.WriteConsole = nullptr;
	RK_R_Params.CallBack = RKREventLoop::winRKEventHandlerWrapper;
	RK_R_Params.YesNoCancel = RAskYesNoCancel;
	RK_R_Params.Busy = RBusy;
	RK_R_Params.ResetConsole = RResetConsole;
	RK_R_Params.CleanUp = RCleanUp; // unfortunately, it seems, we can't safely cancel quitting anymore, here!
	RK_R_Params.Suicide = RSuicide;

	// TODO: callback mechanism(s) for ChosseFile, ShowFiles, EditFiles still not available on Windows?

	RK_R_Params.R_Quiet = Rboolean::FALSE;
	RK_R_Params.R_Interactive = Rboolean::TRUE;
	RK_R_Params.EmitEmbeddedUTF8 = Rboolean::FALSE;
}

void RKRBackend::connectCallbacks() {
	RK_TRACE(RBACKEND);
	RFn::R_SetParams(&RK_R_Params);
}
#else
void RKRBackend::setupCallbacks() {
	RK_TRACE(RBACKEND);
}

void RKRBackend::connectCallbacks() {
	RK_TRACE(RBACKEND);

	// IMPORTANT: see also the #ifdef QS_WS_WIN-portion!
	// connect R standard callback to our own functions. Important: Don't do so, before our own versions are ready to be used!
	ROb(R_Outputfile) = nullptr;
	ROb(R_Consolefile) = nullptr;
	ROb(ptr_R_ShowMessage) = RShowMessage; // rarely used in R on unix
	ROb(ptr_R_ReadConsole) = RReadConsole;
	ROb(ptr_R_WriteConsoleEx) = RWriteConsoleEx;
	ROb(ptr_R_WriteConsole) = nullptr;
	ROb(ptr_R_ResetConsole) = RResetConsole;
	ROb(ptr_R_FlushConsole) = RDoNothing;
	ROb(ptr_R_ClearerrConsole) = RDoNothing;
	ROb(ptr_R_Busy) = RBusy;
	ROb(ptr_R_CleanUp) = RCleanUp; // unfortunately, it seems, we can't safely cancel quitting anymore, here!
	ROb(ptr_R_ShowFiles) = RShowFiles;
	ROb(ptr_R_ChooseFile) = RChooseFile;
	// TODO: R devels disabled this for some reason. We set it anyway...
	ROb(ptr_R_EditFile) = REditFile;
	//	ROb(ptr_R_EditFiles) = REditFiles;		// undefined reference

	// these two, we won't override
	//	ROb(ptr_R_loadhistory) = ... 	// we keep our own history
	//	ROb(ptr_R_savehistory) = ...	// we keep our own history
}
#endif

RKRBackend::~RKRBackend() {
	RK_TRACE(RBACKEND);
}

// TODO: Pass nested/sync as a single enum value, in the first place
SEXP doRCall(SEXP _call, SEXP _args, SEXP _sync, SEXP _nested) {
	RK_TRACE(RBACKEND);

	RFn::R_CheckUserInterrupt();

	bool sync = RKRSupport::SEXPToInt(_sync);
	bool nested = RKRSupport::SEXPToInt(_nested);
	RKRBackend::RequestFlags flags = sync ? (nested ? RKRBackend::SynchronousWithSubcommands : RKRBackend::Synchronous) : RKRBackend::Asynchronous;

	// For now, for simplicity, assume args are always strings, although possibly nested in lists
	auto ret = RKRBackend::this_pointer->doRCallRequest(RKRSupport::SEXPToStringList(_call).value(0), RKRSupport::SEXPToNestedStrings(_args), flags);
	if (!ret.warning.isEmpty()) RFn::Rf_warning("%s", RKTextCodec::toNative(ret.warning).constData()); // print warnings, first, as errors will cause a stop
	if (!ret.error.isEmpty()) RFn::Rf_error("%s", RKTextCodec::toNative(ret.error).constData());

	return RKRSupport::QVariantToSEXP(ret.ret);
}

QString getLibLoc() {
	return RKRBackendProtocolBackend::dataDir() + u"/.rkward_packages/"_s + QString::number(RKRBackend::this_pointer->r_version / 10);
}

// Function to handle several simple calls from R code, that do not need any special arguments, or interaction with the frontend process.
SEXP doSimpleBackendCall(SEXP _call) {
	RK_TRACE(RBACKEND);

	QStringList list = RKRSupport::SEXPToStringList(_call);
	QString call = list[0];

	if (call == QStringLiteral("unused.filename")) {
		QString prefix = list.value(1);
		QString extension = list.value(2);
		QString dirs = list.value(3);
		QDir dir(dirs);
		if (dirs.isEmpty()) {
			dir = QDir(RKRBackendProtocolBackend::dataDir());
		}

		int i = 0;
		while (true) {
			QString candidate = prefix + QString::number(i) + extension;
			if (!dir.exists(candidate)) {
				return (RKRSupport::StringListToSEXP(QStringList(candidate) << dir.absoluteFilePath(candidate))); // return as c (relpath, abspath)
			}
			i++;
		}
	} else if (call == QStringLiteral("tempdir")) {
		return (RKRSupport::StringListToSEXP(QStringList(RKRBackendProtocolBackend::dataDir())));
	} else if (call == QLatin1String("home")) {
		if (list.value(1) == QLatin1String("home")) return RKRSupport::StringListToSEXP(QStringList(RKRBackendProtocolBackend::dataDir()));
		else if (list.value(1) == QLatin1String("lib")) return RKRSupport::StringListToSEXP(QStringList(getLibLoc()));
		else RK_ASSERT(false); // should have been handled in frontend
	} else if (call == QLatin1String("backendSessionInfo")) {
		// Non-translatable on purpose. This is meant for posting to the bug tracker, mostly.
		QStringList lines(u"Debug message file (this may contain relevant diagnostic output in case of trouble):"_s);
		lines.append(RKRBackendProtocolBackend::backendDebugFile());
		lines.append(QString());
		// NOTE: R_SVN_REVISON used to be a string, but has changed to numeric constant in R 3.0.0. QString::arg() handles both.
		lines.append(QStringLiteral("R version (compile time): %1").arg(QStringLiteral(R_MAJOR "." R_MINOR " " R_STATUS " (" R_YEAR "-" R_MONTH "-" R_DAY " r%1)").arg(R_SVN_REVISION)));
		return RKRSupport::StringListToSEXP(lines);
	}

	RK_ASSERT(false); // Unhandled call.
	return ROb(R_NilValue);
}

SEXP doUpdateLocale() {
	RK_TRACE(RBACKEND);

	RK_DEBUG(RBACKEND, DL_WARNING, "Changing locale");
	RKTextCodec::reinit();

	return ROb(R_NilValue);
}

SEXP doGetStructure(SEXP toplevel, SEXP name, SEXP envlevel, SEXP namespacename) {
	RK_TRACE(RBACKEND);

	RKStructureGetter getter(false);
	RData *ret = getter.getStructure(toplevel, name, envlevel, namespacename);
	return RFn::R_MakeExternalPtr(ret, RKWard_RData_Tag, ROb(R_NilValue));
}

SEXP doGetGlobalEnvStructure(SEXP name, SEXP envlevel, SEXP namespacename) {
	RK_TRACE(RBACKEND);

	return doGetStructure(RFn::Rf_findVar(RFn::Rf_installChar(RFn::STRING_ELT(name, 0)), ROb(R_GlobalEnv)), name, envlevel, namespacename);
}

/** copy a symbol without touching it (esp. not forcing any promises) */
SEXP doCopyNoEval(SEXP fromname, SEXP fromenv, SEXP toname, SEXP toenv) {
	RK_TRACE(RBACKEND);

	if (!RFn::Rf_isString(fromname) || RFn::Rf_length(fromname) != 1) RFn::Rf_error("fromname is not a single string");
	if (!RFn::Rf_isString(toname) || RFn::Rf_length(toname) != 1) RFn::Rf_error("toname is not a single string");
	if (!RFn::Rf_isEnvironment(fromenv)) RFn::Rf_error("fromenv is not an environment");
	if (!RFn::Rf_isEnvironment(toenv)) RFn::Rf_error("toenv is not an environment");
	RFn::Rf_defineVar(RFn::Rf_installChar(RFn::STRING_ELT(toname, 0)), RFn::Rf_findVar(RFn::Rf_installChar(RFn::STRING_ELT(fromname, 0)), fromenv), toenv);
	return (ROb(R_NilValue));
}

SEXP doCaptureOutput(SEXP mode, SEXP capture_messages, SEXP capture_output, SEXP suppress_messages, SEXP suppress_output, SEXP allow_nesting) {
	RK_TRACE(RBACKEND);

	if (RKRSupport::SEXPToInt(mode) == 1) {
		int cm = 0;
		if (RKRSupport::SEXPToInt(capture_messages)) cm |= RKROutputBuffer::RecordMessages;
		if (RKRSupport::SEXPToInt(capture_output)) cm |= RKROutputBuffer::RecordOutput;
		if (RKRSupport::SEXPToInt(suppress_messages)) cm |= RKROutputBuffer::SuppressMessages;
		if (RKRSupport::SEXPToInt(suppress_output)) cm |= RKROutputBuffer::SuppressOutput;
		if (!RKRSupport::SEXPToInt(allow_nesting)) cm |= RKROutputBuffer::NoNesting;
		RKRBackend::this_pointer->pushOutputCapture(cm);
		return (ROb(R_NilValue));
	} else {
		return RKRSupport::StringListToSEXP(QStringList(RKRBackend::this_pointer->popOutputCapture(RKRSupport::SEXPToInt(mode) == 2)));
	}
}

SEXP RKStartGraphicsDevice(SEXP width, SEXP height, SEXP pointsize, SEXP family, SEXP bg, SEXP title, SEXP antialias);
SEXP RKGraphicsModRevision(SEXP id);
SEXP RKD_AdjustSize(SEXP devnum, SEXP id);
void doPendingPriorityCommands();

SEXP checkEnv(SEXP a) {
	auto res = RKRShadowEnvironment::diffAndUpdate(a);
	return RFn::Rf_list3(RKRSupport::StringListToSEXP(res.added), RKRSupport::StringListToSEXP(res.removed), RKRSupport::StringListToSEXP(res.changed));
}

bool RKRBackend::startR() {
	RK_TRACE(RBACKEND);

	setupCallbacks();

	RKSignalSupport::saveDefaultSignalHandlers();

	awaiting_sigint = false;
	r_running = true;
	int argc = 3;
	char *argv[3] = {qstrdup("rkward"), qstrdup("--no-save"), qstrdup("--no-restore")};
	RFn::Rf_initialize_R(argc, argv);

#ifdef Q_OS_WIN
	RFn::R_set_command_line_arguments(argc, argv);
	FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
#endif

#ifndef Q_OS_WIN
	// re-direct stdout / stderr to a pipe, so we can read output from system() calls
	int pfd[2];
	int error = pipe(pfd);
	RK_ASSERT(!error); // mostly to silence compile time warning about unused return value
	dup2(pfd[1], STDOUT_FILENO);
	dup2(pfd[1], STDERR_FILENO); // forward both to a single channel to avoid interleaving hell, for now.
	close(pfd[1]);
	stdout_stderr_fd = pfd[0];
#endif

#ifndef Q_OS_WIN
	// It is important to set this *early*, so R does not bail out, if there is an error in .Rprofile.
	// On windows, set in connectCallbacks() for technical reasons, and that seems sufficient.
	ROb(R_Interactive) = Rboolean::TRUE;
#endif

	RFn::setup_Rmainloop();
	RKTextCodec::reinit(); // call again, as locale may have been re-initialized during startup

#ifndef Q_OS_WIN
	// safety check: If we are beyond the stack boundaries already, we better disable stack checking
	// this has to come *after* the first setup_Rmainloop ()!
	Rboolean stack_ok = RFn::R_ToplevelExec([](void *) { RFn::R_CheckStack(); }, nullptr);
	if (!stack_ok) {
		RK_DEBUG(RBACKEND, DL_WARNING, "R_CheckStack() failed during initialization. Will disable stack checking and try to re-initialize.");
		RK_DEBUG(RBACKEND, DL_WARNING, "Whether or not things work after this, *please* submit a bug report.");
		ROb(R_CStackStart) = (uintptr_t)-1;
		ROb(R_CStackLimit) = (uintptr_t)-1;
		RFn::setup_Rmainloop();
	}
#endif

#ifndef Q_OS_WIN
	// I am not sure, whether it is necessary to repeat this, here. It is not in R 3.0.0.
	// But historically, it was placed here (after setup_Rmainloop(), and conceivably there
	// was a reason to that (might have been reset in setup_Rmainloop() in earlier versions
	// of R.
	ROb(R_Interactive) = Rboolean::TRUE;
#endif

	setlocale(LC_NUMERIC, "C"); // Under some conditions something appears to mess with the locale. R will not work correctly without LC_NUMERIC=C

	RBackendRequest req(false, RBackendRequest::SetParamsFromBackend);
	req.params[QStringLiteral("na_real")] = ROb(R_NaReal); // may not be initialized before setup_Rmainloop!
	req.params[QStringLiteral("na_int")] = ROb(R_NaInt);
	handleRequest(&req);

	RKWard_RData_Tag = RFn::Rf_install("RKWard_RData_Tag");

	RKSignalSupport::installSignalProxies(); // for the crash signals
	RKSignalSupport::installSigIntAndUsrHandlers(RK_scheduleIntr);

	// register our functions
	R_CallMethodDef callMethods[] = {
	    // NOTE: Intermediate cast to void* to avoid compiler warning
	    {"rk.check.env", (DL_FUNC)(void *)&checkEnv, 1},
	    {"rk.simple", (DL_FUNC)(void *)&doSimpleBackendCall, 1},
	    {"rk.call", (DL_FUNC)(void *)&doRCall, 4},
	    {"rk.get.structure", (DL_FUNC)(void *)&doGetStructure, 4},
	    {"rk.get.structure.global", (DL_FUNC)(void *)&doGetGlobalEnvStructure, 3},
	    {"rk.copy.no.eval", (DL_FUNC)(void *)&doCopyNoEval, 4},
	    {"rk.edit.files", (DL_FUNC)(void *)&doEditFiles, 4},
	    {"rk.show.files", (DL_FUNC)(void *)&doShowFiles, 5},
	    {"rk.dialog", (DL_FUNC)(void *)&doDialog, 7},
	    {"rk.update.locale", (DL_FUNC)(void *)&doUpdateLocale, 0},
	    {"rk.capture.output", (DL_FUNC)(void *)&doCaptureOutput, 6},
	    {"rk.graphics.device", (DL_FUNC)(void *)&RKStartGraphicsDevice, 7},
	    {"rk.graphics.mod", (DL_FUNC)(void *)&RKGraphicsModRevision, 1},
	    {"rk.graphics.device.resize", (DL_FUNC)(void *)&RKD_AdjustSize, 2},
	    {nullptr, nullptr, 0}};
	RFn::R_registerRoutines(RFn::R_getEmbeddingDllInfo(), nullptr, callMethods, nullptr, nullptr);

	connectCallbacks();
	RKREventLoop::setRKEventHandler(doPendingPriorityCommands);
	default_global_context = ROb(R_GlobalContext);

	// What the??? Somehow the first command we run *will* appear to throw a syntax error. Some init step seems to be missing, but where?
	// Anyway, we just run a dummy to "clear" that trap.
	runDirectCommand(QStringLiteral("\n"));

	// get info on R runtime version
	RCommandProxy *dummy = runDirectCommand(QStringLiteral("(as.numeric(R.version$major)*1000+as.numeric(R.version$minor)*10)\n"), RCommand::GetIntVector);
	if ((dummy->getDataType() == RData::IntVector) && (dummy->getDataLength() == 1)) {
		r_version = dummy->intVector().at(0);
	} else {
		RK_ASSERT(false);
		r_version = 0;
	}
	delete dummy;

	return true;
}

#ifndef Q_OS_WIN
static bool backend_was_forked = false;
void prepareFork() {
	RK_TRACE(RBACKEND);
	if (!RKRBackendProtocolBackend::inRThread()) return;

	// we need to make sure that the transmitter thread does not hold a lock on the mutex!
	RKRBackend::this_pointer->stdout_stderr_mutex.lock();
}

void completeForkMaster() {
	RK_TRACE(RBACKEND);
	if (!RKRBackendProtocolBackend::inRThread()) return;

	RKRBackend::this_pointer->stdout_stderr_mutex.unlock();

	if (backend_was_forked) return;
	backend_was_forked = true;

	// Block SIGCHLD in the main thread from now on. I don't fully understand, why, but otherwise, these signals
	// interrupt the select() call in the fork()ing code of library(parallel)
	sigset_t new_set;
	sigemptyset(&new_set);
	sigaddset(&new_set, SIGCHLD);
	pthread_sigmask(SIG_BLOCK, &new_set, nullptr);

	//	This was used to show a warning message. Unfortunately, however, forks also occur on every popen (i.e. in system(..., intern=TRUE).
	//	RKRBackend::this_pointer->handlePlainGenericRequest (QStringList ("forkNotification"), false);
	RK_DEBUG(RBACKEND, DL_WARNING, "Backend process forked (for the first time, this session)");
	//	NOTE: perhaps we can heuristically differentiate from popen by checking sys.calls() for something with "fork" in it.
	//	esp., in case we discover adverse side-effects of blocking SIGCHLD, we should attempt this
}

void completeForkChild() {
	RKRBackendProtocolBackend::instance()->r_thread_id = QThread::currentThreadId();
	RKRBackend::this_pointer->killed = RKRBackend::AlreadyDead; // Not quite accurate, but disables all communication with the frontend
}
#endif

void RKRBackend::enterEventLoop() {
	RK_TRACE(RBACKEND);

#ifndef Q_OS_WIN
	pthread_atfork(prepareFork, completeForkMaster, completeForkChild);
#endif

	RFn::run_Rmainloop();
	// NOTE: Do NOT run RFn::Rf_endEmbeddedR(). It does more that we want. We rely on RCleanup, instead.
	// NOTE: never reached with R since ?? at least 4.3: RCleanUp is expected to exit the process
	RK_DEBUG(RBACKEND, DL_DEBUG, "R loop finished");
}

bool RKRBackend::runDirectCommand(const QString &command) {
	RK_TRACE(RBACKEND);

	RCommandProxy c(command, RCommand::App | RCommand::Sync | RCommand::Internal);
	runCommand(&c);
	return ((c.status & RCommand::WasTried) && !(c.status & RCommand::Failed));
}

RCommandProxy *RKRBackend::runDirectCommand(const QString &command, RCommand::CommandTypes datatype) {
	RK_TRACE(RBACKEND);
	RK_ASSERT((datatype >= RCommand::GetIntVector) && (datatype <= RCommand::GetStructuredData));

	RCommandProxy *c = new RCommandProxy(command, RCommand::App | RCommand::Sync | RCommand::Internal | datatype);
	runCommand(c);
	return c;
}

static void setWarnOption(int level, bool tryeval = false) {
	SEXP s, t;
	RFn::Rf_protect(t = s = RFn::Rf_allocList(2));
	RFn::SET_TYPEOF(s, LANGSXP);
	RFn::SETCAR(t, RFn::Rf_install("options"));
	t = RFn::CDR(t);
	RFn::SETCAR(t, RFn::Rf_ScalarInteger(level));
	RFn::SET_TAG(t, RFn::Rf_install("warn"));
	// The above is rougly equivalent to parseCommand ("options(warn=" + QString::number (level) + ")", &error), but ~100 times faster
	if (tryeval) {
		int error;
		RFn::R_tryEval(s, ROb(R_GlobalEnv), &error);
	} else RFn::Rf_eval(s, ROb(R_GlobalEnv));
	RFn::Rf_unprotect(1);
}

struct RKParseAndRunData {
	enum ReachedState { None,
		                Parsed,
		                SetWarn,
		                Evalled,
		                ResetWarn };
	ReachedState status = None;
	RCommandProxy *command;
	QByteArray commandbuf; // cannot live on stack inside parseAndRunWorker
	int warn_level;
};
static void parseAndRunWorker(void *_data) {
	RK_TRACE(RBACKEND);
	auto data = static_cast<RKParseAndRunData *>(_data);

	// WARNING: R may longjmp out of this function at several points. Therefore:
	//          No objects with d'tors on the stack, please! (Or exceptions, or other fancy C++ stuff)
	SEXP commandexp, parsed, exp;
	RFn::Rf_protect(commandexp = RFn::Rf_allocVector(STRSXP, 1));
	data->commandbuf = RKTextCodec::toNative(data->command->command);
	RFn::SET_STRING_ELT(commandexp, 0, RFn::Rf_mkChar(data->commandbuf.data()));
	ParseStatus parsestatus;
	RFn::Rf_protect(parsed = RFn::R_ParseVector(commandexp, -1, &parsestatus, ROb(R_NilValue)));
	// if we reached this point, parsing (successful or not) did not result in a longjmp - which may actually happen:
	// trying to parse list(""=1) is an example in R 3.1.1
	data->status = RKParseAndRunData::Parsed;

	if ((parsestatus != PARSE_OK) || (!parsed) || (RFn::TYPEOF(parsed) == NILSXP)) {
		// NOTE: according to historic code, it is possible for parsed to nullptr or R_NilValue, even though
		//       parse_status == PARSE_OK. Not sure, if this is actually true
		if ((parsestatus == PARSE_INCOMPLETE) || (parsestatus == PARSE_EOF)) {
			data->command->status |= RCommand::ErrorIncomplete;
		} else if (parsestatus == PARSE_NULL) {
			data->status = RKParseAndRunData::ResetWarn; // we're done despite returning early
		} else if (parsestatus == PARSE_ERROR) {
			data->command->status |= RCommand::ErrorSyntax;
		}
		RFn::Rf_unprotect(2);
		return;
	}

	// Make sure any warnings arising during the command actually get associated with it (rather than getting printed after the next user command)
	data->warn_level = RKRSupport::SEXPToInt(RFn::Rf_GetOption1(RFn::Rf_install("warn")), 0);
	if (data->warn_level != 1) {
		setWarnOption(1);
		data->status = RKParseAndRunData::SetWarn;
	}

	// evaluate the actual command - also, if it consists of multiple expressions, internally
	RKRBackend::this_pointer->beginAllowInterruptCommand(data->command);
	if (RFn::TYPEOF(parsed) == EXPRSXP && RFn::LENGTH(parsed) > 0) {
		const int len = RFn::LENGTH(parsed);
		for (int bi = 0; bi < len; bi++) {
			SEXP pxp = RFn::VECTOR_ELT(parsed, bi);
			exp = RFn::Rf_eval(pxp, ROb(R_GlobalEnv));
		}
	} else {
		exp = RFn::Rf_eval(parsed, ROb(R_GlobalEnv));
	}
	RFn::Rf_protect(exp);
	RKRBackend::this_pointer->endAllowInterruptCommand(data->command);
	data->status = RKParseAndRunData::Evalled;

	if (data->warn_level != 1) setWarnOption(data->warn_level);
	data->status = RKParseAndRunData::ResetWarn;

	const auto ctype = data->command->type;
	if (ctype & RCommand::GetStringVector) {
		data->command->setData(RKRSupport::SEXPToStringList(exp));
	} else if (ctype & RCommand::GetRealVector) {
		data->command->setData(RKRSupport::SEXPToRealArray(exp));
	} else if (ctype & RCommand::GetIntVector) {
		data->command->setData(RKRSupport::SEXPToIntArray(exp));
	} else if (ctype & RCommand::GetStructuredData) {
		RData *dummy = RKRSupport::SEXPToRData(exp);
		data->command->swallowData(*dummy);
		delete dummy;
	}
	RFn::Rf_unprotect(3); // commandexp, parsed, exp
}

void RKRBackend::runCommand(RCommandProxy *command) {
	RK_TRACE(RBACKEND);
	RK_ASSERT(command);

	int ctype = command->type; // easier typing

	// running user commands is quite different from all other commands and should have been handled by RReadConsole
	RK_ASSERT(!(ctype & RCommand::User));

	if (ctype & RCommand::CCCommand) printCommand(command->command);
	if (ctype & RCommand::CCOutput) startOutputCapture();

	if (ctype & RCommand::QuitCommand) {
		RFn::R_dot_Last(); // should run while communication with frontend is still possible
		RBackendRequest req(true, RBackendRequest::BackendExit);
		req.params[QStringLiteral("regular")] = QVariant(true);
		handleRequest(&req);
		killed = ExitNow;
	} else if (!(ctype & RCommand::EmptyCommand)) {
		repl_status.eval_depth++;
		RKParseAndRunData data;
		data.command = command;
		RFn::R_ToplevelExec(&parseAndRunWorker, &data);
		// fix up after conditions that would have cause parseAndRunWorker() to exit via longjmp
		command->status |= RCommand::WasTried;
		if (data.status < RKParseAndRunData::ResetWarn) {
			markLastWarningAsErrorMessage();
			command->status |= RCommand::Failed;
			if (data.status < RKParseAndRunData::Parsed) {
				command->status |= RCommand::ErrorSyntax;
			} else if (!(command->status & RCommand::Canceled)) {
				command->status |= RCommand::ErrorOther;
			}
			if (data.status >= RKParseAndRunData::SetWarn) {
				setWarnOption(data.warn_level, true);
			}
		}
		repl_status.eval_depth--;
	}
}

void RKRBackend::setPriorityCommand(RCommandProxy *command) {
	RK_TRACE(RBACKEND);
	QMutexLocker lock(&priority_command_mutex);
	RK_ASSERT(!(command && pending_priority_command)); // for the time being, we support only one priority command at a time
	pending_priority_command = command;
	RKREventLoop::wakeRKEventHandler();
}

void doPendingPriorityCommands() {
	RK_TRACE(RBACKEND);

	if (RKRBackend::this_pointer->killed) return;
	RCommandProxy *command = RKRBackend::this_pointer->pending_priority_command;
	RKRBackend::this_pointer->pending_priority_command = nullptr;
	if (command) {
		RK_DEBUG(RBACKEND, DL_DEBUG, "running priority command %s", qPrintable(command->command));
		{
			QMutexLocker lock(&RKRBackend::this_pointer->command_flow_mutex);
			RKRBackend::this_pointer->all_current_commands.append(command);
			RKRBackend::this_pointer->current_command = command;
		}

		RKRBackend::this_pointer->runCommand(command);
		// TODO: Oh boy, what a mess. Sending notifications should be split from fetchNextCommand() (which is not appropriate, here)
		RCommandProxy *previous_command_backup = RKRBackend::this_pointer->previous_command;
		RKRBackend::this_pointer->commandFinished(false);
		RKRBackend::this_pointer->previous_command = previous_command_backup;
		{
			RBackendRequest req(false, RBackendRequest::CommandOut); // NOTE: We do *NOT* want a reply to this one, and in particular, we do *NOT* want to do
			                                                         // (recursive) event processing while handling this.
			req.command = command;
			RKRBackend::this_pointer->handleRequest(&req);
		}
	}
}

// On Windows, using runDirectCommand (".rk.cat.output ...") is not safe during some places where we call this, e.g. in RBusy.
// Not a problem on Linux with R 2.13.0, though
void RKRBackend::catToOutputFile(const QString &out) {
	RK_TRACE(RBACKEND);

	SEXP output_file_r = RFn::Rf_findVar(RFn::Rf_install(".rk.output.html.file"), RKRSupport::getRKVariablesEnv());
	auto output_file = RKRSupport::SEXPToString(output_file_r);
	if (output_file.isEmpty()) {
		RK_ASSERT(false);
		return;
	}
	QFile f(output_file);
	if (!f.open(QIODevice::WriteOnly | QIODevice::Append)) {
		RK_ASSERT(false);
		return;
	}
	f.write(RKTextCodec::toNative(out));
	f.close();
}

void RKRBackend::printCommand(const QString &command) {
	RK_TRACE(RBACKEND);

	QString highlighted = doRCallRequest(QStringLiteral("highlightRCode"), command, RKRBackend::Synchronous).ret.toString();
	catToOutputFile(highlighted);
}

void RKRBackend::startOutputCapture() {
	RK_TRACE(RBACKEND);

	pushOutputCapture(RecordMessages | RecordOutput);
}

void RKRBackend::printAndClearCapturedMessages(bool with_header) {
	RK_TRACE(RBACKEND);

	QString out = popOutputCapture(true);
	if (out.isEmpty()) return;
	if (with_header) out.prepend(u"<h2>Messages, warnings, or errors:</h2>\n"_s);
	catToOutputFile(out);
}

void RKRBackend::run(const QString &locale_dir, bool setup) {
	RK_TRACE(RBACKEND);
	killed = NotKilled;
	previous_command = nullptr;

	initialize(locale_dir, setup);

	enterEventLoop();
}

void RKRBackend::commandFinished(bool check_object_updates_needed) {
	RK_TRACE(RBACKEND);
	RK_DEBUG(RBACKEND, DL_DEBUG, "done running command %s, status: %d", qPrintable(current_command->command), current_command->status);

	fetchStdoutStderr(true);
	if (current_command->type & RCommand::CCOutput) printAndClearCapturedMessages(current_command->type & RCommand::Plugin);
	current_command->status -= (current_command->status & RCommand::Running);
	current_command->status |= RCommand::WasTried;

	if (current_command->type & RCommand::User) {
		RK_ASSERT(repl_status.eval_depth == 0);

		// This method may look a bit over-complex, but remember that repl_status.user_command_successful_up_to works on an *encoded* buffer
		QByteArray remainder_encoded = repl_status.user_command_buffer.mid(repl_status.user_command_successful_up_to);
		QString remainder = RKTextCodec::fromNative(remainder_encoded);
		current_command->has_been_run_up_to = current_command->command.length() - remainder.length();
	}

	if (!current_command->updates_object.isEmpty()) {
		// Update cached value for objects that are known to have been modified, so as to not trigger an additional change notification.
		RKRShadowEnvironment::updateCacheForGlobalenvSymbol(current_command->updates_object);
	}
	if (check_object_updates_needed || (current_command->type & RCommand::ObjectListUpdate)) {
		checkObjectUpdatesNeeded(current_command->type & (RCommand::User | RCommand::ObjectListUpdate));
	}

	previous_command = current_command;

	{
		QMutexLocker lock(&command_flow_mutex);
		if (repl_status.interrupted) {
			repl_status.interrupted = false;
			current_command->status |= RCommand::Canceled;
		}
		all_current_commands.pop_back();
		if (!all_current_commands.isEmpty()) current_command = all_current_commands.last();
	}
}

RCommandProxy *RKRBackend::handleRequest(RBackendRequest *request, bool mayHandleSubstack) {
	RK_TRACE(RBACKEND);
	RK_ASSERT(request);

	// See docs for RBackendRequest for hints to make sense of this mess (and eventually to fix it)

	RKRBackendProtocolBackend::instance()->sendRequest(request);
	if (request->subcommandrequest) {
		handleRequest2(request->subcommandrequest, true);
	}
	return handleRequest2(request, mayHandleSubstack);
}

RCommandProxy *RKRBackend::handleRequest2(RBackendRequest *request, bool mayHandleSubstack) {
	RK_TRACE(RBACKEND);

	if ((!request->synchronous) && (!isKilled())) {
		RK_ASSERT(mayHandleSubstack); // i.e. not called from fetchNextCommand
		RK_ASSERT(!request->subcommandrequest);
		return nullptr;
	}

	int i = 0;
	while (!request->done) {
		if (killed) return nullptr;
		// NOTE: processX11Events() may, conceivably, lead to new requests, which may also wait for sub-commands!
		RKREventLoop::processX11Events();
		// NOTE: sleeping and waking up again can be surprisingly CPU-intensive (yes: more than the event processing, above. I have profiled it).
		// However, we really don't want to introduce too much delay, either.
		// Thus, the logic is this: If there was no reply within 2 seconds, then probably we're waiting for a user event, and can afford some more
		// latency (not too much, though, as we still need to process events).
		if (!request->done) RKRBackendProtocolBackend::msleep(++i < 200 ? 10 : 50);
	}

	RCommandProxy *command = request->takeCommand();
	if (!command) return nullptr;

	{
		QMutexLocker lock(&command_flow_mutex);
		RK_ASSERT(command != current_command);
		all_current_commands.append(command);
		current_command = command;
	}

	if (!mayHandleSubstack) return command;

	while (command) {
		runCommand(command);
		commandFinished(false);

		command = fetchNextCommand();
	};

	handleDeferredInterrupts();

	return nullptr;
}

RCommandProxy *RKRBackend::fetchNextCommand() {
	RK_TRACE(RBACKEND);

	RBackendRequest req(!isKilled(), RBackendRequest::CommandOut); // when killed, we do *not* actually wait for the reply, before the request is deleted.
	req.command = previous_command;
	previous_command = nullptr;

	return (handleRequest(&req, false));
}

GenericRRequestResult RKRBackend::doRCallRequest(const QString &call, const QVariant &params, RequestFlags flags) {
	RK_TRACE(RBACKEND);

	bool synchronous = flags != Asynchronous;
	RBackendRequest request(synchronous, RBackendRequest::RCallRequest);
	request.params[QStringLiteral("call")] = call;
	if (!params.isNull()) request.params[QStringLiteral("args")] = params;
	if (flags == SynchronousWithSubcommands) {
		request.params[QStringLiteral("cid")] = current_command->id;
		request.subcommandrequest = new RBackendRequest(true, RBackendRequest::OtherRequest);
	}
	handleRequest(&request);
	delete request.subcommandrequest;
	return request.getResult();
}

void RKRBackend::initialize(const QString &locale_dir, bool setup) {
	RK_TRACE(RBACKEND);

	// in RInterface::RInterface() we have created a fake RCommand to capture all the output/errors during startup. Fetch it
	repl_status.eval_depth++;
	fetchNextCommand();
	RK_ASSERT(current_command);

	startR();

	bool lib_load_fail = false;
	bool sink_fail = false;
	// Try to load rkward package. If that fails, or is the wrong version, try to install
	// rkward package, then load again.
	QString libloc = getLibLoc();
	QString versioncheck = QStringLiteral("stopifnot(.rk.app.version==\"%1\")").arg(QStringLiteral(RKWARD_VERSION));
	QString command = QStringLiteral("local({\n"
	                                 "  libloc <- ") +
	                  RKRSharedFunctionality::quote(libloc) + QStringLiteral("\n"
	                                                                         "  if (!dir.exists (libloc)) dir.create(libloc, recursive=TRUE)\n"
	                                                                         "  ok <- FALSE\n") +
	                  (setup ? u"# skipping: "_s : u""_s) +
	                  QStringLiteral(" suppressWarnings (try ({library (\"rkward\", lib.loc=libloc); ") +
	                  versioncheck +
	                  QStringLiteral("; ok <- TRUE}))\n"
	                                 "  if (!ok) {\n"
	                                 "    suppressWarnings (try (detach(\"package:rkward\", unload=TRUE)))\n"
	                                 "    install.packages(normalizePath(paste(libloc, \"..\", c (\"rkward.tgz\", \"rkwardtests.tgz\"), sep=\"/\")), lib=libloc, repos=NULL, type=\"source\", INSTALL_opts=\"--no-multiarch\")\n"
	                                 "    library (\"rkward\", lib.loc=libloc)\n"
	                                 "  }\n"
	                                 "  .libPaths(c(.libPaths(), libloc))\n" // Add to end search path: Will be available for help search, but hopefully, not get into the way, otherwise
	                                 "})\n");
	if (!runDirectCommand(command)) lib_load_fail = true;
	RK_setupGettext(locale_dir); // must happen *after* package loading, since R will re-set it
	if (!runDirectCommand(versioncheck + u"\n"_s)) lib_load_fail = true;
	if (!runDirectCommand(QStringLiteral(".rk.fix.assignments ()\n"))) sink_fail = true;

	QString error_messages;
	if (lib_load_fail) {
		error_messages.append(i18n("</p>\t- The 'rkward' R-library either could not be loaded at all, or not in the correct version. This may lead to all sorts of errors, from single missing features to complete failure to function. The most likely cause is that the last installation did not place all files in the correct place. However, in some cases, left-overs from a previous installation that was not cleanly removed may be the cause.</p>\
		<p><b>You should quit RKWard, now, and fix your installation</b>. For help with that, see <a href=\"https://rkward.kde.org/compiling\">https://rkward.kde.org/compiling</a>.</p>\n"));
	}
	if (sink_fail) {
		error_messages.append(i18n("<p>\t-There was a problem setting up the communication with R. Most likely this indicates a broken installation.</p>\
		<p><b>You should quit RKWard, now, and fix your installation</b>. For help with that, see <a href=\"https://rkward.kde.org/compiling\">https://rkward.kde.org/compiling</a>.</p></p>\n"));
	}

	RBackendRequest req(true, RBackendRequest::Started);
	req.params[QStringLiteral("message")] = QVariant(error_messages);
	// blocks until RKWard is set to go (esp, it has displayed startup error messages, etc.)
	// in fact, a number of sub-commands are run while handling this request!
	handleRequest(&req);

	RK_ASSERT(current_command);
	commandFinished(); // the fake startup command
	repl_status.eval_depth--;
}

void RKRBackend::checkObjectUpdatesNeeded(bool check_list) {
	RK_TRACE(RBACKEND);
	if (killed) return;

	if (check_list) {
		// TODO: avoid parsing this over and over again
		RK_DEBUG(RBACKEND, DL_TRACE, "checkObjectUpdatesNeeded: getting search list");
		RCommandProxy *dummy = runDirectCommand(QStringLiteral("list(search(), loadedNamespaces())\n"), RCommand::GetStructuredData);
		QStringList n_toplevel_env_names = dummy->structureVector().value(0)->stringVector();
		QStringList n_loaded_namespaces = dummy->structureVector().value(1)->stringVector();
		delete dummy;
		if (n_toplevel_env_names != toplevel_env_names || n_loaded_namespaces != loaded_namespaces) { // this includes an update of the globalenv, even if not needed
			toplevel_env_names = n_toplevel_env_names;
			loaded_namespaces = n_loaded_namespaces;
			QVariantList args;
			args.append(QVariant(toplevel_env_names));
			args.append(QVariant(loaded_namespaces));
			doRCallRequest(QStringLiteral("syncenvs"), args, SynchronousWithSubcommands);
		}
	}

	auto changes = RKRShadowEnvironment::diffAndUpdate(ROb(R_GlobalEnv));
	if (!changes.isEmpty()) {
		QVariantList args;
		args.append(changes.added);
		args.append(changes.removed);
		args.append(changes.changed);
		doRCallRequest(QStringLiteral("sync"), args, SynchronousWithSubcommands);
	}
}

bool RKRBackend::doMSleep(int msecs) {
	// do not trace!
	if (isKilled()) return false;
	RKRBackendProtocolBackend::msleep(msecs);
	return true;
}

bool RKRBackend::graphicsEngineMismatchMessage(int compiled_version, int runtime_version) {
	static bool shown = false;
	if (!shown) {
		shown = true;
		doDialogHelper(i18n("Graphics version mismatch"), i18n("R Graphics Engine version has changed (from %1 to %2).<br>This change requires a recompilation.<br><a href=\"rkward://page/rkward_incompatible_version\">Additional information</a>", compiled_version, runtime_version), i18n("Ok"), QString(), QString(), QString(), true);
	}
	return false;
}
