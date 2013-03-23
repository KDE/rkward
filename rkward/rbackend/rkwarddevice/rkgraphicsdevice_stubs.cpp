/***************************************************************************
                          rkgraphicsdevice_stubs  -  description
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

/* NOTE: This file is essentially a split-out from rkgraphicsdevice_setup.cpp,
 * not a compilation unit of its own.
 * It is meant to be included, there. */

#include "rkgraphicsdevice_protocol_shared.h"
#include "rkgraphicsdevice_backendtransmitter.h"
#include "../rkrbackend.h"

extern "C" {
#include <R_ext/GraphicsEngine.h>
}

#define RKD_IN_STREAM RKGraphicsDeviceBackendTransmitter::streamer.instream
#define RKD_OUT_STREAM RKGraphicsDeviceBackendTransmitter::streamer.outstream

/** This class is essentially like QMutexLocker. In addition, the constructor waits until the next chunk of the transmission is ready (and does event processing) */
class RKGraphicsDataStreamReadGuard {
public:
	RKGraphicsDataStreamReadGuard () {
#warning TODO: handle cancellation
/* How shall we handle cancellation? If an interrupt is pending, _while waiting for the reply_, we push an RKD_Cancel
 * request down the line. This tells the frontend to send a reply to the last request ASAP (if the frontend has already sent the reply, it will ignore the RKD_Cancel). From there, we simply process the reply as usual, and leave it to R to actually
 * do the interrupt. */
		RKGraphicsDeviceBackendTransmitter::mutex.lock ();
		QIODevice* connection = RKGraphicsDeviceBackendTransmitter::connection;
		BEGIN_SUSPEND_INTERRUPTS {
			while (connection->bytesToWrite ()) {
				connection->waitForBytesWritten (10);
#warning TODO: Use R_CheckUserInterrupt(), instead?
				if (connection->bytesToWrite ()) RKRBackend::processX11Events ();
			}
			while (!RKGraphicsDeviceBackendTransmitter::streamer.readInBuffer ()) {
				RKRBackend::processX11Events ();
				connection->waitForReadyRead (10);
			}
		} END_SUSPEND_INTERRUPTS;
	}

	~RKGraphicsDataStreamReadGuard () {
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
		RKGraphicsDeviceBackendTransmitter::streamer.writeOutBuffer ();
		RKGraphicsDeviceBackendTransmitter::mutex.unlock ();
	}
};

#include <QRectF>

#define RK_TEST_COL 0xABCDEF01
#if not ((R_RED(RK_TEST_COL) == 0x01) && (R_GREEN(RK_TEST_COL) == 0xEF) && (R_BLUE(RK_TEST_COL) == 0xCD) && (R_ALPHA(RK_TEST_COL) == 0xAB))
#error R color format has changed!
#endif

// I'd love to convert to QColor, directly, but that's in QtGui, not QtCore. QRgb used different byte ordering
#define WRITE_COLOR_BYTES(col) (quint8) R_RED (col) << (quint8) R_GREEN (col) << (quint8) R_BLUE (col) << (quint8) R_ALPHA (col)
#define WRITE_HEADER(x,dev) (qint8) x << (quint8) static_cast<RKGraphicsDeviceDesc*> (dev->deviceSpecific)->devnum
#define WRITE_COL() WRITE_COLOR_BYTES (gc->col)
#define WRITE_PEN() WRITE_COL() << (double) gc->lwd << (qint32) gc->lty
#define WRITE_LINE_ENDS() (quint8) gc->lend << (quint8) gc->ljoin << gc->lmitre
#define WRITE_FILL() WRITE_COLOR_BYTES (gc->fill)
#define WRITE_FONT(dev) gc->cex << gc->ps << gc->lineheight << (quint8) gc->fontface << (gc->fontfamily[0] ? QString (gc->fontfamily) : (static_cast<RKGraphicsDeviceDesc*> (dev->deviceSpecific)->default_family))

