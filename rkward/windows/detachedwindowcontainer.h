/***************************************************************************
                          detachedwindowcontainer  -  description
                             -------------------
    begin                : Wed Oct 21 2005
    copyright            : (C) 2005 by Thomas Friedrichsmeier
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

#ifndef DETACHEDWINDOWCONTAINER_H
#define DETACHEDWINDOWCONTAINER_H

#include <kparts/part.h>
#include <kparts/mainwindow.h>

#include <qptrlist.h>

class KMdiChildView;

/**
@author Thomas Friedrichsmeier
*/
class DetachedWindowContainer : public KParts::MainWindow {
	Q_OBJECT
public:
	DetachedWindowContainer (KParts::Part *part_to_capture, KMdiChildView *widget_to_capture);

	~DetachedWindowContainer ();

	static QPtrList<KMdiChildView> *detachedWindows () { return &detached_windows; };
public slots:
	void viewDestroyed (QObject *view);
private:
	static QPtrList<KMdiChildView> detached_windows;
};

#endif
