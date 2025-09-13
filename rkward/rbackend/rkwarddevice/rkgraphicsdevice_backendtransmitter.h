/*
rkgraphicsdevice_backendtransmitter - This file is part of the RKWard project. Created: Mon Mar 18 2013
SPDX-FileCopyrightText: 2013-2025 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef RKGRAPHICSDEVICE_BACKENDTRANSMITTER_H
#define RKGRAPHICSDEVICE_BACKENDTRANSMITTER_H

#include <QMutex>
#include <QThread>

#include "../rkasyncdatastreamhelper.h"

typedef quint32 RKGraphicsDeviceTransmittionLengthType;
class QLocalSocket;

/** This simple class is responsible for handling the backend side of transmitting data / requests for the RKGraphicsDevice
 Also it provides the namespace for some statics.
 As the protocol is really quite simple (only the backend send requests, only one request at a time), so is the transmitter. */
class RKGraphicsDeviceBackendTransmitter : public QThread {
	~RKGraphicsDeviceBackendTransmitter();

  public:
	static void kill();
	static bool connectionAlive();
	static RKGraphicsDeviceBackendTransmitter *instance();
	static RKAsyncDataStreamHelper<quint32> streamer;
	static QMutex mutex;
	// commit the streamer buffer for writing
	static void commitBuffer();
	// wait until all data has been written, and a reply has been received
	static bool waitForReply(int timeout);

  private:
	RKGraphicsDeviceBackendTransmitter();
	static RKGraphicsDeviceBackendTransmitter *_instance;
	bool alive;
	bool expecting_reply;
	bool have_reply;
	bool commit_pending;
	void run() override;
	static QLocalSocket *connection;
	void doWrite();
};

#endif
