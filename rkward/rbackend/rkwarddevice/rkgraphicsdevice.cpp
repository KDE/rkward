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

QHash<int, RKGraphicsDevice*> RKGraphicsDevice::devices;

RKGraphicsDevice::RKGraphicsDevice (double width, double height) : QObject () {
	RK_TRACE (GRAPHICS_DEVICE);
	scene = new QGraphicsScene (0, 0, width, height, this);
	view = new QGraphicsView (scene);
	view->show ();
	item_z = clip_z = 0;
	setClip (scene->sceneRect ());
}

RKGraphicsDevice::~RKGraphicsDevice () {
	RK_TRACE (GRAPHICS_DEVICE);
	delete view;
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

void RKGraphicsDevice::setClip (const QRectF& new_clip) {
	RK_TRACE (GRAPHICS_DEVICE);
	clip = scene->addRect (new_clip, QPen (Qt::NoPen));
	clip->setZValue (clip_z += .1);
	clip->setFlags (QGraphicsItem::ItemClipsChildrenToShape);
	item_z = 0;
}

void RKGraphicsDevice::circle (double x, double y, double r, const QPen& pen, const QBrush& brush) {
	QGraphicsEllipseItem* circle = new QGraphicsEllipseItem (x - r, y - r, r, r, clip);
	circle->setPen (pen);
	circle->setBrush (brush);
	circle->setZValue (item_z += .1);
}

void RKGraphicsDevice::line (double x1, double y1, double x2, double y2, const QPen& pen) {
	QGraphicsLineItem* line = new QGraphicsLineItem (x1, y1, x2, y2, clip);
	line->setPen (pen);
	line->setZValue (item_z += .1);
}

double RKGraphicsDevice::strWidth (const QString& text, const QFont& font) {
	QGraphicsSimpleTextItem t (text, 0);
	t.setFont (font);
	return t.boundingRect ().width ();
}

void RKGraphicsDevice::text (double x, double y, const QString& _text, double rot, double hadj, const QColor& col, const QFont& font) {
	QGraphicsTextItem *text = new QGraphicsTextItem (clip);
	text->setPlainText (_text);
	text->setFont (font);
	QRectF brect = text->boundingRect();
	text->rotate (-rot);
	text->translate (-hadj * brect.width (), - QFontMetricsF (font).height ());
	text->setPos (x, y);
	text->setZValue (item_z += .1);
	text->setTextInteractionFlags (Qt::TextSelectableByMouse);
}