static void RKD_Create (double width, double height, pDevDesc dev) {
	RKGraphicsDataStreamWriteGuard guard;
	RKD_OUT_STREAM << WRITE_HEADER (RKDCreate, dev);
	RKD_OUT_STREAM << width << height;
}

static void RKD_Circle (double x, double y, double r, R_GE_gcontext *gc, pDevDesc dev) {
	RKGraphicsDataStreamWriteGuard guard;
	RKD_OUT_STREAM << WRITE_HEADER (RKDCircle, dev);
	RKD_OUT_STREAM << x << y << r;
	RKD_OUT_STREAM << WRITE_PEN ();
	RKD_OUT_STREAM << WRITE_FILL ();
}

static void RKD_Line (double x1, double y1, double x2, double y2, R_GE_gcontext *gc, pDevDesc dev) {
	RKGraphicsDataStreamWriteGuard guard;
	RKD_OUT_STREAM << WRITE_HEADER (RKDLine, dev);
	RKD_OUT_STREAM << x1 << y1 << x2 << y2;
	RKD_OUT_STREAM << WRITE_PEN ();
	RKD_OUT_STREAM << WRITE_LINE_ENDS ();
}

static void RKD_Polygon (int n, double *x, double *y, R_GE_gcontext *gc, pDevDesc dev) {
	RKGraphicsDataStreamWriteGuard guard;
	RKD_OUT_STREAM << WRITE_HEADER (RKDPolygon, dev);
	quint32 _n = qMin (n, 1 << 25);	// skip stuff exceeding reasonable limits to keep protocol simple
	RKD_OUT_STREAM << _n;
	for (quint32 i = 0; i < _n; ++i) {
		RKD_OUT_STREAM << x[i] << y[i];
	}
	RKD_OUT_STREAM << WRITE_PEN ();
	RKD_OUT_STREAM << WRITE_LINE_ENDS ();
	RKD_OUT_STREAM << WRITE_FILL ();
}

static void RKD_Polyline (int n, double *x, double *y, R_GE_gcontext *gc, pDevDesc dev) {
	RKGraphicsDataStreamWriteGuard guard;
	RKD_OUT_STREAM << WRITE_HEADER (RKDPolyline, dev);
	quint32 _n = qMin (n, 1 << 25);	// skip stuff exceeding reasonable limits to keep protocol simple
	RKD_OUT_STREAM << _n;
	for (quint32 i = 0; i < _n; ++i) {
		RKD_OUT_STREAM << x[i] << y[i];
	}
	RKD_OUT_STREAM << WRITE_PEN ();
	RKD_OUT_STREAM << WRITE_LINE_ENDS ();
}

static void RKD_Rect (double x0, double y0, double x1, double y1, R_GE_gcontext *gc, pDevDesc dev) {
	RKGraphicsDataStreamWriteGuard guard;
	RKD_OUT_STREAM << WRITE_HEADER (RKDPolyline, dev);
	RKD_OUT_STREAM << QRectF (x0, y0, x1-x0, y1-y0);
	RKD_OUT_STREAM << WRITE_PEN ();
	RKD_OUT_STREAM << WRITE_LINE_ENDS ();
	RKD_OUT_STREAM << WRITE_FILL ();
}

static void RKD_TextUTF8 (double x, double y, const char *str, double rot, double hadj, R_GE_gcontext *gc, pDevDesc dev) {
	RKGraphicsDataStreamWriteGuard guard;
	RKD_OUT_STREAM << WRITE_HEADER (RKDTextUTF8, dev);
	RKD_OUT_STREAM << x << y << QString::fromUtf8 (str) << rot << hadj;
	RKD_OUT_STREAM << WRITE_COL ();
	RKD_OUT_STREAM << WRITE_FONT (dev);
}

