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
#include "../rkreventloop.h"
#include "../../debug.h"

extern "C" {
#include <R_ext/GraphicsEngine.h>
}

#define RKD_IN_STREAM RKGraphicsDeviceBackendTransmitter::streamer.instream
#define RKD_OUT_STREAM RKGraphicsDeviceBackendTransmitter::streamer.outstream

static bool rkd_waiting_for_reply = false;
static int rkd_suppress_on_exit = 0;

/** This class is essentially like QMutexLocker. In addition, the constructor waits until the next chunk of the transmission is ready (and does event processing).
 *
 * @note: Never ever call Rf_error(), or any R function that might fail during the lifetime of an RKGraphicsDataStreamReadGuard or
 * RKGraphicsDataStreamWriteGuard. If R decides to long-jump out, the d'tor will not be called, the mutex will be left locked, and
 * the next graphics operation will hang, with no way to interrupt.
 * 
 * At the same time, note that the RKGraphicsDataStreamReadGuard c'tor @em may cause R to long-jump (safely) in case of a user interrupt,
 * or if the connection was killed. Don't rely on the code following the creation of an RKGraphicsDataStreamReadGuard to be called.
 */
class RKGraphicsDataStreamReadGuard {
public:
	RKGraphicsDataStreamReadGuard () {
		RKGraphicsDeviceBackendTransmitter::mutex.lock ();
		have_lock = true;
		rkd_waiting_for_reply = true;
		QIODevice* connection = RKGraphicsDeviceBackendTransmitter::connection;
		BEGIN_SUSPEND_INTERRUPTS {
			while (connection->bytesToWrite ()) {
				if (!connection->waitForBytesWritten (10)) {
					checkHandleError ();
				}
				if (connection->bytesToWrite ()) RKREventLoop::processX11Events ();
			}
			while (!RKGraphicsDeviceBackendTransmitter::streamer.readInBuffer ()) {
				RKREventLoop::processX11Events ();
				if (!connection->waitForReadyRead (10)) {
					if (checkHandleInterrupt (connection)) break;
					checkHandleError ();
				}
			}
			if (R_interrupts_pending) {
				if (have_lock) {
					RKGraphicsDeviceBackendTransmitter::mutex.unlock ();
					have_lock = false;  // Will d'tor still be called? We don't rely on it.
				}
				rkd_waiting_for_reply = false;
			}
		} END_SUSPEND_INTERRUPTS;
	}

	~RKGraphicsDataStreamReadGuard () {
		if (have_lock) RKGraphicsDeviceBackendTransmitter::mutex.unlock ();
		rkd_waiting_for_reply = false;
	}

private:
	bool checkHandleInterrupt (QIODevice *connection) {
		// NOTE: It would be possible, but not exactly easier to rely on GEonExit() rather than R_interrupts_pending
		// Might be an option, if R_interrupts_pending gets hidden one day, though
		if (!R_interrupts_pending) return false;

		// Tell the frontend to finish whatever it was doing ASAP. Don't process any other events until that has happened
		RKGraphicsDeviceBackendTransmitter::streamer.outstream << (quint8) RKDCancel << (quint8) 0;
		RKGraphicsDeviceBackendTransmitter::streamer.writeOutBuffer ();
		while (connection->bytesToWrite ()) {
			if (!connection->waitForBytesWritten (10)) {
				checkHandleError ();
			}
		}
		int loop = 0;
		while (!RKGraphicsDeviceBackendTransmitter::streamer.readInBuffer ()) {
			if (!connection->waitForReadyRead (10)) {
				if (++loop > 500) {
					connection->close ();	// If frontend is unresponsive, kill connection
				}
				checkHandleError ();
			}
		}
		return true;
	}

	void checkHandleError () {
		if (!RKGraphicsDeviceBackendTransmitter::connectionAlive ()) {	// Don't go into endless loop, if e.g. frontend has crashed
			if (have_lock) RKGraphicsDeviceBackendTransmitter::mutex.unlock ();
			have_lock = false;	// Will d'tor still be called? We don't rely on it.
			Rf_error ("RKWard Graphics connection has shut down");
		}
	}
	bool have_lock;
};

