/*
rkdebughandler - This file is part of RKWard (https://rkward.kde.org). Created: Wed Oct 19 2011
SPDX-FileCopyrightText: 2011-2022 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "rkdebughandler.h"

#include "../rbackend/rkrbackendprotocol_frontend.h"

#include "../debug.h"

RKDebugHandler* RKDebugHandler::_instance = nullptr;

RKDebugHandler::RKDebugHandler (QObject *parent) : QObject (parent) {
	RK_TRACE (APP);

	_state = NotInDebugger;
	_request = nullptr;
	_command = nullptr;
	_instance = this;
}

RKDebugHandler::~RKDebugHandler () {
	RK_TRACE (APP);
	RK_ASSERT(_instance == this);
	_instance = nullptr;
}

void RKDebugHandler::debugCall (RBackendRequest *request, RCommand *command) {
	RK_TRACE (APP);

	_command = command;
	_request = request;
	if (command) _output_context = command->fullOutput ();
	else _output_context.clear ();

	_calls = request->params["calls"].toStringList ();
	_functions = request->params["funs"].toStringList ();
	_environments = request->params["envs"].toStringList ();
	_locals = request->params["locals"].toStringList ();
	_prompt = request->params["prompt"].toString ();
	QStringList dummy = request->params["relsrclines"].toStringList ();
	_rel_src_lines.clear ();
	for (int i = 0; i < dummy.size (); ++i) _rel_src_lines.append (dummy.at (i).toInt ());

	_state = InDebugPrompt;
	Q_EMIT newDebugState();
}

void RKDebugHandler::sendCancel () {
	RK_TRACE (APP);

	RK_ASSERT (_request);
	submitDebugString ("Q\n");
}

void RKDebugHandler::submitDebugString (const QString &command) {
	RK_TRACE (APP);

	if (!_request) {
		RK_ASSERT (false);
		return;
	}

	_request->params["result"] = command;

	RKRBackendProtocolFrontend::setRequestCompleted (_request);

	_command = nullptr;
	_state = InDebugRun;
	Q_EMIT newDebugState();
}

void RKDebugHandler::endDebug () {
	RK_TRACE (APP);

	_command = nullptr;
	_request = nullptr;
	_state = NotInDebugger;
	Q_EMIT newDebugState();
}

