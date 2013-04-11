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
#include <QPushButton>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <klocale.h>
#include <sys/stat.h>
#include <kdialog.h>

#include "rkgraphicsdevice_protocol_shared.h"

#include "../../debug.h"

#define UPDATE_INTERVAL 100

QHash<int, RKGraphicsDevice*> RKGraphicsDevice::devices;

RKGraphicsDevice::RKGraphicsDevice (double width, double height, const QString &title, bool antialias) : QObject (), area (qAbs (width) + 1, qAbs (height) + 1), base_title (title) {
	RK_TRACE (GRAPHICS_DEVICE);

	interaction_opcode = -1;
	dialog = 0;
	if (antialias) painter.setRenderHints (QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);
	view = new QLabel ();
	view->installEventFilter (this);
	connect (view, SIGNAL (destroyed(QObject*)), this, SLOT (viewKilled()));
	connect (&updatetimer, SIGNAL (timeout ()), this, SLOT (updateNow ()));
	updatetimer.setSingleShot (true);
	clear ();
	setActive (true);	// sets window title
}

RKGraphicsDevice::~RKGraphicsDevice () {
	RK_TRACE (GRAPHICS_DEVICE);
	painter.end ();
	stopInteraction ();
	delete view;
}

void RKGraphicsDevice::viewKilled () {
	RK_TRACE (GRAPHICS_DEVICE);
	view = 0;
	closeDevice (devices.key (this));
}

void RKGraphicsDevice::triggerUpdate () {
	updatetimer.start (UPDATE_INTERVAL);
}

void RKGraphicsDevice::updateNow () {
	if (painter.isActive ()) painter.end ();
	view->setPixmap (area);
	if (!view->isVisible ()) {
		view->resize (area.size ());
		view->show ();
	}
	painter.begin (&area);
}

RKGraphicsDevice* RKGraphicsDevice::newDevice (int devnum, double width, double height, const QString &title, bool antialias) {
	RK_TRACE (GRAPHICS_DEVICE);

	if (devices.contains (devnum)) {
		RK_DEBUG (GRAPHICS_DEVICE, DL_ERROR, "Graphics device number %d already exists while trying to create it", devnum);
		closeDevice (devnum);
	}
	RKGraphicsDevice* dev = new RKGraphicsDevice (width, height, title.isEmpty () ? i18n ("Graphics Device Number %1").arg (QString::number (devnum+1)) : title, antialias);
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
	setClip (area.rect ());	// R's devX11.c resets clip on clear, so we do this, too.
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

void RKGraphicsDevice::image (const QImage& image, const QRectF& target_rect, double rot, bool interpolate) {
	RK_TRACE (GRAPHICS_DEVICE);

	painter.save ();
	QRectF tr = target_rect;
	painter.translate (tr.x (), tr.y ());
	tr.moveTo (0, 0);
	painter.rotate (-rot);
	painter.setRenderHint (QPainter::SmoothPixmapTransform, interpolate);
	painter.drawImage (tr, image, image.rect ());
	painter.restore ();
	triggerUpdate ();
}

QImage RKGraphicsDevice::capture () const {
	RK_TRACE (GRAPHICS_DEVICE);

	return area.toImage ();
}

void RKGraphicsDevice::setActive (bool active) {
	RK_TRACE (GRAPHICS_DEVICE);

	if (active) view->setWindowTitle (i18n ("%1 (Active)").arg (base_title));
	else view->setWindowTitle (i18n ("%1 (Inactive)").arg (base_title));
	emit (activeChanged (active));
}

void RKGraphicsDevice::locator () {
	RK_TRACE (GRAPHICS_DEVICE);

	RK_ASSERT (interaction_opcode < 0);
	interaction_opcode = RKDLocator;

	view->setCursor (Qt::CrossCursor);
	view->setToolTip (i18n ("<h2>Locating point(s)</h2><p>Use left mouse button to select point(s). Any other mouse button to stop.</p>"));
	view->show ();
	view->raise ();
}

void RKGraphicsDevice::confirmNewPage () {
	RK_TRACE (GRAPHICS_DEVICE);

	RK_ASSERT (interaction_opcode < 0);
	RK_ASSERT (dialog == 0);
	interaction_opcode = RKDNewPageConfirm;

	view->show ();
	view->raise ();
	dialog = new KDialog (view);
	dialog->setCaption (i18n ("Ok to show next plot?"));
	dialog->setButtons (KDialog::Ok | KDialog::Cancel);
	dialog->setMainWidget (new QLabel (i18n ("<p>Press Enter to see next plot, or click 'Cancel' to abort.</p>"), dialog));
//	dialog->setWindowModality (Qt::WindowModal);        // not good: Grays out the plot window
	connect (dialog, SIGNAL (finished (int)), this, SLOT (newPageDialogDone (int)));
	dialog->show ();
}

void RKGraphicsDevice::newPageDialogDone (int result) {
	RK_TRACE (GRAPHICS_DEVICE);

	RK_ASSERT (dialog);
	emit (newPageConfirmDone (result == KDialog::Accepted));
	interaction_opcode = -1;
	stopInteraction ();
}

bool RKGraphicsDevice::eventFilter (QObject *watched, QEvent *event) {
	RK_ASSERT (watched == view);

	if (interaction_opcode == RKDLocator) {
		if (event->type () == QEvent::MouseButtonRelease) {
			QMouseEvent *me = static_cast<QMouseEvent*> (event);
			if (me->button () == Qt::LeftButton) {
				emit (locatorDone (true, me->x (), me->y ()));
				interaction_opcode = -1;
			}
			stopInteraction ();
			return true;
		}
	}

	return false;
}

void RKGraphicsDevice::stopInteraction () {
	RK_TRACE (GRAPHICS_DEVICE);

	if (interaction_opcode == RKDLocator) {
		emit (locatorDone (false, 0.0, 0.0));
	} else if (interaction_opcode == RKDNewPageConfirm) {
		RK_ASSERT (dialog);
		emit (newPageConfirmDone (true));
	}

	if (dialog) {
		dialog->deleteLater ();
		dialog = 0;
	}
	if (view) {	// might already be destroyed
		view->setCursor (Qt::ArrowCursor);
		view->setToolTip (QString ());
	}
	interaction_opcode = -1;
}

#include "rkgraphicsdevice.moc"
