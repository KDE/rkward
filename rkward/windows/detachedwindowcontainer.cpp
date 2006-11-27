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

#include <klocale.h>
#include <kstdaction.h>

#include <qlayout.h>
#include <qwidget.h>

#include "../rkward.h"
#include "rkworkplace.h"
#include "../rkglobals.h"
#include "../debug.h"

DetachedWindowContainer::DetachedWindowContainer (RKMDIWindow *widget_to_capture) : KParts::MainWindow  (RKWardMainWindow::getMain ()) {
	RK_TRACE (APP);

	setHelpMenuEnabled (false);
// create own GUI
	setXMLFile ("detachedwindowcontainer.rc");
	KStdAction::close (this, SLOT (close ()), actionCollection (), "dwindow_close");
	new KAction (i18n ("Attach to main window"), 0, this, SLOT (slotReattach ()), actionCollection (), "dwindow_attach");
	RKWardMainWindow::getMain ()->makeRKWardHelpMenu (this, actionCollection ());
	createShellGUI ();

// capture widget
	setGeometry (widget_to_capture->frameGeometry ());
	widget_to_capture->reparent (this, QPoint (0, 0));
	setCentralWidget (widget_to_capture);
	createGUI (widget_to_capture->getPart ());

// should self-destruct, when child widget is destroyed
	connect (widget_to_capture, SIGNAL (destroyed (QObject *)), this, SLOT (viewDestroyed (QObject *)));
	connect (widget_to_capture, SIGNAL (captionChanged (RKMDIWindow *)), this, SLOT (updateCaption (RKMDIWindow *)));
	setCaption (widget_to_capture->fullCaption ());	// has to come after createGUI!
}

DetachedWindowContainer::~DetachedWindowContainer () {
	RK_TRACE (APP);
}

void DetachedWindowContainer::viewDestroyed (QObject *) {
	RK_TRACE (APP);

	hide ();
	deleteLater ();
}

void DetachedWindowContainer::updateCaption (RKMDIWindow *widget) {
	RK_TRACE (APP);
	RK_ASSERT (widget == centralWidget ());

	setCaption (widget->fullCaption ());
}

void DetachedWindowContainer::slotReattach () {
	RK_TRACE (APP);

	RKMDIWindow *window = static_cast<RKMDIWindow *> (centralWidget ());
// we will not handle any more signals from the window
	disconnect (window, SIGNAL (destroyed (QObject *)), this, SLOT (viewDestroyed (QObject *)));
	disconnect (window, SIGNAL (captionChanged (RKMDIWindow *)), this, SLOT (updateCaption (RKMDIWindow *)));

	window->reparent (0, QPoint (0, 0));
	RKWorkplace::mainWorkplace ()->attachWindow (window);

	hide ();
	deleteLater ();
}

void DetachedWindowContainer::closeEvent (QCloseEvent *e) {
	RK_TRACE (APP);

	RKMDIWindow *window = static_cast<RKMDIWindow *> (centralWidget ());
	if (window->close ()) {
		e->accept ();
	} else {
		e->ignore ();
	}
}


#include "detachedwindowcontainer.moc"
