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

#include "detachedwindowcontainer.h"

#include <kmdichildview.h>
#include <klocale.h>
#include <kstdaction.h>

#include <qlayout.h>

#include "../rkward.h"
#include "../rkglobals.h"
#include "../debug.h"

//static
QPtrList<KMdiChildView> DetachedWindowContainer::detached_windows;

DetachedWindowContainer::DetachedWindowContainer (KParts::Part *part_to_capture, KMdiChildView *widget_to_capture) : KParts::MainWindow  (RKGlobals::rkApp ()) {
	RK_TRACE (APP);

// create own GUI
	setXMLFile ("detachedwindowcontainer.rc");
	KStdAction::close (widget_to_capture, SLOT (close ()), actionCollection (), "dwindow_close");
	new KAction (i18n ("Attach to main window"), 0, this, SLOT (slotReattach ()), actionCollection (), "dwindow_attach");
	createShellGUI ();

// capture widget
	part = part_to_capture;
	widget_to_capture->reparent (this, QPoint (0, 0));
	setCentralWidget (widget_to_capture);
	setCaption (widget_to_capture->caption ());	
	createGUI (part_to_capture);

// add widget to list of detached windows
	detached_windows.append (widget_to_capture);
// should self-destruct, when child widget is destroyed
	connect (widget_to_capture, SIGNAL (destroyed (QObject *)), this, SLOT (viewDestroyed (QObject *)));
}

DetachedWindowContainer::~DetachedWindowContainer () {
	RK_TRACE (APP);

	detached_windows.remove (static_cast<KMdiChildView*> (centralWidget ()));
}

void DetachedWindowContainer::viewDestroyed (QObject *) {
	RK_TRACE (APP);

	delete this;
}

void DetachedWindowContainer::slotReattach () {
	RK_TRACE (APP);

	KMdiChildView *view = static_cast<KMdiChildView *> (centralWidget ());

	view->reparent (0, QPoint (0, 0));
	RKGlobals::rkApp ()->addWindow (view);
	RKGlobals::rkApp ()->m_manager->addPart (part);
	view->show ();
	view->setFocus ();
	delete this;
}

#include "detachedwindowcontainer.moc"
