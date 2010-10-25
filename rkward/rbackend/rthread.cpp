/***************************************************************************
                          rthread  -  description
                             -------------------
    begin                : Mon Aug 2 2004
    copyright            : (C) 2004, 2006, 2007, 2008, 2009, 2010 by Thomas Friedrichsmeier
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

#include "rinterface.h"
#include "rcommandstack.h"
#include "../settings/rksettingsmoduler.h"
#include "../settings/rksettingsmoduleoutput.h"
#include "../settings/rksettingsmodulegraphics.h"
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

	commandFinished ();		// the fake startup command

	enterEventLoop ();
}

void RThread::commandFinished (bool check_object_updates_needed) {
	RK_TRACE (RBACKEND);

	RK_DO (qDebug ("done running command"), RBACKEND, DL_DEBUG);
	MUTEX_LOCK;
	current_command->status -= (current_command->status & RCommand::Running);
	current_command->status |= RCommand::WasTried;
	RCommandStackModel::getModel ()->itemChange (current_command);

	if (check_object_updates_needed || (current_command->type () & RCommand::ObjectListUpdate)) {
		checkObjectUpdatesNeeded (current_command->type () & (RCommand::User | RCommand::ObjectListUpdate));
	}
	RCommandStack::currentStack ()->pop ();
	notifyCommandDone (current_command);	// command may be deleted after this

	all_current_commands.pop_back();
	if (!all_current_commands.isEmpty ()) current_command = all_current_commands.last ();
	MUTEX_UNLOCK;
}

RCommand* RThread::fetchNextCommand (RCommandStack* stack) {
	RK_TRACE (RBACKEND);

	bool main_stack = (stack == RCommandStack::regular_stack);
#warning We would need so much less mutex locking everywhere, if the command would simply be copied between threads!
	while (1) {
		if (killed) {
			return 0;
		}

		processX11Events ();

		MUTEX_LOCK;
		if (previously_idle) {
			if (!RCommandStack::regular_stack->isEmpty ()) {
				qApp->postEvent (RKGlobals::rInterface (), new RKRBackendEvent (RKRBackendEvent::RBusy));
				previously_idle = false;
			}
		}

		if ((!(main_stack && locked)) && stack->isActive ()) {		// do not respect locks in substacks
			RCommand *command = stack->currentCommand ();

			if (command) {
				// notify GUI-thread that a new command is being tried and initialize
				RKRBackendEvent* event = new RKRBackendEvent (RKRBackendEvent::RCommandIn, command);
				qApp->postEvent (RKGlobals::rInterface (), event);
				all_current_commands.append (command);
				current_command = command;

				if ((command->type () & RCommand::EmptyCommand) || (command->status & RCommand::Canceled) || (command->type () & RCommand::QuitCommand)) {
					// some commands are not actually run by R, but handled inline, here
					if (command->status & RCommand::Canceled) {
						command->status |= RCommand::Failed;
					} else if (command->type () & RCommand::QuitCommand) {
						killed = true;
						MUTEX_UNLOCK;
						shutdown (false);
						MUTEX_LOCK;		// I guess we don't get here, though?
					}
					commandFinished ();
				} else {
					RK_DO (qDebug ("running command: %s", command->command ().toLatin1().data ()), RBACKEND, DL_DEBUG);
				
					command->status |= RCommand::Running;	// it is important that this happens before the Mutex is unlocked!
					RCommandStackModel::getModel ()->itemChange (command);

					MUTEX_UNLOCK;
					return command;
				}
			}
		}

		if ((!stack->isActive ()) && stack->isEmpty () && !main_stack) {
			MUTEX_UNLOCK;
			return 0;		// substack depleted
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
			return 0;
		}
		msleep (10);
	}

	return 0;
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

void RThread::handleSubstackCall (QStringList &call) {
	RK_TRACE (RBACKEND);

	if (call.count () == 2) {		// schedule symbol update for later
		if (call[0] == "ws") {
			// always keep in mind: No current command can happen for tcl/tk events.
			if ((!current_command) || (current_command->type () & RCommand::ObjectListUpdate) || (!(current_command->type () & RCommand::Sync))) {		// ignore Sync commands that are not flagged as ObjectListUpdate
				if (!changed_symbol_names.contains (call[1])) changed_symbol_names.append (call[1]);
			}
			return;
		}
	}

	REvalRequest request;
	request.call = call;
	MUTEX_LOCK;
	flushOutput ();
	RCommandStack *reply_stack = new RCommandStack (current_command);
	request.in_chain = reply_stack->startChain (reply_stack);
	MUTEX_UNLOCK;

	RKRBackendEvent* event = new RKRBackendEvent (RKRBackendEvent::REvalRequest, &request);
	qApp->postEvent (RKGlobals::rInterface (), event);

	RCommand *c;
	while ((c = fetchNextCommand (reply_stack))) {
		MUTEX_LOCK;
		runCommand (c);
		MUTEX_UNLOCK;
		commandFinished (false);
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

		if (!(locked || killed)) processX11Events ();
	}

	RK_DO (qDebug ("standard callback done"), RBACKEND, DL_DEBUG);
}

int RThread::initialize () {
	RK_TRACE (RBACKEND);

	int argc = 2;
	char* argv[2] = { qstrdup ("--slave"), qstrdup ("--no-save") };
	RKWardStartupOptions *options = RKWardMainWindow::getStartupOptions ();
	RK_ASSERT (options);

	startR (argc, argv, !(options->no_stack_check));

	connectCallbacks ();

	int status = 0;

	if (!runDirectCommand ("library (\"rkward\")\n")) status |= LibLoadFail;
	if (!runDirectCommand (QString ("stopifnot(.rk.app.version==\"%1\")\n").arg (VERSION))) status |= LibLoadFail;
	if (!runDirectCommand (".rk.fix.assignments ()\n")) status |= LibLoadFail;

// find out about standard library locations
	RCommand *dummy = runDirectCommand (".libPaths ()\n", RCommand::GetStringVector);
	if (dummy->failed ()) status |= OtherFail;
	for (unsigned int i = 0; i < dummy->getDataLength (); ++i) {
		RKSettingsModuleRPackages::defaultliblocs.append (dummy->getStringVector ()[i]);
	}
	delete dummy;

// start help server / determined help base url
	dummy = runDirectCommand (".rk.getHelpBaseUrl ()\n", RCommand::GetStringVector);
	if (dummy->failed ()) status |= OtherFail;
	else {
		RK_ASSERT (dummy->getDataLength () == 1);
		RKSettingsModuleR::help_base_url = dummy->getStringVector ()[0];
	}
	delete dummy;

// apply user configurable run time options
	QStringList commands = RKSettingsModuleR::makeRRunTimeOptionCommands () + RKSettingsModuleRPackages::makeRRunTimeOptionCommands () + RKSettingsModuleOutput::makeRRunTimeOptionCommands () + RKSettingsModuleGraphics::makeRRunTimeOptionCommands ();
	for (QStringList::const_iterator it = commands.begin (); it != commands.end (); ++it) {
		if (!runDirectCommand ((*it).toLocal8Bit ())) {
			status |= OtherFail;
			RK_DO (qDebug ("error in initialization call '%s'", (*it).toLatin1().data ()), RBACKEND, DL_ERROR);
		}
	}

// error/output sink and help browser
	if (!runDirectCommand ("options (error=quote (.rk.do.error ()))\n")) status |= SinkFail;
	if (!runDirectCommand ("rk.set.output.html.file (\"" + RKSettingsModuleGeneral::filesPath () + "/rk_out.html\")\n")) status |= SinkFail;

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

	if (check_list) {	
	// TODO: avoid parsing this over and over again
		RK_DO (qDebug ("checkObjectUpdatesNeeded: getting search list"), RBACKEND, DL_TRACE);
		RCommand *dummy = runDirectCommand ("search ()\n", RCommand::GetStringVector);
		if ((int) dummy->getDataLength () != toplevel_env_names.count ()) {
			search_update_needed = true;
		} else {
			for (unsigned int i = 0; i < dummy->getDataLength (); ++i) {
				// order is important in the search path
				if (toplevel_env_names[i] != dummy->getStringVector ()[i]) {
					search_update_needed = true;
					break;
				}
			}
		}
		if (search_update_needed) {
			toplevel_env_names.clear ();
			for (unsigned int i = 0; i < dummy->getDataLength (); ++i) {
				toplevel_env_names.append (dummy->getStringVector ()[i]);
			}
		}
		delete dummy;
	
	// TODO: avoid parsing this over and over again
		RK_DO (qDebug ("checkObjectUpdatesNeeded: getting globalenv symbols"), RBACKEND, DL_TRACE);
		dummy = runDirectCommand ("ls (globalenv (), all.names=TRUE)\n", RCommand::GetStringVector);
		if ((int) dummy->getDataLength () != global_env_toplevel_names.count ()) {
			globalenv_update_needed = true;
		} else {
			for (unsigned int i = 0; i < dummy->getDataLength (); ++i) {
				// order is not important in the symbol list
				if (!global_env_toplevel_names.contains (dummy->getStringVector ()[i])) {
					globalenv_update_needed = true;
					break;
				}
			}
		}
		if (globalenv_update_needed) {
			global_env_toplevel_names.clear ();
			for (unsigned int i = 0; i < dummy->getDataLength (); ++i) {
				global_env_toplevel_names.append (dummy->getStringVector ()[i]);
			}
		}
		delete dummy;
	
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
		runDirectCommand (".rk.watch.globalenv ()\n");
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
