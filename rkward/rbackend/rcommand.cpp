/***************************************************************************
                          rcommand.cpp  -  description
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

#include "rcommand.h"
#include "rcommandreceiver.h"

#include "../debug.h"

int RCommand::next_id = 0;

RCommand::RCommand(QString command, int type, QString rk_equiv, RCommandReceiver *receiver, int flags){
	RK_TRACE (RBACKEND);
	_id = next_id++;
// if we ever submit enough commands to get a buffer overflow, use only positive numbers.
	if (next_id < 0) {
		next_id = 0;
	}
	_command = command;
	_type = type;
	_flags = flags;
	status = 0;
	string_data = 0;
	real_data = 0;
	integer_data = 0;
	string_count = real_count = integer_count = 0;
	_rk_equiv = rk_equiv;
	RCommand::receiver = receiver;
}

RCommand::~RCommand(){
	RK_TRACE (RBACKEND);
	for (int i = 0; i < string_count; ++i) {
		delete string_data[i];
	}
	delete [] string_data;
	delete real_data;
	delete integer_data;
}

void RCommand::finished () {
	RK_TRACE (RBACKEND);
	if (receiver) {
		receiver->rCommandDone (this);
	}
}
