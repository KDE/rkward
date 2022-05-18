/*
rcommandreceiver - This file is part of RKWard (https://rkward.kde.org). Created: Thu Aug 19 2004
SPDX-FileCopyrightText: 2004-2010 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "rcommandreceiver.h"

#include "rkrinterface.h"

#include "../debug.h"

RCommandReceiver::RCommandReceiver () {
	RK_TRACE (RBACKEND);

	delete_when_done = false;
}

RCommandReceiver::~RCommandReceiver () {
	RK_TRACE (RBACKEND);

	for (RCommandList::const_iterator it = outstanding_commands.constBegin (); it != outstanding_commands.constEnd (); ++it) {
		(*it)->removeReceiver (this);
	}
}

void RCommandReceiver::cancelOutstandingCommands () {
	RK_TRACE (RBACKEND);

	for (RCommandList::const_iterator it = outstanding_commands.constBegin (); it != outstanding_commands.constEnd (); ++it) {
		RInterface::instance()->cancelCommand(*it);
	}
}

void RCommandReceiver::addCommand (RCommand *command) {
	RK_TRACE (RBACKEND);
	outstanding_commands.append (command);
}

void RCommandReceiver::delCommand (RCommand *command) {
	RK_TRACE (RBACKEND);
	outstanding_commands.removeAll (command);

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
