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

#include <QRectF>

/** This class is essentially like QMutexLocker. In addition, the constructor waits until the next chunk of the transmission is ready (and does event processing) */
class RKSocketDataStreamReadGuard {
	RKSocketDataStreamReadGuard () {
#warning TODO: handle cancellation
		rkwarddeviceprotocolmutex.lock ();
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
			transmisison_size << protocol;
			while (connection->bytesAvailable () < transmisison_size) {
				RKRBackend::processX11Events ();
				connection->waitForReadyRead (10);
			}
		} END_SUSPEND_INTERRUPTS;
	}
	~RKSocketDataStreamReadGuard () {
		buffer.resize (0);
		rkwarddeviceprotocolmutex.unlock ();
	}
};

/** This class is essentially like QMutexLocker. In addition, the destructor takes care of pushing anything that was written to the protocol buffer during it lifetime to the transmitter. (Does NOT wait for the transmission itself). */
class RKSocketDataStreamWriteGuard {
	RKSocketDataStreamWriteGuard () {
		rkwarddeviceprotocolmutex.lock ();
	}
	~RKSocketDataStreamWriteGuard () {
		aux_stream << (quint32) buffer.size ();		// TODO: is uint32 always enough?
		connection->write (aux_buffer);
		aux_buffer.resize (0);
		connection->write (buffer);
		buffer.resize (0);
		rkwarddeviceprotocolmutex.unlock ();
	}

	static QByteArray aux_buffer;
	static QDataStream aux_stream;
};

#include <R_ext/GraphicsEngine.h>

#define WRITE_HEADER(x,dev) (qint8) x << (quint8) static_cast<RKGraphicsDeviceDesc*> (dev->deviceSpecific)->devnum
#define WRITE_COL() (qint32) gc->col
#define WRITE_PEN() WRITE_COL() << (double) gc->lwd << (qint32) << gc->lty
#define WRITE_LINE_ENDS() (quint8) gc->lend << (quint8) gc->ljoin << gc->lmitre
#define WRITE_FILL() (qint32) gc->fill
#define WRITE_FONT(dev) gc->cex << gc->ps << gc->lineheight << (quint8) gc->fontface << gc->fontfamily[0] ? QString (gc->fontfamily) : (static_cast<RKGraphicsDeviceDesc*> (dev->deviceSpecific)->default_family)

static void RKD_Create (double width, double height, pDevDesc dev) {
	RKSocketDataStreamWriteGuard guard;
	protocol << WRITE_HEADER (RKDCreate, dev);
	protocol << width << height;
}

static void RKD_Circle (double x, double y, double r, R_GE_gcontext *gc, pDevDesc dev) {
	RKSocketDataStreamWriteGuard guard;
	protocol << WRITE_HEADER (RKDCircle, dev);
	protocol << x << y << r;
	protocol << WRITE_PEN ();
}

static void RKD_Line (double x1, double y1, double x2, double y2, R_GE_gcontext *gc, pDevDesc dev) {
	RKSocketDataStreamWriteGuard guard;
	protocol << WRITE_HEADER (RKDLine, dev);
	protocol << x1 << y1 << x2 << y2;
	protocol << WRITE_PEN ();
}

static void RKD_Polygon (int n, double *x, double *y, R_GE_gcontext *gc, pDevDesc dev) {
	RKSocketDataStreamWriteGuard guard;
	protocol << WRITE_HEADER (RKDPolygon, dev);
	quint32 _n = qMax (n, 1 << 25);	// skip stuff exceeding reasonable limits to keep protocol simple
	protocol << _n;
	for (quint32 i; i < _n; ++i) {
		protocol << x[i] << y[i];
	}
	protocol << WRITE_PEN ();
	protocol << WRITE_LINE_ENDS ();
	protocol << WRITE_FILL ();
}

static void RKD_Polyline (int n, double *x, double *y, R_GE_gcontext *gc, pDevDesc dev) {
	RKSocketDataStreamWriteGuard guard;
	protocol << WRITE_HEADER (RKDPolyline, dev);
	quint32 _n = qMax (n, 1 << 25);	// skip stuff exceeding reasonable limits to keep protocol simple
	protocol << _n;
	for (quint32 i; i < _n; ++i) {
		protocol << x[i] << y[i];
	}
	protocol << WRITE_PEN ();
	protocol << WRITE_LINE_ENDS ();
}

