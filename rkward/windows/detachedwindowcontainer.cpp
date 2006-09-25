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
#include "../misc/rkworkplace.h"
#include "../rkglobals.h"
#include "../debug.h"

DetachedWindowContainer::DetachedWindowContainer (KParts::Part *part_to_capture, QWidget *widget_to_capture) : KParts::MainWindow  (RKGlobals::rkApp ()) {
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
	createGUI (part_to_capture);

// should self-destruct, when child widget is destroyed
	connect (widget_to_capture, SIGNAL (destroyed (QObject *)), this, SLOT (viewDestroyed (QObject *)));
	connect (widget_to_capture, SIGNAL (captionChanged (QWidget *)), this, SLOT (updateCaption (QWidget *)));
	setCaption (widget_to_capture->caption ());	// has to come after createGUI!
}

DetachedWindowContainer::~DetachedWindowContainer () {
	RK_TRACE (APP);
}

void DetachedWindowContainer::viewDestroyed (QObject *) {
	RK_TRACE (APP);

	delete this;
}

void DetachedWindowContainer::updateCaption (QWidget *widget) {
	RK_TRACE (APP);
	RK_ASSERT (widget = centralWidget ());

	setCaption (widget->caption ());
}

void DetachedWindowContainer::slotReattach () {
	RK_TRACE (APP);

	QWidget *window = centralWidget ();
	window->reparent (0, QPoint (0, 0));
	RKWorkplace::mainWorkplace ()->attachWindow (window);

	delete this;
}

#include "detachedwindowcontainer.moc"
