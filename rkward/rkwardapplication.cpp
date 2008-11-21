/***************************************************************************
                          rkwardapplication  -  description
                             -------------------
    begin                : Sun Nov 26 2006
    copyright            : (C) 2006, 2007 by Thomas Friedrichsmeier
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

#include "rkwardapplication.h"

#include "windows/rkmdiwindow.h"

#include <X11/X.h>
#include <X11/Xlib.h>

#include <QX11Info>

#include <kwindowinfo.h>
#include <netwm_def.h>

#include "debug.h"

//static
RKWardApplication *RKWardApplication::rkapp = 0;
Atom wm_name_property;

RKWardApplication::RKWardApplication () : KApplication () {
	//RK_TRACE (APP);	// would be called before initialization of debug-level
	RK_ASSERT (!rkapp);

	rkapp = this;
	detect_x11_creations = false;

	wm_name_property = XInternAtom (QX11Info::display (), "WM_NAME", true);
}

RKWardApplication::~RKWardApplication () {
	RK_TRACE (APP);
}

RKWardApplication *RKWardApplication::getApp () {
	RK_TRACE (APP);
	RK_ASSERT (rkapp == KApplication::kApplication ());
	return rkapp;
}

void RKWardApplication::startWindowCreationDetection () {
	RK_TRACE (APP);
	RK_ASSERT (!detect_x11_creations);

	created_window = 0;
	detect_x11_creations = true;

	XSelectInput (QX11Info::display (), QX11Info::appRootWindow (), SubstructureNotifyMask);
	syncX ();	// this is to make sure we don't miss out on the window creation (if it happens very early). Testing shows, we really need this.
}

WId RKWardApplication::endWindowCreationDetection () {
	RK_TRACE (APP);
	RK_ASSERT (detect_x11_creations);

	if (!created_window) {
		// we did not see the window, yet? Maybe the event simply hasn't been processed, yet.
		syncX ();
		processEvents ();
	}

	detect_x11_creations = false;
	XSelectInput (QX11Info::display (), QX11Info::appRootWindow (), NoEventMask);
	return created_window;
}

void RKWardApplication::registerNameWatcher (WId watched, RKMDIWindow *watcher) {
	RK_TRACE (APP);
	RK_ASSERT (!name_watchers_list.contains (watched));

	XSelectInput (QX11Info::display (), watched, PropertyChangeMask);
	name_watchers_list.insert (watched, watcher);
}

void RKWardApplication::unregisterNameWatcher (WId watched) {
	RK_TRACE (APP);
	RK_ASSERT (name_watchers_list.contains (watched));

	XSelectInput (QX11Info::display (), watched, NoEventMask);
	name_watchers_list.remove (watched);
}

bool RKWardApplication::x11EventFilter (XEvent *e) {
	if (detect_x11_creations) {
		if (e->type == CreateNotify) {
			if (e->xcreatewindow.parent == QX11Info::appRootWindow ()) {
				KWindowInfo info = KWindowInfo (e->xcreatewindow.window, NET::WMName | NET::WMWindowType);
				// at this point, we used to check, whether this window has some name or another. This heuristic allowed to sieve out helper windows of the window manager. However, since R 2.8.0, sometimes the window is mapped, before it has been give a name.
				// Now we rely on the fact (we hope it *is* a fact), that the device window is always the first one created.
				if ((info.windowType (0xFFFF) != 0) && (!created_window)) {
					created_window = e->xcreatewindow.window;
					return true;
				}
			} else {
				RK_ASSERT (false);
			}
		}
	}

	if (e->type == PropertyNotify) {
		if (e->xproperty.atom == wm_name_property) {
			if (name_watchers_list.contains (e->xproperty.window)) {
				KWindowInfo wininfo = KWindowInfo (e->xproperty.window, NET::WMName);
				name_watchers_list[e->xproperty.window]->setCaption (wininfo.name ());
				return true;
			}
		}
	}

	return KApplication::x11EventFilter (e);
}

