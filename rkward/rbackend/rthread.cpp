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

#include "rembed.h"
#include "rinterface.h"
#include "rcommandstack.h"

#include "../debug.h"

#include <qapplication.h>

RThread::RThread (RInterface *parent) : QThread () {
	RK_TRACE (RBACKEND);
	inter = parent;
	current_command = 0;
//	r_get_value_reply = 0;
}

RThread::~RThread() {
	RK_TRACE (RBACKEND);
}

void RThread::run () {
	RK_TRACE (RBACKEND);
	embeddedR = new REmbed (this);
	locked = true;
	int err;
	bool previously_idle = false;
	if ((err = embeddedR->initialize ())) {
		qApp->postEvent (inter, new QCustomEvent (RSTARTUP_ERROR_EVENT + err));
	}
	qApp->postEvent (inter, new QCustomEvent (RSTARTED_EVENT));

	// wait until RKWard is set to go (esp, it has handled any errors during startup, etc.)
	while (locked) {
		msleep (10);
	}
	
	while (1) {
		RInterface::mutex.lock ();
		
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
			}
		}
		
		if (!previously_idle) {
			if (RCommandStack::regular_stack->isEmpty ()) {
				qApp->postEvent (inter, new QCustomEvent (RIDLE_EVENT));
				previously_idle = true;
			}
		}
		
		// if no commands are in queue, sleep for a while
		RInterface::mutex.unlock ();
		msleep (10);
	}
}

void RThread::doCommand (RCommand *command) {
	RK_TRACE (RBACKEND);
	// notify GUI-thread that a new command is being tried
	QCustomEvent *event = new QCustomEvent (RCOMMAND_IN_EVENT);
	event->setData (command);
	qApp->postEvent (inter, event);
	current_command = command;

	// mutex will be unlocked inside
	embeddedR->runCommand (command);

	current_command = 0;
	// notify GUI-thread that command was finished
	event = new QCustomEvent (RCOMMAND_OUT_EVENT);
	event->setData (command);
	qApp->postEvent (inter, event);
}

void RThread::doSubstack (char **call, int call_length) {
	RK_TRACE (RBACKEND);
	
	REvalRequest *request = new REvalRequest;
	request->call = call;
	request->call_length = call_length;
	RInterface::mutex.lock ();
	RCommandStack *reply_stack = new RCommandStack ();
	request->in_chain = reply_stack->startChain (reply_stack);
	RInterface::mutex.unlock ();

	QCustomEvent *event = new QCustomEvent (R_EVAL_REQUEST_EVENT);
	event->setData (request);
	qApp->postEvent (inter, event);
	
	bool done = false;
	while (!done) {
		RInterface::mutex.lock ();
		// while commands are in queue, don't wait
		while (reply_stack->isActive () && !locked) {
			RCommand *command = reply_stack->pop ();
			
			if (command) {
				// mutex will be unlocked inside
				doCommand (command);
			}
		}

		if (reply_stack->isEmpty ()) {
			done = true;
		}
		RInterface::mutex.unlock ();

		// if no commands are in queue, sleep for a while
		msleep (10);
	}

	delete reply_stack;
}
/*
char **RThread::fetchValue (char **call, int call_length, int *reply_length) {
	RK_TRACE (RBACKEND);
	
	RGetValueRequest *request = new RGetValueRequest;
	request->call = call;
	request->call_length = call_length;
	
	RK_ASSERT (r_get_value_reply == 0);

	QCustomEvent *event = new QCustomEvent (R_EVAL_REQUEST_EVENT);
	event->setData (request);
	qApp->postEvent (inter, event);
	
	bool done = false;
	while (!done) {
		RInterface::mutex.lock ();
		// while commands are in queue, don't wait
		while (reply_stack->isActive ()) {
			RCommand *command = reply_stack->pop ();
			
			if (command) {
				RInterface::mutex.unlock ();
				// this statement is the time-consuming one. Thankfully, we do not need a mutex at this point
				doCommand (command);
				RInterface::mutex.lock ();
			}
		}

		if (reply_stack->isEmpty ()) {
			done = true;
		}
		RInterface::mutex.unlock ();

		// if no commands are in queue, sleep for a while
		msleep (10);
	}

	delete reply_stack;

	
	
	RK_ASSERT (false);
	return call;
}

void RThread::setGetValueReply (RGetValueReply *reply) {
	RK_TRACE (RBACKEND);
	RInterface::mutex.lock ();
	RK_ASSERT (r_get_value_reply == 0);
	r_get_value_reply = reply;
	RInterface::mutex.unlock ();
}

*/