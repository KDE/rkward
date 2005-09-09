/***************************************************************************
                          rcommandreceiver  -  description
                             -------------------
    begin                : Thu Aug 19 2004
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
#ifndef RCOMMANDRECEIVER_H
#define RCOMMANDRECEIVER_H

/**
Use this class as a base for all classes that need to handle RCommands. Provides a pure virtual function (rCommandDone ()) for handling of RCommand-results. Reimplement this to interpret the command-results.
For windows/dialogs which interpret RCommand results, the receiver provides a special mechanism. The problem with those windows/dialogs is, that the user may close them, while there are still RCommands due to come in, i.e. they can't be deleted, but rather need to wait for the remaining results to come in.
This class will keep track of which RCommands are still out there (and expected to return to this receiver). If you call deleteThis (), the RCommandReceiver will either self-destruct immediately or wait for any remaining commands to come in. Warning: This means, your class may be delelted immediately. You should return at once after calling deleteThis ().

@author Thomas Friedrichsmeier
*/

class RCommand;

class RCommandReceiver {
public:
    RCommandReceiver () { num_commands_waiting=0; deleted=false; };

    virtual ~RCommandReceiver () {};
	int numCommandsOut () { return num_commands_waiting; };
	void deleteThis () { if (num_commands_waiting > 0) { deleted=true; } else { delete this; } };
protected:
	friend class RCommand;
	virtual void rCommandDone (RCommand *command) = 0;
private:
	int num_commands_waiting;
	bool deleted;
	void addCommand () { ++num_commands_waiting; };
	void delCommand () { if ((--num_commands_waiting <= 0) && deleted) { delete this; } };
};

#endif
