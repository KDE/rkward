/***************************************************************************
                          rthread  -  description
                             -------------------
    begin                : Mon Aug 2 2004
    copyright            : (C) 2004, 2006, 2007, 2008, 2009 by Thomas Friedrichsmeier
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
#include "rthread.h"

#include "rinterface.h"
#include "rcommandstack.h"
#include "../settings/rksettingsmoduler.h"
#include "../settings/rksettingsmodulegeneral.h"
#include "../rkglobals.h"
#include "../rkward.h"		// for startup options
#include "../version.h"

#include "../debug.h"

#include <kapplication.h>
#include <klocale.h>

#include <qstring.h>
#include <QList>
#include <QFileInfo>

#ifndef Q_WS_WIN
#	include <signal.h>		// needed for pthread_kill
#	include <pthread.h>		// seems to be needed at least on FreeBSD
#endif

#define MAX_BUF_LENGTH 4000

RThread::RThread () : QThread (), REmbedInternal () {
	RK_TRACE (RBACKEND);
	current_command = 0;

	RK_ASSERT (this_pointer == 0);
	this_pointer = this;
	RK_ASSERT (this_pointer);
	current_output = 0;
	out_buf_len = 0;
	output_paused = false;

#ifdef Q_WS_WIN
	// we hope that on other platforms the default is reasonable
	setStackSize (0xa00000);	// 10MB as recommended by r_exts-manual
#endif
}

RThread::~RThread() {
	RK_TRACE (RBACKEND);
}

void RThread::interruptProcessing (bool interrupt) {
	if (!interrupt) return;
#ifdef Q_WS_WIN
	RK_scheduleIntr ();
#else
	pthread_kill ((pthread_t) thread_id, SIGINT);
#endif
}

void RThread::run () {
	RK_TRACE (RBACKEND);
	thread_id = currentThreadId ();
	locked = Startup;
	killed = false;
	int err;
	bool previously_idle = false;

	// in RInterface::RInterface() we create a fake RCommand to capture all the output/errors during startup
	MUTEX_LOCK;
	current_command = RCommandStack::regular_stack->currentCommand ();
	all_current_commands.append (current_command);
	RK_ASSERT (current_command);
	MUTEX_UNLOCK;

	if ((err = initialize ())) {
		int* err_c = new int;
		*err_c = err;
		flushOutput ();		// to make errors/warnings available to the main thread
		qApp->postEvent (RKGlobals::rInterface (), new RKRBackendEvent (RKRBackendEvent::RStartupError, err_c));
	}
	qApp->postEvent (RKGlobals::rInterface (), new RKRBackendEvent (RKRBackendEvent::RStarted));

	// wait until RKWard is set to go (esp, it has handled any errors during startup, etc.)
	while (locked) {
		msleep (10);
	}

	MUTEX_LOCK;
	checkObjectUpdatesNeeded (true);
	RCommandStack::regular_stack->pop ();	// remove the fake command
	all_current_commands.pop_back ();
	notifyCommandDone (current_command);
	MUTEX_UNLOCK;

	while (1) {
		MUTEX_LOCK;
		processX11Events ();

		if (previously_idle) {
			if (!RCommandStack::regular_stack->isEmpty ()) {
				qApp->postEvent (RKGlobals::rInterface (), new RKRBackendEvent (RKRBackendEvent::RBusy));
				previously_idle = false;
			}
		}
	
		// while commands are in queue, don't wait
		while ((!locked) && RCommandStack::regular_stack->isActive ()) {
			current_command = RCommandStack::regular_stack->currentCommand ();
			
			if (current_command) {
				// mutex will be unlocked inside
				doCommand (current_command);
				checkObjectUpdatesNeeded (current_command->type () & (RCommand::User | RCommand::ObjectListUpdate));
				processX11Events ();
				RCommandStack::regular_stack->pop ();
				notifyCommandDone (current_command);	// command may be deleted after this
			}
		
			if (killed) {
				shutdown (false);
				MUTEX_UNLOCK;
				return;
			}
		}

		if (!previously_idle) {
			if (RCommandStack::regular_stack->isEmpty ()) {
				qApp->postEvent (RKGlobals::rInterface (), new RKRBackendEvent (RKRBackendEvent::RIdle));
				previously_idle = true;
			}
		}
		
		// if no commands are in queue, sleep for a while
		MUTEX_UNLOCK;
		if (killed) {
			shutdown (false);
			return;
		}
		current_command = 0;
		msleep (10);
	}
}

void RThread::doCommand (RCommand *command) {
	RK_TRACE (RBACKEND);
	// step 1: notify GUI-thread that a new command is being tried and initialize
	RKRBackendEvent* event = new RKRBackendEvent (RKRBackendEvent::RCommandIn, command);
	qApp->postEvent (RKGlobals::rInterface (), event);

	// step 2: actual handling
	if (!((command->type () & RCommand::EmptyCommand) || (command->status & RCommand::Canceled))) {
		all_current_commands.append (command);
		RKWardRError error;
		
		int ctype = command->type ();
		QString ccommand = command->command ();		// easier typing below
		
		RK_DO (qDebug ("running command: %s", ccommand.toLatin1().data ()), RBACKEND, DL_DEBUG);
	
		command->status |= RCommand::Running;	// it is important that this happens before the Mutex is unlocked!
		RCommandStackModel::getModel ()->itemChange (command);

		MUTEX_UNLOCK;
		if (ctype & RCommand::GetStringVector) {
			command->datatype = RData::StringVector;
			command->data = getCommandAsStringVector (ccommand, &(command->length), &error);
		} else if (ctype & RCommand::GetRealVector) {
			command->datatype = RData::RealVector;
			command->data = getCommandAsRealVector (ccommand, &(command->length), &error);
		} else if (ctype & RCommand::GetIntVector) {
			command->datatype = RData::IntVector;
			command->data = getCommandAsIntVector (ccommand, &(command->length), &error);
		} else if (ctype & RCommand::GetStructuredData) {
			RData *data = getCommandAsRData (ccommand, &error);
			if (data) command->setData (data);
		} else {
			runCommandInternal (ccommand, &error, ctype & RCommand::User);
		}
		MUTEX_LOCK;

		#ifdef RKWARD_DEBUG
			int dl = DL_WARNING;		// failed application commands are an issue worth reporting, failed user commands are not
			if (command->type () | RCommand::User) dl = DL_DEBUG;
		#endif
		if (error != NoError) {
			command->status |= RCommand::WasTried | RCommand::Failed;
			if (error == Incomplete) {
				command->status |= RCommand::ErrorIncomplete;
				RK_DO (qDebug ("Command failed (incomplete)"), RBACKEND, dl);
			} else if (error == SyntaxError) {
				command->status |= RCommand::ErrorSyntax;
				RK_DO (qDebug ("Command failed (syntax)"), RBACKEND, dl);
			} else if (command->status & RCommand::Canceled) {
				RK_DO (qDebug ("Command failed (interrupted)"), RBACKEND, dl);
			} else {
				command->status |= RCommand::ErrorOther;
				#ifdef RKWARD_DEBUG
					dl = DL_WARNING;		// always interested in strange errors
				#endif
				RK_DO (qDebug ("Command failed (other)"), RBACKEND, dl);
			}
			RK_DO (qDebug ("failed command was: '%s'", command->command ().toLatin1 ().data ()), RBACKEND, dl);
		} else {
			command->status |= RCommand::WasTried;
		}

		flushOutput ();
		if (command->type () & RCommand::DirectToOutput) {
			QString outp = command->fullOutput();

			if (!outp.isEmpty ()) {
				// all regular output was sink()ed, i.e. all remaining output is a message/warning/error
				RKWardRError error;
				runCommandInternal (".rk.cat.output (\"<h2>Messages, warnings, or errors:</h2>\\n\")", &error, false);
				RK_ASSERT (!error);

				outp.replace ('\\', "\\\\");
				outp.replace ('"', "\\\"");
				runCommandInternal ("rk.print.literal (\"" + outp + "\")", &error, false);
				RK_ASSERT (!error);
			}
		}
		if (!(command->type () & RCommand::Sync)) {
			MUTEX_UNLOCK;
			checkNotifyOutputTouched ();
			MUTEX_LOCK;
		}
	
		if (error) {
			RK_DO (qDebug ("- error message was: '%s'", command->error ().toLatin1 ().data ()), RBACKEND, dl);
	//		runCommandInternal (".rk.init.handlers ()\n", &dummy);
		}
		RK_DO (qDebug ("done running command"), RBACKEND, DL_DEBUG);
		all_current_commands.pop_back();
	} else {
		if (command->status & RCommand::Canceled) {
			command->status |= RCommand::Failed;
		} else if (command->type () & RCommand::QuitCommand) {
			killed = true;
			MUTEX_UNLOCK;
			shutdown (false);
			MUTEX_LOCK;
		}
	}

	command->status -= (command->status & RCommand::Running);
	RCommandStackModel::getModel ()->itemChange (command);
}


void RThread::notifyCommandDone (RCommand *command) {
	RK_TRACE (RBACKEND);

	RK_ASSERT (command == current_command);
	current_command = 0;

	// notify GUI-thread that command was finished
	RKRBackendEvent* event = new RKRBackendEvent (RKRBackendEvent::RCommandOut, command);
	qApp->postEvent (RKGlobals::rInterface (), event);
}

void RThread::waitIfOutputPaused () {
	// don't trace
	while (output_paused) {
		msleep (10);
	}
}

void RThread::handleOutput (const QString &output, int buf_length, bool regular) {
	RK_TRACE (RBACKEND);

// TODO: output sometimes arrives in small chunks. Maybe it would be better to keep an internal buffer, and only append it to the output, when R_FlushConsole gets called?
	if (!buf_length) return;
	waitIfOutputPaused ();

	MUTEX_LOCK;
	ROutput::ROutputType output_type;
	if (regular) {
		output_type = ROutput::Output;
	} else {
		output_type = ROutput::Warning;
	}

	if (current_output) {
		if (current_output->type != output_type) {
			flushOutput ();
		}
	}
	if (!current_output) {	// not an else, might have been set to 0 in the above if
		current_output = new ROutput;
		current_output->type = output_type;
		current_output->output.reserve (MAX_BUF_LENGTH + 50);
	}
	current_output->output.append (output);

	if ((out_buf_len += buf_length) > MAX_BUF_LENGTH) {
		RK_DO (qDebug ("Output buffer has %d characters. Forcing flush", out_buf_len), RBACKEND, DL_DEBUG);
		flushOutput ();
	}
	MUTEX_UNLOCK;
}

void RThread::flushOutput () {
	if (!current_output) return;		// avoid creating loads of traces
	RK_TRACE (RBACKEND);

	if (current_command) {
		for (QList<RCommand*>::const_iterator it = all_current_commands.constBegin (); it != all_current_commands.constEnd(); ++it) {
			ROutput *output = current_output;
			if ((*it) != current_command) {		// this output belongs to several commands at once. So we need to copy it.
				output = new ROutput;
				output->type = current_output->type;
				output->output = current_output->output;
			}

			(*it)->output_list.append (output);
			if (output->type == ROutput::Output) {
				(*it)->status |= RCommand::HasOutput;
			} else if (output->type == ROutput::Warning) {
				(*it)->status |= RCommand::HasWarnings;
			} else if (output->type == ROutput::Error) {
				(*it)->status |= RCommand::HasError;
			}

			// pass a signal to the main thread for real-time update of output
			ROutputContainer *outc = new ROutputContainer;
			outc->output = output;
			outc->command = *it;
			RKRBackendEvent* event = new RKRBackendEvent (RKRBackendEvent::RCommandOutput, outc);
			qApp->postEvent (RKGlobals::rInterface (), event);
		}

		RK_DO (qDebug ("output '%s'", current_output->output.toLatin1 ().data ()), RBACKEND, DL_DEBUG);
	} else {
		// running Rcmdr, eh?
		RK_DO (qDebug ("output without receiver'%s'", current_output->output.toLatin1 ().data ()), RBACKEND, DL_WARNING);
		delete current_output;
	}

// forget output
	current_output = 0;
	out_buf_len = 0;
}

/*
void RThread::handleCondition (char **call, int call_length) {
	RK_TRACE (RBACKEND);

	RK_ASSERT (call_length >= 2);
	if (!call_length) return;

	//RThreadInternal::next_output_is_error = true;
	qDebug ("condition '%s', message '%s'", call[0], call[1]);
} */