/** This class is essentially like QMutexLocker. In addition, the destructor takes care of pushing anything that was written to the protocol buffer during it lifetime to the transmitter. (Does NOT wait for the transmission itself). */
class RKGraphicsDataStreamWriteGuard {
public:
	RKGraphicsDataStreamWriteGuard () {
		if (rkd_waiting_for_reply) {
			// For now, the backend does not support any nesting of graphics operations. It would make the protocol more complex.
			// I believe the only use-case is resizing during interaction, and IMO, that's not a terribly important one to support.
			//
			// In case we do want to support nested operations, I think the plan would be basically:
			// - For every request that awaits a reply (and only those), send a reply token
			// - In RKGraphicsDataStreamReadGuard () wait for that specific token. If another token arrives, instead of the expected one,
			//   put it on a stack, and continue waiting.
			// - When waiting for a reply, also check the stack.
			// What about the mutex?
			// Well, essentially, during rkd_waiting_for_reply, nothing should attempt to obtain a lock. The transmitter thread can simply pause
			// during that time.
			rkd_suppress_on_exit++;
			Rf_error ("Nested graphics operations are not supported by this device (did you try to resize the device during locator()?)");
		}
		RKGraphicsDeviceBackendTransmitter::mutex.lock ();
	}
	~RKGraphicsDataStreamWriteGuard () {
		RKGraphicsDeviceBackendTransmitter::streamer.writeOutBuffer ();
		RKGraphicsDeviceBackendTransmitter::mutex.unlock ();
	}
};

#include <QRectF>
#include <QSizeF>

// This ought to be optimized away by the compiler:
#define SAFE_LINE_END(lend) (quint8) (lend == GE_ROUND_CAP ? RoundLineCap : (lend == GE_BUTT_CAP ? ButtLineCap : SquareLineCap))
// This ought to be optimized away by the compiler:
#define SAFE_LINE_JOIN(ljoin) (quint8) (ljoin == GE_ROUND_JOIN ? RoundJoin : (ljoin == GE_BEVEL_JOIN ? BevelJoin : MitreJoin))

// I'd love to convert to QColor, directly, but that's in QtGui, not QtCore. QRgb used different byte ordering
#define WRITE_COLOR_BYTES(col) \
	if (col == NA_INTEGER) RKD_OUT_STREAM << (quint8) 0xFF << (quint8) 0xFF << (quint8) 0xFF << (quint8) 0x00; \
	else RKD_OUT_STREAM << (quint8) R_RED (col) << (quint8) R_GREEN (col) << (quint8) R_BLUE (col) << (quint8) R_ALPHA (col)
#define WRITE_HEADER_NUM(x,devnum) \
	RKD_OUT_STREAM << (qint8) x << (quint8) devnum
#define WRITE_HEADER(x,dev) \
	WRITE_HEADER_NUM (x,static_cast<RKGraphicsDeviceDesc*> (dev->deviceSpecific)->devnum)
#define WRITE_COL() \
	WRITE_COLOR_BYTES (gc->col)
#define WRITE_PEN() \
	WRITE_COL(); RKD_OUT_STREAM << (double) gc->lwd << (qint32) gc->lty
// actually lmitre is only needed if linejoin is GE_MITRE_JOIN, so we could optimize a bit
#define WRITE_LINE_ENDS() \
	RKD_OUT_STREAM << SAFE_LINE_END (gc->lend) << SAFE_LINE_JOIN (gc->ljoin) << gc->lmitre
#define WRITE_FILL() \
	WRITE_COLOR_BYTES (gc->fill)
