/***************************************************************************
                          rcommand.h  -  description
                             -------------------
    begin                : Mon Nov 11 2002
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

#ifndef RCOMMAND_H
#define RCOMMAND_H

#include <qfile.h>
#include <qstring.h>
#include <qobject.h>

/** This class is used to encapsulate an R-command, so it can be easiyl identified
	in a chain of commands. It is needed, since communication with R is asynchronous
	and it is therefore not possible to get the result of an R-call right away.
	Instead, create an object of this class, specifying the object/slot that should
	be called when the command has finished. You can then retrieve all information
	on the command (including the reply) from the object that is passed to your slot.
	If one slot is to handle the output of several different commands, use the id ()
	function to keep track of the command (it returns a unique id assigned to this
	object). Do not try to keep a pointer to the command. It will be deleted after
	completion (or when/if the R-process dies).
	The type-parameter is used to indicate the significance of this command, and
	esp., whether it should be hidden from the user. (Not yet implemented)
  *@author Thomas Friedrichsmeier
  */

class RCommand : public QObject  {
	Q_OBJECT
public: 
	RCommand(QString command, int type = 0, QString rk_equiv = "", QObject *receiver = 0, const char *slot = 0, int flags=0);
	~RCommand();
	int type () { return _type; };
	QString rkEquivalent () { return _rk_equiv; };
	QString command () { return _command; };
	int id () { return _id; };
/** TODO: Adjust these two functions to allow re-getting of output and error-messages from logs */
	QString output () { return _output; };
	QString error () { return _error; };
/** Adds an additional receiver (i.e. an object/slot that will be notified, when
	this command is completed) */
	void addReceiver (QObject *receiver, const char *slot);
/** Types of commands (potentially more to come), bitwise or-able,
	although partially exclusive. */
	enum CommandTypes {User=1, Plugin=2, PluginCom=4, App=8, Sync=16};
	enum CommandStatus {WasTried=1, Failed=2, HasOutput=4, HasError=8};
	int wasTried () { return (status & WasTried); };
	int failed () { return (status & Failed); };
	int succeeded () { return ((status & WasTried) && !(status & Failed)); };
	int hasOutput () { return (status & HasOutput); }; 
	int hasError () { return (status & HasError); }; 
	int getFlags () { return (_flags); };
signals:
	void commandDone (RCommand *command);
private:
friend class REmbed;
friend class RInterface;
	void finished ();
	QString _output;
	QString _error;
	QString _command;
	int _type;
	int _flags;
	int status;
	QIODevice::Offset output_offset, error_offset;
	int output_length, error_length;
	int logindex;
	QString _rk_equiv;
	int _id;
	static int next_id;
};

#endif
