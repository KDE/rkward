/***************************************************************************
                          rcommandstack  -  description
                             -------------------
    begin                : Mon Sep 6 2004
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
#ifndef RCOMMANDSTACK_H
#define RCOMMANDSTACK_H

#include "rcommand.h"

/**
This class represents a top-level RCommandChain. There are/will be two (types of) such chains: One for commands sent to the R-backend for "regular" evaluation, and one for commands sent in reply to a (modal) request from the R-backend via the RRequestHandler.
The class provides convenience functions for determining the state of the Stack (empty/blocked/active) and inserting and fetching commands in the correct order.
Remember to lock RInterface::mutex before accessing any of these functions!

@author Thomas Friedrichsmeier
*/
class RCommandStack : public RCommandChain {
public:
	RCommandStack ();

	~RCommandStack ();

/** add a command to the given chain (static, as it does no matter, which stack the chain belongs to) */
	static void issueCommand (RCommand *command, RCommandChain *chain);

/** add a sub-chain to the given chain (static, as it does no matter, which stack the chain belongs to) */
	static RCommandChain *startChain (RCommandChain *parent);
/** close the given chain, i.e. signal the chain may be deleted once its remaining commands are done
 (static, as it does no matter, which stack the chain belongs to).  @returns the parent of the closed chain. */
	static RCommandChain *closeChain (RCommandChain *chain);

/** @returns true, if there are no commands or open chains waiting in this stack */
	bool isEmpty ();
/** @returns true, if the currently processed chain is not closed and not empty */
	bool isBlocked ();
/** @returns true, if there are commands to be processed in the current chain */
	bool isActive ();

/** removes the RCommand to be processed next from the stack and returns it. If there is no command to process right now, returns 0 */
	RCommand *pop ();

/** the regular command stack, i.e. not a callback */
	static RCommandStack *regular_stack;
/** return the parent RCommandStack of the given RCommandChain */
	static RCommandStack *chainStack (RCommandChain *child);
private:
	RCommandChain *current_chain;
};

#endif