#define WRITE_FONT(dev) \
	RKD_OUT_STREAM << gc->cex << gc->ps << gc->lineheight << (quint8) gc->fontface << (gc->fontfamily[0] ? QString (gc->fontfamily) : (static_cast<RKGraphicsDeviceDesc*> (dev->deviceSpecific)->getFontFamily (gc->fontface == 5)))

static void RKD_QueryResolution (int *dpix, int *dpiy) {
	{
		RKGraphicsDataStreamWriteGuard wguard;
		WRITE_HEADER_NUM (RKDQueryResolution, 0);
	}
	{
		RKGraphicsDataStreamReadGuard rguard;
		qint32 _dpix, _dpiy;
		RKD_IN_STREAM >> _dpix >> _dpiy;
		*dpix = _dpix; *dpiy = _dpiy;
	}
}

static void RKD_Create (double width, double height, pDevDesc dev, const char *title, bool antialias) {
	RKGraphicsDataStreamWriteGuard guard;
	WRITE_HEADER (RKDCreate, dev);
	RKD_OUT_STREAM << width << height << QString::fromUtf8 (title) << antialias;
}

static void RKD_Size (double *left, double *right, double *top, double *bottom, pDevDesc dev) {
// NOTE: This does *not* query the frontend for the current size. This is only done on request
	*left = dev->left;
	*top = dev->top;
	*right = dev->right;
	*bottom = dev->bottom;
}

static void RKD_SetSize (pDevDesc dev) {
	RKGraphicsDataStreamWriteGuard wguard;
	WRITE_HEADER (RKDSetSize, dev);
	RKD_OUT_STREAM << QSize (qAbs (dev->right - dev->left) + .2, qAbs (dev->bottom - dev->top) + .2);
}

SEXP RKD_AdjustSize (SEXP _devnum) {
	int devnum = Rf_asInteger (_devnum);
	pGEDevDesc gdev = GEgetDevice (devnum);
	if (!gdev) Rf_error ("No such device %d", devnum);
	pDevDesc dev = gdev->dev;
	{
		RKGraphicsDataStreamWriteGuard wguard;
		WRITE_HEADER (RKDGetSize, dev);
	}
	QSizeF size;
	{
		RKGraphicsDataStreamReadGuard rguard;
		RKD_IN_STREAM >> size;
	}
	if (size.isNull ()) Rf_error ("Could not determine current size of device %d. Not an RK device?", devnum);
	dev->left = dev->top = 0;
	dev->right = size.width ();
	dev->bottom = size.height ();

	RKD_SetSize (dev);    // This adjusts the rendering area in the frontend
	GEplayDisplayList (gdev);
	return R_NilValue;
}

static void RKD_Circle (double x, double y, double r, R_GE_gcontext *gc, pDevDesc dev) {
	RKGraphicsDataStreamWriteGuard guard;
	WRITE_HEADER (RKDCircle, dev);
	RKD_OUT_STREAM << x << y << r;
	WRITE_PEN ();
	WRITE_FILL ();
}

static void RKD_Line (double x1, double y1, double x2, double y2, R_GE_gcontext *gc, pDevDesc dev) {
	RKGraphicsDataStreamWriteGuard guard;
	WRITE_HEADER (RKDLine, dev);
	RKD_OUT_STREAM << x1 << y1 << x2 << y2;
	WRITE_PEN ();
	WRITE_LINE_ENDS ();
}

static void RKD_Polygon (int n, double *x, double *y, R_GE_gcontext *gc, pDevDesc dev) {
	RKGraphicsDataStreamWriteGuard guard;
	WRITE_HEADER (RKDPolygon, dev);
	quint32 _n = qMin (n, 1 << 25);	// skip stuff exceeding reasonable limits to keep protocol simple
	RKD_OUT_STREAM << _n;
	for (quint32 i = 0; i < _n; ++i) {
		RKD_OUT_STREAM << x[i] << y[i];
	}
	WRITE_PEN ();
	WRITE_LINE_ENDS ();
	WRITE_FILL ();
}

