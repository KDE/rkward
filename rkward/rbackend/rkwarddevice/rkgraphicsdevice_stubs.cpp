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

#include "rkgraphicsdevice_protocol_shared.h"
#include "rkgraphicsdevice_backendtransmitter.h"
#include "../rkrbackend.h"

extern "C" {
#include <R_ext/GraphicsEngine.h>
}

#include <QRectF>

#define RKD_PROTOCOL RKGraphicsDeviceBackendTransmitter::protocol

/** This class is essentially like QMutexLocker. In addition, the constructor waits until the next chunk of the transmission is ready (and does event processing) */
class RKGraphicsDataStreamReadGuard {
public:
	RKGraphicsDataStreamReadGuard () {
#warning TODO: handle cancellation
/* How shall we handle cancellation? If an interrupt is pending, _while waiting for the reply_, we push an RKD_Cancel
 * request down the line. This tells the frontend to send a reply to the last request ASAP (if the frontend has already sent the reply, it will ignore the RKD_Cancel). From there, we simply process the reply as usual, and leave it to R to actually
 * do the interrupt. */
		RKGraphicsDeviceBackendTransmitter::mutex.lock ();
		QAbstractSocket* connection = RKGraphicsDeviceBackendTransmitter::connection;
		BEGIN_SUSPEND_INTERRUPTS {
			while (connection->bytesToWrite ()) {
				connection->waitForBytesWritten (10);
				if (connection->bytesToWrite ()) RKRBackend::processX11Events ();
			}
			while (connection->bytesAvailable () < sizeof (quint32)) {
				RKRBackend::processX11Events ();
				connection->waitForReadyRead (10);
			}
			quint32 transmisison_size;
			RKD_PROTOCOL >> transmisison_size;
			while (connection->bytesAvailable () < transmisison_size) {
				RKRBackend::processX11Events ();
				connection->waitForReadyRead (10);
			}
			RKGraphicsDeviceBackendTransmitter::buffer = connection->read (transmisison_size);
		} END_SUSPEND_INTERRUPTS;
	}

	~RKGraphicsDataStreamReadGuard () {
		RKGraphicsDeviceBackendTransmitter::buffer.resize (0);
		RKGraphicsDeviceBackendTransmitter::mutex.unlock ();
	}
};

/** This class is essentially like QMutexLocker. In addition, the destructor takes care of pushing anything that was written to the protocol buffer during it lifetime to the transmitter. (Does NOT wait for the transmission itself). */
class RKGraphicsDataStreamWriteGuard {
public:
	RKGraphicsDataStreamWriteGuard () {
		RKGraphicsDeviceBackendTransmitter::mutex.lock ();
	}
	~RKGraphicsDataStreamWriteGuard () {
		aux_stream << (quint32) RKGraphicsDeviceBackendTransmitter::buffer.size ();		// TODO: is uint32 always enough?
		RKGraphicsDeviceBackendTransmitter::connection->write (aux_buffer);
		aux_buffer.resize (0);
		RKGraphicsDeviceBackendTransmitter::connection->write (RKGraphicsDeviceBackendTransmitter::buffer);
		RKGraphicsDeviceBackendTransmitter::buffer.resize (0);
		RKGraphicsDeviceBackendTransmitter::mutex.unlock ();
	}
private:
	static QByteArray aux_buffer;
	static QDataStream aux_stream;
};

QByteArray RKGraphicsDataStreamWriteGuard::aux_buffer;
QDataStream RKGraphicsDataStreamWriteGuard::aux_stream (&RKGraphicsDataStreamWriteGuard::aux_buffer, QIODevice::WriteOnly);

