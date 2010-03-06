/***************************************************************************
                          rthread  -  description
                             -------------------
    begin                : Mon Aug 2 2004
    copyright            : (C) 2004, 2006, 2007, 2010 by Thomas Friedrichsmeier
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

#include "../debug.h"

#include <kapplication.h>
#include <klocale.h>
#include <dcopclient.h>

#include <qstring.h>
#include <qapplication.h>

#include <signal.h>		// needed for pthread_kill

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

	toplevel_env_names = 0;
	toplevel_env_count = 0;
	global_env_toplevel_names = 0;
	global_env_toplevel_count = 0;
}

RThread::~RThread() {
	RK_TRACE (RBACKEND);
}

void RThread::interruptProcessing (bool interrupt) {
// TODO: find a good #ifdef to use the uncommented version below, if on a system without pthreads
	if (interrupt) {
		pthread_kill ((pthread_t) thread_id, SIGINT);
	}
/*	if (interrupt) {
		R_interrupts_pending = 1;
	} else {
		R_interrupts_pending = 0;
	} */
}

void RThread::run () {
	RK_TRACE (RBACKEND);
	thread_id = currentThread ();
	locked = Startup;
	killed = false;
	int err;
	bool previously_idle = false;
	if ((err = initialize ())) {
		qApp->postEvent (RKGlobals::rInterface (), new QCustomEvent (RSTARTUP_ERROR_EVENT + err));
	}
	qApp->postEvent (RKGlobals::rInterface (), new QCustomEvent (RSTARTED_EVENT));

	// wait until RKWard is set to go (esp, it has handled any errors during startup, etc.)
	while (locked) {
		msleep (10);
	}

	MUTEX_LOCK;
	checkObjectUpdatesNeeded (true);
	MUTEX_UNLOCK;
	
	while (1) {
		processX11Events ();

		MUTEX_LOCK;
		if (previously_idle) {
			if (!RCommandStack::regular_stack->isEmpty ()) {
				qApp->postEvent (RKGlobals::rInterface (), new QCustomEvent (RBUSY_EVENT));
				previously_idle = false;
			}
		}
	
		// while commands are in queue, don't wait
		while (RCommandStack::regular_stack->isActive () && !locked) {
			RCommand *command = RCommandStack::regular_stack->pop ();
			
			if (command) {
				// store type. Inside doCommand, command may be deleted (if the other thread gets called)!
				bool check_list = (command->type () & (RCommand::User | RCommand::ObjectListUpdate));
				// mutex will be unlocked inside
				doCommand (command);
				checkObjectUpdatesNeeded (check_list);
			}
		
			if (killed) {
				shutdown (false);
				MUTEX_UNLOCK;
				return;
			}
		}

		if (!previously_idle) {
			if (RCommandStack::regular_stack->isEmpty ()) {
				qApp->postEvent (RKGlobals::rInterface (), new QCustomEvent (RIDLE_EVENT));
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
	QCustomEvent *event = new QCustomEvent (RCOMMAND_IN_EVENT);
	event->setData (command);
	qApp->postEvent (RKGlobals::rInterface (), event);
	current_command = command;

	// step 2: actual handling
	if (!((command->type () & RCommand::EmptyCommand) || (command->status & RCommand::Canceled))) {
		all_current_commands.append (command);
		RKWardRError error;
		
		int ctype = command->type ();
		QString ccommand = command->command ();		// easier typing below
		
		RK_DO (qDebug ("running command: %s", ccommand.latin1()), RBACKEND, DL_DEBUG);
	
		if (command->type () & RCommand::DirectToOutput) {
			runCommandInternal (".rk.cat.output (\"<hr>\\n\")", &error, false);
			RK_ASSERT (!error);
		}

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
		if (!locked || killed) processX11Events ();

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
			RK_DO (qDebug ("failed command was: '%s'", command->command ().latin1 ()), RBACKEND, dl);
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
	
		if (error) {
			RK_DO (qDebug ("- error message was: '%s'", command->error ().latin1 ()), RBACKEND, dl);
	//		runCommandInternal (".rk.init.handlers ()\n", &dummy);
		}
		RK_DO (qDebug ("done running command"), RBACKEND, DL_DEBUG);
		all_current_commands.pop_back();
	} else {
		if (command->type () & RCommand::QuitCommand) {
			killed = true;
			MUTEX_UNLOCK;
			shutdown (false);
			MUTEX_LOCK;
		}
	}

	// notify GUI-thread that command was finished
	event = new QCustomEvent (RCOMMAND_OUT_EVENT);
	event->setData (command);
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
		for (QValueList<RCommand*>::const_iterator it = all_current_commands.constBegin (); it != all_current_commands.constEnd(); ++it) {
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
			QCustomEvent *event = new QCustomEvent (RCOMMAND_OUTPUT_EVENT);
			ROutputContainer *outc = new ROutputContainer;
			outc->output = output;
			outc->command = *it;
			event->setData (outc);
			qApp->postEvent (RKGlobals::rInterface (), event);
		}

		RK_DO (qDebug ("output '%s'", current_output->output.latin1 ()), RBACKEND, DL_DEBUG);
	} else {
		// running Rcmdr, eh?
		RK_DO (qDebug ("output without receiver'%s'", current_output->output.latin1 ()), RBACKEND, DL_WARNING);
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
	// Unfortunately, errors still get printed to the output. We try this crude method for the time being:
	flushOutput ();
	if (current_command) {
		current_command->output_list.last ()->type = ROutput::Error;
		current_command->status |= RCommand::HasError;
	}

	RK_DO (qDebug ("error '%s'", call[0].latin1 ()), RBACKEND, DL_DEBUG);
	MUTEX_UNLOCK;
}

void RThread::handleSubstackCall (QString *call, int call_length) {
	RK_TRACE (RBACKEND);

	if (call_length == 2) {		// schedule symbol update for later
		if (call[0] == "ws") {
			RK_ASSERT (current_command);
			if ((current_command->type () & RCommand::ObjectListUpdate) || (!(current_command->type () & RCommand::Sync))) {		// ignore Sync commands that are not flagged as ObjectListUpdate
				if (!changed_symbol_names.contains (call[1])) changed_symbol_names.append (call[1]);
			}
			return;
		}
	}

	RCommand *prev_command = current_command;
	REvalRequest *request = new REvalRequest;
	request->call = call;
	request->call_length = call_length;
	MUTEX_LOCK;
	flushOutput ();
	RCommandStack *reply_stack = new RCommandStack ();
	request->in_chain = reply_stack->startChain (reply_stack);
	MUTEX_UNLOCK;

	QCustomEvent *event = new QCustomEvent (R_EVAL_REQUEST_EVENT);
	event->setData (request);
	qApp->postEvent (RKGlobals::rInterface (), event);
	
	bool done = false;
	while (!done) {
		processX11Events ();
		MUTEX_LOCK;
		// while commands are in queue, don't wait
		while (reply_stack->isActive () && !locked) {
			if (killed) {
				done = true;
				break;
			}

			RCommand *command = reply_stack->pop ();
			
			if (command) {
				// mutex will be unlocked inside
				bool object_update_forced = (command->type () & RCommand::ObjectListUpdate);
				doCommand (command);
				if (object_update_forced) checkObjectUpdatesNeeded (true);
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

	delete reply_stack;
}

void RThread::handleStandardCallback (RCallbackArgs *args) {
	RK_TRACE (RBACKEND);

	MUTEX_LOCK;
	flushOutput ();
	MUTEX_UNLOCK;
	args->done = false;

	QCustomEvent *event = new QCustomEvent (R_CALLBACK_REQUEST_EVENT);
	event->setData (args);
	qApp->postEvent (RKGlobals::rInterface (), event);
	
	bool *done = &(args->done);
	while (!(*done)) {
		msleep (10); // callback not done yet? Sleep for a while

		if (!(locked || killed)) processX11Events ();
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

	// we create a fake RCommand to capture all the output/errors during startup
	current_command = new RCommand (i18n ("R Startup"), RCommand::App, i18n ("R Startup"));

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
	unsigned int c;
	QString *paths = getCommandAsStringVector ("library.dynam (\"rkward\", \"rkward\")[[\"path\"]]\n", &c, &error);
	if ((error) || (c != 1)) {
		status |= LibLoadFail;
	} else {
		if (!registerFunctions (paths[0].local8Bit ())) status |= LibLoadFail;
	}
	delete [] paths;

// find out about standard library locations
	QString *standardliblocs = getCommandAsStringVector (".libPaths ()\n", &c, &error);
	if (error) status |= OtherFail;
	for (unsigned int i = 0; i < c; ++i) {
		RKSettingsModuleRPackages::defaultliblocs.append (standardliblocs[i]);
	}
	delete [] standardliblocs;

// start help server / determined help base url
	QString *help_base_url = getCommandAsStringVector (".rk.getHelpBaseUrl ()\n", &c, &error);
	if (error) status |= OtherFail;
	else {
		RK_ASSERT (c == 1);
		RKSettingsModuleR::help_base_url = help_base_url[0];
	}
	delete [] help_base_url;

// apply user configurable run time options
	QStringList commands = RKSettingsModuleR::makeRRunTimeOptionCommands () + RKSettingsModuleRPackages::makeRRunTimeOptionCommands ();
	for (QStringList::const_iterator it = commands.begin (); it != commands.end (); ++it) {
		runCommandInternal ((*it).local8Bit (), &error);
		if (error) {
			status |= OtherFail;
			RK_DO (qDebug ("error in initialization call '%s'", (*it).latin1()), RBACKEND, DL_ERROR);
		}
	}

// error/output sink and help browser
	runCommandInternal ("options (error=quote (.rk.do.error ()))\n", &error);
	if (error) status |= SinkFail;
	runCommandInternal ("rk.set.output.html.file (\"" + RKSettingsModuleGeneral::filesPath () + "/rk_out.html\")\n", &error);
	if (error) status |= SinkFail;
	runCommandInternal ("options (browser=\"dcop " + kapp->dcopClient ()->appId () + " rkwardapp openHTMLHelp \")\n", &error);
	if (error) status |= OtherFail;
	// TODO: error-handling?

	MUTEX_LOCK;
	flushOutput ();
	MUTEX_UNLOCK;

	QCustomEvent *event = new QCustomEvent (RCOMMAND_OUT_EVENT);
	event->setData (current_command);
	qApp->postEvent (RKGlobals::rInterface (), event);

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
		if (count != toplevel_env_count) {
			search_update_needed = true;
		} else {
			for (unsigned int i = 0; i < toplevel_env_count; ++i) {
				if (toplevel_env_names[i] != strings[i]) {
					search_update_needed = true;
					break;
				}
			}
		}
		delete [] toplevel_env_names;
		toplevel_env_names = strings;
		toplevel_env_count = count;
	
	// TODO: avoid parsing this over and over again
		RK_DO (qDebug ("checkObjectUpdatesNeeded: getting globalenv symbols"), RBACKEND, DL_TRACE);
		strings = getCommandAsStringVector ("ls (globalenv (), all.names=TRUE)\n", &count, &error);
		if (count != global_env_toplevel_count) {
			globalenv_update_needed = true;
		} else {
			for (unsigned int i = 0; i < global_env_toplevel_count; ++i) {
				bool found = false;
				for (unsigned int j = 0; j < global_env_toplevel_count; ++j) {
					if (global_env_toplevel_names[j] == strings[i]) {
						found = true;
						break;
					}
				}
				if (!found) {
					globalenv_update_needed = true;
					break;
				}
			}
		}
		delete [] global_env_toplevel_names;
		global_env_toplevel_names = strings;
		global_env_toplevel_count = count;
	
		if (search_update_needed) {	// this includes an update of the globalenv, even if not needed
			QString call = "syncall";
			MUTEX_UNLOCK;
			handleSubstackCall (&call, 1);
			MUTEX_LOCK;
		} else if (globalenv_update_needed) {
			QString call = "syncglobal";
			MUTEX_UNLOCK;
			handleSubstackCall (&call, 1);
			MUTEX_LOCK;
		}
	}

	if (search_update_needed || globalenv_update_needed) {
		RK_DO (qDebug ("checkObjectUpdatesNeeded: updating watches"), RBACKEND, DL_TRACE);
		runCommandInternal (".rk.watch.globalenv ()\n", &error);
	}

	if (!changed_symbol_names.isEmpty ()) {
		int call_length = changed_symbol_names.count () + 1;
		QString *call = new QString[call_length];
		call[0] = "sync";
		int i = 1;
		for (QStringList::const_iterator it = changed_symbol_names.constBegin (); it != changed_symbol_names.constEnd (); ++it) {
			call[i++] = *it;
		}
		RK_ASSERT (i == call_length);
		MUTEX_UNLOCK;
		handleSubstackCall (call, call_length);
		MUTEX_LOCK;
		delete [] call;
		changed_symbol_names.clear ();
	}

}
