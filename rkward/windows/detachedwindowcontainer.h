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

/** This class can be used host a (part) window detached from the main window. @see RKwardApp::slotDetachWindow ().

@author Thomas Friedrichsmeier
*/
class DetachedWindowContainer : public KParts::MainWindow {
	Q_OBJECT
public:
/** constructor.
@param part_to_capture The part to use to create the GUI in the detached window
@param widget_to_capture The view to reparent into the detached window */
	DetachedWindowContainer (KParts::Part *part_to_capture, KMdiChildView *widget_to_capture);
/** destructor. Usually you don't call this explicitely, but rather delete/close the child view. The DetachedWindowContainer will then self destruct */
	~DetachedWindowContainer ();

/** static list of all detached windows */
	static QPtrList<KMdiChildView> *detachedWindows () { return &detached_windows; };
signals:
	void detached (KMdiChildView *widget);
	void reattached (KMdiChildView *widget);
public slots:
/** self-destruct, when child view is destroyed */
	void viewDestroyed (QObject *view);
/** re-attach to the main window */
	void slotReattach ();
private:
	static QPtrList<KMdiChildView> detached_windows;
	KParts::Part *part;
};

#endif
