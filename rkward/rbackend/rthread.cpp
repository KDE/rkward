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
#include "../settings/rksettingsmodulelogfiles.h"
#include "../rkglobals.h"

#include "../debug.h"

#include <kapplication.h>
#include <dcopclient.h>

#include <qstring.h>
#include <qapplication.h>


RThread::RThread (RInterface *parent) : QThread (), REmbedInternal () {
	RK_TRACE (RBACKEND);
	inter = parent;
	current_command = 0;

	RK_ASSERT (this_pointer == 0);
	this_pointer = this;
	RK_ASSERT (this_pointer);
}

RThread::~RThread() {
	RK_TRACE (RBACKEND);
}

void RThread::run () {
	RK_TRACE (RBACKEND);
	locked = true;
	killed = false;
	int err;
	bool previously_idle = false;
	if ((err = initialize ())) {
		qApp->postEvent (inter, new QCustomEvent (RSTARTUP_ERROR_EVENT + err));
	}
	qApp->postEvent (inter, new QCustomEvent (RSTARTED_EVENT));

	// wait until RKWard is set to go (esp, it has handled any errors during startup, etc.)
	while (locked) {
		msleep (10);
	}
	
	while (1) {
		MUTEX_LOCK;
		processX11Events ();

		if (previously_idle) {
			if (!RCommandStack::regular_stack->isEmpty ()) {
				qApp->postEvent (inter, new QCustomEvent (RBUSY_EVENT));
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
				MUTEX_UNLOCK
				return;
			}
		}
		
		if (!previously_idle) {
			if (RCommandStack::regular_stack->isEmpty ()) {
				qApp->postEvent (inter, new QCustomEvent (RIDLE_EVENT));
				previously_idle = true;
			}
		}
		
		// if no commands are in queue, sleep for a while
		MUTEX_UNLOCK;
		if (killed) {
			shutdown (false);
			return;
		}
		msleep (10);
	}
}

void RThread::doCommand (RCommand *command) {
	RK_TRACE (RBACKEND);
	// step 1: notify GUI-thread that a new command is being tried and initialize
	QCustomEvent *event = new QCustomEvent (RCOMMAND_IN_EVENT);
	event->setData (command);
	qApp->postEvent (inter, event);
	RCommand *prev_command = current_command;
	current_command = command;

	// step 2: actual handling
	if (command->type () & RCommand::EmptyCommand) return;
	if (command->status & RCommand::Canceled) return;
	
	RKWardRError error;
	
	int ctype = command->type ();
	const char *ccommand = command->command ().latin1 ();
	
	RK_DO (qDebug ("running command: %s", ccommand), RBACKEND, DL_DEBUG);

	if (command->type () & RCommand::DirectToOutput) {
		runCommandInternal ("sink (\"" + RKSettingsModuleLogfiles::filesPath () + "/rk_out.html\", append=TRUE, split=TRUE)\n", &error);
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

	// step 3: cleanup
	current_command = prev_command;
	// notify GUI-thread that command was finished
	event = new QCustomEvent (RCOMMAND_OUT_EVENT);
	event->setData (command);
	qApp->postEvent (inter, event);
}

void RThread::handleOutput (char *buf, int buf_length) {
	RK_TRACE (RBACKEND);

// TODO: output sometimes arrives in small chunks. Maybe it would be better to keep an internal buffer, and only append it to the output, when R_FlushConsole gets called?

	if (!buf_length) return;

	ROutput *out = new ROutput;
	out->type = ROutput::Output;
	out->output = QString (buf);

	current_command->output_list.append (out);
	current_command->status |= RCommand::HasOutput;
	// TODO: pass a signal to the main thread for real-time update of output

	RK_DO (qDebug ("output '%s'", buf), RBACKEND, DL_DEBUG);
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

	// Unfortunately, errors still get printed to the output. We try this crude method for the time being:
	current_command->output_list.last ()->type = ROutput::Error;
	current_command->status |= RCommand::HasError;

/*	// for now we ignore everything but the first string.
	ROutput *out = new ROutput;
	out->type = ROutput::Error;
	out->output = QString (call[0]);

	thread->current_command->output_list.append (out);
	thread->current_command->status |= RCommand::HasError; */
	// TODO: pass a signal to the main thread for real-time update of output

	//RThreadInternal::next_output_is_error = true;
	RK_DO (qDebug ("error '%s'", call[0]), RBACKEND, DL_DEBUG);
}

void RThread::handleSubstackCall (char **call, int call_length) {
	RK_TRACE (RBACKEND);

	REvalRequest *request = new REvalRequest;
	request->call = call;
	request->call_length = call_length;
	MUTEX_LOCK;
	RCommandStack *reply_stack = new RCommandStack ();
	request->in_chain = reply_stack->startChain (reply_stack);
	MUTEX_UNLOCK;

	QCustomEvent *event = new QCustomEvent (R_EVAL_REQUEST_EVENT);
	event->setData (request);
	qApp->postEvent (inter, event);
	
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
		msleep (10);
	}

	delete reply_stack;
}

void RThread::handleStandardCallback (RCallbackArgs *args) {
	RK_TRACE (RBACKEND);

	args->done = false;

	QCustomEvent *event = new QCustomEvent (R_CALLBACK_REQUEST_EVENT);
	event->setData (args);
	qApp->postEvent (inter, event);
	
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

	QString r_home = RKSettingsModuleR::rHomeDir();

	int argc = 2;	
	char* argv[2] = { qstrdup ("--slave"), qstrdup ("--no-save") };

	startR (r_home, argc, argv);

	for (--argc; argc >= 0; --argc) {
		delete argv[argc];
	}

	connectCallbacks ();

	RKWardRError error;
	int status = 0;
	
	runCommandInternal ("library (\"rkward\")\n", &error);
	if (error) status |= LibLoadFail;
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

	QCustomEvent *event = new QCustomEvent (RCOMMAND_OUT_EVENT);
	event->setData (current_command);
	qApp->postEvent (inter, event);

	return status;
}
