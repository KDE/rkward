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

int RCommand::next_id = 0;

RCommand::RCommand(QString command, int type, QString alternative_text, QObject *receiver = 0, const char *slot){
	_id = next_id++;
	qDebug ("creating command %d", _id);
	_command = command;
	_type = type;
	alttext = alternative_text;
	addReceiver (receiver, slot);
// if we ever submit enough commands to get a buffer overflow, use only positive numbers.
	if (next_id < 0) {
		next_id = 0;
	}
}

RCommand::~RCommand(){
	qDebug ("deleting command %d", _id);
}

void RCommand::finished (const QString &reply) {
	qDebug ("finished command %d", _id);
	_reply = reply;
	emit (commandDone (this));
}

void RCommand::addReceiver (QObject *receiver, const char *slot) {
	if (receiver && slot) {
		connect (this, SIGNAL (commandDone (RCommand *)), receiver, slot);
		qDebug ("connecting command %d", _id);
	}
}