static void RKD_Polyline (int n, double *x, double *y, R_GE_gcontext *gc, pDevDesc dev) {
	RKGraphicsDataStreamWriteGuard guard;
	WRITE_HEADER (RKDPolyline, dev);
	quint32 _n = qMin (n, 1 << 25);	// skip stuff exceeding reasonable limits to keep protocol simple
	RKD_OUT_STREAM << _n;
	for (quint32 i = 0; i < _n; ++i) {
		RKD_OUT_STREAM << x[i] << y[i];
	}
	WRITE_PEN ();
	WRITE_LINE_ENDS ();
}

static void RKD_Rect (double x0, double y0, double x1, double y1, R_GE_gcontext *gc, pDevDesc dev) {
	RKGraphicsDataStreamWriteGuard guard;
	WRITE_HEADER (RKDRect, dev);
	RKD_OUT_STREAM << QRectF (x0, y0, x1-x0, y1-y0);
	WRITE_PEN ();
	WRITE_LINE_ENDS ();
	WRITE_FILL ();
}

static void RKD_TextUTF8 (double x, double y, const char *str, double rot, double hadj, R_GE_gcontext *gc, pDevDesc dev) {
	RKGraphicsDataStreamWriteGuard guard;
	WRITE_HEADER (RKDTextUTF8, dev);
	RKD_OUT_STREAM << x << y << QString::fromUtf8 (str) << rot << hadj;	// NOTE: yes, even Symbols are sent as UTF-8, here.
	WRITE_COL ();
	WRITE_FONT (dev);
}

