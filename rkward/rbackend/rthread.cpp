/***************************************************************************
                          rthread  -  description
                             -------------------
    begin                : Mon Aug 2 2004
    copyright            : (C) 2004, 2006, 2007, 2008, 2009, 2010 by Thomas Friedrichsmeier
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
#include "rembedinternal.h"

#include "rkrbackendprotocol.h"
#include "../rkglobals.h"
#include "../version.h"

#include "../debug.h"

#include <QCoreApplication>
#include <klocale.h>

#define MAX_BUF_LENGTH 16000
#define OUTPUT_STRING_RESERVE 1000

void RKRBackend::run () {
	RK_TRACE (RBACKEND);
	killed = NotKilled;
	previous_command = 0;

	initialize ();

	enterEventLoop ();
}

void RKRBackend::commandFinished (bool check_object_updates_needed) {
	RK_TRACE (RBACKEND);
	RK_DO (qDebug ("done running command"), RBACKEND, DL_DEBUG);

	current_command->status -= (current_command->status & RCommand::Running);
	current_command->status |= RCommand::WasTried;

	if (check_object_updates_needed || (current_command->type & RCommand::ObjectListUpdate)) {
		checkObjectUpdatesNeeded (current_command->type & (RCommand::User | RCommand::ObjectListUpdate));
	}

	previous_command = current_command;

	all_current_commands.pop_back();
	if (!all_current_commands.isEmpty ()) current_command = all_current_commands.last ();
}

RCommandProxy* RKRBackend::handleRequest (RBackendRequest *request, bool mayHandleSubstack) {
	RK_TRACE (RBACKEND);
	RK_ASSERT (request);

	RKRBackendProtocolBackend::instance ()->sendRequest (request);

	if (!request->synchronous) {
		RK_ASSERT (mayHandleSubstack);	// i.e. not called from fetchNextCommand
		return 0;
	}

	while (!request->done) {
		if (killed) return 0;
		// NOTE: processX11Events() may, conceivably, lead to new requests, which may also wait for sub-commands!
		processX11Events ();
		if (!request->done) RKRBackendProtocolBackend::msleep (1);
	}

	RCommandProxy* command = request->command;
	if (!command) return 0;

	all_current_commands.append (command);
	current_command = command;

	if (!mayHandleSubstack) return command;
	
	while (command) {
		runCommand (command);
		commandFinished (false);

		command = fetchNextCommand ();
	};

	return 0;
}

RCommandProxy* RKRBackend::fetchNextCommand () {
	RK_TRACE (RBACKEND);

	RBackendRequest req (true, RBackendRequest::CommandOut);
	req.command = previous_command;
	previous_command = 0;

	return (handleRequest (&req, false));
}

void RKRBackend::waitIfOutputBufferExceeded () {
	// don't trace
	while (out_buf_len > MAX_BUF_LENGTH) {
		if (isKilled ()) return;	// don't block. Frontend could be crashed
		RKRBackendProtocolBackend::msleep (10);
	}
}

void RKRBackend::handleOutput (const QString &output, int buf_length, ROutput::ROutputType output_type) {
	if (!buf_length) return;
	RK_TRACE (RBACKEND);

	RK_DO (qDebug ("Output type %d: %s", output_type, qPrintable (output)), RBACKEND, DL_DEBUG);
	waitIfOutputBufferExceeded ();

	output_buffer_mutex.lock ();

	ROutput *current_output = 0;
	if (!output_buffer.isEmpty ()) {
		// Merge with previous output fragment, if of the same type
		current_output = output_buffer.last ();
		if (current_output->type != output_type) current_output = 0;
	}
	if (!current_output) {
		current_output = new ROutput;
		current_output->type = output_type;
		current_output->output.reserve (OUTPUT_STRING_RESERVE);
		output_buffer.append (current_output);
	}
	current_output->output.append (output);
	out_buf_len += buf_length;

	output_buffer_mutex.unlock ();
}

ROutputList RKRBackend::flushOutput (bool forcibly) {
	ROutputList ret;

	if (out_buf_len == 0) return ret;		// if there is absolutely no output, just skip.
	RK_TRACE (RBACKEND);

	if (!forcibly) {
		if (!output_buffer_mutex.tryLock ()) return ret;
	} else {
		output_buffer_mutex.lock ();
	}

	RK_ASSERT (!output_buffer.isEmpty ());	// see check for out_buf_len, above

	ret = output_buffer;
	output_buffer.clear ();
	out_buf_len = 0;

	output_buffer_mutex.unlock ();
	return ret;
}

void RKRBackend::handleHistoricalSubstackRequest (const QStringList &list) {
	RK_TRACE (RBACKEND);

	RBackendRequest request (true, RBackendRequest::HistoricalSubstackRequest);
	request.params["call"] = list;
	handleRequest (&request);
}                                                                        

void RKRBackend::initialize () {
	RK_TRACE (RBACKEND);

	// in RInterface::RInterface() we have created a fake RCommand to capture all the output/errors during startup. Fetch it
	repl_status.eval_depth++;
	fetchNextCommand ();

	startR ();

	bool lib_load_fail = false;
	bool sink_fail = false;
	if (!runDirectCommand ("library (\"rkward\")\n")) lib_load_fail = true;
	if (!runDirectCommand (QString ("stopifnot(.rk.app.version==\"%1\")\n").arg (VERSION))) lib_load_fail = true;
	if (!runDirectCommand (".rk.fix.assignments ()\n")) sink_fail = true;

// error/output sink and help browser
	if (!runDirectCommand ("options (error=quote (.rk.do.error ()))\n")) sink_fail = true;

	QString error_messages;
	if (lib_load_fail) {
		error_messages.append (i18n ("</p>\t- The 'rkward' R-library either could not be loaded at all, or not in the correct version. This may lead to all sorts of errors, from single missing features to complete failure to function. The most likely cause is that the last installation did not place all files in the correct place. However, in some cases, left-overs from a previous installation that was not cleanly removed may be the cause.</p>\
		<p><b>You should quit RKWard, now, and fix your installation</b>. For help with that, see <a href=\"http://p.sf.net/rkward/compiling\">http://p.sf.net/rkward/compiling</a>.</p>\n"));
	}
	if (sink_fail) {
		error_messages.append (i18n ("<p>\t-There was a problem setting up the communication with R. Most likely this indicates a broken installation.</p>\
		<p><b>You should quit RKWard, now, and fix your installation</b>. For help with that, see <a href=\"http://p.sf.net/rkward/compiling\">http://p.sf.net/rkward/compiling</a>.</p></p>\n"));
	}

	RBackendRequest req (true, RBackendRequest::Started);
	req.params["message"] = QVariant (error_messages);
	// blocks until RKWard is set to go (esp, it has displayed startup error messages, etc.)
	// in fact, a number of sub-commands are run while handling this request!
	handleRequest (&req);

	commandFinished ();		// the fake startup command
	repl_status.eval_depth--;
}

void RKRBackend::checkObjectUpdatesNeeded (bool check_list) {
	RK_TRACE (RBACKEND);
	if (killed) return;

	/* NOTE: We're keeping separate lists of the items on the search path, and the toplevel symbols in .GlobalEnv here.
	This info is also present in RObjectList (and it's children). However: a) in a less convenient form, b) in the other thread. To avoid locking, and other complexity, keeping separate lists seems an ok solution. Keep in mind that only the names of only the toplevel objects are kept, here, so the memory overhead should be minimal */

	bool search_update_needed = false;
	bool globalenv_update_needed = false;

	if (check_list) {	
	// TODO: avoid parsing this over and over again
		RK_DO (qDebug ("checkObjectUpdatesNeeded: getting search list"), RBACKEND, DL_TRACE);
		RCommandProxy *dummy = runDirectCommand ("search ()\n", RCommand::GetStringVector);
		if (dummy->getStringVector () != toplevel_env_names) search_update_needed = true;
		if (search_update_needed) toplevel_env_names = dummy->getStringVector ();
		delete dummy;
	
	// TODO: avoid parsing this over and over again
		RK_DO (qDebug ("checkObjectUpdatesNeeded: getting globalenv symbols"), RBACKEND, DL_TRACE);
		dummy = runDirectCommand ("ls (globalenv (), all.names=TRUE)\n", RCommand::GetStringVector);
		QStringList new_globalenv_toplevel_names = dummy->getStringVector ();
		if (new_globalenv_toplevel_names.count () != global_env_toplevel_names.count ()) {
			globalenv_update_needed = true;
		} else {
			for (unsigned int i = 0; i < new_globalenv_toplevel_names.count (); ++i) {
				// order is not important in the symbol list
				if (!global_env_toplevel_names.contains (new_globalenv_toplevel_names[i])) {
					globalenv_update_needed = true;
					break;
				}
			}
		}
		if (globalenv_update_needed) global_env_toplevel_names = new_globalenv_toplevel_names;
		delete dummy;
	
		if (search_update_needed) {	// this includes an update of the globalenv, even if not needed
			QStringList call = toplevel_env_names;
			call.prepend ("syncenvs");	// should be faster than the reverse
			handleHistoricalSubstackRequest (call);
		} 
		if (globalenv_update_needed) {
			QStringList call = global_env_toplevel_names;
			call.prepend ("syncglobal");	// should be faster than the reverse
			handleHistoricalSubstackRequest (call);
		}
	}

	if (search_update_needed || globalenv_update_needed) {
		RK_DO (qDebug ("checkObjectUpdatesNeeded: updating watches"), RBACKEND, DL_TRACE);
		runDirectCommand (".rk.watch.globalenv ()\n");
	}

	if (!changed_symbol_names.isEmpty ()) {
		QStringList call = changed_symbol_names;
		call.prepend (QString ("sync"));	// should be faster than reverse
		handleHistoricalSubstackRequest (call);
		changed_symbol_names.clear ();
	}
}
