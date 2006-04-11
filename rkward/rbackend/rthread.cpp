/***************************************************************************
                          rthread  -  description
                             -------------------
    begin                : Mon Aug 2 2004
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
#include "rthread.h"

#include "rinterface.h"
#include "rcommandstack.h"
#include "../settings/rksettingsmoduler.h"
#include "../settings/rksettingsmodulegeneral.h"
#include "../rkglobals.h"

#include "../debug.h"

#include <kapplication.h>
#include <dcopclient.h>

#include <qstring.h>
#include <qapplication.h>

#include <signal.h>		// needed for pthread_kill

#define MAX_BUF_LENGTH 1000

RThread::RThread () : QThread (), REmbedInternal () {
	RK_TRACE (RBACKEND);
	current_command = 0;

	RK_ASSERT (this_pointer == 0);
	this_pointer = this;
	RK_ASSERT (this_pointer);
	current_output = 0;
	out_buf_len = 0;
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
	
	while (1) {
		MUTEX_LOCK;
		processX11Events ();

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
				// mutex will be unlocked inside
				doCommand (command);
				processX11Events ();
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
		RKWardRError error;
		
		int ctype = command->type ();
		QCString localc = command->command ().local8Bit ();		// needed so the string below does not go out of scope
		const char *ccommand = localc;
		
		RK_DO (qDebug ("running command: %s", ccommand), RBACKEND, DL_DEBUG);
	
		if (command->type () & RCommand::DirectToOutput) {
			runCommandInternal ("sink (\"" + RKSettingsModuleGeneral::filesPath () + "/rk_out.html\", append=TRUE, split=TRUE)\n", &error);
		}
	
		MUTEX_UNLOCK;
	
		if (ctype & RCommand::GetStringVector) {
			command->string_data = getCommandAsStringVector (ccommand, &(command->string_count), &error);
		} else if (ctype & RCommand::GetRealVector) {
			command->real_data = getCommandAsRealVector (ccommand, &(command->real_count), &error);
		} else if (ctype & RCommand::GetIntVector) {
			command->integer_data = getCommandAsIntVector (ccommand, &(command->integer_count), &error);
		} else {
			runCommandInternal (ccommand, &error, ctype & RCommand::User);
		}
	
		MUTEX_LOCK;
		
		if (error != NoError) {
			command->status |= RCommand::WasTried | RCommand::Failed;
			if (error == Incomplete) {
				command->status |= RCommand::ErrorIncomplete;
				RK_DO (qDebug ("Command failed (incomplete)"), RBACKEND, DL_WARNING);
			} else if (error == SyntaxError) {
				command->status |= RCommand::ErrorSyntax;
				RK_DO (qDebug ("Command failed (syntax)"), RBACKEND, DL_WARNING);
			} else {
				command->status |= RCommand::ErrorOther;
				RK_DO (qDebug ("Command failed (other)"), RBACKEND, DL_WARNING);
			}
			RK_DO (qDebug ("failed command was: '%s'", command->command ().latin1 ()), RBACKEND, DL_INFO);
		} else {
			command->status |= RCommand::WasTried;
		}
	
		RKWardRError dummy;
		if (command->type () & RCommand::DirectToOutput) {
			runCommandInternal ("sink ()\n", &dummy);
		}
	
		if (error) {
			RK_DO (qDebug ("- error message was: '%s'", command->error ().latin1 ()), RBACKEND, DL_WARNING);
	//		runCommandInternal (".rk.init.handlers ()\n", &dummy);
		}
		RK_DO (qDebug ("done running command"), RBACKEND, DL_DEBUG);

		flushOutput ();
	}

	// step 3: cleanup
	// notify GUI-thread that command was finished
	event = new QCustomEvent (RCOMMAND_OUT_EVENT);
	event->setData (command);
	qApp->postEvent (RKGlobals::rInterface (), event);
}

void RThread::handleOutput (char *buf, int buf_length) {
	RK_TRACE (RBACKEND);

// TODO: output sometimes arrives in small chunks. Maybe it would be better to keep an internal buffer, and only append it to the output, when R_FlushConsole gets called?

	if (!buf_length) return;

	MUTEX_LOCK;
	if (current_output) {
		if (current_output->type != ROutput::Output) {
			flushOutput ();
		}
	}
	if (!current_output) {	// not an else, might have been set to 0 in the above if
		current_output = new ROutput;
		current_output->type = ROutput::Output;
	}
	current_output->output.append (buf);

	if ((out_buf_len += buf_length) > MAX_BUF_LENGTH) {
		RK_DO (qDebug ("Output buffer has %d characters. Forcing flush", out_buf_len), RBACKEND, DL_DEBUG);
		flushOutput ();
	}
	MUTEX_UNLOCK;
}

void RThread::flushOutput () {
	if (!current_output) return;		// avoid creating loads of traces
	RK_TRACE (RBACKEND);

	current_command->output_list.append (current_output);
	if (current_output->type == ROutput::Output) {
		current_command->status |= RCommand::HasOutput;
	} else if (current_output->type == ROutput::Error) {
		current_command->status |= RCommand::HasError;
	}

// pass a signal to the main thread for real-time update of output
	if (current_command->type () & RCommand::ImmediateOutput) {
		if (!(current_command->receiver)) {
			RK_ASSERT (false);
			return;
		}

		QCustomEvent *event = new QCustomEvent (RCOMMAND_OUTPUT_EVENT);
		ROutputContainer *outc = new ROutputContainer;
		outc->output = current_output;
		outc->command = current_command;
		event->setData (outc);
		qApp->postEvent (RKGlobals::rInterface (), event);
	}

	RK_DO (qDebug ("output '%s'", current_output->output.latin1 ()), RBACKEND, DL_DEBUG);
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

void RThread::handleError (char **call, int call_length) {
	RK_TRACE (RBACKEND);

	if (!call_length) return;

	MUTEX_LOCK;
	// Unfortunately, errors still get printed to the output. We try this crude method for the time being:
	flushOutput ();
	current_command->output_list.last ()->type = ROutput::Error;
	current_command->status |= RCommand::HasError;

	RK_DO (qDebug ("error '%s'", call[0]), RBACKEND, DL_DEBUG);
	MUTEX_UNLOCK;
}

void RThread::handleSubstackCall (char **call, int call_length) {
	RK_TRACE (RBACKEND);

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
		MUTEX_LOCK;
		processX11Events ();
		// while commands are in queue, don't wait
		while (reply_stack->isActive () && !locked) {
			RCommand *command = reply_stack->pop ();
			
			if (command) {
				// mutex will be unlocked inside
				doCommand (command);
				processX11Events ();
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
	
	bool done = false;
	while (!done) {
		// callback not done yet? Sleep for a while
		msleep (10);

		MUTEX_LOCK;
		processX11Events ();

		if (args->done) {
			done = true;		// safe to access only while the mutex is locked
		}
		MUTEX_UNLOCK;
	}
}

int RThread::initialize () {
	RK_TRACE (RBACKEND);

	// we create a fake RCommand to capture all the output/errors during startup
	current_command = new RCommand (QString::null, RCommand::App, "R Startup");

	int argc = 2;	
	char* argv[2] = { qstrdup ("--slave"), qstrdup ("--no-save") };

	startR (argc, argv);

	for (--argc; argc >= 0; --argc) {
		delete argv[argc];
	}

	connectCallbacks ();

	RKWardRError error;
	int status = 0;
	
	char **paths;
	runCommandInternal ("library (\"rkward\")\n", &error);
	if (error) status |= LibLoadFail;
	int c;
	paths = getCommandAsStringVector ("library.dynam (\"rkward\", \"rkward\")$path\n", &c, &error);
	if ((error) || (c != 1)) {
		status |= LibLoadFail;
	} else {
		if (!registerFunctions (paths[0])) status |= LibLoadFail;
	}
	for (int i = (c-1); i >=0; --i) {
		DELETE_STRING (paths[i]);
	}
	delete [] paths;

	QStringList commands = RKSettingsModuleR::makeRRunTimeOptionCommands ();
	for (QStringList::const_iterator it = commands.begin (); it != commands.end (); ++it) {
		runCommandInternal (*it, &error);
		if (error) status |= OtherFail;
	}
	runCommandInternal ("options (error=quote (.rk.do.error ()))\n", &error);
	if (error) status |= SinkFail;
/*	runCommandInternal (".rk.init.handlers ()\n", &error);
	if (error) status |= SinkFail; */
	runCommandInternal ("options (htmlhelp=TRUE); options (browser=\"dcop " + kapp->dcopClient ()->appId () + " rkwardapp openHTMLHelp \")", &error);
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
