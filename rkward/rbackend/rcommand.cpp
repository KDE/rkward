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
#include "rinterface.h"
#include "../rkwatch.h"

#include "../debug.h"
#include "../rkglobals.h"

#define MAX_RECEIVERS 3

int RCommand::next_id = 0;

RCommand::RCommand(const QString &command, int type, const QString &rk_equiv, RCommandReceiver *receiver, int flags){
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
	RCommand::receivers = new RCommandReceiver* [MAX_RECEIVERS];
	num_receivers = 0;
	addReceiver (receiver);
	addReceiver (RKGlobals::rInterface ()->watch);
}

RCommand::~RCommand(){
	RK_TRACE (RBACKEND);
	for (int i = 0; i < string_count; ++i) {
		DELETE_STRING (string_data[i]);
	}
	delete [] string_data;
	delete real_data;
	delete integer_data;

	for (QValueList<ROutput*>::iterator it = output_list.begin (); it != output_list.end (); ++it) {
		delete (*it);
	}
	// The output_list itself is cleared automatically
}

void RCommand::addReceiver (RCommandReceiver *receiver) {
	RK_TRACE (RBACKEND);

	if (!receiver) return;

	if (num_receivers >= MAX_RECEIVERS) {
		RK_DO (qDebug ("Too many receivers for command"), RBACKEND, DL_ERROR);
		return;
	}

	receivers[num_receivers++] = receiver;
	receiver->addCommand ();
}


void RCommand::finished () {
	RK_TRACE (RBACKEND);

	for (int i=0; i < num_receivers; ++i) {
		receivers[i]->rCommandDone (this);
		receivers[i]->delCommand ();
	}
}

void RCommand::newOutput (ROutput *output) {
	RK_TRACE (RBACKEND);

	for (int i=0; i < num_receivers; ++i) {
		receivers[i]->newOutput (this, output);
	}
}

QString RCommand::error () {
	RK_TRACE (RBACKEND);

	QString ret;
	for (QValueList<ROutput*>::const_iterator it = output_list.begin (); it != output_list.end (); ++it) {
		if ((*it)->type == ROutput::Error) {
			ret.append ((*it)->output);
		}
	}
	return ret;
}

QString RCommand::output () {
	RK_TRACE (RBACKEND);

	QString ret;
	for (QValueList<ROutput*>::const_iterator it = output_list.begin (); it != output_list.end (); ++it) {
		if ((*it)->type == ROutput::Output) {
			ret.append ((*it)->output);
		}
	}
	return ret;
}

QString RCommand::warnings () {
	RK_TRACE (RBACKEND);

	QString ret;
	for (QValueList<ROutput*>::const_iterator it = output_list.begin (); it != output_list.end (); ++it) {
		if ((*it)->type == ROutput::Warning) {
			ret.append ((*it)->output);
		}
	}
	return ret;
}

QString RCommand::fullOutput () {
	RK_TRACE (RBACKEND);

	QString ret;
	for (QValueList<ROutput*>::const_iterator it = output_list.begin (); it != output_list.end (); ++it) {
		ret.append ((*it)->output);
	}
	return ret;
}
