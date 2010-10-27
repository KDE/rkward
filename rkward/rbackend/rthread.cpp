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

#define MAX_BUF_LENGTH 16000
#define OUTPUT_STRING_RESERVE 1000

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
	killed = false;
	int err;

	// in RInterface::RInterface() we have created a fake RCommand to capture all the output/errors during startup. Fetch it
	fetchNextCommand ();

	if ((err = initialize ())) {
		int* err_c = new int;
		*err_c = err;
		qApp->postEvent (RKGlobals::rInterface (), new RKRBackendEvent (RKRBackendEvent::RStartupError, err_c));
	}
	bool startup_was_handled = false;
	qApp->postEvent (RKGlobals::rInterface (), new RKRBackendEvent (RKRBackendEvent::RStarted, &startup_was_handled));

	// wait until RKWard is set to go (esp, it has handled any errors during startup, etc.)
	while (!startup_was_handled) {
		msleep (10);
	}

	commandFinished ();		// the fake startup command

	enterEventLoop ();
}

void RThread::commandFinished (bool check_object_updates_needed) {
	RK_TRACE (RBACKEND);

	RK_DO (qDebug ("done running command"), RBACKEND, DL_DEBUG);

	current_command->status -= (current_command->status & RCommand::Running);
	current_command->status |= RCommand::WasTried;

	if (check_object_updates_needed || (current_command->type & RCommand::ObjectListUpdate)) {
		checkObjectUpdatesNeeded (current_command->type & (RCommand::User | RCommand::ObjectListUpdate));
	}

	RKRBackendEvent* event = new RKRBackendEvent (RKRBackendEvent::RCommandOut, current_command);
	qApp->postEvent (RKGlobals::rInterface (), event);		// command may be deleted after this!

	all_current_commands.pop_back();
	if (!all_current_commands.isEmpty ()) current_command = all_current_commands.last ();
}

RCommandProxy* RThread::fetchNextCommand () {
	RK_TRACE (RBACKEND);

	RNextCommandRequest req;
	bool done = false;
	req.done = &done;
	req.command = 0;

	RKRBackendEvent* event = new RKRBackendEvent (RKRBackendEvent::RNextCommandRequest, &req);
	qApp->postEvent (RKGlobals::rInterface (), event);

	while (!done) {
		if (killed) return 0;
		processX11Events ();
		if (!done) msleep (10);
	}

	RCommandProxy *command = req.command;
	if (command) {
		all_current_commands.append (command);
		current_command = command;
	}

	return command;
}

void RThread::waitIfOutputBufferExceeded () {
	// don't trace
	while (out_buf_len > MAX_BUF_LENGTH) {
		msleep (10);
	}
}

void RThread::handleOutput (const QString &output, int buf_length, ROutput::ROutputType output_type) {
	RK_TRACE (RBACKEND);

	if (!buf_length) return;
	RK_DO (qDebug ("Output type %d: ", output_type, qPrintable (output)), RBACKEND, DL_DEBUG);
	waitIfOutputBufferExceeded ();

	output_buffer_mutex.lock ();

	ROutput *current_output = 0;
	if (!output_buffer.isEmpty ()) {
		// Merge with previous output fragment, if of the same type
		current_output = output_buffer.last ();
		if (current_output->type != output_type) current_output = 0;
	}
	if (!current_output) {
		current_output = new ROutput;
		current_output->type = output_type;
		current_output->output.reserve (OUTPUT_STRING_RESERVE);
		output_buffer.append (current_output);
	}
	current_output->output.append (output);
	out_buf_len += buf_length;

	output_buffer_mutex.unlock ();
}

ROutputList RThread::flushOutput (bool forcibly) {
	ROutputList ret;

	if (out_buf_len == 0) return ret;		// if there is absolutely no output, just skip.
	RK_TRACE (RBACKEND);

	if (!forcibly) {
		if (!output_buffer_mutex.tryLock ()) return ret;
	} else {
		output_buffer_mutex.lock ();
	}

	RK_ASSERT (!output_buffer.isEmpty ());	// see check for out_buf_len, above

	ret = output_buffer;
	output_buffer.clear ();
	out_buf_len = 0;

	output_buffer_mutex.unlock ();
	return ret;
}

void RThread::handleSubstackCall (QStringList &call) {
	RK_TRACE (RBACKEND);

	if (call.count () == 2) {		// schedule symbol update for later
		if (call[0] == "ws") {
			// always keep in mind: No current command can happen for tcl/tk events.
			if ((!current_command) || (current_command->type & RCommand::ObjectListUpdate) || (!(current_command->type & RCommand::Sync))) {		// ignore Sync commands that are not flagged as ObjectListUpdate
				if (!changed_symbol_names.contains (call[1])) changed_symbol_names.append (call[1]);
			}
			return;
		}
	}

	REvalRequest request;
	request.call = call;
	RKRBackendEvent* event = new RKRBackendEvent (RKRBackendEvent::REvalRequest, &request);
	qApp->postEvent (RKGlobals::rInterface (), event);

	RCommandProxy *c;
	while ((c = fetchNextCommand ())) {
		runCommand (c);
		commandFinished (false);
	}
}

void RThread::handleStandardCallback (RCallbackArgs *args) {
	RK_TRACE (RBACKEND);

	args->done = false;

	RKRBackendEvent* event = new RKRBackendEvent (RKRBackendEvent::RCallbackRequest, args);
	qApp->postEvent (RKGlobals::rInterface (), event);
	
	bool *done = &(args->done);
	while (!(*done)) {
		msleep (10); // callback not done yet? Sleep for a while

		if (!(RInterface::backendIsLocked () || killed)) processX11Events ();
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
	RCommandProxy *dummy = runDirectCommand (".libPaths ()\n", RCommand::GetStringVector);
	if (dummy->status & RCommand::Failed) status |= OtherFail;
	for (unsigned int i = 0; i < dummy->getDataLength (); ++i) {
		RKSettingsModuleRPackages::defaultliblocs.append (dummy->getStringVector ()[i]);
	}
	delete dummy;

// start help server / determined help base url
	dummy = runDirectCommand (".rk.getHelpBaseUrl ()\n", RCommand::GetStringVector);
	if (dummy->status & RCommand::Failed) status |= OtherFail;
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
		RCommandProxy *dummy = runDirectCommand ("search ()\n", RCommand::GetStringVector);
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
			QStringList call = toplevel_env_names;
			call.prepend ("syncenvs");	// should be faster than the reverse
			handleSubstackCall (call);
		} 
		if (globalenv_update_needed) {
			QStringList call = global_env_toplevel_names;
			call.prepend ("syncglobal");	// should be faster than the reverse
			handleSubstackCall (call);
		}
	}

	if (search_update_needed || globalenv_update_needed) {
		RK_DO (qDebug ("checkObjectUpdatesNeeded: updating watches"), RBACKEND, DL_TRACE);
		runDirectCommand (".rk.watch.globalenv ()\n");
	}

	if (!changed_symbol_names.isEmpty ()) {
		QStringList call = changed_symbol_names;
		call.prepend (QString ("sync"));	// should be faster than reverse
		handleSubstackCall (call);
		changed_symbol_names.clear ();
	}
}
