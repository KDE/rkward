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

#include <kmessagebox.h>

#include "rkwatch.h"

RInterface::RInterface(){
	end_tag = "Done with R-Command";
	connect (this, SIGNAL (receivedStdout (KProcess *, char *, int)), this, SLOT (gotROutput (KProcess *, char *, int)));
	connect (this, SIGNAL (receivedStderr (KProcess *, char *, int)), this, SLOT (gotROutput (KProcess *, char *, int)));
	connect (this, SIGNAL (wroteStdin (KProcess *)), this, SLOT (doneWriting (KProcess *)));
	connect (this, SIGNAL (processExited (KProcess *)), this, SLOT (Rdied (KProcess *)));
	watch = new RKwatch (this);
	watch->show ();
}

RInterface::~RInterface(){
	delete watch;
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

	command_running = sync_command = false;
	commands_waiting = busy_writing = false;

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
		pos = r_output.findRev ("\n", pos - r_output.length ());
		if (pos <= 0) {
			r_output = "";
		}

		qDebug ("-----------------------------------------------");
		qDebug (r_output.left (pos));

		emit (receivedReply (r_output.left (pos)));
		r_output = "";
		command_running = false;
		if (!sync_command) {
			async_command_stack.removeFirst ();
			if (async_command_stack.count ()) {
				QString command = async_command_stack.first ();
				qDebug ("now issueing quequed " + command);
				issue (command);
			}
		} else {
			emit (syncUnblocked ());
			sync_command = false;
		}
	}
}

bool RInterface::issueCommand (const QString &command) {
	if (command_running) {
		qDebug ("could not issue synchronous command");
		return false;
	}
	QString command_string = command;
	command_string.append ("\nprint (\"" + end_tag + "\")\n");

	qDebug (command_string);

	issue (command_string);
	sync_command = true;
	return true;
}

void RInterface::issueAsyncCommand (const QString &command) {
	QString command_string = command;
	command_string.append ("\nprint (\"" + end_tag + "\")\n");

	async_command_stack.append (command_string);

	qDebug (command_string);

	// this is the first item in the stack, start pushing immediately
	if (async_command_stack.count () == 1) {
		issue (command_string);
	} else {
		qDebug ("placed in queue");
	}
}

void RInterface::issue (QString &command) {
	emit (syncBlocked ());
	command_running = true;
	if (!busy_writing) {
		// we must keep a local copy while write is in progress!
		command_write_buffer = qstrdup (command);

		writeStdin (command_write_buffer, command.length ());
		emit (writingRequest (command_write_buffer));
		busy_writing = true;
	} else {
		waiting_commands.append (command);
		commands_waiting = true;
	}
}

void RInterface::doneWriting (KProcess *proc) {
	if (commands_waiting) {
		QString command = waiting_commands.first ();
		// we must keep a local copy while write is in progress!
		command_write_buffer = qstrdup (command);

		writeStdin (command_write_buffer, command.length ());
		emit (writingRequest (command_write_buffer));
		command_running = true;
		busy_writing = true;
		if (waiting_commands.removeFirst ()) {
			commands_waiting = false;
		}
	} else {
		busy_writing = false;
	}
}

void RInterface::Rdied (KProcess *proc) {
	emit (receivedReply (r_output));
	if (KMessageBox::questionYesNo (0, "Oh no!\nThe R-Process died. Probably we did something wrong.\nShould I try to restart it?",
			    "Restart R?") == KMessageBox::Yes) {
		start (NotifyOnExit, All);
		command_running = sync_command = false;
		async_command_stack.clear ();
		r_output = "";
	} else {
		emit (syncBlocked ());
	}
}
