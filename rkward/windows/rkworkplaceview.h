/***************************************************************************
                          rkworkplaceview  -  description
                             -------------------
    begin                : Tue Sep 26 2006
    copyright            : (C) 2006 by Thomas Friedrichsmeier
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

#ifndef RKWORKPLACEVIEW_H
#define RKWORKPLACEVIEW_H

#include <qwidget.h>
#include <qmap.h>

class QTabBar;
class QWidgetStack;
class RKMDIWindow;

/** The widget containing all the MDI document windows. Right now it mostly acts as a QTabWidget (is not one, as unfortunately, QTabWidget always has a sunken frame), but might be extended to allow switching between different view modes */
class RKWorkplaceView : public QWidget {
	Q_OBJECT
public:
	RKWorkplaceView (QWidget *parent);
	~RKWorkplaceView ();

	void addPage (RKMDIWindow *widget);
	void removePage (RKMDIWindow *widget, bool destroyed=false);

	void setActivePage (RKMDIWindow *widget);
	RKMDIWindow *activePage ();
	QString activeCaption ();
	void setCaption (const QString &caption);
signals:
	void pageChanged (RKMDIWindow *widget);
	void captionChanged (const QString &new_caption);
public slots:
	void setPage (int page);
	void childCaptionChanged (RKMDIWindow *widget);
private:
	QTabBar *tabs;
	QWidgetStack *widgets;
	typedef QMap<int, RKMDIWindow*> PageMap;
	PageMap pages;
	int idOfWidget (RKMDIWindow *widget);
};

#endif
