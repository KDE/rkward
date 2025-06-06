/*
rkgraphicsdevice_frontendtransmitter - This file is part of RKWard (https://rkward.kde.org). Created: Mon Mar 18 2013
SPDX-FileCopyrightText: 2013-2024 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "rkgraphicsdevice_frontendtransmitter.h"

#include <KLocalizedString>
#include <QIODevice>
#include <QLocalServer>
#include <QLocalSocket>
#include <kmessagebox.h>
#include <krandom.h>

// for screen resolution
#include <QGuiApplication>
#include <QScreen>

#include "../../version.h"
#include "../../windows/rkworkplace.h"
#include "../rkfrontendtransmitter.h"
#include "rkgraphicsdevice.h"

#include "../../debug.h"

double RKGraphicsDeviceFrontendTransmitter::lwdscale = 72.0 / 96; // NOTE: reinitialized more appropriately, later
RKGraphicsDeviceFrontendTransmitter::RKGraphicsDeviceFrontendTransmitter() : QObject(), dpix(0), dpiy(0) {
	RK_TRACE(GRAPHICS_DEVICE);

	connection = nullptr;
	local_server = nullptr;

	setupServer();
}

RKGraphicsDeviceFrontendTransmitter::~RKGraphicsDeviceFrontendTransmitter() {
	RK_TRACE(GRAPHICS_DEVICE);

	if (connection) connection->close();
	if (local_server->isListening()) local_server->close();
}

void RKGraphicsDeviceFrontendTransmitter::setupServer() {
	RK_TRACE(GRAPHICS_DEVICE);

	RK_ASSERT(!local_server);
	local_server = new QLocalServer(this);
	RK_ASSERT(local_server->listen(u"rkd"_s + KRandom::randomString(8)));
	connect(local_server, &QLocalServer::newConnection, this, &RKGraphicsDeviceFrontendTransmitter::newConnection);
	server_name = local_server->fullServerName();
}

void RKGraphicsDeviceFrontendTransmitter::newConnection() {
	RK_TRACE(GRAPHICS_DEVICE);

	RK_ASSERT(!connection);
	QLocalSocket *con = local_server->nextPendingConnection();
	local_server->close();

	// handshake
	QString token = RKFrontendTransmitter::instance()->connectionToken();
	QString token_c = RKFrontendTransmitter::waitReadLine(con, 2000).trimmed();
	if (token_c != token) {
		KMessageBox::detailedError(nullptr, QStringLiteral("<p>%1</p>").arg(i18n("There has been an error while trying to connect the on-screen graphics backend. This means, on-screen graphics using the RKWard device will not work in this session.")), i18n("Expected connection token %1, but read connection token %2", token, token_c), i18n("Error while connection graphics backend"));
		con->close();
		return;
	}

	connection = con;
	streamer.setIODevice(con);
	connect(connection, &QIODevice::readyRead, this, &RKGraphicsDeviceFrontendTransmitter::newData);
	newData(); // might already be available
}

static QRgb readRgb(QDataStream &instream) {
	quint8 r, g, b, a;
	instream >> r >> g >> b >> a;
	return qRgba(r, g, b, a);
}

static QColor readColor(QDataStream &instream) {
	quint8 r, g, b, a;
	instream >> r >> g >> b >> a;
	return QColor(r, g, b, a);
}

static QPen readSimplePen(QDataStream &instream) {
	QColor col = readColor(instream);
	double lwd;
	qint32 lty;
	instream >> lwd >> lty;
	if ((col.alpha() == 0) || (lty == -1L)) return QPen(Qt::NoPen);

	lwd = qMax(double(qreal(1.0)), lwd); // minimum 1 px as in X11 device
	QPen ret;
	if (lty != 0) { // solid
		QVector<qreal> dashes;
		quint32 nlty = lty;
		for (int i = 0; i < 8; ++i) {
			if (!nlty) break;
			quint8 j = nlty & 0xF;
			if (j < 1) j = 1;
			dashes.append((int)(j * lwd * RKGraphicsDeviceFrontendTransmitter::lwdscale + .5));
			nlty >>= 4;
		}
		if (!dashes.isEmpty()) ret.setDashPattern(dashes);
	}
	ret.setWidth((int)(lwd * RKGraphicsDeviceFrontendTransmitter::lwdscale + .5));
	ret.setColor(col);
	return ret;
}

static QPen readPen(QDataStream &instream) {
	QPen ret = readSimplePen(instream);
	quint8 lends, ljoin;
	double lmitre;
	instream >> lends >> ljoin >> lmitre;
	ret.setCapStyle((Qt::PenCapStyle)mapLineEndStyle(lends));
	ret.setJoinStyle((Qt::PenJoinStyle)mapLineJoinStyle(ljoin));
	ret.setMiterLimit(lmitre);
	return ret;
}

static QBrush readBrush(QDataStream &instream, RKGraphicsDevice *dev) {
	qint8 filltype;
	instream >> filltype;
	if (filltype == ColorFill) {
		QColor col = readColor(instream);
		return QBrush(col);
	} else {
		qint16 pattern_num;
		instream >> pattern_num;
		return dev->getPattern(pattern_num);
	}
}

static void readGradientStopsAndExtent(QDataStream &instream, QGradient *g, bool reverse) {
	QGradientStops stops;
	qint16 nstops;
	instream >> nstops;
	stops.reserve(nstops);
	for (int i = 0; i < nstops; ++i) {
		double pos;
		QColor col = readColor(instream);
		instream >> pos;
		if (reverse) stops.prepend(QGradientStop(1.0 - pos, col)); // lousy efficiency should be tolerable at this point
		else stops.append(QGradientStop(pos, col));
	}
	qint8 extend;
	instream >> extend;
	if (extend == GradientExtendPad) g->setSpread(QGradient::PadSpread);
	else if (extend == GradientExtendReflect) g->setSpread(QGradient::ReflectSpread);
	else if (extend == GradientExtendRepeat) g->setSpread(QGradient::RepeatSpread);
	else {
		// Qt does not provide extend "none", so emulate by adding transparent before the first and after the last stop
		stops.prepend(QGradientStop(0.0, Qt::transparent));
		stops.append(QGradientStop(1.0, Qt::transparent));
	}
	g->setStops(stops);
}

static int readNewPattern(QDataStream &instream, RKGraphicsDevice *device) {
	qint8 patterntype;
	instream >> patterntype;

	if (patterntype == LinearPattern) {
		double x1, x2, y1, y2;
		instream >> x1 >> x2 >> y1 >> y2;
		QLinearGradient g(x1, y1, x2, y2);
		readGradientStopsAndExtent(instream, &g, false);
		return device->registerPattern(QBrush(g));
	} else if (patterntype == RadialPattern) {
		double cx1, cy1, r1, cx2, cy2, r2;
		instream >> cx1 >> cy1 >> r1 >> cx2 >> cy2 >> r2;
		QRadialGradient g;
		// Apparently, Qt needs the focal radius to be smaller than the radius. Reverse, if needed.
		if (r2 > r1) {
			g = QRadialGradient(cx2, cy2, r2, cx1, cy1, r1);
			readGradientStopsAndExtent(instream, &g, false);
		} else {
			g = QRadialGradient(cx1, cy1, r1, cx2, cy2, r2);
			readGradientStopsAndExtent(instream, &g, true);
		}
		return device->registerPattern(QBrush(g));
	} else {
		return -1;
	}
}

static QFont readFont(QDataStream &instream) {
	double cex, ps, lineheight;
	quint8 fontface;
	QString fontfamily;
	instream >> cex >> ps >> lineheight >> fontface >> fontfamily;
#ifdef __GNUC__
#	warning TODO deal with line-height
#endif
	QFont ret;
	if (!(fontfamily.isEmpty() || fontfamily == QLatin1String("Symbol"))) ret.setFamily(fontfamily); // NOTE: QPainter won't paint with "Symbol", somehow
	if (fontface == 2 || fontface == 4) ret.setWeight(QFont::Bold);
	if (fontface == 3 || fontface == 4) ret.setItalic(true);
	ret.setPointSizeF(cex * ps);
	return ret;
}

static QPointF readPoint(QDataStream &instream) {
	double x, y;
	instream >> x >> y;
	return (QPointF(x, y));
}

static QVector<QPointF> readPoints(QDataStream &instream) {
	quint32 n;
	instream >> n;
	QVector<QPointF> points;
	points.reserve(n);
	for (quint32 i = 0; i < n; ++i) {
		points.append(readPoint(instream));
	}
	return points;
}

void RKGraphicsDeviceFrontendTransmitter::newData() {
	RK_TRACE(GRAPHICS_DEVICE);

	while (connection->bytesAvailable()) {
		if (!streamer.readInBuffer()) return; // wait for more data to come in

		quint8 opcode, devnum;
		streamer.instream >> opcode >> devnum;
		RK_DEBUG(GRAPHICS_DEVICE, DL_TRACE, "Received transmission of type %d, devnum %d, size %d", opcode, devnum + 1, streamer.inSize());

		RKGraphicsDevice *device = nullptr;
		if (devnum && opcode == RKDCreate) {
			double width, height;
			QString title;
			bool antialias;
			quint32 id;
			streamer.instream >> width >> height >> title >> antialias >> id;
			device = RKGraphicsDevice::newDevice(devnum, width, height, title, antialias, id);
			RKWorkplace::mainWorkplace()->newRKWardGraphisWindow(device, devnum + 1);
			connect(device, &RKGraphicsDevice::locatorDone, this, &RKGraphicsDeviceFrontendTransmitter::locatorDone);
			connect(device, &RKGraphicsDevice::newPageConfirmDone, this, &RKGraphicsDeviceFrontendTransmitter::newPageConfirmDone);
			connect(this, &RKGraphicsDeviceFrontendTransmitter::stopInteraction, device, &RKGraphicsDevice::stopInteraction);
			streamer.outstream << (qint32)1; // dummy reply
			streamer.writeOutBuffer();
			continue;
		} else {
			if (devnum) device = RKGraphicsDevice::devices.value(devnum);
			if (!device) {
				if (opcode == RKDCancel) {
					RK_DEBUG(GRAPHICS_DEVICE, DL_WARNING, "Graphics operation canceled");
					Q_EMIT stopInteraction();
				} else if (opcode == RKDQueryResolution) {
					if (dpix < 1) {
						auto screen = QGuiApplication::primaryScreen();
						dpix = screen->logicalDotsPerInchX();
						dpiy = screen->logicalDotsPerInchY();
					}
					streamer.outstream << (qreal)dpix << (qreal)dpiy;
					RK_DEBUG(GRAPHICS_DEVICE, DL_INFO, "DPI for device %d: %d by %d", devnum + 1, (int)dpix, (int)dpiy);
					streamer.writeOutBuffer();
					// Actually, this is only needed once, but where to put it...
					// The 96 is taken from devX11.c, where it is hardcoded (historical reasons?)
					RKGraphicsDeviceFrontendTransmitter::lwdscale = dpix / 96;
				} else {
					if (devnum) RK_DEBUG(GRAPHICS_DEVICE, DL_ERROR, "Received transmission of type %d for unknown device number %d. Skipping.", opcode, devnum + 1);
					sendDummyReply(opcode);
				}
				continue;
			}
		}

		/* WARNING: No fixed evaluation order of function arguments. Do not use several readXYZ() calls in the same function call! Use dummy variables, instead. */
		if (opcode == RKDCircle) {
			double x, y, r;
			streamer.instream >> x >> y >> r;
			QPen pen = readSimplePen(streamer.instream);
			device->circle(x, y, r, pen, readBrush(streamer.instream, device));
		} else if (opcode == RKDLine) {
			double x1, y1, x2, y2;
			streamer.instream >> x1 >> y1 >> x2 >> y2;
			device->line(x1, y1, x2, y2, readPen(streamer.instream));
		} else if (opcode == RKDPolygon) {
			QPolygonF pol(readPoints(streamer.instream));
			QPen pen = readPen(streamer.instream);
			device->polygon(pol, pen, readBrush(streamer.instream, device));
		} else if (opcode == RKDPolyline) {
			QPolygonF pol(readPoints(streamer.instream));
			device->polyline(pol, readPen(streamer.instream));
		} else if (opcode == RKDPath) {
			quint32 npol;
			streamer.instream >> npol;
			QVector<QPolygonF> polygons;
			polygons.reserve(npol);
			for (quint32 i = 0; i < npol; ++i) {
				polygons.append(readPoints(streamer.instream));
			}
			bool winding;
			streamer.instream >> winding;
			QPen pen = readPen(streamer.instream);
			device->polypath(polygons, winding, pen, readBrush(streamer.instream, device));
		} else if (opcode == RKDRect) {
			QRectF rect;
			streamer.instream >> rect;
			QPen pen = readPen(streamer.instream);
			device->rect(rect, pen, readBrush(streamer.instream, device));
		} else if (opcode == RKDStrWidthUTF8) {
			QString out;
			streamer.instream >> out;
			double w = device->strSize(out, readFont(streamer.instream)).width();
			streamer.outstream << w;
			streamer.writeOutBuffer();
		} else if (opcode == RKDMetricInfo) {
			QChar c;
			double ascent, descent, width;
			streamer.instream >> c;
			device->metricInfo(c, readFont(streamer.instream), &ascent, &descent, &width);
			streamer.outstream << ascent << descent << width;
			streamer.writeOutBuffer();
		} else if (opcode == RKDTextUTF8) {
			double x, y, rot, hadj;
			QString out;
			streamer.instream >> x >> y >> out >> rot >> hadj;
			QColor col = readColor(streamer.instream);
			device->text(x, y, out, rot, hadj, col, readFont(streamer.instream));
		} else if (opcode == RKDNewPage) {
			device->clear(readBrush(streamer.instream, device));
		} else if (opcode == RKDClose) {
			RKGraphicsDevice::closeDevice(devnum);
			sendDummyReply(opcode);
		} else if (opcode == RKDActivate) {
			device->setActive(true);
		} else if (opcode == RKDDeActivate) {
			device->setActive(false);
		} else if (opcode == RKDClip) {
			QRectF clip;
			streamer.instream >> clip;
			device->setClip(clip);
		} else if (opcode == RKDMode) {
			quint8 m;
			streamer.instream >> m;
			if (m == 0) device->triggerUpdate();
		} else if (opcode == RKDRaster) {
			quint32 w, h;
			streamer.instream >> w >> h;
			QImage image(w, h, QImage::Format_ARGB32);
			for (quint32 col = 0; col < h; ++col) {
				for (quint32 row = 0; row < w; ++row) {
					image.setPixel(row, col, readRgb(streamer.instream));
				}
			}
			QRectF target;
			double rotation;
			bool interpolate;
			streamer.instream >> target >> rotation >> interpolate;
			device->image(image, target.normalized(), rotation, interpolate);
		} else if (opcode == RKDSetPattern) {
			streamer.outstream << (qint32)readNewPattern(streamer.instream, device);
			streamer.writeOutBuffer();
		} else if (opcode == RKDStartRecordTilingPattern) {
			double width, height, x, y;
			streamer.instream >> width >> height;
			streamer.instream >> x >> y;
			device->startRecordTilingPattern(width, height, x, y);
		} else if (opcode == RKDEndRecordTilingPattern) {
			qint8 extend;
			streamer.instream >> extend;
			streamer.outstream << (qint32)device->finalizeTilingPattern((RKDGradientExtend)extend);
			streamer.writeOutBuffer();
		} else if (opcode == RKDSetClipPath) {
			qint32 index;
			streamer.instream >> index;
			qint8 ok = device->setClipToCachedPath(index) ? 1 : 0;
			streamer.outstream << ok;
			streamer.writeOutBuffer();
		} else if (opcode == RKDReleaseCachedResource) {
			quint8 type;
			qint32 len;
			streamer.instream >> type >> len;
			for (int i = 0; i < len; ++i) {
				qint32 index;
				streamer.instream >> index;
				if (type == RKDPattern) device->destroyPattern(index);
				else if (type == RKDClipPath) device->destroyCachedPath(index);
				else if (type == RKDMask) device->destroyMask(index);
				else if (type == RKDGroup) device->destroyGroup(index);
				else RK_ASSERT(false);
			}
		} else if (opcode == RKDStartRecordClipPath) {
			device->startRecordPath();
		} else if (opcode == RKDEndRecordClipPath) {
			qint8 fillrule;
			streamer.instream >> fillrule;
			QPainterPath p = device->endRecordPath(mapFillRule(fillrule));
			qint32 index = device->cachePath(p);
			device->setClipToCachedPath(index);
			streamer.outstream << (qint32)index;
			streamer.writeOutBuffer();
		} else if (opcode == RKDSetMask) {
			qint32 index;
			streamer.instream >> index;
			qint8 ok = device->setMask(index) ? 1 : 0;
			streamer.outstream << ok;
			streamer.writeOutBuffer();
		} else if (opcode == RKDStartRecordMask) {
			device->startRecordMask();
		} else if (opcode == RKDEndRecordMask) {
			qint8 luminance_mask;
			streamer.instream >> luminance_mask;
			QImage m = device->endRecordMask((bool)luminance_mask);
			qint32 index = device->registerMask(m);
			device->setMask(index);
			streamer.outstream << (qint32)index;
			streamer.writeOutBuffer();
		} else if (opcode == RKDFillStrokePathBegin) {
			device->startRecordPath();
		} else if (opcode == RKDFillStrokePathEnd) {
			quint8 fill, stroke;
			qint8 fillrule = Qt::OddEvenFill;
			QBrush brush;
			QPen pen(Qt::NoPen);
			streamer.instream >> fill;
			if (fill) {
				streamer.instream >> fillrule;
				fillrule = mapFillRule(fillrule);
				brush = readBrush(streamer.instream, device);
			}
			streamer.instream >> stroke;
			if (stroke) {
				pen = readPen(streamer.instream);
			}
			QPainterPath p = device->endRecordPath(fillrule);
			device->fillStrokePath(p, brush, pen);
		} else if (opcode == RKDDefineGroupBegin) {
			device->startRecordGroup();
		} else if (opcode == RKDDefineGroupStep2) {
			qint8 compositing_operator;
			streamer.instream >> compositing_operator;
			device->recordGroupStage2(compositing_operator);
		} else if (opcode == RKDDefineGroupEnd) {
			qint32 index = device->endRecordGroup();
			streamer.outstream << index;
			streamer.writeOutBuffer();
		} else if (opcode == RKDUseGroup) {
			qint32 index;
			qint8 have_trans;
			streamer.instream >> index;
			streamer.instream >> have_trans;
			QTransform matrix;
			if (have_trans) {
				double m[6];
				for (int i = 0; i < 6; ++i)
					streamer.instream >> m[i];
				// order in cairo terms: xx, xy, x0, yx, yy, y0
				//                       11, 21, 31, 12, 22, 32
				matrix = QTransform(m[0], m[3], m[1], m[4], m[2], m[5]);
			}
			device->useGroup(index, matrix);
		} else if (opcode == RKDGlyph) {
			QString font, family;
			quint8 index, style;
			quint32 weight;
			double rot, size;
			QColor col = readColor(streamer.instream);
			streamer.instream >> font >> index >> family >> weight >> style >> size >> rot;
			quint32 n;
			streamer.instream >> n;
			QVector<QPointF> points;
			QVector<quint32> glyphs;
			points.reserve(n);
			glyphs.reserve(n);
			for (decltype(n) i = 0; i < n; ++i) {
				qint32 glyphi;
				points.append(readPoint(streamer.instream));
				streamer.instream >> glyphi;
				glyphs.append(glyphi);
			}
			// NOTE: contrary to other font sizes, size here is given in device independent "bigpts" (1 inch / 72), which need to be scaled to resolution
			size = size * (dpix / 72.0);
			QString ret = device->glyph(font, index, family, weight, static_cast<QFont::Style>(style), size, col, rot, points, glyphs);
			streamer.outstream << ret;
			streamer.writeOutBuffer();
		} else if (opcode == RKDCapture) {
			QImage image = device->capture();
			quint32 w = image.width();
			quint32 h = image.height();
			streamer.outstream << w << h;
			for (quint32 col = 0; col < h; ++col) {
				for (quint32 row = 0; row < w; ++row) {
					QRgb pixel = image.pixel(row, col);
					streamer.outstream << (quint8)qRed(pixel) << (quint8)qGreen(pixel) << (quint8)qBlue(pixel) << (quint8)qAlpha(pixel);
				}
			}
			streamer.writeOutBuffer();
		} else if (opcode == RKDGetSize) {
			streamer.outstream << device->currentSize();
			streamer.writeOutBuffer();
		} else if (opcode == RKDSetSize) {
			QSize size;
			streamer.instream >> size;
			device->setAreaSize(size);
		} else if (opcode == RKDLocator) {
			device->locator();
		} else if (opcode == RKDStartGettingEvents) {
			QString prompt;
			streamer.instream >> prompt;
			device->startGettingEvents(prompt);
		} else if (opcode == RKDStopGettingEvents) {
			device->stopGettingEvents();
		} else if (opcode == RKDFetchNextEvent) {
			RKGraphicsDevice::StoredEvent ev = device->fetchNextEvent();
			streamer.outstream << (qint8)ev.event_code;
			if (ev.event_code == RKDKeyPress) {
				streamer.outstream << ev.keytext << (qint32)ev.keycode << (qint32)ev.modifiers;
			} else if ((ev.event_code == RKDMouseDown) || (ev.event_code == RKDMouseUp) || (ev.event_code == RKDMouseMove)) {
				streamer.outstream << (qint8)ev.buttons << (double)ev.x << (double)ev.y;
			} else {
				RK_ASSERT((ev.event_code == RKDNothing) || (ev.event_code == RKDFrontendCancel));
			}
			streamer.writeOutBuffer();
		} else if (opcode == RKDNewPageConfirm) {
			device->confirmNewPage();
		} else {
			RK_DEBUG(GRAPHICS_DEVICE, DL_ERROR, "Unhandled operation of type %d for device number %d. Skipping.", opcode, devnum + 1);
		}

		if (!streamer.instream.atEnd()) {
			RK_DEBUG(GRAPHICS_DEVICE, DL_ERROR, "Failed to read all data for operation of type %d on device number %d.", opcode, devnum + 1);
		}
	}
}