static double RKD_StrWidthUTF8 (const char *str, R_GE_gcontext *gc, pDevDesc dev) {
	{
		RKGraphicsDataStreamWriteGuard guard;
		WRITE_HEADER (RKDStrWidthUTF8, dev);
		RKD_OUT_STREAM << QString::fromUtf8 (str);	// NOTE: yes, even Symbols are sent as UTF-8, here.
		WRITE_FONT (dev);
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
	WRITE_HEADER (RKDNewPage, dev);
	WRITE_FILL ();
}

static void RKD_MetricInfo (int c, R_GE_gcontext *gc, double* ascent, double* descent, double* width, pDevDesc dev) {
	{
		RKGraphicsDataStreamWriteGuard wguard;
		WRITE_HEADER (RKDMetricInfo, dev);
		QChar unichar;
		if (c < 0) unichar = QChar (-c);
		else {		// correct?! Do we get utf8, here?
			int inbuf[2];
			inbuf[0] = c;
			inbuf[1] = 0;
			QString dummy = QString::fromUtf8 ((char*) inbuf);
			if (!dummy.isEmpty ()) unichar = dummy.at (0);
		}
		RKD_OUT_STREAM << unichar;
		WRITE_FONT (dev);
	}
	{
		RKGraphicsDataStreamReadGuard rguard;
		RKD_IN_STREAM >> *ascent >> *descent >> *width;
	}
}

static void RKD_Close (pDevDesc dev) {
	RKGraphicsDataStreamWriteGuard guard;
	WRITE_HEADER (RKDClose, dev);
	delete static_cast<RKGraphicsDeviceDesc*> (dev->deviceSpecific);
}

static void RKD_Activate (pDevDesc dev) {
	RKGraphicsDataStreamWriteGuard guard;
	WRITE_HEADER (RKDActivate, dev);
}

static void RKD_Deactivate (pDevDesc dev) {
	RKGraphicsDataStreamWriteGuard guard;
	WRITE_HEADER (RKDDeActivate, dev);
}

static void RKD_Clip (double left, double right, double top, double bottom, pDevDesc dev) {
	dev->clipLeft = left;
	dev->clipRight = right;
	dev->clipTop = top;
	dev->clipBottom = bottom;
	RKGraphicsDataStreamWriteGuard guard;
	WRITE_HEADER (RKDClip, dev);
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

static void RKD_Raster (unsigned int *raster, int w, int h, double x, double y, double width, double height, double rot, Rboolean interpolate, const pGEcontext gc, pDevDesc dev) {
	Q_UNUSED (gc);

	RKGraphicsDataStreamWriteGuard wguard;
	WRITE_HEADER (RKDRaster, dev);

	int *_raster = reinterpret_cast<int*> (raster);	// shut up warning in WRITE_COLOR_BYTES. It's just four separete bytes, anyway
	quint32 _w = qMin (w, 1 << 15);	// skip stuff exceeding reasonable limits to keep protocol simple
	RKD_OUT_STREAM << _w;
	quint32 _h = qMin (h, 1 << 15);
	RKD_OUT_STREAM << _h;
	for (quint32 col = 0; col < _h; ++col) {
		for (quint32 row = 0; row < _w; ++row) {
			WRITE_COLOR_BYTES (_raster[(col*_w) + row]);
		}
	}
	RKD_OUT_STREAM << QRectF (x, y, width, height) << rot << (bool) interpolate;
}

static SEXP RKD_Capture (pDevDesc dev) {
	{
		RKGraphicsDataStreamWriteGuard wguard;
		WRITE_HEADER (RKDCapture, dev);
	}

	quint32 w, h;
	quint32 size;
	int *buffer;
	{
		RKGraphicsDataStreamReadGuard rguard;
		quint8 r, g, b, a;
		RKD_IN_STREAM >> w >> h;
		size = w*h;
 		buffer = new int[size];// Although unlikely, allocVector below could fail. We don't want to be left with a locked mutex (from the rguard) in this case. (Being left with a dead pointer looks benign in comparison; Note that the vector may easily be too large for allocation on the stack)
		quint32 i = 0;
		for (quint32 col = 0; col < h; ++col) {
			for (quint32 row = 0; row < w; ++row) {
				RKD_IN_STREAM >> r >> g >> b >> a;
				buffer[i++] = R_RGBA (r, g, b, a);
			}
		}
	}
	SEXP ret, dim;
	PROTECT (ret = Rf_allocVector (INTSXP, size));
	int* ret_vals = INTEGER (ret);
	for (quint32 i = 0; i < size; ++i) {
		ret_vals[i] = buffer[i];
	}
	delete (buffer);

	// Documentation does not mention it, but cap expects dim information to be returned
	PROTECT(dim = Rf_allocVector (INTSXP, 2));
	INTEGER (dim)[0] = w;
	INTEGER (dim)[1] = h;
	Rf_setAttrib (ret, R_DimSymbol, dim);

	UNPROTECT (2);
	return ret;
}

static Rboolean RKD_Locator (double *x, double *y, pDevDesc dev) {
	{
		RKGraphicsDataStreamWriteGuard wguard;
		WRITE_HEADER (RKDLocator, dev);
	}
	{
		RKGraphicsDataStreamReadGuard rguard;
		bool ok;
		RKD_IN_STREAM >> ok >> *x >> *y;
		if (ok) return (Rboolean) TRUE;
		return (Rboolean) FALSE;
	}
}

static Rboolean RKD_NewFrameConfirm (pDevDesc dev) {
	{
		RKGraphicsDataStreamWriteGuard wguard;
		WRITE_HEADER (RKDNewPageConfirm, dev);
	}
	bool ok;
	{
		RKGraphicsDataStreamReadGuard rguard;
		RKD_IN_STREAM >> ok;
	}
	if (!ok) Rf_error ("Aborted by user");
	return (Rboolean) TRUE;
	// Return value FALSE: Let R ask, instead
}

#if R_VERSION >= R_Version (2, 12, 0)
void RKD_EventHelper (pDevDesc dev, int code) {
	{
		RKGraphicsDataStreamWriteGuard wguard;
		if (code == 1) {
			QString prompt;
			if (Rf_isEnvironment (dev->eventEnv)) {
				SEXP sprompt = Rf_findVar (Rf_install ("prompt"), dev->eventEnv);
				if (Rf_length (sprompt) == 1) prompt = QString::fromUtf8 (CHAR (Rf_asChar (sprompt)));
			}
			WRITE_HEADER (RKDStartGettingEvents, dev);
			RKD_OUT_STREAM << prompt;
			return;
		} else if (code == 0) {
			WRITE_HEADER (RKDStopGettingEvents, dev);
			return;
		} else {
			WRITE_HEADER (RKDFetchNextEvent, dev);
		}
	}
	RK_ASSERT (code == 2);

	// NOTE: The event handler functions doKeybd() and doMouseEvent() could conceivably produce errors -> longjump
	// Thus we need to make sure the read-guard has gone out of scope before that. Thus, we take the somewhat clunky
	// route of first reading the full reply, then processing it.
	qint8 event_code;
	QString text;
	qint32 keycode, modifiers;
	double x, y;
	qint8 buttons;
	{
		RKGraphicsDataStreamReadGuard rguard;
		RKD_IN_STREAM >> event_code;
		if ((event_code == RKDNothing) || (event_code == RKDFrontendCancel)) {
			// nothing
		} else if (event_code == RKDKeyPress) {
			RKD_IN_STREAM >> text >> keycode >> modifiers;
		} else {  // a mouse event
			RKD_IN_STREAM >> buttons >> x >> y;
		}
	}

	if (event_code == RKDFrontendCancel) {
		Rf_error ("Interrupted by user");
		return;  // not reached
	}
	if (event_code == RKDNothing) return;
	else if (event_code == RKDKeyPress) {
		if (modifiers - (modifiers & Qt::ShiftModifier)) {  // any other modifier than Shift, alone. NOTE: devX11.c and devWindows.c handle Ctrl, only as of R 3.0.0
			QString mod_text;
			if (modifiers & Qt::ControlModifier) mod_text.append ("ctrl-");
			if (modifiers & Qt::AltModifier) mod_text.append ("alt-");
			if (modifiers & Qt::MetaModifier) mod_text.append ("meta-");
			if (text.isEmpty () && (modifiers & Qt::ShiftModifier)) mod_text.append ("shift-");     // don't apply shift when there is text (where it has already been handled)
			text = mod_text + text.toUpper ();
		}

		R_KeyName r_key_name = knUNKNOWN;
		if (keycode == Qt::Key_Left) r_key_name = knLEFT;
		else if (keycode == Qt::Key_Right) r_key_name = knRIGHT;
		else if (keycode == Qt::Key_Up) r_key_name = knUP;
		else if (keycode == Qt::Key_Down) r_key_name = knDOWN;
		else if ((keycode >= Qt::Key_F1) && (keycode <= Qt::Key_F12)) r_key_name = (R_KeyName) (knF1 + (keycode - Qt::Key_F1));
		else if (keycode == Qt::Key_PageUp) r_key_name = knPGUP;
		else if (keycode == Qt::Key_PageDown) r_key_name = knPGDN;
		else if (keycode == Qt::Key_End) r_key_name = knEND;
		else if (keycode == Qt::Key_Home) r_key_name = knHOME;
		else if (keycode == Qt::Key_Insert) r_key_name = knINS;
		else if (keycode == Qt::Key_Delete) r_key_name = knDEL;

		Rf_doKeybd (dev, r_key_name, text.toUtf8 ());
	} else {    // all others are mouse events
		Rf_doMouseEvent (dev, event_code == RKDMouseDown ? meMouseDown : (event_code == RKDMouseUp ? meMouseUp : meMouseMove), buttons, x, y);
	}
}

void RKD_onExit (pDevDesc dev) {
	if (rkd_suppress_on_exit > 0) {
		--rkd_suppress_on_exit;
		return;
	}
	if (dev->gettingEvent) {
		RKGraphicsDataStreamWriteGuard wguard;
		WRITE_HEADER (RKDStopGettingEvents, dev);
	}
	dev->gettingEvent = (Rboolean) false;
}

#endif
