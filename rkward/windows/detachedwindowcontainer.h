/*
detachedwindowcontainer - This file is part of the RKWard project. Created: Wed Oct 21 2005
SPDX-FileCopyrightText: 2005-2016 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

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
	explicit DetachedWindowContainer (RKMDIWindow *widget_to_capture, bool copy_geometry);
/** destructor. Usually you don't call this explicitly, but rather delete/close the child view. The DetachedWindowContainer will then self destruct via viewDestroyed () */
	~DetachedWindowContainer ();
public Q_SLOTS:
/** self-destruct, when child view is destroyed */
	void viewDestroyed (QObject *view);
/** re-attach to the main window */
	void slotReattach ();
/** update own caption, when the window's caption has changed */
	void updateCaption (RKMDIWindow *);
	void slotSetStatusBarText (const QString &text) override;
/** Hide any emtpy menus.
@param ignore do nothing if true. For internal use, only. */
	void hideEmptyMenus (bool ignore=false);
protected:
/** when receiving a close event, dispatch to the embedded window */
	void closeEvent (QCloseEvent *e) override;
private:
	RKMDIWindow *captured;
	RKTopLevelWindowGUI *toplevel_actions;
};

#endif
