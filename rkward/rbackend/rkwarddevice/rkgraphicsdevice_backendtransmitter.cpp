/***************************************************************************
                          rkgraphicsdevice_backendtransmitter  -  description
                             -------------------
    begin                : Mon Mar 18 20:06:08 CET 2013
    copyright            : (C) 2013 by Thomas Friedrichsmeier 
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

#include "rkgraphicsdevice_backendtransmitter.h"

#include <QLocalSocket>
#include "../rkrbackendprotocol_backend.h"
#define RKWARD_SPLIT_PROCESS 1
#include "../rkbackendtransmitter.h"

#include "../../debug.h"

RKAsyncDataStreamHelper RKGraphicsDeviceBackendTransmitter::streamer;
QIODevice* RKGraphicsDeviceBackendTransmitter::connection = 0;
QMutex RKGraphicsDeviceBackendTransmitter::mutex;
RKGraphicsDeviceBackendTransmitter* RKGraphicsDeviceBackendTransmitter::_instance = 0;

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
	return 0;
}

bool RKGraphicsDeviceBackendTransmitter::connectionAlive () {
	if (!_instance) return false;
	if (!_instance->is_local_socket) return true;
	return static_cast<QLocalSocket*> (_instance->connection)->state () == QLocalSocket::ConnectedState;
}

void RKGraphicsDeviceBackendTransmitter::run () {
	RK_TRACE (GRAPHICS_DEVICE);

	while (alive) {
		msleep (10);	// it's ok to be lazy. If a request expects a reply, RKGraphicsDataStreamReadGuard will take care of pushing everything, itself. Essentially, this thread's job is simply to make sure we don't lag *too* far behind.
		mutex.lock ();
		connection->waitForBytesWritten (100);
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
		_instance = 0;
	}
}
