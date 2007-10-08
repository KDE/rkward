/***************************************************************************
                          rcommandreceiver  -  description
                             -------------------
    begin                : Thu Aug 19 2004
    copyright            : (C) 2004, 2006, 2007 by Thomas Friedrichsmeier
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

#include <q3valuelist.h>

#include "rcommand.h"

/**
Use this class as a base for all classes that need to handle RCommands. Most importantly it provides a virtual function (rCommandDone ()) for handling of RCommand-results. Reimplement this to interpret the command-results.
For windows/dialogs which interpret RCommand results, the receiver provides a special mechanism. The problem with those windows/dialogs is, that the user may close them, while there are still RCommands due to come in, i.e. they can't be deleted, but rather need to wait for the remaining results to come in.
This class will keep track of which RCommands are still out there (and expected to return to this receiver). When deleting the object, it will unregister from all outstanding commands, so there are invalid pointer operations.

TODO: this mechanism may be slightly costly, if there are *many* commands outstanding. Maybe for special receivers like RKWatch, which are never destroyed at run-time, the mechanism should be disabled.

@author Thomas Friedrichsmeier
*/

class RCommand;
struct ROutput;
typedef Q3ValueList<RCommand*> RCommandList;

class RCommandReceiver {
public:
/** constructor. No args */
	RCommandReceiver ();
/** destructor */
	virtual ~RCommandReceiver ();
/** Causes the receiver to wait until all outstanding_commands (if any) are finished, then deletes itself */
	void autoDeleteWhenDone ();
protected:
	friend class RCommand;
	friend class RInterface;
/** This function is called when a command for this receiver is finished (and before it is deleted). You have to implement it in your subclass to do the actual handling.
@param command A pointer to the command. The pointer is still valid during this call, but the RCommand will be deleted shortly after! */
	virtual void rCommandDone (RCommand *);
/** This function is called when there is new output for a command or this receiver. Default implementation does nothing. Reimplement if you want to get at a command's output immediately (i.e. before the command is fully completed).
@param command A pointer to the command
@param output The new output-fragment */
	virtual void newOutput (RCommand *, ROutput *);
protected:
	RCommandList outstanding_commands;
	void cancelOutstandingCommands ();
private:
	bool delete_when_done;
	void addCommand (RCommand *command);
	void delCommand (RCommand *command);
};

#endif
