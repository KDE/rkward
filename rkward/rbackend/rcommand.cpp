/***************************************************************************
                          rcommand.cpp  -  description
                             -------------------
    begin                : Mon Nov 11 2002
    copyright            : (C) 2002, 2006, 2007 by Thomas Friedrichsmeier
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
#include "../windows/rkcommandlog.h"
#include "rkrbackendprotocol_shared.h"

#include "../debug.h"
#include "../rkglobals.h"

RCommandBase::RCommandBase (bool is_chain) {
	is_command_chain = is_chain;
	RCommandBase::parent = 0;
}

RCommand* RCommandBase::commandPointer () {
	if (is_command_chain) return 0;
	return static_cast<RCommand*> (this);
}

RCommandChain* RCommandBase::chainPointer () {
	if (!is_command_chain) return 0;
	return static_cast<RCommandChain*> (this);
}


RCommandNotifier::RCommandNotifier () : QObject () {
	RK_TRACE (RBACKEND);
}

RCommandNotifier::~RCommandNotifier () {
	RK_TRACE (RBACKEND);
}

int RCommand::next_id = 0;

RCommand::RCommand(const QString &command, int type, const QString &rk_equiv, RCommandReceiver *receiver, int flags) : RData (), RCommandBase (false) {
	RK_TRACE (RBACKEND);
	_id = next_id++;
// if we ever submit enough commands to get a buffer overflow, use only positive numbers.
	if (next_id < 0) {
		next_id = 0;
	}
	_type = type;
	_flags = flags;
	if (type & Plugin) _command = command.trimmed ();
	else _command = command;
	if (_command.isEmpty ()) _type |= EmptyCommand;
	status = 0;
	has_been_run_up_to = 0;
	_rk_equiv = rk_equiv;
	_notifier = 0;
	for (int i = 0; i < MAX_RECEIVERS_PER_RCOMMAND; ++i) receivers[i] = 0;
	if (!(type & Internal)) {
		addReceiver (receiver);
		addReceiver (RKCommandLog::getLog ());
	}
}

RCommand::~RCommand(){
	RK_TRACE (RBACKEND);

	for (QList<ROutput*>::const_iterator it = output_list.constBegin (); it != output_list.constEnd (); ++it) {
		delete (*it);
	}
	// The output_list itself is cleared automatically

	if (_notifier) delete _notifier;
}

RCommandNotifier* RCommand::notifier () {
	if (!_notifier) {
		RK_TRACE (RBACKEND);
		_notifier = new RCommandNotifier ();
		RK_ASSERT (_notifier);
	}
	return _notifier;
}

void RCommand::addReceiver (RCommandReceiver *receiver) {
	RK_TRACE (RBACKEND);

	if (!receiver) return;

	for (int i = 0; i < MAX_RECEIVERS_PER_RCOMMAND; ++i) {
		if (receivers[i] == 0) {
			receivers[i] = receiver;
			receiver->addCommand (this);
			return;
		}
	}

	RK_DO (qDebug ("Too many receivers for command"), RBACKEND, DL_ERROR);
}

void RCommand::removeReceiver (RCommandReceiver *receiver) {
	RK_TRACE (RBACKEND);

	if (!receiver) return;

	for (int i = 0; i < MAX_RECEIVERS_PER_RCOMMAND; ++i) {
		if (receivers[i] == receiver) {
			receivers[i] = 0;
			return;
		}
	}

	RK_DO (qDebug ("Was not a receiver in RCommand::removeReceiver: %p", receiver), RBACKEND, DL_WARNING);
}

void RCommand::finished () {
	RK_TRACE (RBACKEND);

	for (int i=0; i < MAX_RECEIVERS_PER_RCOMMAND; ++i) {
		if (receivers[i] == 0) continue;
		receivers[i]->delCommand (this);
		receivers[i]->rCommandDone (this);
	}
	if (_notifier) _notifier->emitFinished (this);
}

void RCommand::newOutput (ROutput *output) {
	RK_TRACE (RBACKEND);

	for (int i=0; i < MAX_RECEIVERS_PER_RCOMMAND; ++i) {
		if (receivers[i] == 0) continue;
		receivers[i]->newOutput (this, output);
	}
}

void RCommand::commandLineIn () {
	RK_TRACE (RBACKEND);
	RK_ASSERT (_type & User);

	for (int i=0; i < MAX_RECEIVERS_PER_RCOMMAND; ++i) {
		if (receivers[i] == 0) continue;
		receivers[i]->userCommandLineIn (this);
	}
}

QString RCommand::error () const {
	RK_TRACE (RBACKEND);

	QString ret;
	for (ROutputList::const_iterator it = output_list.begin (); it != output_list.end (); ++it) {
		if ((*it)->type == ROutput::Error) {
			ret.append ((*it)->output);
		}
	}
	return ret;
}

QString RCommand::output () const {
	RK_TRACE (RBACKEND);

	QString ret;
	for (ROutputList::const_iterator it = output_list.begin (); it != output_list.end (); ++it) {
		if ((*it)->type == ROutput::Output) {
			ret.append ((*it)->output);
		}
	}
	return ret;
}

QString RCommand::warnings () const {
	RK_TRACE (RBACKEND);

	QString ret;
	for (ROutputList::const_iterator it = output_list.begin (); it != output_list.end (); ++it) {
		if ((*it)->type == ROutput::Warning) {
			ret.append ((*it)->output);
		}
	}
	return ret;
}

QString RCommand::fullOutput () const {
	RK_TRACE (RBACKEND);

	QString ret;
	for (ROutputList::const_iterator it = output_list.begin (); it != output_list.end (); ++it) {
		ret.append ((*it)->output);
	}
	return ret;
}

QString RCommand::remainingCommand () const {
	RK_TRACE (RBACKEND);
	RK_ASSERT (_type & User);	// not a grave problem, if it's not, but not useful, either

	return _command.mid (has_been_run_up_to);
}

void RCommand::mergeAndDeleteProxy (RCommandProxy *proxy) {
	RK_TRACE (RBACKEND);

	RK_ASSERT (proxy);
	RK_ASSERT (proxy->id == _id);
	RK_ASSERT (proxy->type == _type);

	status = proxy->status;
	has_been_run_up_to = proxy->has_been_run_up_to;
	swallowData (*proxy);
	delete proxy;
}

RCommandProxy* RCommand::makeProxy () const {
	RK_TRACE (RBACKEND);
	RK_ASSERT (status == 0);	// Initialization from an already touched command is not a real problem, but certainly no expected usage
	RK_ASSERT (has_been_run_up_to == 0);
	RK_ASSERT (getDataType () == RData::NoData);

	RCommandProxy *ret = new RCommandProxy (_command, _type);
	ret->id = _id,
	ret->status = status;
	ret->has_been_run_up_to = has_been_run_up_to;
	return ret;
}

#include "rcommand.moc"
