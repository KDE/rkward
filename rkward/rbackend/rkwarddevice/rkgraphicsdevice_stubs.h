// NOTE: heavy copying from QGraphicsSceneDevice
#include <QRect>
#include <R_ext/GraphicsEngine.h>
#include <limits.h>

#define WRITE_HEADER(x,dev) (qint8) x << (quint8) static_cast<RKGraphicsDeviceDesc*> (dev->deviceSpecific)->devnum
#define WRITE_COL() (qint32) gc->col
#define WRITE_PEN() WRITE_COL() << (double) gc->lwd << (qint32) << gc->lty
#define WRITE_LINE_ENDS() (quint8) gc->lend << (quint8) gc->ljoin << gc->lmitre
#define WRITE_FILL() (qint32) gc->fill
#define WRITE_FONT(dev) gc->cex << gc->ps << gc->lineheight << (quint8) gc->fontface << gc->fontfamily[0] ? QString (gc->fontfamily) : (static_cast<RKGraphicsDeviceDesc*> (dev->deviceSpecific)->default_family)

QByteArray aux_buffer;
QDataStream aux_stream;
QByteArray buffer;
QDataStream connection (&buffer);
QMutex rkwarddeviceprotocolmutex;

class RKSocketDataStreamWriteGuard {
	RKSocketDataStreamWriteGuard () {
		rkwarddeviceprotocolmutex.lock ();
	}
	~RKSocketDataStreamWriteGuard () {
		aux_stream << (quint32) buffer.size ();
		device.write (aux_stream);
		aux_stream.resize (0);
		device.write (buffer);
		buffer.resize (0);
		rkwarddeviceprotocolmutex.unlock ();
	}
};

enum {
	// Asynchronous operations
	RKDCircle,
	RKDLine,
	RKDPolygon,
	RKDPolyline,
	RKDRect,
	RKDTextUTF8,
	RKDNewPage,
	RKDClose,
	RKDActivate,
	RKDDeActivate,
	RKDClip,
	RKDMode,

	// Synchronous operations
	RKD_First_Synchronous_Request,
	RKDStrWidthUTF8,
	RKDMetricInfo,
	RKDLocator,
	RKDNewPageConfirm
} OpCodes;

static void RKD_Circle (double x, double y, double r, R_GE_gcontext *gc, pDevDesc dev) {
	RKSocketDataStreamWriteGuard guard;
	connection << WRITE_HEADER (RKDCircle, dev);
	connection << x << y << r;
	connection << WRITE_PEN ();
}

static void RKD_Line (double x1, double y1, double x2, double y2, R_GE_gcontext *gc, pDevDesc dev) {
	RKSocketDataStreamWriteGuard guard;
	connection << WRITE_HEADER (RKDLine, dev);
	connection << x1 << y1 << x2 << y2;
	connection << WRITE_PEN ();
}

static void RKD_Polygon (int n, double *x, double *y, R_GE_gcontext *gc, pDevDesc dev) {
	RKSocketDataStreamWriteGuard guard;
	connection << WRITE_HEADER (RKDPolygon, dev);
	quint32 _n = qMax (n, std::numeric_limits<quint32>::max());
	connection << _n;
	for (quint32 i; i < n; ++i) {
		connection << x[i] << y[i];
	}
	connection << WRITE_PEN ();
	connection << WRITE_LINE_ENDS ();
	connection << WRITE_FILL ();
}

static void RKD_Polyline (int n, double *x, double *y, R_GE_gcontext *gc, pDevDesc dev) {
	RKSocketDataStreamWriteGuard guard;
	connection << WRITE_HEADER (RKDPolyline, dev);
	quint32 _n = qMax (n, std::numeric_limits<quint32>::max());
	connection << _n;
	for (quint32 i; i < n; ++i) {
		connection << x[i] << y[i];
	}
	connection << WRITE_PEN ();
	connection << WRITE_LINE_ENDS ();
}

