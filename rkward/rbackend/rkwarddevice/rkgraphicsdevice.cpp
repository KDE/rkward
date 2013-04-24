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
#include "../rinterface.h"
#include "../../rkglobals.h"

#include "../../debug.h"

#define UPDATE_INTERVAL 100

QHash<int, RKGraphicsDevice*> RKGraphicsDevice::devices;

RKGraphicsDevice::RKGraphicsDevice (double width, double height, const QString &title, bool antialias) : QObject (), area (qAbs (width) + 1, qAbs (height) + 1), base_title (title) {
	RK_TRACE (GRAPHICS_DEVICE);

	interaction_opcode = -1;
	dialog = 0;
	view = new QLabel ();
	view->installEventFilter (this);
	view->setScaledContents (true);    // this is just for preview during scaling. The area will be re-sized and re-drawn from R.
	view->setFocusPolicy (Qt::StrongFocus);   // for receiving key events for R's getGraphicsEvent()
	connect (view, SIGNAL (destroyed(QObject*)), this, SLOT (viewKilled()));
	connect (&updatetimer, SIGNAL (timeout ()), this, SLOT (updateNow ()));
	updatetimer.setSingleShot (true);
	clear ();
	if (antialias) painter.setRenderHints (QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);
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
	if (view->size () != area.size ()) {
		RKGlobals::rInterface ()->issueCommand (new RCommand ("rkward:::RK.resize (" + QString::number (devices.key (this) + 1) + ")", RCommand::PriorityCommand));
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

void RKGraphicsDevice::setAreaSize (const QSize& size) {
	if (painter.isActive ()) painter.end ();
	RK_DEBUG (GRAPHICS_DEVICE, DL_INFO, "New Size %d, %d (view size is %d, %d)", size.width (), size.height (), view->width (), view->height ());
	area = QPixmap (size.width (), size.height ());
	clear ();
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
	emit (captionChanged (view->windowTitle ()));
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

void RKGraphicsDevice::startGettingEvents (const QString& prompt) {
	RK_TRACE (GRAPHICS_DEVICE);

	RK_ASSERT (interaction_opcode < 0);
	stored_events.clear ();
	interaction_opcode = RKDStartGettingEvents;

	view->setCursor (Qt::CrossCursor);
	view->setToolTip (prompt);
	view->show ();
	view->raise ();
}

RKGraphicsDevice::StoredEvent RKGraphicsDevice::fetchNextEvent () {
	RK_TRACE (GRAPHICS_DEVICE);

	if (stored_events.isEmpty ()) {
		StoredEvent ret;
		ret.event_code = RKDNothing;
		return ret;
	}
	return stored_events.takeFirst ();
}

void RKGraphicsDevice::stopGettingEvents () {
	RK_TRACE (GRAPHICS_DEVICE);

	RK_ASSERT (interaction_opcode == RKDStartGettingEvents);
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
	} else if (interaction_opcode == RKDStartGettingEvents) {
		if ((event->type () == QEvent::MouseButtonPress) || (event->type () == QEvent::MouseButtonRelease) || (event->type () == QEvent::MouseMove)) {
			QMouseEvent *me = static_cast<QMouseEvent*> (event);
			StoredEvent sev;

			sev.event_code = event->type () == QEvent::MouseButtonPress ? RKDMouseDown : (event->type () == QEvent::MouseButtonRelease ? RKDMouseUp : RKDMouseMove);
			sev.x = me->x ();
			sev.y = me->y ();
			sev.buttons = 0;
			if (me->buttons () & Qt::LeftButton) sev.buttons |= RKDMouseLeftButton;
			if (me->buttons () & Qt::MiddleButton) sev.buttons |= RKDMouseMiddleButton;
			if (me->buttons () & Qt::RightButton) sev.buttons |= RKDMouseRightButton;

			// Mouse move event may be generated much faster than R can handle them. We simply lump them together
			// (unless any other event type (click, release, keypress) has occurred meanwhile, of course.
			if (!stored_events.isEmpty () && (sev.event_code == RKDMouseMove)) {
				if (stored_events.last ().event_code == RKDMouseMove) stored_events.pop_back ();
			}

			stored_events.append (sev);
			return (true);
		} else if (event->type () == QEvent::KeyPress) {
			QKeyEvent *ke = static_cast<QKeyEvent*> (event);
			StoredEvent sev;

			sev.event_code = RKDKeyPress;
			sev.keytext = ke->text ();
			sev.keycode = ke->key ();
			sev.modifiers = ke->modifiers ();
			if (sev.modifiers - (sev.modifiers & Qt::ShiftModifier)) {
				// well, the text returned in ke->text() is a bit strange, sometimes when modifiers are involved.
				// This HACK does some sanitizing. Too much sanitizing? Umlauts don't get through this, for one thing.
				sev.keytext = QKeySequence (sev.modifiers | sev.keycode).toString ();
				sev.keytext = sev.keytext.right (1);
			}

			stored_events.append (sev);
			return (true);
		}
	}

	if (event->type () == QEvent::Resize) triggerUpdate ();

	return false;
}

void RKGraphicsDevice::stopInteraction () {
	RK_TRACE (GRAPHICS_DEVICE);

	if (interaction_opcode == RKDLocator) {
		emit (locatorDone (false, 0.0, 0.0));
	} else if (interaction_opcode == RKDNewPageConfirm) {
		RK_ASSERT (dialog);
		emit (newPageConfirmDone (true));
	} else if (interaction_opcode == RKDStartGettingEvents) {
		// not much to do, fortunately, as getting graphics events is non-blocking
		stored_events.clear ();
		StoredEvent sev;
		sev.event_code = RKDFrontendCancel;
		stored_events.append (sev);   // In case the backend keeps asking (without calling RKDStartGettingEvents again, first), we'll tell it to stop.
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