#define WRITE_HEADER(x,dev) (qint8) x << (quint8) static_cast<RKGraphicsDeviceDesc*> (dev->deviceSpecific)->devnum
#define WRITE_COL() (qint32) gc->col
#define WRITE_PEN() WRITE_COL() << gc->col << (double) gc->lwd << (qint32) gc->lty
#define WRITE_LINE_ENDS() (quint8) gc->lend << (quint8) gc->ljoin << gc->lmitre
#define WRITE_FILL() (qint32) gc->fill
#define WRITE_FONT(dev) gc->cex << gc->ps << gc->lineheight << (quint8) gc->fontface << (gc->fontfamily[0] ? QString (gc->fontfamily) : (static_cast<RKGraphicsDeviceDesc*> (dev->deviceSpecific)->default_family))

static void RKD_Create (double width, double height, pDevDesc dev) {
	RKGraphicsDataStreamWriteGuard guard;
	RKD_PROTOCOL << WRITE_HEADER (RKDCreate, dev);
	RKD_PROTOCOL << width << height;
}

static void RKD_Circle (double x, double y, double r, R_GE_gcontext *gc, pDevDesc dev) {
	RKGraphicsDataStreamWriteGuard guard;
	RKD_PROTOCOL << WRITE_HEADER (RKDCircle, dev);
	RKD_PROTOCOL << x << y << r;
	RKD_PROTOCOL << WRITE_PEN ();
}

static void RKD_Line (double x1, double y1, double x2, double y2, R_GE_gcontext *gc, pDevDesc dev) {
	RKGraphicsDataStreamWriteGuard guard;
	RKD_PROTOCOL << WRITE_HEADER (RKDLine, dev);
	RKD_PROTOCOL << x1 << y1 << x2 << y2;
	RKD_PROTOCOL << WRITE_PEN ();
}

static void RKD_Polygon (int n, double *x, double *y, R_GE_gcontext *gc, pDevDesc dev) {
	RKGraphicsDataStreamWriteGuard guard;
	RKD_PROTOCOL << WRITE_HEADER (RKDPolygon, dev);
	quint32 _n = qMax (n, 1 << 25);	// skip stuff exceeding reasonable limits to keep protocol simple
	RKD_PROTOCOL << _n;
	for (quint32 i = 0; i < _n; ++i) {
		RKD_PROTOCOL << x[i] << y[i];
	}
	RKD_PROTOCOL << WRITE_PEN ();
	RKD_PROTOCOL << WRITE_LINE_ENDS ();
	RKD_PROTOCOL << WRITE_FILL ();
}

static void RKD_Polyline (int n, double *x, double *y, R_GE_gcontext *gc, pDevDesc dev) {
	RKGraphicsDataStreamWriteGuard guard;
	RKD_PROTOCOL << WRITE_HEADER (RKDPolyline, dev);
	quint32 _n = qMax (n, 1 << 25);	// skip stuff exceeding reasonable limits to keep protocol simple
	RKD_PROTOCOL << _n;
	for (quint32 i = 0; i < _n; ++i) {
		RKD_PROTOCOL << x[i] << y[i];
	}
	RKD_PROTOCOL << WRITE_PEN ();
	RKD_PROTOCOL << WRITE_LINE_ENDS ();
}

static void RKD_Rect (double x0, double y0, double x1, double y1, R_GE_gcontext *gc, pDevDesc dev) {
	RKGraphicsDataStreamWriteGuard guard;
	RKD_PROTOCOL << WRITE_HEADER (RKDPolyline, dev);
	RKD_PROTOCOL << QRectF (x0, y0, x1-x0, y1-y0);
	RKD_PROTOCOL << WRITE_PEN ();
	RKD_PROTOCOL << WRITE_LINE_ENDS ();
	RKD_PROTOCOL << WRITE_FILL ();
}

static void RKD_TextUTF8 (double x, double y, const char *str, double rot, double hadj, R_GE_gcontext *gc, pDevDesc dev) {
	RKGraphicsDataStreamWriteGuard guard;
	RKD_PROTOCOL << WRITE_HEADER (RKDTextUTF8, dev);
	RKD_PROTOCOL << x << y << QString::fromUtf8 (str) << rot << hadj;
	RKD_PROTOCOL << WRITE_COL ();
	RKD_PROTOCOL << WRITE_FONT (dev);
}

