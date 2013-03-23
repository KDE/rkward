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
#include <kmessagebox.h>
#include <klocale.h>

// NOTE: This is but the latest nail in the coffin of the single process variant of RKWard.
// *IF* the RKWard Graphics Device works out as hoped, the single process variant can finally be ditched for good.
#define RKWARD_SPLIT_PROCESS 1
#include "../rkfrontendtransmitter.h"
#include "rkgraphicsdevice.h"
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
		KMessageBox::detailedError (0, QString ("<p>%1</p>").arg (i18n ("There has been an error while trying to connect the on-screen graphics backend. This means, on-screen graphics using the RKWard device will not work in this session.")), i18n ("Expected connection token %1, but read connection token %2").arg (token).arg (token_c), i18n ("Error while connection graphics backend"));
		con->close ();
		return;
	}

	connection = con;
	streamer.setIODevice (con);
	connect (connection, SIGNAL (readyRead ()), this, SLOT (newData ()));
}

static QPen readSimplePen (QDataStream &instream) {
	quint8 r, g, b, a;
	double lwd;
	qint32 lty;
	instream >> r >> g >> b >> a >> lwd >> lty;
#warning: Line style!
	QPen ret;
	ret.setWidthF (lwd);
	ret.setColor (QColor (r, g, b, a));
	return ret;
}

static QPen readPen (QDataStream &instream) {
	QPen ret = readSimplePen (instream);
#warning: Read further attribs!
	return ret;
}

static QBrush readBrush (QDataStream &instream) {
#warning: Read brush!
	return QBrush ();
}

static QFont readFont (QDataStream &instream) {
#warning: Handle all attribs!
	double cex, ps, lineheight;
	quint8 fontface;
	QString fontfamily;
	instream >> cex >> ps >> lineheight >> fontface >> fontfamily;
	QFont ret (fontfamily);
	ret.setPointSizeF (cex*ps);
	return ret;
}

void RKGraphicsDeviceFrontendTransmitter::newData () {
	RK_TRACE (GRAPHICS_DEVICE);

	while (connection->bytesAvailable ()) {
		if (!streamer.readInBuffer ()) return;	// wait for more data to come in

		quint8 opcode, devnum;
		streamer.instream >> opcode >> devnum;
		RK_DEBUG (GRAPHICS_DEVICE, DL_DEBUG, "Received transmission of type %d, devnum %d, size %d", opcode, devnum, streamer.inSize ());

		RKGraphicsDevice *device = 0;
		if (devnum && opcode == RKDCreate) {
			double width, height;
			streamer.instream >> width >> height;
			device = RKGraphicsDevice::newDevice (devnum, width, height);
		} else if (devnum) {
			device = RKGraphicsDevice::devices.value (devnum);
			if (!device) {
				RK_DEBUG (GRAPHICS_DEVICE, DL_ERROR, "Received transmission of type %d for unknown device number %d. Skippping.", opcode, devnum);
				sendDummyReply (opcode);
				continue;
			}
		}

		if (opcode == RKDCircle) {
			double x, y, r;
			streamer.instream >> x >> y >> r;
			device->circle (x, y, r, readSimplePen (streamer.instream), readBrush (streamer.instream));
			continue;
		} else if (opcode == RKDLine) {
			double x1, y1, x2, y2;
			streamer.instream >> x1 >> y1 >> x2 >> y2;
			device->line (x1, y1, x2, y2, readPen (streamer.instream));
			continue;
		} else if (opcode == RKDPolygon) {
		} else if (opcode == RKDPolyline) {
		} else if (opcode == RKDRect) {
		} else if (opcode == RKDStrWidthUTF8) {
			QString out;
			streamer.instream >> out;
			double w = device->strWidth (out, readFont (streamer.instream));
			streamer.outstream << w;
			streamer.writeOutBuffer ();
			continue;
		} else if (opcode == RKDMetricInfo) {
		} else if (opcode == RKDTextUTF8) {
			double x, y, rot, hadj;
			QString out;
			QRgb col;
			streamer.instream >> x >> y >> out >> rot >> hadj >> col;
			device->text (x, y, out, rot, hadj, col, readFont (streamer.instream));
			continue;
		} else if (opcode == RKDNewPage) {
		} else if (opcode == RKDClose) {
		} else if (opcode == RKDActivate) {
		} else if (opcode == RKDDeActivate) {
		} else if (opcode == RKDClip) {
		} else if (opcode == RKDMode) {
		} else if (opcode == RKDClip) {
		} else if (opcode == RKDLocator) {
		} else if (opcode == RKDNewPageConfirm) {
		}

		RK_ASSERT (streamer.instream.atEnd ());
		RK_DEBUG (GRAPHICS_DEVICE, DL_ERROR, "Unhandled operation of type %d for device number %d. Skippping.", opcode, devnum);

#warning TODO: Actually handle the data!
		sendDummyReply (opcode);
	}
}

void RKGraphicsDeviceFrontendTransmitter::sendDummyReply (quint8 opcode) {
	RK_TRACE (GRAPHICS_DEVICE);

	if (opcode == RKDLocator) {
		bool ok = true;
		double x, y;
		x = y = 0;
		streamer.outstream << ok << x << y;
	} else if (opcode == RKDMetricInfo) {
		double ascent, descent, width;
		ascent = descent = width = 0.1;
		streamer.outstream << ascent << descent << width;
	} else if (opcode == RKDNewPageConfirm) {
		bool ok = true;
		streamer.outstream << ok;
	} else if (opcode == RKDStrWidthUTF8) {
		double width = 1;
		streamer.outstream << width;
	} else {
		return;	// nothing to write
	}

	streamer.writeOutBuffer ();
}


#include "rkgraphicsdevice_frontendtransmitter.moc"
