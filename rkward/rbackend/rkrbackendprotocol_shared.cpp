/*
rkrbackendprotocol - This file is part of RKWard (https://rkward.kde.org). Created: Thu Nov 04 2010
SPDX-FileCopyrightText: 2010-2017 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "rkrbackendprotocol_shared.h"

#include "../debug.h"

RCommandProxy::RCommandProxy(const QString &command, int type) : outer_command(nullptr) {
	RK_TRACE(RBACKEND);

	RCommandProxy::command = command;
	RCommandProxy::type = type;
	id = -1;
	status = 0;
	interruptible_stage = false;
}

RCommandProxy::~RCommandProxy() {
	RK_TRACE(RBACKEND);
}

int RBackendRequest::_id = 0;
RBackendRequest::RBackendRequest(bool synchronous, RCallbackType type) {
	RK_TRACE(RBACKEND);

	RBackendRequest::synchronous = synchronous;
	RBackendRequest::type = type;
	if (++_id >= (1 << 30)) { // Not likely to happen, but never overflow the 32 bit int used for transmission
		_id = 1;
	}
	id = _id;
	done = false;
	command = nullptr;
	output = nullptr;
	subcommandrequest = nullptr;
}

RBackendRequest::~RBackendRequest() {
	RK_TRACE(RBACKEND);

	delete command;
	delete output;
};

void RBackendRequest::mergeReply(RBackendRequest *reply) {
	RK_TRACE(RBACKEND);

	RK_ASSERT(reply->id == id);
	command = reply->command;
	params = reply->params;
	output = reply->output;
	reply->command = nullptr;
	reply->output = nullptr;
}

RBackendRequest *RBackendRequest::duplicate() {
	RK_TRACE(RBACKEND);

	RBackendRequest *ret = new RBackendRequest(synchronous, type);
	--_id; // for pretty, consecutive numbering
	ret->id = id;
	ret->done = done;
	ret->command = command;
	ret->params = params;
	ret->output = output;
	// prevent double deletion issues
	command = nullptr;
	output = nullptr;
	return ret;
}

void RBackendRequest::setResult(const GenericRRequestResult &res) {
	if (!res.warning.isNull()) params[QStringLiteral(".w")] = res.warning;
	if (!res.error.isNull()) params[QStringLiteral(".e")] = res.error;
	else params[QStringLiteral(".r")] = res.ret;
}

GenericRRequestResult RBackendRequest::getResult() const {
	return GenericRRequestResult(params.value(QStringLiteral(".r")), params.value(QStringLiteral(".w")).toString(), params.value(QStringLiteral(".e")).toString());
}

#define MAX_BUF_LENGTH 16000
#define OUTPUT_STRING_RESERVE 1000

RKROutputBuffer::RKROutputBuffer() {
	RK_TRACE(RBACKEND);

	out_buf_len = 0;
}

RKROutputBuffer::~RKROutputBuffer() {
	RK_TRACE(RBACKEND);

	if (!output_captures.isEmpty()) RK_DEBUG(RBACKEND, DL_WARNING, "%d requests for recording output still active on interface shutdown", output_captures.size());
}

void RKROutputBuffer::pushOutputCapture(int capture_mode) {
	RK_TRACE(RBACKEND);

	OutputCapture capture;
	capture.mode = capture_mode;
	output_captures.append(capture);
}

QString RKROutputBuffer::popOutputCapture(bool highlighted) {
	RK_TRACE(RBACKEND);

	if (output_captures.isEmpty()) {
		RK_ASSERT(!output_captures.isEmpty());
		return QString();
	}
	OutputCapture capture = output_captures.takeLast();
	if (capture.recorded.isEmpty()) return QString();

	QString ret;
	ROutput::ROutputType previous_type = ROutput::NoOutput;
	for (int i = 0; i < capture.recorded.length(); ++i) {
		const auto output = capture.recorded[i];
		if (output.output.isEmpty()) continue;

		if (output.type != ROutput::Error) { // NOTE: skip error output. It has already been written as a warning.
			if (highlighted && (output.type != previous_type)) {
				if (!ret.isEmpty()) ret.append(u"</pre>\n"_s);

				if (output.type == ROutput::Output) ret.append(u"<pre class=\"output_normal\">"_s);
				else if (output.type == ROutput::Warning) ret.append(u"<pre class=\"output_warning\">"_s);
				else {
					RK_ASSERT(false);
					ret.append(u"<pre>"_s);
				}
			}
			if (highlighted) ret.append(output.output.toHtmlEscaped());
			else ret.append(output.output);

			previous_type = output.type;
		}
	}
	if (highlighted && !ret.isEmpty()) ret.append(u"</pre>\n"_s);
	return ret;
}

void appendToOutputList(ROutputList *list, const QString &output, ROutput::ROutputType output_type) {
	// No trace
	// Merge with previous output fragment, if of the same type
	if (!list->isEmpty() && list->last().type == output_type && output_type != ROutput::CommandLineIn) {
		list->last().output.append(output);
	} else {
		QString spaced = output;
		spaced.reserve(OUTPUT_STRING_RESERVE);
		list->append(ROutput(output_type, spaced));
	}
}

bool RKROutputBuffer::handleOutput(const QString &output, int buf_length, ROutput::ROutputType output_type, bool allow_blocking) {
	if (!buf_length) return false;
	RK_TRACE(RBACKEND);
	RK_DEBUG(RBACKEND, DL_DEBUG, "Output type %d: %s", output_type, qPrintable(output));

	// wait while the output buffer is exceeded to give downstream threads a chance to catch up
	while ((out_buf_len > MAX_BUF_LENGTH) && allow_blocking) {
		if (!doMSleep(10)) break;
	}

	QMutexLocker lock(&output_buffer_mutex);
	bool previously_empty = (out_buf_len <= 0);

	for (int i = output_captures.length() - 1; i >= 0; --i) {
		OutputCapture &cap = output_captures[i];
		if (output_type == ROutput::Output) {
			if (cap.mode & RecordOutput) appendToOutputList(&(cap.recorded), output, output_type);
			if (cap.mode & SuppressOutput) return previously_empty;
		} else {
			if (cap.mode & RecordMessages) appendToOutputList(&(cap.recorded), output, output_type);
			if (cap.mode & SuppressMessages) return previously_empty;
		}
		if (cap.mode & NoNesting) break;
	}

	appendToOutputList(&output_buffer, output, output_type);
	out_buf_len += buf_length;

	return previously_empty;
}

ROutputList RKROutputBuffer::flushOutput(bool forcibly) {
	ROutputList ret;

	if (out_buf_len == 0) return ret; // if there is absolutely no output, just skip.
	RK_TRACE(RBACKEND);

	if (!forcibly) {
		if (!output_buffer_mutex.tryLock()) return ret;
	} else {
		output_buffer_mutex.lock();
	}

	RK_ASSERT(!output_buffer.isEmpty()); // see check for out_buf_len, above

	ret = output_buffer;
	output_buffer.clear();
	out_buf_len = 0;

	output_buffer_mutex.unlock();
	return ret;
}

QString RKRSharedFunctionality::quote(const QString &string) {
	QString ret;
	int s = string.size();
	ret.reserve(s + 2); // typical case: Only quotes added, no escapes needed.
	ret.append(u'\"');
	for (int i = 0; i < s; ++i) {
		const QChar c = string[i];
		if ((c == u'\\') || (c == u'\"')) ret.append(u'\\');
		ret.append(c);
	}
	ret.append(u'\"');

	return ret;
}

#include <QFile>
/* These definitions don't really belong into this file, but, importantly, they need to be compiled for both frontend and backend. */
namespace RK_Debug {
int RK_Debug_Level = 0;
int RK_Debug_Flags = DEBUG_ALL;
int RK_Debug_CommandStep = 0;
QFile *debug_file = nullptr;

bool setupLogFile(const QString &basename) {
	QStringList all_debug_files(basename);
	all_debug_files << basename + u".0"_s << basename + u".1"_s;
	for (int i = all_debug_files.size() - 1; i >= 0; --i) {
		QFile oldfile(all_debug_files[i]);
		if (oldfile.exists()) {
			if (i < all_debug_files.size() - 1) {
				oldfile.rename(all_debug_files[i + 1]);
			} else {
				oldfile.remove();
			}
		}
	}
	debug_file = new QFile(basename);
	return (debug_file->open(QIODevice::WriteOnly | QIODevice::Truncate));
}
}; // namespace RK_Debug