static void RKD_Rect (double x0, double y0, double x1, double y1, R_GE_gcontext *gc, pDevDesc dev) {
	RKSocketDataStreamWriteGuard guard;
	protocol << WRITE_HEADER (RKDPolyline, dev);
	protocol << QRectF (x0, y0, x1-x0, y1-y0);
	protocol << WRITE_PEN ();
	protocol << WRITE_LINE_ENDS ();
	protocol << WRITE_FILL ();
}

static void RKD_TextUTF8 (double x, double y, char *str, double rot, double hadj, R_GE_gcontext *gc, pDevDesc dev) {
	RKSocketDataStreamWriteGuard guard;
	protocol << WRITE_HEADER (RKDTextUTF8, dev);
	protocol << x << y << QString::fromUtf8 (str) << rot << hadj;
	protocol << WRITE_COL ();
	protocol << WRITE_FONT (dev);
}

static double RKD_StrWidthUTF8 (char *str, R_GE_gcontext *gc, pDevDesc dev) {
	{
		RKSocketDataStreamWriteGuard guard;
		protocol << WRITE_HEADER (RKDStrWidthUTF8, dev);
		protocol << QString::fromUtf8 (str);
		protocol << WRITE_FONT (dev);
	}
	double ret;
	{
		RKSocketDataStreamReadGuard guard;
		ret << protocol;
	}
	return ret;
}

static void RKD_NewPage (R_GE_gcontext *gc, pDevDesc dev) {
	RKSocketDataStreamWriteGuard guard;
	protocol << WRITE_HEADER (RKDNewPage, dev);
	protocol << WRITE_FILL ();
}

static void RKD_MetricInfo (int c, R_GE_gcontext *gc, double* ascent, double* descent, double* width, pDevDesc dev) {
	{
		RKSocketDataStreamWriteGuard wguard;
		protocol << WRITE_HEADER (RKDMetricInfo, dev);
		protocol << QChar (c);
		protocol << WRITE_FONT (dev);
	}
	{
		RKSocketDataStreamReadGuard rguard;
		*ascent << protocol;
		*descent << protocol;
		*width << protocol;
	}
}

static void RKD_Close (pDevDesc dev) {
	RKSocketDataStreamWriteGuard guard;
	protocol << WRITE_HEADER (RKDClose, dev);
}

static void RKD_Activate (pDevDesc dev) {
	RKSocketDataStreamWriteGuard guard;
	protocol << WRITE_HEADER (RKDActivate, dev);
}

static void RKD_Deactivate (pDevDesc dev) {
	RKSocketDataStreamWriteGuard guard;
	protocol << WRITE_HEADER (RKDDeActivate, dev);
}

static void RKD_Clip (double left, double right, double top, double bottom, pDevDesc dev) {
	dev->clipLeft = left;
	dev->clipRight = right;
	dev->clipTop = top;
	dev->clipBottom = bottom;
	RKSocketDataStreamWriteGuard guard;
	protocol << WRITE_HEADER (RKDClip, dev);
	protocol << QRectF (left, top, right - left, bottom - top);
}

static void RKD_Mode (int mode, pDevDesc dev) {
/* Left empty for now. 1 is start signal, 0 is stop signal. Might be useful for flushing, though.

	RKSocketDataStreamWriteGuard guard;
	protocol << WRITE_HEADER (RKDMode, dev);
	connectoin << (qint8) mode; */
}

static Rboolean RKD_Locator (double *x, double *y, pDevDesc dev) {
	{
		RKSocketDataStreamWriteGuard wguard;
		protocol << WRITE_HEADER (RKDLocator, dev);
	}
	{
		RKSocketDataStreamReadGuard rguard;
		bool ok;
		ok << protocol;
		*x << protocol;
		*y << protocol;
		if (ok) return TRUE;
		return FALSE;
	}
}

static Rboolean RKD_NewFrameConfirm (pDevDesc dev) {
	{
		RKSocketDataStreamWriteGuard wguard;
		protocol << WRITE_HEADER (RKDNewPageConfirm, dev);
	}
	{
		RKSocketDataStreamReadGuard rguard;
		bool ok << protocol;
		if (ok) return TRUE;
		return FALSE;
	}
}
