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

class QWidget;

/** This class can be used host a (part) window detached from the main window. @see RKwardApp::slotDetachWindow ().

@author Thomas Friedrichsmeier
*/
class DetachedWindowContainer : public KParts::MainWindow {
	Q_OBJECT
public:
/** constructor.
@param part_to_capture The part to use to create the GUI in the detached window
@param widget_to_capture The view to reparent into the detached window */
	DetachedWindowContainer (KParts::Part *part_to_capture, QWidget *widget_to_capture);
/** destructor. Usually you don't call this explicitely, but rather delete/close the child view. The DetachedWindowContainer will then self destruct */
	~DetachedWindowContainer ();

public slots:
/** self-destruct, when child view is destroyed */
	void viewDestroyed (QObject *view);
/** re-attach to the main window */
	void slotReattach ();
	void updateCaption (QWidget *);
private:
	KParts::Part *part;
};

#endif
