/***************************************************************************
                          rkrbackendprotocol  -  description
                             -------------------
    begin                : Thu Nov 04 2010
    copyright            : (C) 2010, 2011, 2013 by Thomas Friedrichsmeier
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

#include "rkrbackendprotocol_shared.h"

#include "../debug.h"

RCommandProxy::RCommandProxy (const QString &command, int type) {
	RK_TRACE (RBACKEND);

	RCommandProxy::command = command;
	RCommandProxy::type = type;
	id = -1;
	status = 0;
}

RCommandProxy::~RCommandProxy () {
	RK_TRACE (RBACKEND);

	RK_ASSERT ((type & RCommand::Internal) || (getDataType () == RData::NoData));
}


int RBackendRequest::_id = 0;
RBackendRequest::RBackendRequest (bool synchronous, RCallbackType type) {
	RK_TRACE (RBACKEND);

	RBackendRequest::synchronous = synchronous;
	RBackendRequest::type = type;
	id = ++_id;
	done = false;
	command = 0;
	output = 0;
}

RBackendRequest::~RBackendRequest () {
	RK_TRACE (RBACKEND);

	delete command;
	delete output;
};

void RBackendRequest::mergeReply (RBackendRequest *reply) {
	RK_TRACE (RBACKEND);

	RK_ASSERT (reply->id == id);
	command = reply->command;
	params = reply->params;
	output = reply->output;
	reply->command = 0;
	reply->output = 0;
}

RBackendRequest* RBackendRequest::duplicate () {
	RK_TRACE (RBACKEND);

	RBackendRequest* ret = new RBackendRequest (synchronous, type);
	--_id;   // for pretty, consecutive numbering
	ret->id = id;
	ret->done = done;
	ret->command = command;
	ret->params = params;
	ret->output = output;
	// prevent double deletion issues
	command = 0;
	output = 0;
	return ret;
}



#define MAX_BUF_LENGTH 16000
#define OUTPUT_STRING_RESERVE 1000

RKROutputBuffer::RKROutputBuffer () {
	RK_TRACE (RBACKEND);

	out_buf_len = 0;
}

RKROutputBuffer::~RKROutputBuffer () {
	RK_TRACE (RBACKEND);
}

bool RKROutputBuffer::handleOutput (const QString &output, int buf_length, ROutput::ROutputType output_type, bool allow_blocking) {
	if (!buf_length) return false;
	RK_TRACE (RBACKEND);

	RK_DEBUG (RBACKEND, DL_DEBUG, "Output type %d: %s", output_type, qPrintable (output));

	// wait while the output buffer is exceeded to give downstream threads a chance to catch up
	while ((out_buf_len > MAX_BUF_LENGTH) && allow_blocking) {
		if (!doMSleep (10)) break;
	}

	output_buffer_mutex.lock ();
	bool previously_empty = (out_buf_len <= 0);

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
	return previously_empty;
}

ROutputList RKROutputBuffer::flushOutput (bool forcibly) {
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



QString RKRSharedFunctionality::quote (const QString &string) {
	QString ret;
	int s = string.size ();
	ret.reserve (s + 2);	// typical case: Only quotes added, no escapes needed.
	ret.append ('\"');
	for (int i = 0; i < s; ++i) {
		const QChar c = string[i];
		if ((c == '\\') || (c == '\"')) ret.append ('\\');
		ret.append (c);
	}
	ret.append ('\"');

	return ret;
}
