/***************************************************************************
                          rinterface.h  -  description
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

#ifndef RINTERFACE_H
#define RINTERFACE_H

#include <kprocess.h>
#include <qstrlist.h>
#include <qstring.h>
#include <qptrlist.h>

class RKwatch;
class RCommand;

/** This class does the rather low-level interfacing to the R-processor. The
	interface can be used by submitting new commands with issueCommand () (see
	the RCommand-class). Your command will then be placed in a first in first
	out stack of commands to be executed. If you specified a receiver/slot in the
	constructor of the RCommand, you will be notified, when the command has
	finished.

	Note that since communication with R is asynchronous, there is no way to get
	R-output within the same function, the request is submitted. You have to
	provide a callback-slot, if you're interested in the output. (@see RCommand)
  *@author Thomas Friedrichsmeier
  */

class RInterface : public KProcess  {
	Q_OBJECT
public: 
	RInterface();
	~RInterface();
	bool startR (QStrList &commandline);
	void shutdown ();
	void issueCommand (RCommand *command);
	bool commandRunning () { return command_running; };
signals:
/** Emitted, when synchronous commands are blocked (i.e. there is another command running) */
//	void syncBlocked ();
/** Emitted, when synchronous commands are allowed again */
//	void syncUnblocked ();
private:
friend class RKwardApp;
	QPtrList<RCommand> command_stack;
/** Keeps everything R has so far responded to the last command */
	QString r_output;
/** We have to keep a local buffer for Stdinput to R. */
	QString command_write_buffer;
/** This is used to identify, when a command has finished.
	Should be a unique string. */
	QString end_tag;
	bool command_running;
	bool busy_writing;
/** Commits the next command in the stack, if it can safely be written */
	void tryNextCommand ();
	RKwatch *watch;
/** This is the last step in the chain of committing a command, and actually writes it */
	void write (RCommand *command);
private slots:
/** This slot receives raw R-output */
	void gotROutput (KProcess *proc, char *buffer, int buflen);
/** This slot receives the signal "finished writing Stdinput" */
	void doneWriting (KProcess *proc);
/** This slot gets called, if/when the R-Process dies */
	void Rdied (KProcess *proc);
};

#endif