void RKGraphicsDeviceFrontendTransmitter::sendDummyReply(quint8 opcode) {
	RK_TRACE(GRAPHICS_DEVICE);

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
	} else if (opcode == RKDCapture) {
		streamer.outstream << (quint32)0 << (quint32)0;
		//	} else if (opcode == RKDQueryResolution) {
		//		streamer.outstream << (qint32) 0;
	} else if (opcode == RKDGetSize) {
		streamer.outstream << QSizeF();
	} else if (opcode == RKDSetPattern || opcode == RKDEndRecordTilingPattern || opcode == RKDEndRecordClipPath) {
		streamer.outstream << (qint32)-1;
	} else if (opcode == RKDSetClipPath) {
		streamer.outstream << (qint32)0;
	} else if (opcode == RKDFetchNextEvent) {
		streamer.outstream << (qint8)RKDNothing;
	} else if (opcode == RKDClose) {
		streamer.outstream << (qint8)RKDNothing;
	} else {
		return; // nothing to write
	}

	streamer.writeOutBuffer();
}

void RKGraphicsDeviceFrontendTransmitter::locatorDone(bool ok, double x, double y) {
	RK_TRACE(GRAPHICS_DEVICE);

	streamer.outstream << ok << x << y;
	streamer.writeOutBuffer();
}

void RKGraphicsDeviceFrontendTransmitter::newPageConfirmDone(bool accepted) {
	RK_TRACE(GRAPHICS_DEVICE);

	streamer.outstream << accepted;
	streamer.writeOutBuffer();
}
