/***************************************************************************
                          rkgraphicsdevice_backendtransmitter  -  description
                             -------------------
    begin                : Mon Mar 18 20:06:08 CET 2013
    copyright            : (C) 2013 by Thomas Friedrichsmeier 
    email                : thomas.friedrichsmeier@kdemail.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef RKGRAPHICSDEVICE_BACKENDTRANSMITTER_H
#define RKGRAPHICSDEVICE_BACKENDTRANSMITTER_H

#include <QIODevice>
#include <QThread>
#include <QMutex>

#include "rkgraphicsdevice_protocol_shared.h"
#include "../rkasyncdatastreamhelper.h"

/** This simple class is responsible for handling the backend side of transmitting data / requests for the RKGraphicsDevice
 Also it provides the namespace for some statics.
 As the protocol is really quite simple (only the backend send requests, only one request at a time), so is the transmitter. */
class RKGraphicsDeviceBackendTransmitter : public QThread {
	RKGraphicsDeviceBackendTransmitter (QIODevice *connection, bool is_q_local_socket);
	~RKGraphicsDeviceBackendTransmitter ();
public:
	static void kill ();
	static bool connectionAlive ();
	static RKGraphicsDeviceBackendTransmitter* instance ();
	static RKAsyncDataStreamHelper<RKGraphicsDeviceTransmittionLengthType> streamer;
	static QIODevice* connection;
	static QMutex mutex;
private:
	static RKGraphicsDeviceBackendTransmitter* _instance;
	bool alive;
	bool is_local_socket;
	void run () override;
};

#endif
