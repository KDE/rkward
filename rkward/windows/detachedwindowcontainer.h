/***************************************************************************
                          detachedwindowcontainer  -  description
                             -------------------
    begin                : Wed Oct 21 2005
    copyright            : (C) 2005, 2009 by Thomas Friedrichsmeier
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

class RKMDIWindow;
class RKTopLevelWindowGUI;
class QCloseEvent;

/** This class is used to host a (KPart enabled) window detached from the main window. @see RKWorkplace::detachWindow ().

@author Thomas Friedrichsmeier
*/
class DetachedWindowContainer : public KParts::MainWindow {
	Q_OBJECT
public:
/** constructor.
@param widget_to_capture The window to reparent into the detached window */
	explicit DetachedWindowContainer (RKMDIWindow *widget_to_capture);
/** destructor. Usually you don't call this explicitly, but rather delete/close the child view. The DetachedWindowContainer will then self destruct via viewDestroyed () */
	~DetachedWindowContainer ();
public slots:
/** self-destruct, when child view is destroyed */
	void viewDestroyed (QObject *view);
/** re-attach to the main window */
	void slotReattach ();
/** update own caption, when the window's caption has changed */
	void updateCaption (RKMDIWindow *);
	void slotSetStatusBarText (const QString &text);
protected:
/** when receiving a close event, dispatch to the embedded window */
	void closeEvent (QCloseEvent *e);
private:
	RKMDIWindow *captured;
	RKTopLevelWindowGUI *toplevel_actions;
};

#endif
