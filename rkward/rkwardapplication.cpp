/***************************************************************************
                          rkwardapplication  -  description
                             -------------------
    begin                : Sun Nov 26 2006
    copyright            : (C) 2006, 2007, 2010 by Thomas Friedrichsmeier
    email                : thomas.friedrichsmeier@kdemail.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "rkwardapplication.h"

#include "windows/rkmdiwindow.h"

#include <KWindowInfo>
#include <KWindowSystem>
#include <KUrlAuthorized>
#include <kurl.h>

#include "debug.h"

//static
RKWardApplication *RKWardApplication::rkapp = 0;

RKWardApplication::RKWardApplication (int argc, char *argv[]) : QApplication (argc, argv) {
	//RK_TRACE (APP);	// would be called before initialization of debug-level
	RK_ASSERT (!rkapp);

	// Don't complain when linking rkward://-pages from Rd pages
	KUrlAuthorized::allowUrlAction ("redirect", KUrl ("http://"), KUrl ("rkward://"));
	// Don't complain when trying to open help pages
	KUrlAuthorized::allowUrlAction ("redirect", KUrl ("rkward://"), KUrl ("help:"));

	rkapp = this;
}

RKWardApplication::~RKWardApplication () {
	RK_TRACE (APP);
}

RKWardApplication *RKWardApplication::getApp () {
	RK_TRACE (APP);
	RK_ASSERT (rkapp == QApplication::instance ());
	return rkapp;
}

void RKWardApplication::startWindowCreationDetection () {
	RK_TRACE (APP);

	created_window = 0;

	connect (KWindowSystem::self (), SIGNAL (windowAdded(WId)), this, SLOT (windowAdded(WId)));
}

WId RKWardApplication::endWindowCreationDetection () {
	RK_TRACE (APP);

	if (!created_window) {
		// we did not see the window, yet? Maybe the event simply hasn't been processed, yet.
		processEvents ();
	}
	disconnect (KWindowSystem::self (), SIGNAL (windowAdded(WId)), this, SLOT (windowAdded(WId)));
	return created_window;
}

void RKWardApplication::windowAdded (WId id) {
	RK_TRACE (APP);

	// KF5 TODO: Note: Previously, on windows we checked for IsWindow (hwnd) and IsWindowVisible (hwnd), as sometimes invisible ghost windows
	// would be created in addition to the real device window. Is this still needed?
	created_window = id;
}

void RKWardApplication::windowChanged (WId id, NET::Properties properties, NET::Properties2 properties2) {
	Q_UNUSED (properties2);
	if (!(properties & NET::WMName)) return;
	RK_TRACE (APP);
	RKMDIWindow *watcher = name_watchers_list.value (id);
	if (!watcher) return;
	watcher->setCaption (KWindowInfo (id, NET::WMName).name ());
}

void RKWardApplication::registerNameWatcher (WId watched, RKMDIWindow *watcher) {
	RK_TRACE (APP);
	RK_ASSERT (!name_watchers_list.contains (watched));

	if (name_watchers_list.isEmpty ()) {
		connect (KWindowSystem::self (), SIGNAL (windowChanged(WId,NET::Properties,NET::Properties2)), this, SLOT (windowChanged(WId,NET::Properties,NET::Properties2)));
	}
	name_watchers_list.insert (watched, watcher);
}

void RKWardApplication::unregisterNameWatcher (WId watched) {
	RK_TRACE (APP);
	RK_ASSERT (name_watchers_list.contains (watched));

	name_watchers_list.remove (watched);
	if (name_watchers_list.isEmpty ()) {
		disconnect (KWindowSystem::self (), SIGNAL (windowChanged(WId,NET::Properties,NET::Properties2)), this, SLOT (windowChanged(WId,NET::Properties,NET::Properties2)));
	}
}
