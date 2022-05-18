/*
rkgraphicsdevice_backendtransmitter - This file is part of the RKWard project. Created: Mon Mar 18 2013
SPDX-FileCopyrightText: 2013 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef RKGRAPHICSDEVICE_BACKENDTRANSMITTER_H
#define RKGRAPHICSDEVICE_BACKENDTRANSMITTER_H

#include <QIODevice>
#include <QThread>
#include <QMutex>

#include "../rkasyncdatastreamhelper.h"

typedef quint32 RKGraphicsDeviceTransmittionLengthType;

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
	static RKAsyncDataStreamHelper<quint32> streamer;
	static QIODevice* connection;
	static QMutex mutex;
private:
	static RKGraphicsDeviceBackendTransmitter* _instance;
	bool alive;
	bool is_local_socket;
	void run () override;
};

#endif