void RThread::handleError (QString *call, int call_length) {
	RK_TRACE (RBACKEND);

	if (!call_length) return;
	waitIfOutputPaused ();

	MUTEX_LOCK;
	// Unfortunately, errors still get printed to the output, UNLESS a sink() is in place. We try this crude method for the time being:
	flushOutput ();
	if (current_command) {
		if (!current_command->output_list.isEmpty ()) {
			if (current_command->output_list.last ()->output == call[0]) {
				current_command->output_list.last ()->type = ROutput::Error;
			}
		}
		current_command->status |= RCommand::HasError;
	}

	RK_DO (qDebug ("error '%s'", call[0].toLatin1 ().data ()), RBACKEND, DL_DEBUG);
	MUTEX_UNLOCK;
}

void RThread::checkNotifyOutputTouched () {
	RK_TRACE (RBACKEND);

	QFileInfo info (active_output_file);
	if (info.exists ()) {
		if ((!output_last_modified.isValid ()) || (output_last_modified < info.lastModified ())) {
			output_last_modified = info.lastModified ();
			handleSubstackCall (QStringList () << "refreshOutput" << active_output_file);
		}
	}
}

void RThread::handleSubstackCall (QStringList &call) {
	RK_TRACE (RBACKEND);

	if (call.count () == 2) {		// schedule symbol update for later
		if (call[0] == "ws") {
			RK_ASSERT (current_command);
			if ((current_command->type () & RCommand::ObjectListUpdate) || (!(current_command->type () & RCommand::Sync))) {		// ignore Sync commands that are not flagged as ObjectListUpdate
				if (!changed_symbol_names.contains (call[1])) changed_symbol_names.append (call[1]);
			}
			return;
		} else if (call[0] == "set.output.file") {
			checkNotifyOutputTouched ();
			active_output_file = call[1];
			output_last_modified = QFileInfo (active_output_file).lastModified ();
			return;
		}
	}

	RCommand *prev_command = current_command;
	REvalRequest request;
	request.call = call;
	MUTEX_LOCK;
	flushOutput ();
	RCommandStack *reply_stack = new RCommandStack (prev_command);
	request.in_chain = reply_stack->startChain (reply_stack);
	MUTEX_UNLOCK;

	RKRBackendEvent* event = new RKRBackendEvent (RKRBackendEvent::REvalRequest, &request);
	qApp->postEvent (RKGlobals::rInterface (), event);
	
	bool done = false;
	while (!done) {
		MUTEX_LOCK;
		processX11Events ();
		// while commands are in queue, don't wait
		while (reply_stack->isActive ()) {		// substack calls are to be considered "sync", and don't respect locks
			if (killed) {
				done = true;
				break;
			}

			current_command = reply_stack->currentCommand ();
			if (current_command) {
				// mutex will be unlocked inside
				bool object_update_forced = (current_command->type () & RCommand::ObjectListUpdate);
				doCommand (current_command);
				if (!(locked || killed)) processX11Events ();
				else msleep (100);
				if (object_update_forced) checkObjectUpdatesNeeded (true);
				reply_stack->pop ();
				notifyCommandDone (current_command);	// command may be deleted after this
			} else {
				msleep (10);
			}
		}

		if (reply_stack->isEmpty ()) {
			done = true;
		}
		MUTEX_UNLOCK;

		// if no commands are in queue, sleep for a while
		current_command = prev_command;
		msleep (10);
	}

	MUTEX_LOCK;
	delete reply_stack;
	MUTEX_UNLOCK;
}

