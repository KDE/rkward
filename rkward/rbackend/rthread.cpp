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

#include "../debug.h"

#include <qapplication.h>

RThread::RThread (RInterface *parent) : QThread () {
	RK_TRACE (RBACKEND);
	inter = parent;
	top_chain = new RCommandChain;
	top_chain->closed = true;
	top_chain->parent = 0;
	current_chain = top_chain;
}

RThread::~RThread() {
	RK_TRACE (RBACKEND);
}

void RThread::issueCommand (RCommand *command, RCommandChain *chain) {
	RK_TRACE (RBACKEND);
	mutex.lock ();
	if (!chain) chain = top_chain;
	
	RChainOrCommand *coc = new RChainOrCommand;
	coc->command = command;
	coc->chain = 0;
	chain->commands.append (coc);
	mutex.unlock ();
}

RCommandChain *RThread::startChain (RCommandChain *parent) {
	RK_TRACE (RBACKEND);
	mutex.lock ();
	if (!parent) parent = top_chain;

	RChainOrCommand *coc = new RChainOrCommand;
	coc->command = 0;
	coc->chain = new RCommandChain;
	coc->chain->closed = false;
	coc->chain->parent = parent;
	parent->commands.append (coc);
	
	mutex.unlock ();
	return coc->chain;
}

RCommandChain *RThread::closeChain (RCommandChain *chain) {
	RK_TRACE (RBACKEND);
	if (!chain) return 0;

	mutex.lock ();
	chain->closed = true;
	RCommandChain *ret = chain->parent;
	if (ret == top_chain) ret = 0;
	
	// lets see, whether we can do some cleanup
	while (current_chain->commands.isEmpty () && current_chain->closed && current_chain->parent) {
		RCommandChain *temp = current_chain;
		current_chain = current_chain->parent;
		delete temp;
	}
	
	mutex.unlock ();
	return ret;
}

void RThread::run () {
	RK_TRACE (RBACKEND);
	embeddedR = new REmbed ();
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
		mutex.lock ();
		
		if ((!current_chain->commands.isEmpty ()) || (!current_chain->closed)) {
			if (previously_idle) {
				qApp->postEvent (inter, new QCustomEvent (RBUSY_EVENT));
				previously_idle = false;
			}
		}
	
		// while commands are in queue, don't wait
		while (!current_chain->commands.isEmpty ()) {
			RChainOrCommand *coc = current_chain->commands.first ();
			current_chain->commands.removeFirst ();
			if (coc->command) {
				RCommand *command = coc->command;
				
				mutex.unlock ();
				
				// this statement is the time-consuming one. Thankfully, we do not need a mutex at this point
				doCommand (command);
				
				mutex.lock ();
				
				delete coc;
			} else {
				current_chain = coc->chain;
				delete coc;
			}
			
			// reached end of chain and chain is closed? walk up
			while (current_chain->commands.isEmpty () && current_chain->closed && current_chain->parent) {
				RCommandChain *temp = current_chain;
				current_chain = current_chain->parent;
				delete temp;
			}
		}
		
		// if no commands are in queue, sleep for a while
		if (current_chain->closed) {
			if (!previously_idle) {
				qApp->postEvent (inter, new QCustomEvent (RIDLE_EVENT));
				previously_idle = true;
			}
		}
		
		mutex.unlock ();
		msleep (10);
	}
}

void RThread::doCommand (RCommand *command) {
	RK_TRACE (RBACKEND);
	// notify GUI-thread that a new command is being tried
	QCustomEvent *event = new QCustomEvent (RCOMMAND_IN_EVENT);
	event->setData (command);
	qApp->postEvent (inter, event);
	
	embeddedR->runCommand (command);

	// notify GUI-thread that command was finished
	event = new QCustomEvent (RCOMMAND_OUT_EVENT);
	event->setData (command);
	qApp->postEvent (inter, event);
}
