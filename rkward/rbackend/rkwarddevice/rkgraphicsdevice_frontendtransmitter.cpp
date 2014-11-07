/***************************************************************************
                          rkgraphicsdevice_frontendtransmitter  -  description
                             -------------------
    begin                : Mon Mar 18 20:06:08 CET 2013
    copyright            : (C) 2013-2014 by Thomas Friedrichsmeier 
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

// for screen resolution
#include <QApplication>
#include <QDesktopWidget>

#include "../rkfrontendtransmitter.h"
#include "../../windows/rkworkplace.h"
#include "rkgraphicsdevice.h"
#include "../../version.h"

#include "../../debug.h"

double RKGraphicsDeviceFrontendTransmitter::lwdscale = 72/96;
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
	newData ();	// might already be available
}

static QRgb readRgb (QDataStream &instream) {
	quint8 r, g, b, a;
	instream >> r >> g >> b >> a;
	return qRgba (r, g, b, a);
}

static QColor readColor (QDataStream &instream) {
	quint8 r, g, b, a;
	instream >> r >> g >> b >> a;
	if (a == 0x00) return QColor ();
	return QColor (r, g, b, a);
}

static QPen readSimplePen (QDataStream &instream) {
	QColor col = readColor (instream);
	double lwd;
	qint32 lty;
	instream >> lwd >> lty;
	if (!col.isValid () || (lty == -1L)) return QPen (Qt::NoPen);

	lwd = qMax (1.0, lwd);	// minimum 1 px as in X11 device
	QPen ret;
	if (lty != 0) {	// solid
		QVector<qreal> dashes;
		quint32 nlty = lty;
		for (int i = 0; i < 8; ++i) {
			if (!nlty) break;
			quint8 j = nlty & 0xF;
			if (j < 1) j = 1;
			dashes.append ((int) (j * lwd * RKGraphicsDeviceFrontendTransmitter::lwdscale + .5));
			nlty >>= 4;
		}
		if (!dashes.isEmpty ()) ret.setDashPattern (dashes);
	}
	ret.setWidth ((int) (lwd * RKGraphicsDeviceFrontendTransmitter::lwdscale + .5));
	ret.setColor (col);
	return ret;
}

static QPen readPen (QDataStream &instream) {
	QPen ret = readSimplePen (instream);
	quint8 lends, ljoin;
	double lmitre;
	instream >> lends >> ljoin >> lmitre;
	ret.setCapStyle (lends == RoundLineCap ? Qt::RoundCap : (lends == ButtLineCap ? Qt::FlatCap : Qt::SquareCap));
	ret.setJoinStyle (ljoin == RoundJoin ? Qt::RoundJoin : (ljoin == BevelJoin ? Qt::BevelJoin : Qt::MiterJoin));
	ret.setMiterLimit (lmitre);
	return ret;
}

static QBrush readBrush (QDataStream &instream) {
	QColor col = readColor (instream);
	if (!col.isValid ()) return QBrush ();
	return QBrush (col);
}

static QFont readFont (QDataStream &instream) {
	double cex, ps, lineheight;
	quint8 fontface;
	QString fontfamily;
	instream >> cex >> ps >> lineheight >> fontface >> fontfamily;
#warning TODO deal with line-height
	QFont ret (fontfamily);
	if (fontface == 2 || fontface == 4) ret.setWeight (QFont::Bold);
	if (fontface == 3 || fontface == 4) ret.setItalic (true);
	ret.setPointSizeF (cex*ps);
	return ret;
}

static QVector<QPointF> readPoints (QDataStream &instream) {
	quint32 n;
	instream >> n;
	QVector<QPointF> points;
	points.reserve (n);
	for (quint32 i = 0; i < n; ++i) {
		double x, y;
		instream >> x >> y;
		points.append (QPointF (x, y));
	}
	return points;
}

void RKGraphicsDeviceFrontendTransmitter::newData () {
	RK_TRACE (GRAPHICS_DEVICE);

	while (connection->bytesAvailable ()) {
		if (!streamer.readInBuffer ()) return;	// wait for more data to come in

		quint8 opcode, devnum;
		streamer.instream >> opcode >> devnum;
		RK_DEBUG (GRAPHICS_DEVICE, DL_DEBUG, "Received transmission of type %d, devnum %d, size %d", opcode, devnum+1, streamer.inSize ());

		RKGraphicsDevice *device = 0;
		if (devnum && opcode == RKDCreate) {
			double width, height;
			QString title;
			bool antialias;
			streamer.instream >> width >> height >> title >> antialias;
			device = RKGraphicsDevice::newDevice (devnum, width, height, title, antialias);
			RKWorkplace::mainWorkplace ()->newRKWardGraphisWindow (device, devnum+1);
			connect (device, SIGNAL (locatorDone(bool,double,double)), this, SLOT (locatorDone(bool,double,double)));
			connect (device, SIGNAL (newPageConfirmDone(bool)), this, SLOT (newPageConfirmDone(bool)));
			connect (this, SIGNAL (stopInteraction()), device, SLOT (stopInteraction()));
			continue;
		} else {
			if (devnum) device = RKGraphicsDevice::devices.value (devnum);
			if (!device) {
				if (opcode == RKDCancel) {
					RK_DEBUG (GRAPHICS_DEVICE, DL_WARNING, "Graphics operation cancelled");
					emit (stopInteraction());
				} else if (opcode == RKDQueryResolution) {
					QDesktopWidget *desktop = QApplication::desktop ();
					streamer.outstream << (qint32) desktop->physicalDpiX () << (qint32) desktop->physicalDpiY ();
					RK_DEBUG (GRAPHICS_DEVICE, DL_INFO, "DPI for device %d: %d by %d", devnum+1, desktop->physicalDpiX (), desktop->physicalDpiY ());
					streamer.writeOutBuffer ();
					// Actually, this is only needed once, but where to put it...
					RKGraphicsDeviceFrontendTransmitter::lwdscale = desktop->physicalDpiX () / 96;   // taken from devX11.c
				} else {
					if (devnum) RK_DEBUG (GRAPHICS_DEVICE, DL_ERROR, "Received transmission of type %d for unknown device number %d. Skippping.", opcode, devnum+1);
					sendDummyReply (opcode);
				}
				continue;
			}
		}

/* WARNING: No fixed evaluation order of function arguments. Do not use several readXYZ() calls in the same function call! Use dummy variables, instead. */
		if (opcode == RKDCircle) {
			double x, y, r;
			streamer.instream >> x >> y >> r;
			QPen pen = readSimplePen (streamer.instream);
			device->circle (x, y, r, pen, readBrush (streamer.instream));
		} else if (opcode == RKDLine) {
			double x1, y1, x2, y2;
			streamer.instream >> x1 >> y1 >> x2 >> y2;
			device->line (x1, y1, x2, y2, readPen (streamer.instream));
		} else if (opcode == RKDPolygon) {
			QPolygonF pol (readPoints (streamer.instream));
			QPen pen = readPen (streamer.instream);
			device->polygon (pol, pen, readBrush (streamer.instream));
		} else if (opcode == RKDPolyline) {
			QPolygonF pol (readPoints (streamer.instream));
			device->polyline (pol, readPen (streamer.instream));
		} else if (opcode == RKDPath) {
			quint32 npol;
			streamer.instream >> npol;
			QVector<QPolygonF> polygons;
			polygons.reserve (npol);
			for (quint32 i = 0; i < npol; ++i) {
				polygons.append (readPoints (streamer.instream));
			}
			bool winding;
			streamer.instream >> winding;
			QPen pen = readPen (streamer.instream);
			device->polypath (polygons, winding, pen, readBrush (streamer.instream));
		} else if (opcode == RKDRect) {
			QRectF rect;
			streamer.instream >> rect;
			QPen pen = readPen (streamer.instream);
			device->rect (rect, pen, readBrush (streamer.instream));
		} else if (opcode == RKDStrWidthUTF8) {
			QString out;
			streamer.instream >> out;
			double w = device->strSize (out, readFont (streamer.instream)).width ();
			streamer.outstream << w;
			streamer.writeOutBuffer ();
		} else if (opcode == RKDMetricInfo) {
			QChar c;
			double ascent, descent, width;
			streamer.instream >> c;
			device->metricInfo (c, readFont (streamer.instream), &ascent, &descent, &width);
			streamer.outstream << ascent << descent << width;
			streamer.writeOutBuffer ();
		} else if (opcode == RKDTextUTF8) {
			double x, y, rot, hadj;
			QString out;
			streamer.instream >> x >> y >> out >> rot >> hadj;
			QColor col = readColor (streamer.instream);
			device->text (x, y, out, rot, hadj, col, readFont (streamer.instream));
		} else if (opcode == RKDNewPage) {
			device->clear (readColor (streamer.instream));
		} else if (opcode == RKDClose) {
			RKGraphicsDevice::closeDevice (devnum);
		} else if (opcode == RKDActivate) {
			device->setActive (true);
		} else if (opcode == RKDDeActivate) {
			device->setActive (false);
		} else if (opcode == RKDClip) {
			QRectF clip;
			streamer.instream >> clip;
			device->setClip (clip);
		} else if (opcode == RKDMode) {
			quint8 m;
			streamer.instream >> m;
			if (m == 0) device->triggerUpdate ();
		} else if (opcode == RKDRaster) {
			quint32 w, h;
			streamer.instream >> w >> h;
			QImage image (w, h, QImage::Format_ARGB32);
			for (quint32 col = 0; col < h; ++col) {
				for (quint32 row = 0; row < w; ++row) {
					image.setPixel (row, col, readRgb (streamer.instream));
				}
			}
			QRectF target;
			double rotation;
			bool interpolate;
			streamer.instream >> target >> rotation >> interpolate;
			device->image (image, target.normalized (), rotation, interpolate);
		} else if (opcode == RKDCapture) {
			QImage image = device->capture ();
			quint32 w = image.width ();
			quint32 h = image.height ();
			streamer.outstream << w << h;
			for (quint32 col = 0; col < h; ++col) {
				for (quint32 row = 0; row < w; ++row) {
					QRgb pixel = image.pixel (row, col);
					streamer.outstream << (quint8) qRed (pixel) << (quint8) qGreen (pixel) << (quint8) qBlue (pixel) << (quint8) qAlpha (pixel);
				}
			}
			streamer.writeOutBuffer ();
		} else if (opcode == RKDGetSize) {
			streamer.outstream << device->currentSize ();
			streamer.writeOutBuffer ();
		} else if (opcode == RKDSetSize) {
			QSize size;
			streamer.instream >> size;
			device->setAreaSize (size);
		} else if (opcode == RKDLocator) {
			device->locator ();
		} else if (opcode == RKDStartGettingEvents) {
			QString prompt;
			streamer.instream >> prompt;
			device->startGettingEvents (prompt);
		} else if (opcode == RKDStopGettingEvents) {
			device->stopGettingEvents ();
		} else if (opcode == RKDFetchNextEvent) {
			RKGraphicsDevice::StoredEvent ev = device->fetchNextEvent ();
			streamer.outstream << (qint8) ev.event_code;
			if (ev.event_code == RKDKeyPress) {
				streamer.outstream << ev.keytext << (qint32) ev.keycode << (qint32) ev.modifiers;
			} else if ((ev.event_code == RKDMouseDown) || (ev.event_code == RKDMouseUp) || (ev.event_code == RKDMouseMove)) {
				streamer.outstream << (qint8) ev.buttons << (double) ev.x << (double) ev.y;
			} else {
				RK_ASSERT ((ev.event_code == RKDNothing) || (ev.event_code == RKDFrontendCancel));
			}
			streamer.writeOutBuffer ();
		} else if (opcode == RKDNewPageConfirm) {
			device->confirmNewPage ();
		} else {
			RK_DEBUG (GRAPHICS_DEVICE, DL_ERROR, "Unhandled operation of type %d for device number %d. Skippping.", opcode, devnum+1);
		}

		if (!streamer.instream.atEnd ()) {
			RK_DEBUG (GRAPHICS_DEVICE, DL_ERROR, "Failed to read all data for operation of type %d on device number %d.", opcode, devnum+1);
		}
	}
}

void RKGraphicsDeviceFrontendTransmitter::sendDummyReply (quint8 opcode) {
	RK_TRACE (GRAPHICS_DEVICE);

	if (opcode == RKDLocator) {
		bool ok = false;
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
	} else if (opcode == RKDGetSize) {
		streamer.outstream << QSizeF ();
	} else if (opcode == RKDFetchNextEvent) {
		streamer.outstream << (qint8) RKDNothing;
	} else {
		return;	// nothing to write
	}

	streamer.writeOutBuffer ();
}

void RKGraphicsDeviceFrontendTransmitter::locatorDone (bool ok, double x, double y) {
	RK_TRACE (GRAPHICS_DEVICE);

	streamer.outstream << ok << x << y;
	streamer.writeOutBuffer ();
}

void RKGraphicsDeviceFrontendTransmitter::newPageConfirmDone (bool accepted) {
	RK_TRACE (GRAPHICS_DEVICE);

	streamer.outstream << accepted;
	streamer.writeOutBuffer ();
}

#include "rkgraphicsdevice_frontendtransmitter.moc"
