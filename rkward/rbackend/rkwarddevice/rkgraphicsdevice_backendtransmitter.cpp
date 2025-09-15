/*
rkgraphicsdevice_backendtransmitter - This file is part of RKWard (https://rkward.kde.org). Created: Mon Mar 18 2013
SPDX-FileCopyrightText: 2013-2025 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "rkgraphicsdevice_backendtransmitter.h"

#include "../rkbackendtransmitter.h"
#include "../rkrbackendprotocol_backend.h"

#include <QLocalSocket>
#include <QWaitCondition>

#include "../../debug.h"

RKAsyncDataStreamHelper<RKGraphicsDeviceTransmittionLengthType> RKGraphicsDeviceBackendTransmitter::streamer;
QLocalSocket *RKGraphicsDeviceBackendTransmitter::connection = nullptr;
QMutex RKGraphicsDeviceBackendTransmitter::mutex;
RKGraphicsDeviceBackendTransmitter *RKGraphicsDeviceBackendTransmitter::_instance = nullptr;

QWaitCondition write_available;
QWaitCondition read_available;

RKGraphicsDeviceBackendTransmitter::RKGraphicsDeviceBackendTransmitter() : QThread(), alive(true), expecting_reply(false), have_reply(false), commit_pending(false) {
	RK_TRACE(GRAPHICS_DEVICE);

	RK_ASSERT(!_instance); // singleton!
	start();
}

RKGraphicsDeviceBackendTransmitter::~RKGraphicsDeviceBackendTransmitter() {
	RK_TRACE(GRAPHICS_DEVICE);
	delete connection;
}

RKGraphicsDeviceBackendTransmitter *RKGraphicsDeviceBackendTransmitter::instance() {
	if (_instance) return _instance;
	RK_TRACE(GRAPHICS_DEVICE);

	QMutexLocker lock(&mutex);
	_instance = new RKGraphicsDeviceBackendTransmitter();
	read_available.wait(&mutex); // signifies startup state, here
	return _instance;
}

bool RKGraphicsDeviceBackendTransmitter::connectionAlive() {
	if (!_instance) return false;
	return _instance->alive;
}

// NOTE: called from R thread. It is already holding a lock
void RKGraphicsDeviceBackendTransmitter::commitBuffer() {
	write_available.wakeAll();
	_instance->commit_pending = true;
}

// NOTE: called from R thread. It is already holding a lock
bool RKGraphicsDeviceBackendTransmitter::waitForReply(int timeout) {
	if (!streamer.instream.atEnd()) return true;

	if (!_instance->have_reply) {
		_instance->expecting_reply = true;
		write_available.wakeAll();
		read_available.wait(&mutex, QDeadlineTimer(timeout));
	}
	if (_instance->have_reply) {
		_instance->have_reply = false;
		return true;
	}
	return false;
}

void RKGraphicsDeviceBackendTransmitter::doWrite() {
	if (commit_pending) {
		QMutexLocker lock(&mutex);
		streamer.writeOutBuffer();
		commit_pending = false;
	}
	connection->waitForBytesWritten(1000);
	if (connection->state() != QLocalSocket::ConnectedState) alive = false;
}

void RKGraphicsDeviceBackendTransmitter::run() {
	RK_TRACE(GRAPHICS_DEVICE);
	RK_ASSERT(!connection);

	connection = new QLocalSocket();
	connection->connectToServer(RKRBackendProtocolBackend::rkdServerName());
	connection->waitForConnected(2000);
	if (connection->state() == QLocalSocket::ConnectedState) {
		connection->write(RKRBackendTransmitter::instance()->connectionToken().toLocal8Bit().data());
		connection->write("\n");
		connection->waitForBytesWritten(1000);
	}
	streamer.setIODevice(connection);
	read_available.wakeAll();

	while (alive) {
		doWrite();
		if (connection->bytesToWrite()) continue;
		{
			QMutexLocker lock(&mutex);
			if (commit_pending) continue;
			if (expecting_reply) {
				if (!streamer.instream.atEnd() || streamer.readInBuffer()) {
					have_reply = true;
					expecting_reply = false;
					read_available.wakeAll();
					continue;
				}
				lock.unlock();
				connection->waitForReadyRead(100); // don't wait too long: An RKDCancel may have been committed, meanwhile!
				lock.relock();
			} else {
				write_available.wait(&mutex);
			}
		}
	}

	connection->close();

	RK_TRACE(GRAPHICS_DEVICE);
}

void RKGraphicsDeviceBackendTransmitter::kill() {
	if (_instance) {
		RK_TRACE(GRAPHICS_DEVICE);
		mutex.lock();
		_instance->alive = false;
		mutex.unlock();
		_instance->wait(1000);
		delete _instance;
		_instance = nullptr;
	}
}
