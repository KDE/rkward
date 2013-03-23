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

class QGraphicsRectItem;
class QGraphicsView;
class QGraphicsScene;

/** This is the class that actually does all the drawing for the RKGraphicsDevice */
class RKGraphicsDevice : public QObject {
protected:
	RKGraphicsDevice (double width, double height);
	~RKGraphicsDevice ();
public:
	static RKGraphicsDevice* newDevice (int devnum, double width, double height);
	static void closeDevice (int devnum);
	static QHash<int, RKGraphicsDevice*> devices;

	void circle (double x, double y, double r, const QPen& pen, const QBrush& brush);
	void line (double x1, double y1, double x2, double y2, const QPen& pen);
	double strWidth (const QString &text, const QFont& font);
	void text (double x, double y, const QString &text, double rot, double hadj, const QColor& col, const QFont& font);
	void setClip (const QRectF& new_clip);
private:
	QGraphicsScene* scene;
	QGraphicsView* view;
	QGraphicsRectItem* clip;
	double clip_z;
	double item_z;
};

#endif
