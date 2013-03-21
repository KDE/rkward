/***************************************************************************
                          rkgraphicsdevice  -  description
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

#include "../../debug.h"

QByteArray RKGraphicsDeviceBackendTransmitter::buffer;
QDataStream RKGraphicsDeviceBackendTransmitter::protocol (&buffer, QIODevice::ReadWrite);
QAbstractSocket* RKGraphicsDeviceBackendTransmitter::connection = 0;
QMutex RKGraphicsDeviceBackendTransmitter::mutex;

RKGraphicsDeviceBackendTransmitter::RKGraphicsDeviceBackendTransmitter (QAbstractSocket* _connection) : QThread () {
	RK_TRACE (RBACKEND);

	RK_ASSERT (!connection);
	RK_ASSERT (_connection);
	connection = _connection;
	alive = true;
	start ();
}

RKGraphicsDeviceBackendTransmitter::~RKGraphicsDeviceBackendTransmitter () {
	RK_TRACE (RBACKEND);
}

void RKGraphicsDeviceBackendTransmitter::run () {
	RK_TRACE (RBACKEND);

	while (alive) {
		msleep (10);	// it's ok to be lazy. If a request expects a reply, RKGraphicsDataStreamReadGuard will take care of pushing everything, itself. Essentially, this thread's job is simply to make sure we don't lag *too* far behind.
		mutex.lock ();
		connection->waitForBytesWritten (100);
		mutex.unlock ();
	}

	RK_TRACE (RBACKEND);
}

void RKGraphicsDeviceBackendTransmitter::kill () {
	RK_TRACE (RBACKEND);
	mutex.lock ();
	alive = false;
	mutex.unlock ();
	wait (1000);
}
