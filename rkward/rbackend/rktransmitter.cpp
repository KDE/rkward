/*
rktransmitter - This file is part of RKWard (https://rkward.kde.org). Created: Thu Nov 18 2010
SPDX-FileCopyrightText: 2010-2022 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "rktransmitter.h"

#include "../debug.h"

void RKRBackendSerializer::serialize(const RBackendRequest &request, QDataStream &stream) {
	RK_TRACE(RBACKEND);

	stream << (qint32)request.id;
	stream << (qint8)request.type;
	stream << request.synchronous;
	stream << request.done; // well, not really needed, but...
	if (request.command) {
		stream << true;
		serializeProxy(*(request.command), stream);
	} else {
		stream << false;
	}
	if (request.output) {
		RK_ASSERT(request.type == RBackendRequest::Output);
		stream << true;
		serializeOutput(*(request.output), stream);
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

template <typename T>
static T readS(QDataStream &stream) {
	T ret;
	stream >> ret;
	return ret;
}

RBackendRequest *RKRBackendSerializer::unserialize(QDataStream &stream) {
	RK_TRACE(RBACKEND);

	RBackendRequest *request = new RBackendRequest(false, RBackendRequest::SubcommandRequest); // will be overwritten
	RBackendRequest::_id--;

	request->id = readS<qint32>(stream);
	request->type = (RBackendRequest::RCallbackType)readS<qint8>(stream);
	stream >> request->synchronous;
	request->done = readS<bool>(stream);
	if (readS<bool>(stream)) request->command = unserializeProxy(stream);
	if (readS<bool>(stream)) request->output = unserializeOutput(stream);
	stream >> request->params;
	if (readS<bool>(stream)) request->subcommandrequest = unserialize(stream);

	return request;
}

void RKRBackendSerializer::serializeOutput(const ROutputList &list, QDataStream &stream) {
	RK_TRACE(RBACKEND);

	stream << (qint32)list.size();
	for (qint32 i = 0; i < list.size(); ++i) {
		stream << (qint8)list[i].type;
		stream << list[i].output;
	}
}

ROutputList *RKRBackendSerializer::unserializeOutput(QDataStream &stream) {
	RK_TRACE(RBACKEND);

	ROutputList *ret = new ROutputList();
	auto len = readS<qint32>(stream);
	ret->reserve(len);

	for (qint32 i = 0; i < len; ++i) {
		ROutput out;
		out.type = (ROutput::ROutputType)readS<qint8>(stream);
		stream >> out.output;
		ret->append(out);
	}

	return ret;
}

void RKRBackendSerializer::serializeData(const RData &data, QDataStream &stream) {
	RK_TRACE(RBACKEND);

	RData::RDataType type = data.getDataType();
	stream << (qint8)type;
	if (type == RData::IntVector) stream << data.intVector();
	else if (type == RData::StringVector) stream << data.stringVector();
	else if (type == RData::RealVector) stream << data.realVector();
	else if (type == RData::StructureVector) {
		RData::RDataStorage list = data.structureVector();
		qint32 len = list.size();
		stream << len;
		for (qint32 i = 0; i < list.size(); ++i) {
			serializeData(*(list[i]), stream);
		}
	} else {
		RK_ASSERT(type == RData::NoData);
	}
}

RData *RKRBackendSerializer::unserializeData(QDataStream &stream) {
	RK_TRACE(RBACKEND);

	RData *ret = new RData;
	RData::RDataType type;
	type = (RData::RDataType)readS<qint8>(stream);
	if (type == RData::IntVector) {
		RData::IntStorage data;
		stream >> data;
		ret->setData(data);
	} else if (type == RData::StringVector) {
		RData::StringStorage data;
		stream >> data;
		ret->setData(data);
	} else if (type == RData::RealVector) {
		RData::RealStorage data;
		stream >> data;
		ret->setData(data);
	} else if (type == RData::StructureVector) {
		RData::RDataStorage data;
		auto len = readS<qint32>(stream);
		data.reserve(len);
		for (qint32 i = 0; i < len; ++i) {
			data.append(unserializeData(stream));
		}
		ret->setData(data);
	} else {
		RK_ASSERT(type == RData::NoData);
	}

	return ret;
}

void RKRBackendSerializer::serializeProxy(const RCommandProxy &proxy, QDataStream &stream) {
	RK_TRACE(RBACKEND);

	stream << proxy.command;
	stream << (qint32)proxy.type;
	stream << (qint32)proxy.id;
	stream << (qint32)proxy.status;
	stream << (qint32)proxy.has_been_run_up_to;
	stream << proxy.updates_object;

	serializeData(proxy, stream);
}

RCommandProxy *RKRBackendSerializer::unserializeProxy(QDataStream &stream) {
	RK_TRACE(RBACKEND);

	auto command = readS<QString>(stream);
	auto type = readS<qint32>(stream);
	RCommandProxy *ret = new RCommandProxy(command, type);
	ret->id = readS<qint32>(stream);
	ret->status = readS<qint32>(stream);
	ret->has_been_run_up_to = readS<qint32>(stream);
	stream >> (ret->updates_object);

	RData *data = unserializeData(stream);
	ret->swallowData(*data);
	delete (data);

	return ret;
}

#include <QLocalSocket>
#include <QTimer>
RKAbstractTransmitter *RKAbstractTransmitter::_instance = nullptr;
RKAbstractTransmitter::RKAbstractTransmitter() : QThread() {
	RK_TRACE(RBACKEND);

	RK_ASSERT(_instance == nullptr); // NOTE: Although there are two instances of an abstract transmitter in an RKWard session, these live in different processes.
	_instance = this;
	connection = nullptr;

	moveToThread(this);
}

RKAbstractTransmitter::~RKAbstractTransmitter() {
	RK_TRACE(RBACKEND);
	RK_ASSERT(_instance == this);
	_instance = nullptr;
}

void RKAbstractTransmitter::transmitRequest(RBackendRequest *request) {
	RK_TRACE(RBACKEND);
	RK_ASSERT(connection);

	if (!connection->isOpen()) {
		handleTransmissionError(u"Connection not open while trying to write request. Last error was: "_s + connection->errorString());
		return;
	}

	RKRBackendSerializer::serialize(*request, streamer.outstream);
	RK_DEBUG(RBACKEND, DL_DEBUG, "Transmitting request type %d of length %d", (int)request->type, streamer.outSize());
	streamer.writeOutBuffer();
}

void RKAbstractTransmitter::customEvent(QEvent *e) {
	RK_TRACE(RBACKEND);

	if (((int)e->type()) == ((int)RKRBackendEvent::RKWardEvent)) {
		RKRBackendEvent *ev = static_cast<RKRBackendEvent *>(e);
		writeRequest(ev->data());
	} else {
		RK_ASSERT(false);
		return;
	}
}

void RKAbstractTransmitter::fetchTransmission() {
	RK_TRACE(RBACKEND);

	while (connection->bytesAvailable()) {
		if (!streamer.readInBuffer()) break;

		requestReceived(RKRBackendSerializer::unserialize(streamer.instream));
		RK_ASSERT(streamer.instream.atEnd()); // full transmission should have been read
	}

	if (!connection->isOpen()) {
		handleTransmissionError(u"Connection closed unexepctedly. Last error was: "_s + connection->errorString());
		return;
	}
}

void RKAbstractTransmitter::setConnection(QLocalSocket *_connection) {
	RK_TRACE(RBACKEND);
	RK_ASSERT(!connection);

	connection = _connection;
	streamer.setIODevice(connection);
	RK_ASSERT(connection->isOpen());

	connect(connection, &QLocalSocket::readyRead, this, &RKAbstractTransmitter::fetchTransmission);
	connect(connection, &QLocalSocket::disconnected, this, &RKAbstractTransmitter::disconnected);

	// In case something is pending already.
	if (connection->bytesAvailable()) QTimer::singleShot(0, this, &RKAbstractTransmitter::fetchTransmission);
}

void RKAbstractTransmitter::disconnected() {
	RK_TRACE(RBACKEND);

	if (!connection) return; // -> May happen in RKRBackendTransmitter::doExit()
	handleTransmissionError(u"Connection closed unexpectedly. Last error was: "_s + connection->errorString());
}
