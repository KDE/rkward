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
 
#ifndef RKGRAPHICSDEVICE_H
#define RKGRAPHICSDEVICE_H

#include <QHash>
#include <QPen>
#include <QTimer>
#include <QPixmap>
#include <QPainter>
#include <QLabel>

class KDialog;

/** This is the class that actually does all the drawing for the RKGraphicsDevice */
class RKGraphicsDevice : public QObject {
	Q_OBJECT
protected:
	RKGraphicsDevice (double width, double height, const QString &title, bool antialias);
	~RKGraphicsDevice ();
public:
	static RKGraphicsDevice* newDevice (int devnum, double width, double height, const QString &title, bool antialias);
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
	void clear (const QColor& col=QColor());
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
	QSizeF currentSize () const { return view->size (); }
	void setAreaSize (const QSize &size);
public slots:
	void stopInteraction ();
signals:
	void goingInteractive (bool interactive, const QString &prompt);
	void activeChanged (bool);
	void locatorDone (bool ok, double x, double y);
	void newPageConfirmDone (bool accepted);
	void captionChanged (const QString &caption);
private slots:
	void updateNow ();
	void newPageDialogDone (int result);
	void viewKilled ();
private:
	void goInteractive (const QString &prompt);
	bool eventFilter (QObject *watched, QEvent *event);
	void checkSize ();

	QTimer updatetimer;
	QPixmap area;
	QPainter painter;
	QLabel *view;
	QString base_title;
	KDialog *dialog;

	int interaction_opcode;	/**< Current interactive operation (from RKDOpcodes enum), or -1 is there is no current interactive operation */

	QList<StoredEvent> stored_events;
};

#endif
