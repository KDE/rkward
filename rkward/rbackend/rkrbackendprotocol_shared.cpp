/***************************************************************************
                          rkrbackendprotocol  -  description
                             -------------------
    begin                : Thu Nov 04 2010
    copyright            : (C) 2010 by Thomas Friedrichsmeier
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

RBackendRequest::RBackendRequest (bool synchronous, RCallbackType type) {
	RK_TRACE (RBACKEND);

	RBackendRequest::synchronous = synchronous;
	RBackendRequest::type = type;
	done = false;
	command = 0;
	output = 0;
}

RBackendRequest::~RBackendRequest () {
	RK_TRACE (RBACKEND);

	delete command;
	delete output;
};

#ifndef RKWARD_THREADED
void RBackendRequest::mergeReply (RBackendRequest *reply) {
	RK_TRACE (RBACKEND);

	command = reply->command;
	params = reply->params;
	output = reply->output;
	reply->command = 0;
	reply->output = 0;
}
#endif

RBackendRequest* RBackendRequest::duplicate () {
	RK_TRACE (RBACKEND);

	RBackendRequest* ret = new RBackendRequest (synchronous, type);
	ret->done = done;
	ret->command = command;
	ret->params = params;
	ret->output = output;
	// prevent double deletion issues
	command = 0;
	output = 0;
	return ret;
}


#ifndef RKWARD_THREADED
	QByteArray RKRBackendSerializer::serialize (const RBackendRequest &request) {
		RK_TRACE (RBACKEND);

		QByteArray ret;
		QDataStream stream (&ret, QIODevice::WriteOnly);

		stream << (qint8) request.type;
		stream << request.synchronous;
		stream << request.done;		// well, not really needed, but...
		if (request.command) {
			stream << true;
			serializeProxy (*(request.command), stream);
		} else {
			stream << false;
		}
		if (request.output) {
			RK_ASSERT (request.type == RBackendRequest::Output);
			sream << true;
			serializeOutput (*(request.output), stream);
		} else {
			stream << false;
		}
		stream << request.params;

		return ret;
	}

	RBackendRequest *RKRBackendSerializer::unserialize (const QByteArray &buffer) {
		RK_TRACE (RBACKEND);

		QDataStream stream (buffer);
		RBackendRequest *request = new RBackendRequest (false, RBackendRequest::OtherType);		// will be overwritten

		stream >> (qint8) (RBackendRequest::RCallbackType) (*request.type);
		stream >> (*request.synchronous);
		stream >> (*request.done);
		bool has_command << stream;
		if (has_command) (*request.command) = unserializeProxy (stream);
		bool has_output << stream;
		if (has_output) (*request.output) = unserializeProxy (stream);
		(*request.params) << stream;

		return request;
	}

	void RKRBackendSerializer::serializeOutput (const ROutputList &list, QDataStream &stream) {
		RK_TRACE (RBACKEND);

		stream << (qint32) list.size ();
		for (qint32 i = 0; i < list.size (); ++i) {
			stream << (qint8) list[i].type;
			stream << list[i].output;
		}
	}

	*ROutputList RKRBackendSerializer::unserializeOutput (QDataStream &stream) {
		RK_TRACE (RBACKEND);

		ROutputList *ret = new ROutputList ();
		qint32 len << stream;
#if QT_VERSION >= 0x040700
		ret->reserve (len);
#endif

		for (qint32 i = 0; i < len; ++i) {
			ROutput out;
			out.type << (ROutput::ROutputType) (qint8) stream;
			out.output << stream;
			ret->append (out);
		}

		return ret;
	}

	void RKRBackendSerializer::serializeData (const RData &data, QDataStream &stream) {
		RK_TRACE (RBACKEND);

		RDataType type = data.getDataType ();
		stream << (qint8) type;
		if (type == RData::IntVector) stream << data.getIntVector ();
		else if (type == RData::StringVector) stream << data.getStringVector ();
		else if (type == RData::RealVector) stream << data.getRealVector ();
		else if (type == RData::StructureVector) {
			RData::RDataStorage list = data.getStructureVector ();
			qint32 len = list.size ();
			stream << len;
			for (qint32 i = 0; i < list.size (); ++i) {
				serializeData (*(list[i]));
			}
		} else {
			RK_ASSERT (type == RData::NoData);
		}
	}

	RData* RKRBackendSerializer::unserializeData (QDataStream &stream) {
		RK_TRACE (RBACKEND);

		RData* ret = new RData;
		#error
	}

	void RKRBackendSerializer::serializeProxy (const RCommandProxy &proxy, QDataStream &stream);
#error
	*RCommandProxy RKRBackendSerializer::unserializeProxy (QDataStream &stream);
#endif
