/***************************************************************************
                          rkwardapplication  -  description
                             -------------------
    begin                : Sun Nov 26 2006
    copyright            : (C) 2006 by Thomas Friedrichsmeier
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

#include <X11/X.h>
#include <X11/Xlib.h>

#include <kwin.h>
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

	wm_name_property = XInternAtom (qt_xdisplay (), "WM_NAME", true);
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

	XSelectInput (qt_xdisplay (), qt_xrootwin (), SubstructureNotifyMask);
}

WId RKWardApplication::endWindowCreationDetection () {
	RK_TRACE (APP);
	RK_ASSERT (detect_x11_creations);

	detect_x11_creations = false;
	XSelectInput (qt_xdisplay (), qt_xrootwin (), NoEventMask);
	return created_window;
}

void RKWardApplication::registerNameWatcher (WId watched, QWidget *watcher) {
	RK_TRACE (APP);
	RK_ASSERT (!name_watchers_list.contains (watched));

	XSelectInput (qt_xdisplay (), watched, PropertyChangeMask);
	name_watchers_list.insert (watched, watcher);
}

void RKWardApplication::unregisterNameWatcher (WId watched) {
	RK_TRACE (APP);
	RK_ASSERT (name_watchers_list.contains (watched));

	XSelectInput (qt_xdisplay (), watched, NoEventMask);
	name_watchers_list.remove (watched);
}

bool RKWardApplication::x11EventFilter (XEvent *e) {
	if (detect_x11_creations) {
		if (e->type == CreateNotify) {
			if (e->xcreatewindow.parent == qt_xrootwin ()) {
				KWin::WindowInfo info = KWin::windowInfo (e->xcreatewindow.window);
				if ((info.windowType (0xFFFF) != 0) && (!info.name ().isEmpty ())) {
					RK_ASSERT (!created_window);
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
				KWin::WindowInfo wininfo = KWin::windowInfo (e->xproperty.window);
				name_watchers_list[e->xproperty.window]->setCaption (wininfo.name ());
				return true;
			}
		}
	}

	return KApplication::x11EventFilter (e);
}

