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
#include <qptrlist.h>

class RCommandReceiver;

class RCommand;
class RChainOrCommand;
/// this struct is needed by the rthread.
class RCommandChain {
private:
friend class RThread;
	QPtrList<RChainOrCommand> commands;
	bool closed;
	RCommandChain *parent;
};
/// this struct is needed by the rthread.
class RChainOrCommand {
private:
friend class RThread;
	RCommand *command;
	RCommandChain *chain;
};


/** This class is used to encapsulate an R-command, so it can be easiyl identified
	in a chain of commands. It is needed, since communication with R is asynchronous
	and it is therefore not possible to get the result of an R-call right away.
	Instead, create an object of this class, specifying the object/slot that should
	be called when the command has finished. You can then retrieve all information
	on the command (including the reply) from the object that is passed to your slot.
	If one slot is to handle the output of several different commands, use the id ()
	function to keep track of the command (it returns a unique id assigned to this
	object).
	The type-parameter is used to indicate the significance of this command, and
	esp., whether it should be hidden from the user. (Not yet implemented)
	There are several ways to identify a command when it's finished:
		- storing the id ()
		- passing appropriate flags to know how to handle the command
		- keeping the pointer (CAUTION: don't use that pointer except to compare it with the pointer of an incoming command. Commands get deleted when they are finished, and maybe (in the future) if they become obsolete etc. Hence the pointers you keep may be invalid!)
		(- checking the command-string)
	Note that an RCommand carries a whole lot of information around. However, RCommands generally don't get
	kept around very long, so they should not be a memory issue.
  *@author Thomas Friedrichsmeier
  */
  
class RCommand {
public: 
	RCommand(QString command, int type = 0, QString rk_equiv = "", RCommandReceiver *receiver=0, int flags=0);
	~RCommand();
	int type () { return _type; };
	QString rkEquivalent () { return _rk_equiv; };
	QString command () { return _command; };
	int id () { return _id; };
/** TODO: Adjust these two functions to allow re-getting of output and error-messages from logs */
	QString output () { return _output; };
	QString error () { return _error; };
/** Types of commands (potentially more to come), bitwise or-able,
	although partially exclusive. */
	enum CommandTypes {User=1, Plugin=2, PluginCom=4, App=8, Sync=16, EmptyCommand=32, GetIntVector=512, GetStringVector=1024, GetRealVector=2048, DirectToOutput=4096};
	enum CommandStatus {WasTried=1, Failed=2, HasOutput=4, HasError=8};
	bool wasTried () { return (status & WasTried); };
	bool failed () { return (status & Failed); };
	bool succeeded () { return ((status & WasTried) && !(status & Failed)); };
	bool hasOutput () { return (status & HasOutput); };
	bool hasError () { return (status & HasError); };
	int stringVectorLength () { return (string_count); };
	int realVectorLength () { return (real_count); };
	int intVectorLength () { return (integer_count); };
	char **getStringVector () { return (string_data); };
	double *getRealVector () { return (real_data); };
	int *getIntVector () { return (integer_data); };
/// if you want to keep the data, use this function to detach it from the RCommand (after reading it), so it won't be deleted with the RCommand
	void detachStringVector () { string_data = 0; string_count = 0; };
/// if you want to keep the data, use this function to detach it from the RCommand (after reading it), so it won't be deleted with the RCommand
	void detachRealVector () { real_data = 0; real_count = 0; };
/// if you want to keep the data, use this function to detach it from the RCommand (after reading it), so it won't be deleted with the RCommand
	void detachIntVector () { integer_data = 0; integer_count = 0; };
	int getFlags () { return (_flags); };
private:
friend class REmbed;
friend class RInterface;
	void finished ();
	QString _output;
	QString _error;
	QString _command;
	char **string_data;
	int string_count;
	double *real_data;
	int real_count;
	int *integer_data;
	int integer_count;
	int _type;
	int _flags;
	int status;
	QString _rk_equiv;
	int _id;
	static int next_id;
	RCommandReceiver *receiver;
};

#endif
