/*
rcommand.cpp - This file is part of RKWard (https://rkward.kde.org). Created: Mon Nov 11 2002
SPDX-FileCopyrightText: 2002-2007 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "rcommand.h"
#include "rkrinterface.h"
#include "../windows/rkcommandlog.h"
#include "rkrbackendprotocol_shared.h"
#include "../core/robject.h"

#include "../debug.h"


RCommand* RCommandChain::toCommand() {
	return (is_command ? static_cast<RCommand*>(this) : nullptr);
}


RCommandNotifier::RCommandNotifier () : QObject () {
	RK_TRACE (RBACKEND);
}

RCommandNotifier::~RCommandNotifier () {
	RK_TRACE (RBACKEND);
}

int RCommand::next_id = 0;

RCommand::RCommand(const QString &command, int type, const QString &rk_equiv) : RData (), RCommandChain (false) {
	RK_TRACE (RBACKEND);
	_id = next_id++;
// if we ever submit enough commands to get a buffer overflow, use only positive numbers.
	if (next_id < 0) {
		next_id = 0;
	}
	_type = type;
	if (type & Plugin) _command = command.trimmed ();
	else _command = command;
	if (_command.isEmpty ()) _type |= EmptyCommand;
	status = 0;
	has_been_run_up_to = 0;
	_rk_equiv = rk_equiv;
	_notifier = nullptr;
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

void RCommand::finished () {
	RK_TRACE (RBACKEND);

	RKCommandLog::getLog()->rCommandDone(this);
	if (_notifier) _notifier->emitFinished (this);
}

void RCommand::newOutput (ROutput *output) {
	RK_TRACE (RBACKEND);

	RKCommandLog::getLog()->newOutput(this, output);
	if (_notifier) _notifier->emitOutput(this, output);
}

void RCommand::commandLineIn () {
	RK_TRACE (RBACKEND);
	RK_ASSERT (_type & User);
	if (_notifier) _notifier->emitLineIn(this);
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

void RCommand::setUpdatesObject(const RObject *object) {
	RK_TRACE(RBACKEND);
	RK_ASSERT(_updated_object.isNull());
	_updated_object = object->globalEnvSymbol()->getShortName();
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
	ret->updates_object = _updated_object;
	ret->id = _id,
	ret->status = status;
	ret->has_been_run_up_to = has_been_run_up_to;
	return ret;
}

QString RCommand::rQuote (const QString &quoted) {
	return RObject::rQuote (quoted);
}

