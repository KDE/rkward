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
#include "rcommandstack.h"

#include "rinterface.h"

#include "../debug.h"

//static
RCommandStack *RCommandStack::regular_stack;

RCommandStack::RCommandStack () : RCommandChain () {
	RK_TRACE (RBACKEND);
	closed = true;
	parent = 0;
	current_chain = this;
}

RCommandStack::~RCommandStack () {
	RK_TRACE (RBACKEND);
}

void RCommandStack::issueCommand (RCommand *command, RCommandChain *chain) {
	RK_TRACE (RBACKEND);
	if (!chain) chain = regular_stack;
	
	RChainOrCommand *coc = new RChainOrCommand;
	coc->command = command;
	coc->chain = 0;
	chain->commands.append (coc);
}

RCommandChain *RCommandStack::startChain (RCommandChain *parent) {
	RK_TRACE (RBACKEND);
	if (!parent) parent = regular_stack;

	RChainOrCommand *coc = new RChainOrCommand;
	coc->command = 0;
	coc->chain = new RCommandChain;
	coc->chain->closed = false;
	coc->chain->parent = parent;
	parent->commands.append (coc);

	return coc->chain;
}

RCommandChain *RCommandStack::closeChain (RCommandChain *chain) {
	RK_TRACE (RBACKEND);
	if (!chain) return 0;

	chain->closed = true;
	RCommandChain *ret = chain->parent;
	
	RCommandStack *stack = chainStack (chain);
	
	// lets see, whether we can do some cleanup
	while (stack->current_chain->commands.isEmpty () && stack->current_chain->closed && stack->current_chain->parent) {
		RCommandChain *temp = stack->current_chain;
		stack->current_chain = stack->current_chain->parent;
		delete temp;
	}
	
	return ret;
}

bool RCommandStack::isEmpty () {
//	RK_TRACE (RBACKEND);
	return (current_chain->commands.isEmpty () && current_chain->closed);
}

bool RCommandStack::isBlocked () {
//	RK_TRACE (RBACKEND);
	return ((!current_chain->closed) && (!current_chain->commands.isEmpty ()));
}

bool RCommandStack::isActive () {
//	RK_TRACE (RBACKEND);
	return (!current_chain->commands.isEmpty ());
}

//static
RCommandStack *RCommandStack::chainStack (RCommandChain *child) {
	RK_TRACE (RBACKEND);
	while (child->parent) {
		child = child->parent;
	}
	return static_cast<RCommandStack *> (child);
}

RCommand *RCommandStack::pop () {
	RK_TRACE (RBACKEND);

	if (!isActive ()) return 0;
	RCommand *command = 0;
	
	RChainOrCommand *coc = current_chain->commands.first ();
	current_chain->commands.removeFirst ();
	if (coc->command) {
		command = coc->command;
		delete coc;
	} else {
		current_chain = coc->chain;
		delete coc;
		command = pop ();
	}

	// reached end of chain and chain is closed? walk up
	while (current_chain->commands.isEmpty () && current_chain->closed && current_chain->parent) {
		RCommandChain *temp = current_chain;
		current_chain = current_chain->parent;
		delete temp;
	}
	
	return command;
}
