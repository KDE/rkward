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

class RKwatch;

/** This class does the rather low-level interfacing to the R-processor.
  *@author Thomas Friedrichsmeier
  */

class RInterface : public KProcess  {
	Q_OBJECT
public: 
	RInterface();
	~RInterface();
	bool startR (QStrList &commandline);
/** Submit a synchronous command to R (i.e. generally one, that gives relevant output) */
	bool issueCommand (const QString &command);
/** Submit an asynchronous command (i.e. one, where you don't care about output) */
	void issueAsyncCommand (const QString &command);
	bool commandRunning () { return command_running; };
signals:
	void receivedReply (QString result);
	void writingRequest (QString request);
/** Emitted, when synchronous commands are blocked (i.e. there is another command running) */
	void syncBlocked ();
/** Emitted, when synchronous commands are allowed again */
	void syncUnblocked ();
private:
friend class RKwardApp;

	QStrList async_command_stack;
/** Keeps everything R has so far responded to the last command */
	QString r_output;
/** We have to keep a local buffer for Stdinput to R. */
	QString command_write_buffer;
/** This is used to identify, when a command has finished.
	Should be a unique string. */
	QString end_tag;
	bool command_running;
	bool sync_command;
	bool busy_writing;
	bool commands_waiting;
	QStrList waiting_commands;
	void issue (QString &command);
	RKwatch *watch;
private slots:
/** This slot receives raw R-output */
	void gotROutput (KProcess *proc, char *buffer, int buflen);
/** This slot receives the signal "finished writing Stdinput" */
	void doneWriting (KProcess *proc);
/** This slot gets called, if/when the R-Process dies */
	void Rdied (KProcess *proc);
};

#endif