static double RKD_StrWidthUTF8 (const char *str, R_GE_gcontext *gc, pDevDesc dev) {
	{
		RKGraphicsDataStreamWriteGuard guard;
		RKD_PROTOCOL << WRITE_HEADER (RKDStrWidthUTF8, dev);
		RKD_PROTOCOL << QString::fromUtf8 (str);
		RKD_PROTOCOL << WRITE_FONT (dev);
	}
	double ret;
	{
		RKGraphicsDataStreamReadGuard guard;
		RKD_PROTOCOL >> ret;
	}
	return ret;
}

static void RKD_NewPage (R_GE_gcontext *gc, pDevDesc dev) {
	RKGraphicsDataStreamWriteGuard guard;
	RKD_PROTOCOL << WRITE_HEADER (RKDNewPage, dev);
	RKD_PROTOCOL << WRITE_FILL ();
}

static void RKD_MetricInfo (int c, R_GE_gcontext *gc, double* ascent, double* descent, double* width, pDevDesc dev) {
	{
		RKGraphicsDataStreamWriteGuard wguard;
		RKD_PROTOCOL << WRITE_HEADER (RKDMetricInfo, dev);
		RKD_PROTOCOL << QChar (c);
		RKD_PROTOCOL << WRITE_FONT (dev);
	}
	{
		RKGraphicsDataStreamReadGuard rguard;
		RKD_PROTOCOL >> *ascent;
		RKD_PROTOCOL >> *descent;
		RKD_PROTOCOL >> *width;
	}
}

static void RKD_Close (pDevDesc dev) {
	RKGraphicsDataStreamWriteGuard guard;
	RKD_PROTOCOL << WRITE_HEADER (RKDClose, dev);
}

static void RKD_Activate (pDevDesc dev) {
	RKGraphicsDataStreamWriteGuard guard;
	RKD_PROTOCOL << WRITE_HEADER (RKDActivate, dev);
}

static void RKD_Deactivate (pDevDesc dev) {
	RKGraphicsDataStreamWriteGuard guard;
	RKD_PROTOCOL << WRITE_HEADER (RKDDeActivate, dev);
}

static void RKD_Clip (double left, double right, double top, double bottom, pDevDesc dev) {
	dev->clipLeft = left;
	dev->clipRight = right;
	dev->clipTop = top;
	dev->clipBottom = bottom;
	RKGraphicsDataStreamWriteGuard guard;
	RKD_PROTOCOL << WRITE_HEADER (RKDClip, dev);
	RKD_PROTOCOL << QRectF (left, top, right - left, bottom - top);
}

static void RKD_Mode (int mode, pDevDesc dev) {
	Q_UNUSED (mode);
	Q_UNUSED (dev);
/* Left empty for now. 1 is start signal, 0 is stop signal. Might be useful for flushing, though.

	RKGraphicsDataStreamWriteGuard guard;
	RKD_PROTOCOL << WRITE_HEADER (RKDMode, dev);
	connectoin << (qint8) mode; */
}

static Rboolean RKD_Locator (double *x, double *y, pDevDesc dev) {
	{
		RKGraphicsDataStreamWriteGuard wguard;
		RKD_PROTOCOL << WRITE_HEADER (RKDLocator, dev);
	}
	{
		RKGraphicsDataStreamReadGuard rguard;
		bool ok;
		RKD_PROTOCOL >> ok;
		RKD_PROTOCOL >> *x;
		RKD_PROTOCOL >> *y;
		if (ok) return (Rboolean) TRUE;
		return (Rboolean) FALSE;
	}
}

static Rboolean RKD_NewFrameConfirm (pDevDesc dev) {
	{
		RKGraphicsDataStreamWriteGuard wguard;
		RKD_PROTOCOL << WRITE_HEADER (RKDNewPageConfirm, dev);
	}
	{
		RKGraphicsDataStreamReadGuard rguard;
		bool ok;
		RKD_PROTOCOL >> ok;
		if (ok) return (Rboolean) TRUE;
		return (Rboolean) FALSE;
	}
}
