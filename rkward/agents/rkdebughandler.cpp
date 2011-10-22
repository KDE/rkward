/***************************************************************************
                          rkdebughandler  -  description
                             -------------------
    begin                : Wed Oct 19 2011
    copyright            : (C) 2011 by Thomas Friedrichsmeier
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

#include "rkdebughandler.h"

#include "../rbackend/rkrbackendprotocol_frontend.h"

#include "../debug.h"

RKDebugHandler* RKDebugHandler::_instance = 0;

RKDebugHandler::RKDebugHandler (QObject *parent) : QObject (parent) {
	RK_TRACE (APP);

	_state = NotInDebugger;
	_request = 0;
	_command = 0;
	_instance = this;
}

RKDebugHandler::~RKDebugHandler () {
	RK_TRACE (APP);
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

	_state = InDebugPrompt;
	newDebugState ();
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

	_command = 0;
	_state = InDebugRun;
	newDebugState ();
}

void RKDebugHandler::endDebug () {
	RK_TRACE (APP);

	_command = 0;
	_request = 0;
	_state = NotInDebugger;
	newDebugState ();
}

#include "rkdebughandler.moc"
