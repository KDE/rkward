/*
rkgraphicsdevice_backendtransmitter - This file is part of RKWard (https://rkward.kde.org). Created: Mon Mar 18 2013
SPDX-FileCopyrightText: 2013 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "rkgraphicsdevice_backendtransmitter.h"

#include <QLocalSocket>
#include "../rkrbackendprotocol_backend.h"
#include "../rkbackendtransmitter.h"

#include "../../debug.h"

RKAsyncDataStreamHelper<RKGraphicsDeviceTransmittionLengthType> RKGraphicsDeviceBackendTransmitter::streamer;
QIODevice* RKGraphicsDeviceBackendTransmitter::connection = nullptr;
QMutex RKGraphicsDeviceBackendTransmitter::mutex;
RKGraphicsDeviceBackendTransmitter* RKGraphicsDeviceBackendTransmitter::_instance = nullptr;

RKGraphicsDeviceBackendTransmitter::RKGraphicsDeviceBackendTransmitter (QIODevice* _connection, bool is_q_local_socket) : QThread () {
	RK_TRACE (GRAPHICS_DEVICE);

	RK_ASSERT (!connection);
	RK_ASSERT (_connection);
	connection = _connection;
	streamer.setIODevice (connection);
	alive = true;
	is_local_socket = is_q_local_socket;
	start ();
}

RKGraphicsDeviceBackendTransmitter::~RKGraphicsDeviceBackendTransmitter () {
	RK_TRACE (GRAPHICS_DEVICE);
	delete connection;
}

RKGraphicsDeviceBackendTransmitter* RKGraphicsDeviceBackendTransmitter::instance () {
	if (_instance) return _instance;
	RK_TRACE (GRAPHICS_DEVICE);

	QLocalSocket *con = new QLocalSocket ();
	con->connectToServer (RKRBackendProtocolBackend::rkdServerName ());
	con->waitForConnected (2000);
	if (con->state () == QLocalSocket::ConnectedState) {
		con->write (RKRBackendTransmitter::instance ()->connectionToken ().toLocal8Bit ().data ());
		con->write ("\n");
		con->waitForBytesWritten (1000);
		_instance = new RKGraphicsDeviceBackendTransmitter (con, true);
		return _instance;
	}
	return nullptr;
}

bool RKGraphicsDeviceBackendTransmitter::connectionAlive () {
	if (!_instance) return false;
	if (!_instance->is_local_socket) return true;
	return static_cast<QLocalSocket*> (_instance->connection)->state () == QLocalSocket::ConnectedState;
}

void RKGraphicsDeviceBackendTransmitter::run () {
	RK_TRACE (GRAPHICS_DEVICE);

	bool more_left = false;
	while (alive) {
		msleep (more_left ? 10 : 50);	// it's ok to be lazy. If a request expects a reply, RKGraphicsDataStreamReadGuard will take care of pushing everything, itself. Essentially, this thread's job is simply to make sure we don't lag *too* far behind.
		// See note in RKRBackend::handleRequest(): sleeping short is CPU-intensive
		mutex.lock ();
		connection->waitForBytesWritten (100);
		more_left = connection->bytesToWrite ();
		mutex.unlock ();
	}

	RK_TRACE (GRAPHICS_DEVICE);
}

void RKGraphicsDeviceBackendTransmitter::kill () {
	if (_instance) {
		RK_TRACE (GRAPHICS_DEVICE);
		mutex.lock ();
		_instance->alive = false;
		mutex.unlock ();
		_instance->wait (1000);
		delete _instance;
		_instance = nullptr;
	}
}
