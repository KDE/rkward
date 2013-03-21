/***************************************************************************
                          rkgraphicsdevice_frontendtransmitter  -  description
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

#include "rkgraphicsdevice_frontendtransmitter.h"

#include <QLocalServer>
#include <QLocalSocket>
#include <QIODevice>
#include <krandom.h>

// NOTE: This is but the latest nail in the coffin of the single process variant of RKWard.
// *IF* the RKWard Graphics Device works out as hoped, the single process variant can finally be ditched for good.
#define RKWARD_SPLIT_PROCESS 1
#include "../rkfrontendtransmitter.h"
#include "../../version.h"

#include "../../debug.h"

RKGraphicsDeviceFrontendTransmitter::RKGraphicsDeviceFrontendTransmitter () : QObject () {
	RK_TRACE (GRAPHICS_DEVICE);

	connection = 0;
	local_server = 0;

	setupServer ();
}

RKGraphicsDeviceFrontendTransmitter::~RKGraphicsDeviceFrontendTransmitter () {
	RK_TRACE (GRAPHICS_DEVICE);

	if (connection) connection->close ();
}

void RKGraphicsDeviceFrontendTransmitter::setupServer () {
	RK_TRACE (GRAPHICS_DEVICE);

	RK_ASSERT (!local_server);
	local_server = new QLocalServer ();
	RK_ASSERT (local_server->listen ("rkd" + KRandom::randomString (8)));
	connect (local_server, SIGNAL (newConnection ()), this, SLOT (newConnection()));
	server_name = local_server->fullServerName ();
}

void RKGraphicsDeviceFrontendTransmitter::newConnection () {
	RK_TRACE (GRAPHICS_DEVICE);

	RK_ASSERT (!connection);
	QLocalSocket *con = local_server->nextPendingConnection ();
	local_server->close ();

	// handshake
	QString token = RKFrontendTransmitter::instance ()->connectionToken ();
	if (!con->canReadLine ()) con->waitForReadyRead (1000);
	QString token_c = QString::fromLocal8Bit (con->readLine ());
	token_c.chop (1);
	if (token_c != token) {
#warning TODO error handling
		con->close ();
		return;
	}

	connection = con;
	streamer.setIODevice (con);
	connect (connection, SIGNAL (readyRead ()), this, SLOT (newData ()));
}

void RKGraphicsDeviceFrontendTransmitter::newData () {
//	RK_TRACE (GRAPHICS_DEVICE);

	if (!streamer.readInBuffer ()) return;	// wait for more data to come in

	quint8 opcode, devnum;
	streamer.instream >> opcode;
	streamer.instream >> devnum;
	RK_DEBUG (DL_WARNING, GRAPHICS_DEVICE, "Received transmission of type %d, devnum %d, size %d", opcode, devnum, streamer.inSize ());

#warning TODO: Actually handle the data!
	if (opcode == RKDLocator) {
		bool ok = true;
		double x, y;
		x = y = 0;
		streamer.outstream << ok << x << y;
		streamer.writeOutBuffer ();
	} else if (opcode == RKDMetricInfo) {
		double ascent, descent, width;
		ascent = descent = width = 0.1;
		streamer.outstream << ascent << descent << width;
		streamer.writeOutBuffer ();
	} else if (opcode == RKDNewPageConfirm) {
		bool ok = true;
		streamer.outstream << ok;
		streamer.writeOutBuffer ();
	} else if (opcode == RKDStrWidthUTF8) {
		double width = 1;
		streamer.outstream << width;
		streamer.writeOutBuffer ();
	}

	if (connection->bytesAvailable ()) newData ();
}

#include "rkgraphicsdevice_frontendtransmitter.moc"