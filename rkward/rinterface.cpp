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
#include <klocale.h>

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

void RInterface::shutdown () {
	disconnect (this, SIGNAL (processExited (KProcess *)), this, SLOT (Rdied (KProcess *)));
	kill ();
	connect (this, SIGNAL (processExited (KProcess *)), this, SLOT (Rdied (KProcess *)));
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

		if (sync_command) {
			emit (syncUnblocked ());
			sync_command = false;
		}
		command_running = false;
		emit (receivedReply (r_output.left (pos)));
		r_output = "";
		if (!sync_command) {
			async_command_stack.removeFirst ();
			if (async_command_stack.count ()) {
				qDebug ("now issueing queued ");
				issue (async_command_stack.first ());
			}
		}
	}
}

bool RInterface::issueCommand (const QString &command) {
	if (command_running) {
		qDebug ("could not issue synchronous command");
		return false;
	}

	issue (command);
	sync_command = true;
	return true;
}

void RInterface::issueAsyncCommand (const QString &command) {
	async_command_stack.append (command);

	// this is the first item in the stack, start pushing immediately
	if (async_command_stack.count () == 1) {
		issue (command);
	} else {
		qDebug ("placed in queue");
	}
}

void RInterface::issue (const QString &command) {
	emit (syncBlocked ());
	command_running = true;
	if (!busy_writing) {
		write (command);
	} else {
		waiting_commands.append (command);
		commands_waiting = true;
	}
}

void RInterface::write (const QString &command) {
	// we must keep a local copy while write is in progress!
	command_write_buffer = qstrdup (command);
	command_write_buffer.append ("\nprint (\"" + end_tag + "\")\n");

	writeStdin (command_write_buffer, command_write_buffer.length ());
	emit (writingRequest (command));
	busy_writing = true;
}

void RInterface::doneWriting (KProcess *proc) {
	if (commands_waiting) {
		QString command = waiting_commands.first ();
		write (command);
		command_running = true;
		if (waiting_commands.removeFirst ()) {
			commands_waiting = false;
		}
	} else {
		busy_writing = false;
	}
}

void RInterface::Rdied (KProcess *proc) {
	emit (receivedReply (r_output));
	command_running = sync_command = false;
	async_command_stack.clear ();
	r_output = "";
	if (KMessageBox::questionYesNo (0, i18n ("Oh no!\nThe R-Process died. Probably we did something wrong.\nShould I try to restart it?\nYou can do so manually via Settings->R Settings."),
			    i18n ("Restart R?")) == KMessageBox::Yes) {
		start (NotifyOnExit, All);
	} else {
		emit (syncBlocked ());
	}
}
