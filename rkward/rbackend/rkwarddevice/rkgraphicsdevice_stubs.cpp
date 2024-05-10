/*
rkgraphicsdevice_stubs - This file is part of RKWard (https://rkward.kde.org). Created: Mon Mar 18 2013
SPDX-FileCopyrightText: 2013-2024 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

/* NOTE: This file is essentially a split-out from rkgraphicsdevice_setup.cpp,
 * not a compilation unit of its own.
 * It is meant to be included, there. */

#define RKD_BACKEND_CODE
#define RKD_RGE_VERSION R_GE_version
#include "rkgraphicsdevice_protocol_shared.h"
#include "rkgraphicsdevice_backendtransmitter.h"
#include "../rkrbackend.h"
#include "../rkreventloop.h"
#include "../../debug.h"

#undef RK_TRACE
#define RK_TRACE(flags)
/*
#define RK_TRACE(flags) RKFullTrace _rk_full_trace(__FILE__, __FUNCTION__, __LINE__);
class RKFullTrace {
public:
	RKFullTrace(char *file, const char *function, int line) : _function(function){
		qDebug("Trace enter: %s - function %s line %d", file, _function, line);
	};
	~RKFullTrace() {
		qDebug("Trace leave: function %s", _function);
	};
	const char *_function;
}; */


#include "../rkrapi.h"

#define RKD_IN_STREAM RKGraphicsDeviceBackendTransmitter::streamer.instream
#define RKD_OUT_STREAM RKGraphicsDeviceBackendTransmitter::streamer.outstream

static bool rkd_waiting_for_reply = false;
static int rkd_suppress_on_exit = 0;

