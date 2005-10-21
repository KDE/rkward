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

#include <qlayout.h>

#include "../rkward.h"
#include "../rkglobals.h"
#include "../debug.h"

//static
QPtrList<KMdiChildView> DetachedWindowContainer::detached_windows;

DetachedWindowContainer::DetachedWindowContainer (KParts::Part *part_to_capture, KMdiChildView *widget_to_capture) : KParts::MainWindow  (RKGlobals::rkApp ()) {
	RK_TRACE (APP);

	setXMLFile ("adslk");

	widget_to_capture->reparent (this, QPoint (0, 0));
	setCentralWidget (widget_to_capture);
	setCaption (widget_to_capture->caption ());	
	createGUI (part_to_capture);

	detached_windows.append (widget_to_capture);
	connect (widget_to_capture, SIGNAL (destroyed (QObject *)), this, SLOT (viewDestroyed (QObject *)));
}

DetachedWindowContainer::~DetachedWindowContainer () {
	RK_TRACE (APP);

	detached_windows.remove (static_cast<KMdiChildView*> (centralWidget ()));
}

void DetachedWindowContainer::viewDestroyed (QObject *view) {
	RK_TRACE (APP);

	delete this;
}

#include "detachedwindowcontainer.moc"