static void RKD_Rect (double x0, double y0, double x1, double y1, R_GE_gcontext *gc, pDevDesc dev) {
	RKSocketDataStreamWriteGuard guard;
	connection << WRITE_HEADER (RKDPolyline, dev);
	connection << QRectF (x0, y0, x1-x0, y1-y0);
	connection << WRITE_PEN ();
	connection << WRITE_LINE_ENDS ();
	connection << WRITE_FILL ();
}

static void RKD_TextUTF8 (double x, double y, char *str, double rot, double hadj, R_GE_gcontext *gc, pDevDesc dev) {
	RKSocketDataStreamWriteGuard guard;
	connection << WRITE_HEADER (RKDTextUTF8, dev);
	connection << x << y << QString::fromUtf8 (str) << rot << hadj;
	connection << WRITE_COL ();
	connection << WRITE_FONT (dev);
}

static double RKD_StrWidthUTF8 (char *str, R_GE_gcontext *gc, pDevDesc dev) {
	{
		RKSocketDataStreamWriteGuard guard;
		connection << WRITE_HEADER (RKDStrWidthUTF8, dev);
		connection << QString::fromUtf8 (str);
		connection << WRITE_FONT (dev);
	}
	double ret;
	{
		RKSocketDataStreamReadGuard guard;
		ret << connection;
	}
	return ret;
}

static void RKD_NewPage (R_GE_gcontext *gc, pDevDesc dev) {
	RKSocketDataStreamWriteGuard guard;
	connection << WRITE_HEADER (RKDNewPage, dev);
	connection << WRITE_FILL ();
}

static void RKD_MetricInfo (int c, R_GE_gcontext *gc, double* ascent, double* descent, double* width, pDevDesc dev) {
	{
		RKSocketDataStreamWriteGuard guard;
		connection << WRITE_HEADER (RKDMetricInfo, dev);
		connection << QChar (c);
		connection << WRITE_FONT (dev);
	}
	{
		RKSocketDataStreamReadGuard guard;
		*ascent << connection;
		*descent << connection;
		*width << connection;
	}
}

static void RKD_Close (pDevDesc dev) {
	RKSocketDataStreamWriteGuard guard;
	connection << WRITE_HEADER (RKDClose, dev);
}

static void RKD_Activate(pDevDesc dev) {
	RKSocketDataStreamWriteGuard guard;
	connection << WRITE_HEADER (RKDActivate, dev);
}

static void RKD_Deactivate(pDevDesc dev) {
	RKSocketDataStreamWriteGuard guard;
	connection << WRITE_HEADER (RKDDeActivate, dev);
}

static void RKD_Clip(double left, double right, double top, double bottom, pDevDesc dev) {
	dev->clipLeft = left;
	dev->clipRight = right;
	dev->clipTop = top;
	dev->clipBottom = bottom;
	RKSocketDataStreamWriteGuard guard;
	connection << WRITE_HEADER (RKDClip, dev);
	connection << QRectF (left, top, right - left, bottom - top);
}

static void RKD_Mode (int mode, pDevDesc dev) {
/* Left empty for now. 1 is start signal, 0 is stop signal. Might be useful for flushing, though.

	RKSocketDataStreamWriteGuard guard;
	connection << WRITE_HEADER (RKDMode, dev);
	connectoin << (qint8) mode; */
}

static Rboolean RKD_Locator (double *x, double *y, pDevDesc dev) {
	{
		RKSocketDataStreamWriteGuard guard;
		connection << WRITE_HEADER (RKDLocator, dev);
	}
#warning TODO: handle cancellation from backend
	{
		RKSocketDataStreamReadGuard guard;
		bool ok;
		ok << connection;
		*x << connection;
		*y << connection;
		if (ok) return TRUE;
		return FALSE;
	}
}

static Rboolean RKD_NewFrameConfirm (pDevDesc dev) {
	{
		RKSocketDataStreamWriteGuard guard;
		connection << WRITE_HEADER (RKDNewPageConfirm, dev);
	}
#warning TODO: handle cancellation from backend
	{
		RKSocketDataStreamReadGuard guard;
		bool ok << connection;
		if (ok) return TRUE;
		return FALSE;
	}
}
