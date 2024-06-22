/*
rkgraphicsdevice - This file is part of the RKWard project. Created: Mon Mar 18 2013
SPDX-FileCopyrightText: 2013-2024 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
 
#ifndef RKGRAPHICSDEVICE_H
#define RKGRAPHICSDEVICE_H

#include <QHash>
#include <QPen>
#include <QTimer>
#include <QPainter>
#include <QPainterPath>
#include <QLabel>

#ifndef Q_OS_WIN
// On Mac, drawing on a pixmap does not work correctly. Probably can only be done inside paint
// events. (MacOSX 10.6.8, Qt 4.8.4).
// On X11, similar problems seem to occur on some, but not all systems. Only on old versions?
// See http://sourceforge.net/p/rkward/bugs/129/ .
// Version not working with QPixmap: qt 4.7.0~beta2, libx11 1.3.2, xserver-xorg 7.5
// Version working with QPixmap: qt 4.8.4, libx11 1.5.0, xserver-xorg 7.7
//
// Fortunately, a QImage based buffer does not seem to be _that_ much slower
// (around 5-10% on X11, on plot (rnorm (100000)))
#	define USE_QIMAGE_BUFFER
#endif
#ifdef USE_QIMAGE_BUFFER
#	include <QImage>
#else
#	include <QPixmap>
#endif

class QDialog;

/** This is the class that actually does all the drawing for the RKGraphicsDevice */
class RKGraphicsDevice : public QObject {
	Q_OBJECT
protected:
	RKGraphicsDevice (double width, double height, const QString &title, bool antialias);
	~RKGraphicsDevice ();
public:
	static RKGraphicsDevice* newDevice (int devnum, double width, double height, const QString &title, bool antialias, quint32 id);
	static void closeDevice (int devnum);
	static QHash<int, RKGraphicsDevice*> devices;

	void circle (double x, double y, double r, const QPen& pen, const QBrush& brush);
	void line (double x1, double y1, double x2, double y2, const QPen& pen);
	void rect (const QRectF& rec, const QPen& pen, const QBrush& brush);
	QSizeF strSize (const QString &text, const QFont& font);
	void text (double x, double y, const QString &text, double rot, double hadj, const QColor& col, const QFont& font);
	void metricInfo (const QChar& c, const QFont& font, double *ascent, double *descent, double *width);
	void setClip (const QRectF& new_clip);
	void polygon (const QPolygonF& pol, const QPen& pen, const QBrush &brush);
	void polyline (const QPolygonF& pol, const QPen& pen);
	void polypath (const QVector<QPolygonF>& polygons, bool winding, const QPen& pen, const QBrush& brush);
	void clear(const QBrush& col=QBrush());
	void image (const QImage &image, const QRectF &target_rect, double rot, bool interpolate);
	QImage capture () const;
	void setActive (bool active);
	void triggerUpdate ();
	void locator ();
	void confirmNewPage ();

	// graphics event handling
/** Simple struct to keep info about both mouse and keyboard events, so we can store them in a list, until R fetches them. */
	struct StoredEvent {
		StoredEvent () : event_code (0), buttons (0), modifiers (0), keycode (0), x (0), y (0) {};
		qint8 event_code;
		qint8 buttons;
		qint32 modifiers;
		qint32 keycode;
		QString keytext;
		double x, y;
	};
	void startGettingEvents (const QString &prompt);
	StoredEvent fetchNextEvent ();
	void stopGettingEvents ();

 	QWidget* viewPort () const { return view; };
	QSizeF currentSize () const { return view ? view->size() : QSizeF(); }
	void setAreaSize (const QSize &size);

/** Patterns / gradients are registered per device in R */
	int registerPattern(const QBrush &brush);
	void destroyPattern(int id);
	QBrush getPattern(int id) const { return patterns.value(id); };
	void startRecordTilingPattern(double width, double height, double x, double y);
	int finalizeTilingPattern(int extend);
	void startRecordPath();
	QPainterPath endRecordPath(int fillrule);
	int cachePath(QPainterPath &path);
	void destroyCachedPath(int index);
	bool setClipToCachedPath(int index);
	void startRecordMask();
	QImage endRecordMask(bool luminance);
	int registerMask(const QImage &mask);
	void destroyMask(int index);
	bool setMask(int index);
	void fillStrokePath(const QPainterPath &path, const QBrush &brush, const QPen &pen);
	void startRecordGroup();
	void recordGroupStage2(int compositing_op);
	int endRecordGroup();
	void useGroup(int index, const QTransform &matrix);
	void destroyGroup(int index);
public Q_SLOTS:
	void stopInteraction ();
Q_SIGNALS:
	void goingInteractive (bool interactive, const QString &prompt);
	void activeChanged (bool);
	void locatorDone (bool ok, double x, double y);
	void newPageConfirmDone (bool accepted);
	void captionChanged (const QString &caption);
	void deviceClosed(int devnum);
private Q_SLOTS:
	void updateNow ();
	void newPageDialogDone (int result);
	void viewKilled ();
private:
	void goInteractive (const QString &prompt);
	bool eventFilter (QObject *watched, QEvent *event) override;
	void checkSize ();

	QTimer updatetimer;
#ifdef USE_QIMAGE_BUFFER
	QImage area;
#else
	QPixmap area;
#endif
	QPainter painter;
	QLabel *view;
	QString base_title;
	QDialog *dialog;
	QHash<int, QBrush> patterns;
	QHash<int, QPainterPath> cached_paths;
	QHash<int, QImage> cached_masks;
	QHash<int, QImage> cached_groups;
	// NOTE on path recording: In principle, we could really do _all_ painting on QPainterPath, but in regular operation stroke and fill right away.
	// However, that is noticably slower.
	QPainterPath recorded_path;
	QList<QPainterPath> stashed_paths;
	bool recording_path;
	int current_mask;

	int interaction_opcode;	/**< Current interactive operation (from RKDOpcodes enum), or -1 is there is no current interactive operation */
	quint32 id;

	QList<StoredEvent> stored_events;

	struct PaintContext {
		QImage surface;
		QTransform transform;
		QRect capture_coords;
	};
	QList<PaintContext> contexts;
	// make sure the painter is active on the current context
	void beginPainter();
	void pushContext(double width, double height, double x, double y);
	PaintContext popContext();
	void initMaskedDraw();
	void commitMaskedDraw();
};

#endif
