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

#include <qapplication.h>

RThread::RThread(RInterface *parent) : QThread() {
	inter = parent;
}

RThread::~RThread() {
}

// warning: This function gets called from the main thread. Do we need locking?
void RThread::issueCommand (RCommand *command) {
	command_stack.append (command);
}

void RThread::run () {
	embeddedR = new REmbed ();

	while (1) {
		if (!command_stack.isEmpty ()) {
			qApp->postEvent (inter, new QCustomEvent (RBUSY_EVENT));
		}
	
		// while commands are in queue, don't wait
		while (!command_stack.isEmpty ()) {
			write (command_stack.first ());
		}
		
		// if no commands are in queue, sleep for a while
		qApp->postEvent (inter, new QCustomEvent (RIDLE_EVENT));
		msleep (10);
	}
}

void RThread::write (RCommand *command) {
	// notify GUI-thread that a new command is being tried
	QCustomEvent *event = new QCustomEvent (RCOMMAND_IN_EVENT);
	event->setData (command);
	qApp->postEvent (inter, event);
	
	embeddedR->runCommand (command);
	qDebug ("ran command %d", command->id ());

	// notify GUI-thread that command was finished
	event = new QCustomEvent (RCOMMAND_OUT_EVENT);
	event->setData (command);
	qApp->postEvent (inter, event);
	
	command_stack.removeFirst ();
}