/** This class is essentially like QMutexLocker. In addition, the constructor waits until the next chunk of the transmission is ready (and does event processing).
 *
 * @note: Never ever call RFn::Rf_error(), or any R function that might fail during the lifetime of an RKGraphicsDataStreamReadGuard or
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
		{
			RKRSupport::InterruptSuspension susp;
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
			if (ROb(R_interrupts_pending)) {
				if (have_lock) {
					RKGraphicsDeviceBackendTransmitter::mutex.unlock ();
					have_lock = false;  // Will d'tor still be called? We don't rely on it.
				}
				rkd_waiting_for_reply = false;
			}
		};
	}

	~RKGraphicsDataStreamReadGuard () {
		if (have_lock) RKGraphicsDeviceBackendTransmitter::mutex.unlock ();
		rkd_waiting_for_reply = false;
	}

private:
	bool checkHandleInterrupt (QIODevice *connection) {
		// NOTE: It would be possible, but not exactly easier to rely on GEonExit() rather than R_interrupts_pending
		// Might be an option, if R_interrupts_pending gets hidden one day, though
		if (!ROb(R_interrupts_pending)) return false;

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
			RFn::Rf_error("RKWard Graphics connection has shut down");
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
			RFn::Rf_error("Nested graphics operations are not supported by this device (did you try to resize the device during locator()?)");
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

// I'd love to convert to QColor, directly, but that's in QtGui, not QtCore. QRgb used different byte ordering
// TODO: is the check against NA_INTEGER needed at all?
#define WRITE_COLOR_BYTES(col) \
	RKD_OUT_STREAM << (quint8) R_RED (col) << (quint8) R_GREEN (col) << (quint8) R_BLUE (col) << (quint8) R_ALPHA (col)
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
	RKD_OUT_STREAM << (quint8) mapLineEndStyle(gc->lend) << (quint8) mapLineJoinStyle(gc->ljoin) << gc->lmitre
#if R_VERSION >= R_Version(4, 1, 0)
#  define WRITE_FILL() \
	if (gc->patternFill != ROb(R_NilValue)) RKD_OUT_STREAM << (qint8) PatternFill << (qint16) (RFn::INTEGER(gc->patternFill)[0]); \
	else { \
		RKD_OUT_STREAM << (qint8) ColorFill; WRITE_COLOR_BYTES (gc->fill); \
	}
#else
#  define WRITE_FILL() \
	RKD_OUT_STREAM << (qint8) ColorFill; WRITE_COLOR_BYTES (gc->fill);
#endif
#define WRITE_FONT(dev) \
	RKD_OUT_STREAM << gc->cex << gc->ps << gc->lineheight << (quint8) gc->fontface << (gc->fontfamily[0] ? QString (gc->fontfamily) : (static_cast<RKGraphicsDeviceDesc*> (dev->deviceSpecific)->getFontFamily (gc->fontface == 5)))

static void RKD_QueryResolution (int *dpix, int *dpiy) {
	RK_TRACE(GRAPHICS_DEVICE);
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

static void RKD_Create (double width, double height, pDevDesc dev, const char *title, bool antialias, quint32 id) {
	RK_TRACE(GRAPHICS_DEVICE);
	{
		RKGraphicsDataStreamWriteGuard guard;
		WRITE_HEADER (RKDCreate, dev);
		RKD_OUT_STREAM << width << height << QString::fromUtf8 (title) << antialias << id;
	}
	{
		// Reading a reply in order to force this to be synchronous. .rk.with.placement.hint() may run into race conditions, otherwise.
		RKGraphicsDataStreamReadGuard rguard;
		quint32 dummy;
		RKD_IN_STREAM >> dummy;
	}
}

static void RKD_Size (double *left, double *right, double *bottom, double *top, pDevDesc dev) {
	RK_TRACE(GRAPHICS_DEVICE);
// NOTE: This does *not* query the frontend for the current size. This is only done on request
	*left = dev->left;
	*top = dev->top;
	*right = dev->right;
	*bottom = dev->bottom;
}

static void RKD_SetSize (pDevDesc dev) {
	RK_TRACE(GRAPHICS_DEVICE);
	RKGraphicsDataStreamWriteGuard wguard;
	WRITE_HEADER (RKDSetSize, dev);
	RKD_OUT_STREAM << QSize (qAbs (dev->right - dev->left) + .2, qAbs (dev->bottom - dev->top) + .2);
}

static void RKD_Activate (pDevDesc dev) {
	RK_TRACE(GRAPHICS_DEVICE);
	RKGraphicsDataStreamWriteGuard guard;
	WRITE_HEADER (RKDActivate, dev);
}

static void RKD_Deactivate (pDevDesc dev) {
	RK_TRACE(GRAPHICS_DEVICE);
	RKGraphicsDataStreamWriteGuard guard;
	WRITE_HEADER (RKDDeActivate, dev);
}

SEXP RKD_AdjustSize(SEXP _devnum, SEXP _id) {
	RK_TRACE(GRAPHICS_DEVICE);
	int devnum = RFn::Rf_asInteger(_devnum);
	quint32 id = RFn::Rf_asInteger(_id);
	pGEDevDesc gdev = RFn::GEgetDevice(devnum);
	if (!gdev) RFn::Rf_error("No such device %d", devnum);
	pDevDesc dev = gdev->dev;
	// This is called from rkward:::RK.resize(), which in turn may be out of sync with R's device list. Before doing anything,
	// double check that this is really the device we think it is.
	if (dev->activate != RKD_Activate) RFn::Rf_error("Not an RKWard device", devnum);
	if (static_cast<RKGraphicsDeviceDesc*>(dev->deviceSpecific)->id != id) RFn::Rf_error("Graphics device mismatch", devnum);

	{
		RKGraphicsDataStreamWriteGuard wguard;
		WRITE_HEADER(RKDGetSize, dev);
	}
	QSizeF size;
	{
		RKGraphicsDataStreamReadGuard rguard;
		RKD_IN_STREAM >> size;
	}
	if (size.isNull()) RFn::Rf_error("Could not determine current size of device %d. Device closed?", devnum);
	dev->left = dev->top = 0;
	dev->right = size.width();
	dev->bottom = size.height();

	RKD_SetSize(dev);    // This adjusts the rendering area in the frontend
	if(gdev->dirty) RFn::GEplayDisplayList(gdev);
	return ROb(R_NilValue);
}

static void RKD_Circle (double x, double y, double r, R_GE_gcontext *gc, pDevDesc dev) {
	RK_TRACE(GRAPHICS_DEVICE);
	RKGraphicsDataStreamWriteGuard guard;
	WRITE_HEADER (RKDCircle, dev);
	RKD_OUT_STREAM << x << y << r;
	WRITE_PEN ();
	WRITE_FILL ();
}

static void RKD_Line (double x1, double y1, double x2, double y2, R_GE_gcontext *gc, pDevDesc dev) {
	RK_TRACE(GRAPHICS_DEVICE);
	RKGraphicsDataStreamWriteGuard guard;
	WRITE_HEADER (RKDLine, dev);
	RKD_OUT_STREAM << x1 << y1 << x2 << y2;
	WRITE_PEN ();
	WRITE_LINE_ENDS ();
}

static void RKD_Polygon (int n, double *x, double *y, R_GE_gcontext *gc, pDevDesc dev) {
	RK_TRACE(GRAPHICS_DEVICE);
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
	RK_TRACE(GRAPHICS_DEVICE);
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

static void RKD_Path (double *x, double *y, int npoly, int *nper, Rboolean winding, R_GE_gcontext *gc, pDevDesc dev) {
	RK_TRACE(GRAPHICS_DEVICE);
	RKGraphicsDataStreamWriteGuard guard;
	WRITE_HEADER (RKDPath, dev);
	quint32 total_points = 0;
	quint32 _n = qMin (npoly, 1 << 24);	// skip stuff exceeding reasonable limits to keep protocol simple
	RKD_OUT_STREAM << _n;
	for (quint32 i = 0; i < _n; ++i) {
		quint32 np = nper[i];	// Actually, a quint8 would probably do?
		RKD_OUT_STREAM << np;
		for (quint32 j = 0; j < np; ++j) {
			RKD_OUT_STREAM << x[total_points] << y[total_points];
			total_points++;
		}
	}
	RKD_OUT_STREAM << (bool) winding;
	WRITE_PEN ();
	WRITE_LINE_ENDS ();
	WRITE_FILL ();
}

static void RKD_Rect (double x0, double y0, double x1, double y1, R_GE_gcontext *gc, pDevDesc dev) {
	RK_TRACE(GRAPHICS_DEVICE);
	RKGraphicsDataStreamWriteGuard guard;
	WRITE_HEADER (RKDRect, dev);
	RKD_OUT_STREAM << QRectF (x0, y0, x1-x0, y1-y0);
	WRITE_PEN ();
	WRITE_LINE_ENDS ();
	WRITE_FILL ();
}

static void RKD_TextUTF8 (double x, double y, const char *str, double rot, double hadj, R_GE_gcontext *gc, pDevDesc dev) {
	RK_TRACE(GRAPHICS_DEVICE);
	RKGraphicsDataStreamWriteGuard guard;
	WRITE_HEADER (RKDTextUTF8, dev);
	RKD_OUT_STREAM << x << y << QString::fromUtf8 (str) << rot << hadj;	// NOTE: yes, even Symbols are sent as UTF-8, here.
	WRITE_COL ();
	WRITE_FONT (dev);
}

static double RKD_StrWidthUTF8 (const char *str, R_GE_gcontext *gc, pDevDesc dev) {
	RK_TRACE(GRAPHICS_DEVICE);
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
	RK_TRACE(GRAPHICS_DEVICE);
	RKGraphicsDataStreamWriteGuard guard;
	WRITE_HEADER (RKDNewPage, dev);
	WRITE_FILL ();
}

static void RKD_MetricInfo (int c, R_GE_gcontext *gc, double* ascent, double* descent, double* width, pDevDesc dev) {
	RK_TRACE(GRAPHICS_DEVICE);
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
	RK_TRACE(GRAPHICS_DEVICE);
	{
		RKGraphicsDataStreamWriteGuard guard;
		WRITE_HEADER (RKDClose, dev);
		delete static_cast<RKGraphicsDeviceDesc*> (dev->deviceSpecific);
	}
	{
		RKGraphicsDataStreamReadGuard rguard;
		qint8 dummy;
		RKD_IN_STREAM >> dummy;
	}
}

static void RKD_Clip (double left, double right, double top, double bottom, pDevDesc dev) {
	RK_TRACE(GRAPHICS_DEVICE);
	dev->clipLeft = left;
	dev->clipRight = right;
	dev->clipTop = top;
	dev->clipBottom = bottom;
	RKGraphicsDataStreamWriteGuard guard;
	WRITE_HEADER (RKDClip, dev);
	RKD_OUT_STREAM << QRectF (left, top, right - left, bottom - top);
}

static void RKD_Mode (int mode, pDevDesc dev) {
	RK_TRACE(GRAPHICS_DEVICE);
	Q_UNUSED (mode);
	Q_UNUSED (dev);
/* Left empty for now. 1 is start signal, 0 is stop signal. Might be useful for flushing, though.

	RKGraphicsDataStreamWriteGuard guard;
	RKD_OUT_STREAM << WRITE_HEADER (RKDMode, dev);
	connection << (qint8) mode; */
}

