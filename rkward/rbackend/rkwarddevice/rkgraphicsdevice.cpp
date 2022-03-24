/***************************************************************************
                          rkgraphicsdevice_backendtransmitter  -  description
                             -------------------
    begin                : Mon Mar 18 20:06:08 CET 2013
    copyright            : (C) 2013-2021 by Thomas Friedrichsmeier
    email                : thomas.friedrichsmeier@kdemail.net
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

#include <QHBoxLayout>
#include <QMouseEvent>
#include <QDialog>

#include <KLocalizedString>
#include <sys/stat.h>

#include "rkgraphicsdevice_protocol_shared.h"
#include "../rkrinterface.h"
#include "../../rkglobals.h"
#include "../../misc/rkdialogbuttonbox.h"

#include "../../debug.h"

#define UPDATE_INTERVAL 100

QHash<int, RKGraphicsDevice*> RKGraphicsDevice::devices;

RKGraphicsDevice::RKGraphicsDevice (double width, double height, const QString &title, bool antialias) : 
		QObject (),
#ifdef USE_QIMAGE_BUFFER
		area (qAbs (width) + 1, qAbs (height) + 1, QImage::Format_ARGB32),
#else
		area (qAbs (width) + 1, qAbs (height) + 1),
#endif
		base_title (title) {
	RK_TRACE (GRAPHICS_DEVICE);

	interaction_opcode = -1;
	dialog = 0;
	id = 0;
	recording_path = false;
	current_mask = 0;
	view = new QLabel ();
	view->installEventFilter (this);
	view->setScaledContents (true);    // this is just for preview during scaling. The area will be re-sized and re-drawn from R.
	view->setFocusPolicy (Qt::StrongFocus);   // for receiving key events for R's getGraphicsEvent()
	connect (view, &QLabel::destroyed, this, &RKGraphicsDevice::viewKilled);
	connect (&updatetimer, &QTimer::timeout, this, &RKGraphicsDevice::updateNow);
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

void RKGraphicsDevice::beginPainter() {
	if(!painter.isActive()) {
		if (contexts.isEmpty()) {
			painter.begin(&area);  // plain old painting on the canvas itself
			recording_path = false;
		} else {
			auto &c = contexts.last();
			painter.begin(&(c.surface));
			painter.setTransform(c.transform);
		}
	}
}

void RKGraphicsDevice::pushContext(double width, double height, double x, double y) {
	RK_TRACE (GRAPHICS_DEVICE);
	painter.end();
	PaintContext c;
// NOTE: R cairo device uses an all different method for pattern capture:
// drawing is scaled up to full device coordinates, then shrunk and offset back to pattern size.
// probably due to cairo internals, somehow. Here, instead we paint on a separate surface with the same coords,
// then extract the rectangle of interest.
	c.surface = QImage(area.width(), area.height(), QImage::Format_ARGB32);
	c.surface.fill(Qt::transparent);
	if (width < 0) { // may happen, at least in R 4.1.2
		width = -width;
		x -= width;
	}
	if (height < 0) {
		height = -height;
		y -= height;
	}
	c.capture_coords = QRect(x, y, width, height);
	contexts.push_back(c);
	beginPainter();
}

RKGraphicsDevice::PaintContext RKGraphicsDevice::popContext() {
	RK_TRACE (GRAPHICS_DEVICE);

	if (contexts.isEmpty()) {
		RK_ASSERT(!contexts.isEmpty());
		return PaintContext();
	}

	painter.end();
	auto ret = contexts.takeLast();
	beginPainter();
	return ret;
}

void RKGraphicsDevice::initMaskedDraw(){
	RK_ASSERT(current_mask);
	pushContext(area.width(), area.height(), 0, 0);
}

void RKGraphicsDevice::commitMaskedDraw() {
	RK_ASSERT(current_mask);
	QImage mask = cached_masks.value(current_mask);
	QImage masked = popContext().surface;
	masked.setAlphaChannel(mask); // NOTE: Qt docs: "If the image already has an alpha channel, the existing alpha channel is multiplied with the new one."
	painter.drawImage(0, 0, masked);
}

void RKGraphicsDevice::startRecordTilingPattern(double width, double height, double x, double y) {
	RK_TRACE (GRAPHICS_DEVICE);
	pushContext(width, height, x, y);
}

int RKGraphicsDevice::finalizeTilingPattern(int extend) {
	RK_TRACE (GRAPHICS_DEVICE);

	auto c = popContext();
	if (extend == GradientExtendNone || extend == GradientExtendPad) {
		// For extend type pad it is unclear, what that should even mean, we simply treat it the same as none.
		// For none, obviously, we want to not repeat the pattern. This is not so easy to achieve in QBrush, but also, it does
		// look like a very like use of a tiling pattern. What we do, therefore, is to simply copy the _full_ surface, rather than
		// just the reqion of interest. This way, we will - usually - not see any repeats.
		return (registerPattern(QBrush(c.surface)));
	}
	if (extend == GradientExtendReflect) {
		QImage single = c.surface.copy(c.capture_coords);
		QImage reflected(single.width()*2, single.height()*2, single.format());
		reflected.fill(Qt::transparent);
		QPainter p(&reflected);
		p.drawImage(0, 0, single);
		p.drawImage(single.width(), 0, single.mirrored(true, false));
		p.drawImage(0, single.height(), single.mirrored(false, true));
		p.drawImage(single.width(), single.height(), single.mirrored(true, true));
		p.end();
		QBrush brush(reflected);
		brush.setTransform(QTransform().translate(c.capture_coords.left(), c.capture_coords.top()));
		return registerPattern(brush);
	}

	// else: GradientExtendRepeat. This is the standard QBrush behavior
	QImage img = c.surface.copy(c.capture_coords);
	QBrush brush(img);
	brush.setTransform(QTransform().translate(c.capture_coords.left(), c.capture_coords.top()));
	return registerPattern(brush);
}

void RKGraphicsDevice::viewKilled () {
	RK_TRACE (GRAPHICS_DEVICE);
	view = 0;
//	closeDevice(devices.key(this));  // Do not do this, here. Don't mark the device as dead until R thinks so, too, and tells us about it.
}

void RKGraphicsDevice::triggerUpdate () {
	updatetimer.start (UPDATE_INTERVAL);
}

void RKGraphicsDevice::updateNow () {
	if (!view) return;	// device windows already killed, but this instance not yet removed.
	if (painter.isActive ()) painter.end ();
#ifdef USE_QIMAGE_BUFFER
	view->setPixmap (QPixmap::fromImage (area));
#else
	view->setPixmap (area);
#endif
	if (!view->isVisible ()) {
		view->resize (area.size ());
		view->show ();
	}
	checkSize ();
	beginPainter();
}

void RKGraphicsDevice::checkSize() {
	RK_TRACE (GRAPHICS_DEVICE);
	if(!view) return;
	if (view->size () != area.size ()) {
		RKGlobals::rInterface()->issueCommand(new RCommand ("rkward:::RK.resize(" + QString::number(devices.key(this) + 1) + ',' + QString::number(id) + ')', RCommand::PriorityCommand));
	}
}

RKGraphicsDevice* RKGraphicsDevice::newDevice (int devnum, double width, double height, const QString &title, bool antialias, quint32 id) {
	RK_TRACE (GRAPHICS_DEVICE);

	if (devices.contains (devnum)) {
		RK_DEBUG (GRAPHICS_DEVICE, DL_ERROR, "Graphics device number %d already exists while trying to create it", devnum);
		closeDevice (devnum);
	}
	RKGraphicsDevice* dev = new RKGraphicsDevice (width, height, title.isEmpty () ? i18n ("Graphics Device Number %1", QString::number (devnum+1)) : title, antialias);
	dev->id = id;
	devices.insert (devnum, dev);
	return (dev);
}

void RKGraphicsDevice::closeDevice (int devnum) {
	RK_TRACE (GRAPHICS_DEVICE);

	RK_ASSERT (devices.contains (devnum));
	devices.take (devnum)->deleteLater ();
}

void RKGraphicsDevice::clear(const QBrush& brush) {
	RK_TRACE (GRAPHICS_DEVICE);

	if (recording_path) {
		recorded_path = QPainterPath();
		setClip(area.rect());	// R's devX11.c resets clip on clear, so we do this, too.
		return;
	}

	if (current_mask) initMaskedDraw();
	if (brush.style() == Qt::NoBrush) {
		painter.setBrush(QColor(255, 255, 255, 255));
	} else {
		painter.setBrush(brush);
	}
	painter.drawRect(0, 0, area.width(), area.height());
	if (current_mask) commitMaskedDraw();

	updateNow ();
	setClip (area.rect ());	// R's devX11.c resets clip on clear, so we do this, too.
}

void RKGraphicsDevice::setAreaSize (const QSize& size) {
	if (painter.isActive ()) painter.end ();
	RK_DEBUG (GRAPHICS_DEVICE, DL_INFO, "New Size %d, %d (view size is %d, %d)", size.width (), size.height (), view->width (), view->height ());
#ifdef USE_QIMAGE_BUFFER
	area = QImage (size.width (), size.height (), QImage::Format_ARGB32);
#else 
	area = QPixmap (size.width (), size.height ());
#endif
	clear ();
}

int RKGraphicsDevice::registerPattern(const QBrush& brush) {
	RK_TRACE(GRAPHICS_DEVICE);
	static int id = 0;
	patterns.insert(++id, brush);
	return id;
}

void RKGraphicsDevice::destroyPattern(int id) {
	RK_TRACE(GRAPHICS_DEVICE);
	if (id == 0) patterns.clear();
	else patterns.remove(id);
}

void RKGraphicsDevice::startRecordPath() {
	RK_TRACE(GRAPHICS_DEVICE);

	stashed_paths.append(recorded_path);
	recorded_path = QPainterPath();
	recording_path = true;
}

QPainterPath RKGraphicsDevice::endRecordPath(int fillrule) {
	RK_TRACE(GRAPHICS_DEVICE);

	QPainterPath ret = recorded_path;
	if (fillrule == NonZeroWindingRule) ret.setFillRule(Qt::WindingFill);
	else ret.setFillRule(Qt::OddEvenFill);

	RK_ASSERT(!stashed_paths.isEmpty());
	recorded_path = stashed_paths.takeLast();
	recording_path = !stashed_paths.isEmpty();
	return ret;
}

int RKGraphicsDevice::cachePath(QPainterPath& path) {
	RK_TRACE(GRAPHICS_DEVICE);
	static int id = 0;
	cached_paths.insert(++id, path);
	return id;
}

void RKGraphicsDevice::destroyCachedPath(int index) {
	RK_TRACE(GRAPHICS_DEVICE);
	if (index < 0) cached_paths.clear();
	else cached_paths.remove(index);
}

bool RKGraphicsDevice::setClipToCachedPath(int index){
	RK_TRACE(GRAPHICS_DEVICE);
	if (cached_paths.contains(index)) {
		painter.setClipPath(cached_paths[index]);
		return true;
	}
	return false;
}

void RKGraphicsDevice::startRecordMask() {
	RK_TRACE (GRAPHICS_DEVICE);
	pushContext(area.width(), area.height(), 0, 0);
}

QImage RKGraphicsDevice::endRecordMask(bool luminance) {
	RK_TRACE (GRAPHICS_DEVICE);

	QImage ret = popContext().surface;
	// KF6 TODO: instead paint on a grayscale surface from the start?
	if (luminance) {
		return ret.convertToFormat(QImage::Format_Grayscale8);
	}
	return ret.convertToFormat(QImage::Format_Alpha8);
}

int RKGraphicsDevice::registerMask(const QImage& mask) {
	RK_TRACE (GRAPHICS_DEVICE);
	static int id = 0;
	cached_masks.insert(++id, mask);
	return id;
}

void RKGraphicsDevice::destroyMask(int index) {
	RK_TRACE (GRAPHICS_DEVICE);
	if (index < 0) cached_paths.clear();
	else cached_paths.remove(index);
}

bool RKGraphicsDevice::setMask(int index) {
	RK_TRACE (GRAPHICS_DEVICE);

	int set = 0;
	if (index > 0 && cached_masks.contains(index)) set = index;
	current_mask = set;
	return set == index;
}

void RKGraphicsDevice::setClip (const QRectF& new_clip) {
	RK_TRACE (GRAPHICS_DEVICE);

	beginPainter();
	painter.setClipRect (new_clip);
}

void RKGraphicsDevice::circle (double x, double y, double r, const QPen& pen, const QBrush& brush) {
	RK_TRACE (GRAPHICS_DEVICE);

	if (recording_path) {
		recorded_path.addEllipse(x-r, y-r, r+r, r+r);
		return;
	}
	if (current_mask) initMaskedDraw();
	painter.setPen (pen);
	painter.setBrush (brush);
	painter.drawEllipse (x - r, y - r, r+r, r+r);
	if (current_mask) commitMaskedDraw();
	triggerUpdate ();
}

void RKGraphicsDevice::line (double x1, double y1, double x2, double y2, const QPen& pen) {
	RK_TRACE (GRAPHICS_DEVICE);

	if (recording_path) {
		recorded_path.moveTo(x1,y1);
		recorded_path.lineTo(x2, y2);
		return;
	}

	if (current_mask) initMaskedDraw();
	painter.setPen (pen);
	// HACK: There seems to be a bug in QPainter (Qt 4.8.4), which can shift connected lines (everything but the first polyline)
	//       towards the direction where the previous line came from. The result is that line drawn via drawLine() and drawPolyline() do
	//       not match, exactly. This is particularly evident for the plot frame.
	//       We hack around this, by doing all line drawing via drawPolyline.
	QPointF points [2];
	points[0] = QPointF (x1, y1);
	points[1] = QPointF (x2, y2);
	painter.drawPolyline (points, 2);
//	painter.drawLine (x1, y1, x2, y2);
	if (current_mask) commitMaskedDraw();

	triggerUpdate ();
}

void RKGraphicsDevice::rect (const QRectF& rec, const QPen& pen, const QBrush& brush) {
	RK_TRACE (GRAPHICS_DEVICE);

	if (recording_path) {
		recorded_path.addRect(rec);
		return;
	}

	if (current_mask) initMaskedDraw();
	painter.setPen (pen);
	painter.setBrush (brush);
	painter.drawRect (rec);
	if (current_mask) commitMaskedDraw();

	triggerUpdate ();
}

QSizeF RKGraphicsDevice::strSize(const QString& text, const QFont& font) {
	RK_TRACE(GRAPHICS_DEVICE);

	painter.setFont(font);
	QSizeF size = painter.fontMetrics().boundingRect(text).size();
	return size;
}

void RKGraphicsDevice::text(double x, double y, const QString& text, double rot, double hadj, const QColor& col, const QFont& font) {
	RK_TRACE(GRAPHICS_DEVICE);

	if (recording_path) {
		QPainterPath sub;
		QSizeF size = strSize(text, font);
		sub.addText(-(hadj * size.width()), y, font, text);
		QMatrix trans;
		trans.translate(x, y);
		trans.rotate(-rot);
		recorded_path.addPath(trans.map(sub));
		return;
	}

	if (current_mask) initMaskedDraw();
	painter.save();
	QSizeF size = strSize(text, font);  // NOTE: side-effect of setting font!
//	painter.setFont(font);
	painter.setPen(QPen(col));
	painter.translate(x, y);
	painter.rotate(-rot);
	painter.drawText(-(hadj * size.width()), 0, text);
//	painter.drawRect(painter.fontMetrics().boundingRect(text));  // for debugging
	painter.restore();  // undo rotation / translation
	if (current_mask) commitMaskedDraw();

	triggerUpdate();
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

	if (recording_path) {
		recorded_path.addPolygon(pol);
		return;
	}
	if (current_mask) initMaskedDraw();
	painter.setPen (pen);
	painter.setBrush (brush);
	painter.drawPolygon (pol);
	if (current_mask) commitMaskedDraw();
	triggerUpdate ();
}

void RKGraphicsDevice::polyline (const QPolygonF& pol, const QPen& pen) {
	RK_TRACE (GRAPHICS_DEVICE);

	if (recording_path) {
		recorded_path.addPolygon(pol);
		return;
	}

	if (current_mask) initMaskedDraw();
	painter.setPen (pen);
	painter.drawPolyline (pol);
	if (current_mask) commitMaskedDraw();
	triggerUpdate ();
}

void RKGraphicsDevice::polypath (const QVector<QPolygonF>& polygons, bool winding, const QPen& pen, const QBrush& brush) {
	RK_TRACE (GRAPHICS_DEVICE);

	QPainterPath path;
	if (winding) path.setFillRule (Qt::WindingFill);
	for (int i = 0; i < polygons.size (); ++i) {
		path.addPolygon (polygons[i]);
		path.closeSubpath ();
	}

	if (recording_path) {
		recorded_path.addPath(path);
		return;
	}

	if (current_mask) initMaskedDraw();
	painter.setPen (pen);
	painter.setBrush (brush);
	painter.drawPath (path);
	if (current_mask) commitMaskedDraw();
	triggerUpdate ();
}

void RKGraphicsDevice::image (const QImage& image, const QRectF& target_rect, double rot, bool interpolate) {
	RK_TRACE (GRAPHICS_DEVICE);

	if (current_mask) initMaskedDraw();
	painter.save ();
	QRectF tr = target_rect;
	painter.translate (tr.x (), tr.y ());
	tr.moveTo (0, 0);
	painter.rotate (-rot);
	painter.setRenderHint (QPainter::SmoothPixmapTransform, interpolate);
	painter.drawImage (tr, image, image.rect ());
	painter.restore ();
	if (current_mask) commitMaskedDraw();
	triggerUpdate ();
}

QImage RKGraphicsDevice::capture () const {
	RK_TRACE (GRAPHICS_DEVICE);
#ifdef USE_QIMAGE_BUFFER
	return area;
#else
	return area.toImage ();
#endif
}

void RKGraphicsDevice::setActive (bool active) {
	RK_TRACE (GRAPHICS_DEVICE);

	if (!view) return;
	if (active) view->setWindowTitle (i18nc ("Window title", "%1 (Active)", base_title));
	else view->setWindowTitle (i18nc ("Window title", "%1 (Inactive)", base_title));
	emit (activeChanged (active));
	emit (captionChanged (view->windowTitle ()));
}

void RKGraphicsDevice::goInteractive (const QString& prompt) {
	RK_TRACE (GRAPHICS_DEVICE);

	// The backend does not support trying to resize while it is waiting for a reply, and will produce error message in this case.
	// To avoid confusion and loads of scary error messages, we disable resizing in the frontend while interactive.
	// NOTE: It would certainly be possible, but rather cumbersome to support resizing during interaction in the backend.
	// It does not seem to be important enough, however.
	view->setSizePolicy (QSizePolicy::Fixed, QSizePolicy::Fixed);
	view->setCursor (Qt::CrossCursor);
	view->setToolTip (prompt);
	view->show ();
	view->raise ();
	emit (goingInteractive (true, prompt));
}

void RKGraphicsDevice::locator () {
	RK_TRACE (GRAPHICS_DEVICE);

	RK_ASSERT (interaction_opcode < 0);
	interaction_opcode = RKDLocator;
	goInteractive (i18n ("<h2>Locating point(s)</h2><p>Use left mouse button to select point(s). Any other mouse button to stop.</p>"));
}

void RKGraphicsDevice::confirmNewPage () {
	RK_TRACE (GRAPHICS_DEVICE);

	RK_ASSERT (interaction_opcode < 0);
	RK_ASSERT (dialog == 0);
	interaction_opcode = RKDNewPageConfirm;

	QString msg = i18n ("<p>Press Enter to see next plot, or click 'Cancel' to abort.</p>");
	goInteractive (msg);

	dialog = new QDialog (view);
	dialog->setWindowTitle (i18n ("Ok to show next plot?"));

	QVBoxLayout *layout = new QVBoxLayout (dialog);
	layout->addWidget (new QLabel (msg, dialog));

	RKDialogButtonBox *buttons = new RKDialogButtonBox (QDialogButtonBox::Ok | QDialogButtonBox::Cancel, dialog);
	layout->addWidget (buttons);

//	dialog->setWindowModality (Qt::WindowModal);        // not good: Grays out the plot window
	connect (dialog, &QDialog::finished, this, &RKGraphicsDevice::newPageDialogDone);
	dialog->show ();
}

void RKGraphicsDevice::newPageDialogDone (int result) {
	RK_TRACE (GRAPHICS_DEVICE);

	RK_ASSERT (dialog);
	emit (newPageConfirmDone (result == QDialog::Accepted));
	interaction_opcode = -1;
	stopInteraction ();
}

void RKGraphicsDevice::startGettingEvents (const QString& prompt) {
	RK_TRACE (GRAPHICS_DEVICE);

	RK_ASSERT (interaction_opcode < 0);
	stored_events.clear ();
	interaction_opcode = RKDStartGettingEvents;

	goInteractive (prompt);
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
	if (!view) return false; // view was killed, but events may still be pending
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
			if (me->buttons () & Qt::MidButton) sev.buttons |= RKDMouseMiddleButton;
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
	} else if (interaction_opcode == RKDNewPageConfirm) {
		if (event->type () == QEvent::KeyPress) {
			QKeyEvent *ke = static_cast<QKeyEvent*> (event);
			if ((ke->key () == Qt::Key_Return) || (ke->key () == Qt::Key_Enter)) {
				newPageDialogDone (QDialog::Accepted);
				return true;
			} else if (ke->key () == Qt::Key_Escape) {
				newPageDialogDone (QDialog::Rejected);
				return true;
			}
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
		checkSize ();
		view->setSizePolicy (QSizePolicy::Preferred, QSizePolicy::Preferred);
	}
	emit (goingInteractive (false, QString ()));
	interaction_opcode = -1;
}

