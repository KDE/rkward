/***************************************************************************
                          rinterface.cpp  -  description
                             -------------------
    begin                : Fri Nov 1 2002
    copyright            : (C) 2002 by Thomas Friedrichsmeier
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

#include "rinterface.h"

#include <qcstring.h>

RInterface::RInterface(){
	command_running = false;
	end_tag = "Done with R-Command";
	connect (this, SIGNAL (receivedStdout (KProcess *, char *, int)), this, SLOT (gotROutput (KProcess *, char *, int)));
	connect (this, SIGNAL (receivedStderr (KProcess *, char *, int)), this, SLOT (gotROutput (KProcess *, char *, int)));
}

RInterface::~RInterface(){
}

bool RInterface::startR (QStrList &commandline) {
	if (isRunning ()) {
		return false;
	}

	clearArguments ();

	commandline.first ();
	do {
		*this << commandline.current ();
	} while (commandline.next ());

	return start (NotifyOnExit, All);
}

void RInterface::gotROutput (KProcess *proc, char *buffer, int buflen) {
	QString buf_copy;

	// unfortunately QString obviously cant be made to simply copy the thing,
	// without looking for a terminator
	for (int i=0; i < buflen; i++) {
		buf_copy.append (buffer[i]);
	}
	r_output.append (buf_copy);

	qDebug (buf_copy);

	// check, whether output seems to be done
	if (r_output.right (end_tag.length () + 5).contains (end_tag)) {
		int pos = r_output.findRev (end_tag, -1);
		pos = r_output.findRev ("\n", -1);

		qDebug ("-----------------------------------------------");
		qDebug (r_output.left (pos));

		emit (receivedReply (r_output.left (pos)));
		r_output = "";
		command_running = false;
	}
}

void RInterface::issueCommand (const QString &command) {
	if (!command_running) {
		QString command_string = command;
		command_string.append ("\nprint (\"" + end_tag + "\")\n");

		qDebug (command_string);

		// TOOD: remember to delete command_buffer
		command_buffer = qstrdup (command_string.local8Bit ());
		writeStdin (command_buffer, command_string.length ());
		command_running = true;
	}
}