static void RKD_Raster (unsigned int *raster, int w, int h, double x, double y, double width, double height, double rot, Rboolean interpolate, const pGEcontext gc, pDevDesc dev) {
	RK_TRACE(GRAPHICS_DEVICE);
	Q_UNUSED(gc); // No idea what this is supposed to be good for. R's own cairo device ignores it.

	RKGraphicsDataStreamWriteGuard wguard;
	WRITE_HEADER (RKDRaster, dev);

	quint32 _w = qMin (w, 1 << 15);	// skip stuff exceeding reasonable limits to keep protocol simple
	RKD_OUT_STREAM << _w;
	quint32 _h = qMin (h, 1 << 15);
	RKD_OUT_STREAM << _h;
	for (quint32 col = 0; col < _h; ++col) {
		for (quint32 row = 0; row < _w; ++row) {
			WRITE_COLOR_BYTES (raster[(col*_w) + row]);
		}
	}
	RKD_OUT_STREAM << QRectF (x, y, width, height) << rot << (bool) interpolate;
}

static SEXP RKD_Capture(pDevDesc dev) {
	RK_TRACE(GRAPHICS_DEVICE);
	{
		RKGraphicsDataStreamWriteGuard wguard;
		WRITE_HEADER (RKDCapture, dev);
	}

	quint32 w, h;
	quint32 size;
	QVector<int> buffer;
	{
		RKGraphicsDataStreamReadGuard rguard;
		quint8 r, g, b, a;
		RKD_IN_STREAM >> w >> h;
		size = w*h;
		buffer.reserve(size);// Although unlikely, allocVector below could fail. We don't want to be left with a locked mutex (from the rguard) in this case. (Being left with a dead pointer looks benign in comparison; Note that the vector may easily be too large for allocation on the stack)
		for (quint32 row = 0; row < h; ++row) {
			for (quint32 col = 0; col < w; ++col) {
				RKD_IN_STREAM >> r >> g >> b >> a;
				buffer.append(R_RGBA(r, g, b, a));
			}
		}
	}
	SEXP ret, dim;
	RFn::Rf_protect(ret = RFn::Rf_allocVector(INTSXP, size));
	for (quint32 i = 0; i < size; ++i) {
		RFn::INTEGER(ret)[i] = buffer[i];
	}

	// Documentation does not mention it, but cap expects dim information to be returned
	RFn::Rf_protect(dim = RFn::Rf_allocVector(INTSXP, 2));
	RFn::INTEGER(dim)[0] = w;
	RFn::INTEGER(dim)[1] = h;
	RFn::Rf_setAttrib(ret, ROb(R_DimSymbol), dim);

	RFn::Rf_unprotect(2);
	return ret;
}

