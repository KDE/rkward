/*
rkgraphicsdevice_frontendtransmitter - This file is part of the RKWard project. Created: Mon Mar 18 2013
SPDX-FileCopyrightText: 2013 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef RKGRAPHICSDEVICE_FRONTENDTRANSMITTER_H
#define RKGRAPHICSDEVICE_FRONTENDTRANSMITTER_H

#include <QPainter> // for enums
#include "rkgraphicsdevice_protocol_shared.h"
#include "rkgraphicsdevice_backendtransmitter.h"
#include "../rkasyncdatastreamhelper.h"

class QIODevice;
class QLocalServer;

/** Handles the frontend side of RKWard Graphics Device transmissions. Since the
 * frontend has a running Qt event loop, We can use simple signals and slots, here. */
class RKGraphicsDeviceFrontendTransmitter : public QObject {
	Q_OBJECT
public:
	RKGraphicsDeviceFrontendTransmitter ();
	~RKGraphicsDeviceFrontendTransmitter ();
	QString serverName () const { return server_name; };
	static double lwdscale;
public Q_SLOTS:
	void newData ();
	void newConnection ();
	void locatorDone (bool ok, double x, double y);
	void newPageConfirmDone (bool accepted);
Q_SIGNALS:
	void stopInteraction ();
private:
	void setupServer ();
	void sendDummyReply (quint8 opcode);
	QString server_name;
	QIODevice *connection;
	QLocalServer *local_server;
	RKAsyncDataStreamHelper<RKGraphicsDeviceTransmittionLengthType> streamer;
};

#endif