void RThread::handleStandardCallback (RCallbackArgs *args) {
	RK_TRACE (RBACKEND);

	MUTEX_LOCK;
	flushOutput ();
	MUTEX_UNLOCK;
	args->done = false;

	RKRBackendEvent* event = new RKRBackendEvent (RKRBackendEvent::RCallbackRequest, args);
	qApp->postEvent (RKGlobals::rInterface (), event);
	
	bool *done = &(args->done);
	while (!(*done)) {
		msleep (10); // callback not done yet? Sleep for a while

		if (!(locked || killed)) {			// what's with that lock? If the current command is cancelled, while we're in this loop, we must not lock the mutex and/or call anything in R. We may get long-jumped out of the loop before we get a chance to unlock
			MUTEX_LOCK;
			if (!(locked || killed)) processX11Events ();
			MUTEX_UNLOCK;
		}
	}

	RK_DO (qDebug ("standard callback done"), RBACKEND, DL_DEBUG);
}

void RThread::currentCommandWasCancelled () {
	RK_TRACE (RBACKEND);

	RK_ASSERT (current_command);
	current_command->status |= RCommand::Canceled;
}

int RThread::initialize () {
	RK_TRACE (RBACKEND);

	int argc = 2;
	char* argv[2] = { qstrdup ("--slave"), qstrdup ("--no-save") };
	RKWardStartupOptions *options = RKWardMainWindow::getStartupOptions ();
	RK_ASSERT (options);

	startR (argc, argv, !(options->no_stack_check));

	connectCallbacks ();

	RKWardRError error;
	int status = 0;
	
	runCommandInternal ("library (\"rkward\")\n", &error);
	if (error) status |= LibLoadFail;
	runCommandInternal (QString ("stopifnot(.rk.app.version==\"%1\")\n").arg (VERSION), &error);
	if (error) status |= LibLoadFail;

// find out about standard library locations
	unsigned int c;
	QString *standardliblocs = getCommandAsStringVector (".libPaths ()\n", &c, &error);
	if (error) status |= OtherFail;
	for (unsigned int i = 0; i < c; ++i) {
		RKSettingsModuleRPackages::defaultliblocs.append (standardliblocs[i]);
	}
	delete [] standardliblocs;

// apply user configurable run time options
	QStringList commands = RKSettingsModuleR::makeRRunTimeOptionCommands () + RKSettingsModuleRPackages::makeRRunTimeOptionCommands ();
	for (QStringList::const_iterator it = commands.begin (); it != commands.end (); ++it) {
		runCommandInternal ((*it).toLocal8Bit (), &error);
		if (error) {
			status |= OtherFail;
			RK_DO (qDebug ("error in initialization call '%s'", (*it).toLatin1().data ()), RBACKEND, DL_ERROR);
		}
	}

// error/output sink and help browser
	runCommandInternal ("options (error=quote (.rk.do.error ()))\n", &error);
	if (error) status |= SinkFail;
	runCommandInternal ("rk.set.output.html.file (\"" + RKSettingsModuleGeneral::filesPath () + "/rk_out.html\")\n", &error);
	if (error) status |= SinkFail;

	MUTEX_LOCK;
	flushOutput ();
	MUTEX_UNLOCK;

	return status;
}