static double RKD_StrWidthUTF8 (const char *str, R_GE_gcontext *gc, pDevDesc dev) {
	{
		RKGraphicsDataStreamWriteGuard guard;
		RKD_OUT_STREAM << WRITE_HEADER (RKDStrWidthUTF8, dev);
		RKD_OUT_STREAM << QString::fromUtf8 (str);
		RKD_OUT_STREAM << WRITE_FONT (dev);
	}
	double ret;
	{
		RKGraphicsDataStreamReadGuard guard;
		RKD_IN_STREAM >> ret;
	}
	return ret;
}

static void RKD_NewPage (R_GE_gcontext *gc, pDevDesc dev) {
	RKGraphicsDataStreamWriteGuard guard;
	RKD_OUT_STREAM << WRITE_HEADER (RKDNewPage, dev);
	RKD_OUT_STREAM << WRITE_FILL ();
}

static void RKD_MetricInfo (int c, R_GE_gcontext *gc, double* ascent, double* descent, double* width, pDevDesc dev) {
	{
		RKGraphicsDataStreamWriteGuard wguard;
		RKD_OUT_STREAM << WRITE_HEADER (RKDMetricInfo, dev);
		RKD_OUT_STREAM << QChar (c);
		RKD_OUT_STREAM << WRITE_FONT (dev);
	}
	{
		RKGraphicsDataStreamReadGuard rguard;
		RKD_IN_STREAM >> *ascent;
		RKD_IN_STREAM >> *descent;
		RKD_IN_STREAM >> *width;
	}
}

static void RKD_Close (pDevDesc dev) {
	RKGraphicsDataStreamWriteGuard guard;
	RKD_OUT_STREAM << WRITE_HEADER (RKDClose, dev);
}

static void RKD_Activate (pDevDesc dev) {
	RKGraphicsDataStreamWriteGuard guard;
	RKD_OUT_STREAM << WRITE_HEADER (RKDActivate, dev);
}

static void RKD_Deactivate (pDevDesc dev) {
	RKGraphicsDataStreamWriteGuard guard;
	RKD_OUT_STREAM << WRITE_HEADER (RKDDeActivate, dev);
}

static void RKD_Clip (double left, double right, double top, double bottom, pDevDesc dev) {
	dev->clipLeft = left;
	dev->clipRight = right;
	dev->clipTop = top;
	dev->clipBottom = bottom;
	RKGraphicsDataStreamWriteGuard guard;
	RKD_OUT_STREAM << WRITE_HEADER (RKDClip, dev);
	RKD_OUT_STREAM << QRectF (left, top, right - left, bottom - top);
}

static void RKD_Mode (int mode, pDevDesc dev) {
	Q_UNUSED (mode);
	Q_UNUSED (dev);
/* Left empty for now. 1 is start signal, 0 is stop signal. Might be useful for flushing, though.

	RKGraphicsDataStreamWriteGuard guard;
	RKD_OUT_STREAM << WRITE_HEADER (RKDMode, dev);
	connectoin << (qint8) mode; */
}

static Rboolean RKD_Locator (double *x, double *y, pDevDesc dev) {
	{
		RKGraphicsDataStreamWriteGuard wguard;
		RKD_OUT_STREAM << WRITE_HEADER (RKDLocator, dev);
	}
	{
		RKGraphicsDataStreamReadGuard rguard;
		bool ok;
		RKD_IN_STREAM >> ok;
		RKD_IN_STREAM >> *x;
		RKD_IN_STREAM >> *y;
		if (ok) return (Rboolean) TRUE;
		return (Rboolean) FALSE;
	}
}

static Rboolean RKD_NewFrameConfirm (pDevDesc dev) {
	{
		RKGraphicsDataStreamWriteGuard wguard;
		RKD_OUT_STREAM << WRITE_HEADER (RKDNewPageConfirm, dev);
	}
	{
		RKGraphicsDataStreamReadGuard rguard;
		bool ok;
		RKD_IN_STREAM >> ok;
		if (ok) return (Rboolean) TRUE;
		return (Rboolean) FALSE;
	}
}
