/***************************************************************************
                          rcommandreceiver  -  description
                             -------------------
    begin                : Thu Aug 19 2004
    copyright            : (C) 2004, 2006 by Thomas Friedrichsmeier
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
#include "rcommandreceiver.h"

#include "../debug.h"

RCommandReceiver::RCommandReceiver () {
	RK_TRACE (RBACKEND);

	delete_when_done = false;
}

RCommandReceiver::~RCommandReceiver () {
	RK_TRACE (RBACKEND);

	for (RCommandList::const_iterator it = outstanding_commands.begin (); it != outstanding_commands.end (); ++it) {
		(*it)->removeReceiver (this);
	}
}

void RCommandReceiver::rCommandDone (RCommand *) {
	RK_TRACE (RBACKEND);
}

void RCommandReceiver::newOutput (RCommand *, ROutput *) {
	RK_TRACE (RBACKEND);
}

void RCommandReceiver::addCommand (RCommand *command) {
	RK_TRACE (RBACKEND);
	outstanding_commands.append (command);
}

void RCommandReceiver::delCommand (RCommand *command) {
	RK_TRACE (RBACKEND);
	outstanding_commands.remove (command);

	if (delete_when_done && outstanding_commands.isEmpty ()) delete this;
}

void RCommandReceiver::autoDeleteWhenDone () {
	RK_TRACE (RBACKEND);

	if (outstanding_commands.isEmpty ()) {
		delete this;
		return;
	}
	delete_when_done = true;
}
