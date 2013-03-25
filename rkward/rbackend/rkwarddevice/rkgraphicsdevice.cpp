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

/******************************* ACKNOWLEDGEMENT ***************************
 * 
 * The drawing functions in this file are heavily inspired, in some parts even copied from package qtutils, version 0.1-3
 * by Deepayan Sarkar. Package qtutils is available from http://qtinterfaces.r-forge.r-project.org
 * under GNU LPGL 2 or later.
 * 
 ***************************************************************************/

#include "rkgraphicsdevice.h"

#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsRectItem>

#include "../../debug.h"

#define UPDATE_INTERVAL 100

QHash<int, RKGraphicsDevice*> RKGraphicsDevice::devices;

RKGraphicsDevice::RKGraphicsDevice (double width, double height) : QObject () {
	RK_TRACE (GRAPHICS_DEVICE);
	scene = new QGraphicsScene (0, 0, width, height, this);
	view = new QGraphicsView (scene);
//	view->setOptimizationFlags (QGraphicsView::DontSavePainterState);
//	view->setRenderHints (QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
//	scene->setItemIndexMethod (QGraphicsScene::NoIndex);
//	scene->setBspTreeDepth (16);
//	view->setViewportUpdateMode (QGraphicsView::NoViewportUpdate);
	connect (&updatetimer, SIGNAL (timeout ()), this, SLOT (updateNow ()));
	view->show ();
	item_z = clip_z = 0;
	clip = 0;
	setClip (scene->sceneRect ());
}

RKGraphicsDevice::~RKGraphicsDevice () {
	RK_TRACE (GRAPHICS_DEVICE);
	delete view;
}

void RKGraphicsDevice::triggerUpdate () {
	view->setUpdatesEnabled (false);
	updatetimer.start (UPDATE_INTERVAL);
}

void RKGraphicsDevice::updateNow () {
//	QList<QRectF> dummy;
//	dummy.append (scene->sceneRect ());
//	view->updateScene (dummy);
	view->setUpdatesEnabled (true);
	view->update ();
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

void RKGraphicsDevice::clear (const QBrush& bg) {
	RK_TRACE (GRAPHICS_DEVICE);

	scene->clear ();
	clip = 0;
	setClip (scene->sceneRect ());
	rect (scene->sceneRect (), QPen (Qt::NoPen), bg);
	view->show ();
	updatetimer.start (UPDATE_INTERVAL);
}

void RKGraphicsDevice::setClip (const QRectF& new_clip) {
	RK_TRACE (GRAPHICS_DEVICE);

//	QGraphicsItem* old_clip = clip;
	clip = scene->addRect (new_clip, QPen (Qt::NoPen));
	clip->setZValue (clip_z += .1);
	clip->setFlags (QGraphicsItem::ItemClipsChildrenToShape);
	item_z = 0;
//	if (old_clip) old_clip->setCacheMode (QGraphicsItem::DeviceCoordinateCache);
}

void RKGraphicsDevice::addItem (QGraphicsItem* item) {
	if (item_z > 1000) {
		updatetimer.stop ();
		QGraphicsItem *old_clip = clip;
		QRectF brect = clip->rect ().normalized ();
		QPixmap cache (brect.width () + 1, brect.height () + 1);
		cache.fill (QColor (0, 0, 0, 0));
		QPainter painter;
		painter.begin (&cache);
		scene->render (&painter, cache.rect (), brect);
		painter.end ();
		setClip (brect);
		QGraphicsPixmapItem *cached = new QGraphicsPixmapItem (cache);
		cached->setPos (brect.x (), brect.y ());
		addItem (cached);
		delete old_clip;
	}
	item->setZValue (item_z += .1);
	item->setParentItem (clip);
	triggerUpdate ();
}

void RKGraphicsDevice::circle (double x, double y, double r, const QPen& pen, const QBrush& brush) {
	RK_TRACE (GRAPHICS_DEVICE);

	QGraphicsEllipseItem* circle = new QGraphicsEllipseItem (x - r, y - r, r+r, r+r);
	circle->setPen (pen);
	circle->setBrush (brush);
	addItem (circle);
}

void RKGraphicsDevice::line (double x1, double y1, double x2, double y2, const QPen& pen) {
	RK_TRACE (GRAPHICS_DEVICE);

	QGraphicsLineItem* line = new QGraphicsLineItem (x1, y1, x2, y2);
	line->setPen (pen);
	addItem (line);
}

void RKGraphicsDevice::rect (const QRectF& rec, const QPen& pen, const QBrush& brush) {
	RK_TRACE (GRAPHICS_DEVICE);

	QGraphicsRectItem* rect = new QGraphicsRectItem (rec);
	rect->setPen (pen);
	rect->setBrush (brush);
	addItem (rect);
}

double RKGraphicsDevice::strWidth (const QString& text, const QFont& font) {
	RK_TRACE (GRAPHICS_DEVICE);

	QGraphicsTextItem t (text);
	t.setFont (font);
	return t.boundingRect ().width ();
}

void RKGraphicsDevice::text (double x, double y, const QString& _text, double rot, double hadj, const QColor& col, const QFont& font) {
	RK_TRACE (GRAPHICS_DEVICE);

	QGraphicsTextItem *text = new QGraphicsTextItem ();
	text->setPlainText (_text);
	text->setFont (font);
	text->setDefaultTextColor (col);
	QRectF brect = text->boundingRect ();
	text->rotate (-rot);
	text->translate (-hadj * brect.width (), - QFontMetricsF (font).height ());
	text->setPos (x, y);
	text->setTextInteractionFlags (Qt::TextSelectableByMouse);
	addItem (text);
}

void RKGraphicsDevice::metricInfo (const QChar& c, const QFont& font, double* ascent, double* descent, double* width) {
	RK_TRACE (GRAPHICS_DEVICE);

	QFontMetricsF fm (font);
	*ascent = fm.ascent ();	// TODO: or should we return the metrics of this particular char (similar to strWidth)
	*descent = fm.descent ();
	*width = fm.width (c);
/*	QGraphicsTextItem t;
	t.setPlainText (QString (c));
	t.setFont (font);
	*ascent = t.boundingRect().height();
	*descent = 0;
	*width = t.boundingRect().width(); */
}

void RKGraphicsDevice::polygon (const QPolygonF& pol, const QPen& pen, const QBrush& brush) {
	RK_TRACE (GRAPHICS_DEVICE);

	QGraphicsPolygonItem *poli = new QGraphicsPolygonItem (pol);
	poli->setPen (pen);
	poli->setBrush (brush);
	addItem (poli);
}

void RKGraphicsDevice::polyline (const QPolygonF& pol, const QPen& pen) {
	RK_TRACE (GRAPHICS_DEVICE);
// Qt insistes that all QGraphicsPolygonItems must be closed. So the this does not work:
// QGraphicsPolygonItem *poli = new QGraphicsPolygonItem (pol, clip);
	if (pol.isEmpty ()) return;

	QPainterPath path;
	path.moveTo (pol[0]);
	for (int i = 1; i < pol.size (); ++i) {
		path.lineTo (pol[i]);
	}
	QGraphicsPathItem *poli = new QGraphicsPathItem (path);
	poli->setPen (pen);
	addItem (poli);
}

void RKGraphicsDevice::setActive (bool active) {
	RK_TRACE (GRAPHICS_DEVICE);
	emit (activeChanged (active));
}

#include "rkgraphicsdevice.moc"
