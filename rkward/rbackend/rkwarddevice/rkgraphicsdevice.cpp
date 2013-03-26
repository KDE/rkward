/***************************************************************************
                          rkgraphicsdevice_backendtransmitter  -  description
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

#include "rkgraphicsdevice.h"

#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsRectItem>
#include <qmath.h>

#include "../../debug.h"

#define UPDATE_INTERVAL 100

QHash<int, RKGraphicsDevice*> RKGraphicsDevice::devices;

RKGraphicsDevice::RKGraphicsDevice (double width, double height) : QObject (), area (qAbs (width) + 1, qAbs (height) + 1) {
	RK_TRACE (GRAPHICS_DEVICE);
	painter.setRenderHints (QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);
	view = new QLabel ();
	view->setFixedSize (area.size ());
	connect (&updatetimer, SIGNAL (timeout ()), this, SLOT (updateNow ()));
	updatetimer.setSingleShot (true);
	clear ();
}

RKGraphicsDevice::~RKGraphicsDevice () {
	RK_TRACE (GRAPHICS_DEVICE);
	painter.end ();
	delete view;
}

void RKGraphicsDevice::triggerUpdate () {
	updatetimer.start (UPDATE_INTERVAL);
}

void RKGraphicsDevice::updateNow () {
	if (painter.isActive ()) painter.end ();
	view->setPixmap (area);
	view->show ();
	painter.begin (&area);
}

RKGraphicsDevice* RKGraphicsDevice::newDevice (int devnum, double width, double height) {
	RK_TRACE (GRAPHICS_DEVICE);

	if (devices.contains (devnum)) {
		RK_DEBUG (GRAPHICS_DEVICE, DL_ERROR, "Graphics device number %d already exists while trying to create it", devnum);
		closeDevice (devnum);
	}
	RKGraphicsDevice* dev = new RKGraphicsDevice (width, height);
	devices.insert (devnum, dev);
	return (dev);
}

void RKGraphicsDevice::closeDevice (int devnum) {
	RK_TRACE (GRAPHICS_DEVICE);

	RK_ASSERT (devices.contains (devnum));
	devices.take (devnum)->deleteLater ();
}

void RKGraphicsDevice::clear (const QColor& col) {
	RK_TRACE (GRAPHICS_DEVICE);

	if (painter.isActive ()) painter.end ();
 	if (col.isValid ()) area.fill (col);
	else area.fill (QColor (255, 255, 255, 255));
	updateNow ();
}

void RKGraphicsDevice::setClip (const QRectF& new_clip) {
	RK_TRACE (GRAPHICS_DEVICE);

	if (!painter.isActive ()) painter.begin (&area);
	painter.setClipRect (new_clip);
}

void RKGraphicsDevice::circle (double x, double y, double r, const QPen& pen, const QBrush& brush) {
	RK_TRACE (GRAPHICS_DEVICE);

	painter.setPen (pen);
	painter.setBrush (brush);
	painter.drawEllipse (x - r, y - r, r+r, r+r);
	triggerUpdate ();
}

void RKGraphicsDevice::line (double x1, double y1, double x2, double y2, const QPen& pen) {
	RK_TRACE (GRAPHICS_DEVICE);

	painter.setPen (pen);
	painter.drawLine (x1, y1, x2, y2);
	triggerUpdate ();
}

void RKGraphicsDevice::rect (const QRectF& rec, const QPen& pen, const QBrush& brush) {
	RK_TRACE (GRAPHICS_DEVICE);

	painter.setPen (pen);
	painter.setBrush (brush);
	painter.drawRect (rec);
	triggerUpdate ();
}

QSizeF RKGraphicsDevice::strSize (const QString& text, const QFont& font) {
	RK_TRACE (GRAPHICS_DEVICE);

	painter.setFont (font);
	QSizeF size = painter.boundingRect (QRectF (area.rect ()), text).size ();
	return size;
}

void RKGraphicsDevice::text (double x, double y, const QString& text, double rot, double hadj, const QColor& col, const QFont& font) {
	RK_TRACE (GRAPHICS_DEVICE);

	painter.save ();
	QSizeF size = strSize (text, font);	// NOTE: side-effect of setting font!
//	painter.setFont (font);
	painter.setPen (QPen (col));
	painter.translate (x, y);
	painter.rotate (-rot);
	painter.drawText (-(hadj * size.width ()), 0, text);
	painter.restore ();	// undo rotation / translation
	triggerUpdate ();
}

void RKGraphicsDevice::metricInfo (const QChar& c, const QFont& font, double* ascent, double* descent, double* width) {
	RK_TRACE (GRAPHICS_DEVICE);

	// Don't touch! This is the result of a lot of trial and error, and replicates the behavior of X11() on the ?plotmath examples
	QFontMetricsF fm (font);
	QRectF rect = fm.boundingRect (c);
	*ascent = -rect.top ();
	*descent = rect.bottom ();
	*width = fm.width (c);
}

void RKGraphicsDevice::polygon (const QPolygonF& pol, const QPen& pen, const QBrush& brush) {
	RK_TRACE (GRAPHICS_DEVICE);

	painter.setPen (pen);
	painter.setBrush (brush);
	painter.drawPolygon (pol);
	triggerUpdate ();
}

void RKGraphicsDevice::polyline (const QPolygonF& pol, const QPen& pen) {
	RK_TRACE (GRAPHICS_DEVICE);

	painter.setPen (pen);
	painter.drawPolyline (pol);
	triggerUpdate ();
}

void RKGraphicsDevice::setActive (bool active) {
	RK_TRACE (GRAPHICS_DEVICE);
	emit (activeChanged (active));
}

#include "rkgraphicsdevice.moc"
