/*
rktransmitter - This file is part of RKWard (https://rkward.kde.org). Created: Thu Nov 18 2010
SPDX-FileCopyrightText: 2010-2022 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "rktransmitter.h"

#include "../debug.h"

void RKRBackendSerializer::serialize (const RBackendRequest &request, QDataStream &stream) {
	RK_TRACE (RBACKEND);

	stream << (qint16) request.id;
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
		stream << true;
		serializeOutput (*(request.output), stream);
	} else {
		stream << false;
	}
	stream << request.params;
	if (request.subcommandrequest) {
		stream << true;
		serialize(*(request.subcommandrequest), stream);
	} else {
		stream << false;
	}
}

RBackendRequest *RKRBackendSerializer::unserialize (QDataStream &stream) {
	RK_TRACE (RBACKEND);

	RBackendRequest *request = new RBackendRequest (false, RBackendRequest::OtherRequest);		// will be overwritten
	RBackendRequest::_id--;

	bool dummyb;
	qint8 dummy8;
	qint16 dummy16;
	stream >> dummy16;
	request->id = dummy16;
	stream >> dummy8;
	request->type = (RBackendRequest::RCallbackType) dummy8;
	stream >> request->synchronous;
	stream >> dummyb;
	request->done = dummyb;
	stream >> dummyb;
	if (dummyb) request->command = unserializeProxy (stream);
	stream >> dummyb;
	if (dummyb) request->output = unserializeOutput (stream);
	stream >> request->params;
	stream >> dummyb;
	if (dummyb) request->subcommandrequest = unserialize(stream);

	return request;
}

void RKRBackendSerializer::serializeOutput (const ROutputList &list, QDataStream &stream) {
	RK_TRACE (RBACKEND);

	stream << (qint32) list.size ();
	for (qint32 i = 0; i < list.size (); ++i) {
		stream << (qint8) list[i]->type;
		stream << list[i]->output;
	}
}

ROutputList* RKRBackendSerializer::unserializeOutput (QDataStream &stream) {
	RK_TRACE (RBACKEND);

	ROutputList *ret = new ROutputList ();
	qint32 len;
	stream >> len;
	ret->reserve (len);

	for (qint32 i = 0; i < len; ++i) {
		ROutput* out = new ROutput;
		qint8 dummy8;
		stream >> dummy8;
		out->type = (ROutput::ROutputType) dummy8;
		stream >> out->output;
		ret->append (out);
	}

	return ret;
}

void RKRBackendSerializer::serializeData (const RData &data, QDataStream &stream) {
	RK_TRACE (RBACKEND);

	RData::RDataType type = data.getDataType ();
	stream << (qint8) type;
	if (type == RData::IntVector) stream << data.intVector ();
	else if (type == RData::StringVector) stream << data.stringVector ();
	else if (type == RData::RealVector) stream << data.realVector ();
	else if (type == RData::StructureVector) {
		RData::RDataStorage list = data.structureVector ();
		qint32 len = list.size ();
		stream << len;
		for (qint32 i = 0; i < list.size (); ++i) {
			serializeData (*(list[i]), stream);
		}
	} else {
		RK_ASSERT (type == RData::NoData);
	}
}

RData* RKRBackendSerializer::unserializeData (QDataStream &stream) {
	RK_TRACE (RBACKEND);

	RData* ret = new RData;
	RData::RDataType type;
	qint8 dummy8;
	stream >> dummy8;
	type = (RData::RDataType) dummy8;
	if (type == RData::IntVector) {
		RData::IntStorage data;
		stream >> data;
		ret->setData (data);
	} else if (type == RData::StringVector) {
		RData::StringStorage data;
		stream >> data;
		ret->setData (data);
	} else if (type == RData::RealVector) {
		RData::RealStorage data;
		stream >> data;
		ret->setData (data);
	} else if (type == RData::StructureVector) {
		RData::RDataStorage data;
		qint32 len;
		stream >> len;
		data.reserve (len);
		for (qint32 i = 0; i < len; ++i) {
			data.append (unserializeData (stream));
		}
		ret->setData (data);
	} else {
		RK_ASSERT (type == RData::NoData);
	}

	return ret;
}

void RKRBackendSerializer::serializeProxy (const RCommandProxy &proxy, QDataStream &stream) {
	RK_TRACE (RBACKEND);

	stream << proxy.command;
	stream << (qint32) proxy.type;
	stream << (qint32) proxy.id;
	stream << (qint32) proxy.status;
	stream << (qint32) proxy.has_been_run_up_to;
	stream << proxy.updates_object;

	serializeData (proxy, stream);
}


RCommandProxy* RKRBackendSerializer::unserializeProxy (QDataStream &stream) {
	RK_TRACE (RBACKEND);

	QString command;
	stream >> command;
	qint32 type;
	stream >> type;
	RCommandProxy* ret = new RCommandProxy (command, type);
	qint32 dummy32;
	stream >> dummy32;
	ret->id = dummy32;
	stream >> dummy32;
	ret->status = dummy32;
	stream >> dummy32;
	ret->has_been_run_up_to = dummy32;
	stream >> (ret->updates_object);

	RData *data = unserializeData (stream);
	ret->swallowData (*data);
	delete (data);

	return ret;
}


#include <QTimer>
#include <QLocalSocket>
RKAbstractTransmitter* RKAbstractTransmitter::_instance = nullptr;
RKAbstractTransmitter::RKAbstractTransmitter() : QThread() {
	RK_TRACE (RBACKEND);

	RK_ASSERT(_instance == nullptr);  // NOTE: Although there are two instances of an abstract transmitter in an RKWard session, these live in different processes.
	_instance = this;
	connection = nullptr;

	moveToThread(this);
}

RKAbstractTransmitter::~RKAbstractTransmitter() {
	RK_TRACE(RBACKEND);
	RK_ASSERT(_instance == this);
	_instance = nullptr;
}

void RKAbstractTransmitter::transmitRequest (RBackendRequest *request) {
	RK_TRACE (RBACKEND);
	RK_ASSERT (connection);

	if (!connection->isOpen ()) {
		handleTransmissionError ("Connection not open while trying to write request. Last error was: " + connection->errorString ());
		return;
	}

	RKRBackendSerializer::serialize (*request, streamer.outstream);
	RK_DEBUG (RBACKEND, DL_DEBUG, "Transmitting request type %d of length %d", (int) request->type, streamer.outSize ());
	streamer.writeOutBuffer ();
}

void RKAbstractTransmitter::customEvent (QEvent *e) {
	RK_TRACE (RBACKEND);

	if (((int) e->type ()) == ((int) RKRBackendEvent::RKWardEvent)) {
		RKRBackendEvent *ev = static_cast<RKRBackendEvent*> (e);
		writeRequest (ev->data ());
	} else {
		RK_ASSERT (false);
		return;
	}
}

void RKAbstractTransmitter::fetchTransmission () {
	RK_TRACE (RBACKEND);

	while (connection->bytesAvailable ()) {
		if (!streamer.readInBuffer ()) break;

		requestReceived (RKRBackendSerializer::unserialize (streamer.instream));
		RK_ASSERT (streamer.instream.atEnd ());   // full transmission should have been read
	}

	if (!connection->isOpen ()) {
		handleTransmissionError ("Connection closed unexepctedly. Last error was: " + connection->errorString ());
		return;
	}
}

void RKAbstractTransmitter::setConnection (QLocalSocket *_connection) {
	RK_TRACE (RBACKEND);
	RK_ASSERT (!connection);

	connection = _connection;
	streamer.setIODevice (connection);
	RK_ASSERT (connection->isOpen ());

	connect (connection, &QLocalSocket::readyRead, this, &RKAbstractTransmitter::fetchTransmission);
	connect (connection, &QLocalSocket::disconnected, this, &RKAbstractTransmitter::disconnected);

	// In case something is pending already.
	if (connection->bytesAvailable ()) QTimer::singleShot(0, this, &RKAbstractTransmitter::fetchTransmission);
}

void RKAbstractTransmitter::disconnected () {
	RK_TRACE (RBACKEND);

	if (!connection) return;  // -> May happen in RKRBackendTransmitter::doExit()
	handleTransmissionError ("Connection closed unexpectedly. Last error was: " + connection->errorString ());
}

