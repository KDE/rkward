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

	static void issueCommand (RCommand *command, RCommandChain *chain);
	
	static RCommandChain *startChain (RCommandChain *parent);
	static RCommandChain *closeChain (RCommandChain *chain);

	bool isEmpty ();
	bool isBlocked ();
	bool isActive ();
	
	RCommand *pop ();
	
	static RCommandStack *regular_stack;
	static RCommandStack *chainStack (RCommandChain *child);
private:
	RCommandChain *current_chain;
};

#endif