void RThread::checkObjectUpdatesNeeded (bool check_list) {
	RK_TRACE (RBACKEND);
	if (killed) return;

	/* NOTE: We're keeping separate lists of the items on the search path, and the toplevel symbols in .GlobalEnv here.
	This info is also present in RObjectList (and it's children). However: a) in a less convenient form, b) in the other thread. To avoid locking, and other complexity, keeping separate lists seems an ok solution. Keep in mind that only the names of only the toplevel objects are kept, here, so the memory overhead should be minimal */

	bool search_update_needed = false;
	bool globalenv_update_needed = false;
	RKWardRError error;

	if (check_list) {
		unsigned int count;
		QString *strings;
	
	// TODO: avoid parsing this over and over again
		RK_DO (qDebug ("checkObjectUpdatesNeeded: getting search list"), RBACKEND, DL_TRACE);
		strings = getCommandAsStringVector ("search ()\n", &count, &error);
		if ((int) count != toplevel_env_names.count ()) {
			search_update_needed = true;
		} else {
			for (unsigned int i = 0; i < count; ++i) {
				// order is important in the search path
				if (toplevel_env_names[i] != strings[i]) {
					search_update_needed = true;
					break;
				}
			}
		}
		if (search_update_needed) {
			toplevel_env_names.clear ();
			for (unsigned int i = 0; i < count; ++i) {
				toplevel_env_names.append (strings[i]);
			}
		}
		delete [] strings;
	
	// TODO: avoid parsing this over and over again
		RK_DO (qDebug ("checkObjectUpdatesNeeded: getting globalenv symbols"), RBACKEND, DL_TRACE);
		strings = getCommandAsStringVector ("ls (globalenv (), all.names=TRUE)\n", &count, &error);
		if ((int) count != global_env_toplevel_names.count ()) {
			globalenv_update_needed = true;
		} else {
			for (unsigned int i = 0; i < count; ++i) {
				// order is not important in the symbol list
				if (!global_env_toplevel_names.contains (strings[i])) {
					globalenv_update_needed = true;
					break;
				}
			}
		}
		if (globalenv_update_needed) {
			global_env_toplevel_names.clear ();
			for (unsigned int i = 0; i < count; ++i) {
				global_env_toplevel_names.append (strings[i]);
			}
		}
		delete [] strings;
	
		if (search_update_needed) {	// this includes an update of the globalenv, even if not needed
			MUTEX_UNLOCK;
			QStringList call = toplevel_env_names;
			call.prepend ("syncenvs");	// should be faster than the reverse
			handleSubstackCall (call);
			MUTEX_LOCK;
		} 
		if (globalenv_update_needed) {
			MUTEX_UNLOCK;
			QStringList call = global_env_toplevel_names;
			call.prepend ("syncglobal");	// should be faster than the reverse
			handleSubstackCall (call);
			MUTEX_LOCK;
		}
	}

	if (search_update_needed || globalenv_update_needed) {
		RK_DO (qDebug ("checkObjectUpdatesNeeded: updating watches"), RBACKEND, DL_TRACE);
		runCommandInternal (".rk.watch.globalenv ()\n", &error);
	}

	if (!changed_symbol_names.isEmpty ()) {
		QStringList call = changed_symbol_names;
		call.prepend (QString ("sync"));	// should be faster than reverse
		MUTEX_UNLOCK;
		handleSubstackCall (call);
		MUTEX_LOCK;
		changed_symbol_names.clear ();
	}
}