static Rboolean RKD_Locator (double *x, double *y, pDevDesc dev) {
	RK_TRACE(GRAPHICS_DEVICE);
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
	RK_TRACE(GRAPHICS_DEVICE);
	{
		RKGraphicsDataStreamWriteGuard wguard;
		WRITE_HEADER (RKDNewPageConfirm, dev);
	}
	bool ok;
	{
		RKGraphicsDataStreamReadGuard rguard;
		RKD_IN_STREAM >> ok;
	}
	if (!ok) RFn::Rf_error("Aborted by user");
	return Rboolean::TRUE;
	// Return value FALSE: Let R ask, instead
}

void RKD_EventHelper (pDevDesc dev, int code) {
	RK_TRACE(GRAPHICS_DEVICE);
	{
		RKGraphicsDataStreamWriteGuard wguard;
		if (code == 1) {
			QString prompt;
			if (RFn::Rf_isEnvironment(dev->eventEnv)) {
				SEXP sprompt = RFn::Rf_findVar(RFn::Rf_install("prompt"), dev->eventEnv);
				if (RFn::Rf_length(sprompt) == 1) prompt = QString::fromUtf8(RFn::R_CHAR(RFn::Rf_asChar(sprompt)));
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
		RFn::Rf_error ("Interrupted by user");
		return;  // not reached
	}
	if (event_code == RKDNothing) {
		if (RFn::Rf_doesIdle(dev)) RFn::Rf_doIdle(dev);
		return;
	} else if (event_code == RKDKeyPress) {
		if (modifiers - (modifiers & Qt::ShiftModifier)) {  // any other modifier than Shift, alone. NOTE: devX11.c and devWindows.c handle Ctrl, only as of R 3.0.0
			QString mod_text;
			if (modifiers & Qt::ControlModifier) mod_text.append ("ctrl-");
			if (modifiers & Qt::AltModifier) mod_text.append ("alt-");
			if (modifiers & Qt::MetaModifier) mod_text.append ("meta-");
			if (text.isEmpty () && (modifiers & Qt::ShiftModifier)) mod_text.append ("shift-");     // don't apply shift when there is text (where it has already been handled)
			text = mod_text + text.toUpper ();
		}

		R_KeyName r_key_name = R_KeyName::knUNKNOWN;
		if (keycode == Qt::Key_Left) r_key_name = R_KeyName::knLEFT;
		else if (keycode == Qt::Key_Right) r_key_name = R_KeyName::knRIGHT;
		else if (keycode == Qt::Key_Up) r_key_name = R_KeyName::knUP;
		else if (keycode == Qt::Key_Down) r_key_name = R_KeyName::knDOWN;
		else if ((keycode >= Qt::Key_F1) && (keycode <= Qt::Key_F12)) r_key_name = (R_KeyName) (R_KeyName::knF1 + (keycode - Qt::Key_F1));
		else if (keycode == Qt::Key_PageUp) r_key_name = R_KeyName::knPGUP;
		else if (keycode == Qt::Key_PageDown) r_key_name = R_KeyName::knPGDN;
		else if (keycode == Qt::Key_End) r_key_name = R_KeyName::knEND;
		else if (keycode == Qt::Key_Home) r_key_name = R_KeyName::knHOME;
		else if (keycode == Qt::Key_Insert) r_key_name = R_KeyName::knINS;
		else if (keycode == Qt::Key_Delete) r_key_name = R_KeyName::knDEL;

		RFn::Rf_doKeybd(dev, r_key_name, text.toUtf8 ().data());
	} else {    // all others are mouse events
		RFn::Rf_doMouseEvent(dev, event_code == RKDMouseDown ? R_MouseEvent::meMouseDown : (event_code == RKDMouseUp ? R_MouseEvent::meMouseUp : R_MouseEvent::meMouseMove), buttons, x, y);
	}
}

void RKD_onExit (pDevDesc dev) {
	RK_TRACE(GRAPHICS_DEVICE);
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

int RKD_HoldFlush (pDevDesc, int) {
	RK_TRACE(GRAPHICS_DEVICE);
	// deliberately left unimplemented: Drawing is in a separte thread, anyway, and only done after a timeout
	return 0;
}

#if R_VERSION >= R_Version (4, 1, 0)
qint8 getGradientExtend(int Rextent) {
	if (Rextent == R_GE_patternExtendPad) return GradientExtendPad;
	if (Rextent == R_GE_patternExtendReflect) return GradientExtendReflect;
	if (Rextent == R_GE_patternExtendRepeat) return GradientExtendRepeat;
	/* if (Rextent == R_GE_patternExtendNone) */ return GradientExtendNone;
}

SEXP makeInt(int val) {
	SEXP ret;
	RFn::Rf_protect(ret = RFn::Rf_allocVector(INTSXP, 1));
	RFn::INTEGER(ret)[0] = val;
	RFn::Rf_unprotect(1);
	return ret;
}

static void RK_tryCall(SEXP func) {
	int error;
	SEXP call = RFn::Rf_protect(RFn::Rf_lang1(func));
	RFn::R_tryEval(call, ROb(R_GlobalEnv), &error);
	RFn::Rf_unprotect(1);
}

SEXP RKD_SetPattern (SEXP pattern, pDevDesc dev) {
	RK_TRACE(GRAPHICS_DEVICE);
	auto ptype = RFn::R_GE_patternType(pattern);
	if ((ptype == R_GE_linearGradientPattern) || (ptype == R_GE_radialGradientPattern)) {
		RKGraphicsDataStreamWriteGuard wguard;
		WRITE_HEADER(RKDSetPattern, dev);
		if (ptype == R_GE_linearGradientPattern) {
			RKD_OUT_STREAM << (qint8) RKDPatternType::LinearPattern;
			RKD_OUT_STREAM << (double) RFn::R_GE_linearGradientX1(pattern) << (double) RFn::R_GE_linearGradientX2(pattern) << (double) RFn::R_GE_linearGradientY1(pattern) << (double) RFn::R_GE_linearGradientY2(pattern);
			qint16 nstops = RFn::R_GE_linearGradientNumStops(pattern);
			RKD_OUT_STREAM << nstops;
			for (int i = 0; i < nstops; ++i) {
				WRITE_COLOR_BYTES(RFn::R_GE_linearGradientColour(pattern, i));
				RKD_OUT_STREAM << (double) RFn::R_GE_linearGradientStop(pattern, i);
			}
			RKD_OUT_STREAM << getGradientExtend(RFn::R_GE_linearGradientExtend(pattern));
		} else if (ptype == R_GE_radialGradientPattern) {
			RKD_OUT_STREAM << (qint8) RKDPatternType::RadialPattern;
			RKD_OUT_STREAM << (double) RFn::R_GE_radialGradientCX1(pattern) << (double) RFn::R_GE_radialGradientCY1(pattern) << (double) RFn::R_GE_radialGradientR1(pattern);
			RKD_OUT_STREAM << (double) RFn::R_GE_radialGradientCX2(pattern) << (double) RFn::R_GE_radialGradientCY2(pattern) << (double) RFn::R_GE_radialGradientR2(pattern);
			qint16 nstops = RFn::R_GE_radialGradientNumStops(pattern);
			RKD_OUT_STREAM << nstops;
			for (int i = 0; i < nstops; ++i) {
				WRITE_COLOR_BYTES(RFn::R_GE_radialGradientColour(pattern, i));
				RKD_OUT_STREAM << (double) RFn::R_GE_radialGradientStop(pattern, i);
			}
			RKD_OUT_STREAM << getGradientExtend(RFn::R_GE_radialGradientExtend(pattern));
		}
	} else if (ptype == R_GE_tilingPattern) {
		{
			RKGraphicsDataStreamWriteGuard wguard;
			WRITE_HEADER(RKDStartRecordTilingPattern, dev);
			RKD_OUT_STREAM << (double) RFn::R_GE_tilingPatternWidth(pattern) << (double) RFn::R_GE_tilingPatternHeight(pattern);
			RKD_OUT_STREAM << (double) RFn::R_GE_tilingPatternX(pattern) << (double) RFn::R_GE_tilingPatternY(pattern);
		}
		// Play the pattern generator function. Contrary to cairo device, we use tryEval, here, to avoid getting into a
		// bad device state in case of errors
		int error;
		SEXP pattern_func = RFn::Rf_protect(RFn::Rf_lang1(RFn::R_GE_tilingPatternFunction(pattern)));
		RFn::R_tryEval(pattern_func, ROb(R_GlobalEnv), &error);
		RFn::Rf_unprotect(1);
		{
			RKGraphicsDataStreamWriteGuard wguard;
			WRITE_HEADER(RKDEndRecordTilingPattern, dev);
			RKD_OUT_STREAM << getGradientExtend(RFn::R_GE_tilingPatternExtend(pattern));
		}
	} else {
		RFn::Rf_warning("Pattern type not (yet) supported");
		return makeInt(-1);
	}

	qint32 index;
	{
		RKGraphicsDataStreamReadGuard rguard;
		RKD_IN_STREAM >> index;
	}

	// NOTE: we are free to chose a return value of our liking. It is used as an identifier for this pattern.
	if (index < 0) RFn::Rf_warning("Pattern type not (yet) supported");
	return makeInt(index);
}

void releaseCachedResource(RKDCachedResourceType type, SEXP ref, pDevDesc dev) {
	RK_TRACE(GRAPHICS_DEVICE);
	{
		RKGraphicsDataStreamWriteGuard wguard;
		WRITE_HEADER(RKDReleaseCachedResource, dev);
		RKD_OUT_STREAM << (quint8) type;
		if (RFn::Rf_isNull(ref)) {
			RKD_OUT_STREAM << (qint32) 1 << (qint32) -1; // means: destroy all objects of that type
		} else {
			qint32 len = RFn::Rf_length(ref);
			RKD_OUT_STREAM << len;
			for (int i = 0; i < len; ++i) {
				RKD_OUT_STREAM << (qint32) RFn::INTEGER(ref)[i];
			}
		}
	}
}

void RKD_ReleasePattern (SEXP ref, pDevDesc dev) {
	RK_TRACE(GRAPHICS_DEVICE);
	releaseCachedResource(RKDPattern, ref, dev);
}

SEXP RKD_SetClipPath (SEXP path, SEXP ref, pDevDesc dev) {
	RK_TRACE(GRAPHICS_DEVICE);
	qint32 index = -1;
	if (!RFn::Rf_isNull(ref)) index = RFn::INTEGER(ref)[0];
	// NOTE: just because we have a reference, doesn't mean, it's also valid, according to R sources
	if (index >= 0) {
		{
			RKGraphicsDataStreamWriteGuard wguard;
			WRITE_HEADER(RKDSetClipPath, dev);
			RKD_OUT_STREAM << index;
		}
		{
			RKGraphicsDataStreamReadGuard rguard;
			qint8 ok;
			RKD_IN_STREAM >> ok;
			if (!ok) RFn::Rf_warning("Invalid reference to clipping path");
			else return ROb(R_NilValue);
		}
	}

	// No index, or not a valid index: create new path
	{
		RKGraphicsDataStreamWriteGuard wguard;
		WRITE_HEADER(RKDStartRecordClipPath, dev);
	}
	// Play generator function
	RK_tryCall(path);
	{
		RKGraphicsDataStreamWriteGuard wguard;
		WRITE_HEADER(RKDEndRecordClipPath, dev);
#if R_VERSION >= R_Version(4, 2, 0)
		RKD_OUT_STREAM << (qint8) mapFillRule(RFn::R_GE_clipPathFillRule(path));
#else
		RKD_OUT_STREAM << (qint8) 0;  // NOTE: 0 == Qt::OddEvenFill
#endif
	}
	{
		RKGraphicsDataStreamReadGuard rguard;
		RKD_IN_STREAM >> index;
	}
	return makeInt(index);
}

void RKD_ReleaseClipPath (SEXP ref, pDevDesc dev) {
	RK_TRACE(GRAPHICS_DEVICE);
	releaseCachedResource(RKDClipPath, ref, dev);
}

SEXP RKD_SetMask (SEXP mask, SEXP ref, pDevDesc dev) {
	RK_TRACE(GRAPHICS_DEVICE);
	// Same logic as RKD_SetClipPath

	qint32 index = 0;
	if (!RFn::Rf_isNull(ref)) index = RFn::INTEGER(ref)[0];  // ref==NULL means the mask is not yet registered, will be recorded, below
	if (index > 0 || RFn::Rf_isNull(mask)) {  // mask==NULL means to unset the current mask. signalled to the frontend as index=0
		{
			RKGraphicsDataStreamWriteGuard wguard;
			WRITE_HEADER(RKDSetMask, dev);
			RKD_OUT_STREAM << index;
		}
		{
			RKGraphicsDataStreamReadGuard rguard;
			qint8 ok;
			RKD_IN_STREAM >> ok;
			if (!ok) RFn::Rf_warning("Invalid reference to mask");
			else return ROb(R_NilValue);
		}
	}

	// No index, or not a valid index: create new mask
	{
		RKGraphicsDataStreamWriteGuard wguard;
		WRITE_HEADER(RKDStartRecordMask, dev);
	}
	// Play generator function
	RK_tryCall(mask);
	{
		RKGraphicsDataStreamWriteGuard wguard;
		WRITE_HEADER(RKDEndRecordMask, dev);
#if R_VERSION >= R_Version(4,2,0)
		RKD_OUT_STREAM << (qint8) (RFn::R_GE_maskType(mask) == R_GE_luminanceMask ? 1 : 0);
#else
		RKD_OUT_STREAM << (qint8) 0;
#endif
	}
	{
		RKGraphicsDataStreamReadGuard rguard;
		RKD_IN_STREAM >> index;
	}
	return makeInt(index);
}

void RKD_ReleaseMask (SEXP ref, pDevDesc dev) {
	RK_TRACE(GRAPHICS_DEVICE);
	releaseCachedResource(RKDMask, ref, dev);
}

#endif

#if R_VERSION >= R_Version(4,2,0)
SEXP RKD_DefineGroup(SEXP source, int op, SEXP destination, pDevDesc dev) {
	RK_TRACE(GRAPHICS_DEVICE);
	{
		RKGraphicsDataStreamWriteGuard wguard;
		WRITE_HEADER(RKDDefineGroupBegin, dev);
	}

	// Play generator function for destination
	if (destination != ROb(R_NilValue)) {
		RK_tryCall(destination);
	}

	{
		RKGraphicsDataStreamWriteGuard wguard;
		WRITE_HEADER(RKDDefineGroupStep2, dev);
		RKD_OUT_STREAM << (qint8) mapCompositionModeEnum(op);
	}

	// Play generator function for source
	RK_tryCall(source);

	{
		RKGraphicsDataStreamWriteGuard wguard;
		WRITE_HEADER(RKDDefineGroupEnd, dev);
	}
	qint32 index = -1;
	{
		RKGraphicsDataStreamReadGuard rguard;
		RKD_IN_STREAM >> index;
	}
	return makeInt(index);
}

void RKD_UseGroup(SEXP ref, SEXP trans, pDevDesc dev) {
	RK_TRACE(GRAPHICS_DEVICE);

	// NOTE: chaching parameters before starting the write, in case they are ill-formed and produce errors
	qint32 index = 0;
	if (!RFn::Rf_isNull(ref)) index = RFn::INTEGER(ref)[0];
	bool have_trans = (trans != ROb(R_NilValue));
	double matrix[6];
	if (have_trans) {
		for (int i = 0; i < 6; ++i) matrix[i] = RFn::REAL(trans)[i];  // order in cairo terms: xx, xy, x0, yx, yy, y0
	}

	{
		RKGraphicsDataStreamWriteGuard wguard;
		WRITE_HEADER(RKDUseGroup, dev);
		RKD_OUT_STREAM << index;
		if (have_trans) {
			RKD_OUT_STREAM << (qint8) 1;
			for (int i = 0; i < 6; ++i) RKD_OUT_STREAM << matrix[i];
		} else {
			RKD_OUT_STREAM << (qint8) 0;
		}
	}
}

void RKD_ReleaseGroup(SEXP ref, pDevDesc dev) {
	RK_TRACE(GRAPHICS_DEVICE);
	releaseCachedResource(RKDGroup, ref, dev);
}

void doFillAndOrStroke(SEXP path, const pGEcontext gc, pDevDesc dev, bool fill, int rule, bool stroke) {
	RK_TRACE(GRAPHICS_DEVICE);
	{
		RKGraphicsDataStreamWriteGuard wguard;
		WRITE_HEADER(RKDFillStrokePathBegin, dev);
	}

	// record the actual path
	RK_tryCall(path);

	{
		RKGraphicsDataStreamWriteGuard wguard;
		WRITE_HEADER(RKDFillStrokePathEnd, dev);
		RKD_OUT_STREAM << (quint8) fill;
		if (fill) {
			RKD_OUT_STREAM << (qint8) mapFillRule(rule);
			WRITE_FILL();
		}
		RKD_OUT_STREAM << (quint8) stroke;
		if (stroke) {
			WRITE_PEN();
		}
	}
}

void RKD_Stroke(SEXP path, const pGEcontext gc, pDevDesc dev) {
	doFillAndOrStroke(path, gc, dev, false, 0, true);
}

void RKD_Fill(SEXP path, int rule, const pGEcontext gc, pDevDesc dev) {
	doFillAndOrStroke(path, gc, dev, true, rule, false);
}

void RKD_FillStroke(SEXP path, int rule, const pGEcontext gc, pDevDesc dev) {
	doFillAndOrStroke(path, gc, dev, true, rule, true);
}
#endif
