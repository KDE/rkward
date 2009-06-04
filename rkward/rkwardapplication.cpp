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

#ifdef Q_WS_WIN
#	include <windows.h>
#else
#	include <X11/X.h>
#	include <X11/Xlib.h>

#	include <QX11Info>

#	include <kwindowinfo.h>
#	include <netwm_def.h>
//static
Atom wm_name_property;
#endif	//Q_WS_WIN

#include "debug.h"

#warning TODO: We could really use the detection logic from windows for x11, too. It seems much easier.
#ifdef Q_WS_WIN
#include <stdio.h>
namespace RKWardApplicationPrivate {
	QList<WId> toplevel_windows;

	BOOL CALLBACK EnumWindowsCallback (HWND hwnd, LPARAM) {
		toplevel_windows.append (hwnd);
		return true;
	}

	void updateToplevelWindowList () {
		RK_TRACE (APP);

		toplevel_windows.clear ();
		EnumWindows (EnumWindowsCallback, 0);
	};

// TODO: this is a test, only
	BOOL CALLBACK ChildWindowCallback (HWND hwnd, LPARAM) {
		WINDOWINFO info;
		info.cbSize = sizeof (WINDOWINFO);
		GetWindowInfo (hwnd, &info);
qDebug ("%p: rect: %d, %d, %d, %d, client: %d, %d, %d, %d, style: %d, exstyle: %d, status: %d, borders: %d %d", hwnd,
		info.rcWindow.left, info.rcWindow.top, info.rcWindow.right, info.rcWindow.bottom,
		info.rcClient.left, info.rcClient.top, info.rcClient.right, info.rcClient.bottom,
		info.dwStyle, info.dwExStyle, info.dwWindowStatus, info.cxWindowBorders, info.cyWindowBorders);

		return true;
	}

	void showWindowChildren (HWND hwnd) {
		ChildWindowCallback (hwnd, 0);
		EnumChildWindows (hwnd, ChildWindowCallback, 0);
	}
}
#endif	//Q_WS_WIN

//static
RKWardApplication *RKWardApplication::rkapp = 0;

RKWardApplication::RKWardApplication () : KApplication () {
	//RK_TRACE (APP);	// would be called before initialization of debug-level
	RK_ASSERT (!rkapp);

	rkapp = this;
	detect_x11_creations = false;

#ifndef Q_WS_WIN
	wm_name_property = XInternAtom (QX11Info::display (), "WM_NAME", true);
#endif	//nQ_WS_WIN
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

#ifdef Q_WS_WIN
	RKWardApplicationPrivate::updateToplevelWindowList ();
#else	//Q_WS_WIN
	XSelectInput (QX11Info::display (), QX11Info::appRootWindow (), SubstructureNotifyMask);
	syncX ();	// this is to make sure we don't miss out on the window creation (if it happens very early). Testing shows, we really need this.
#endif	//Q_WS_WIN
}

WId RKWardApplication::endWindowCreationDetection () {
	RK_TRACE (APP);
	RK_ASSERT (detect_x11_creations);

#ifdef Q_WS_WIN
	QList<WId> old_windows = RKWardApplicationPrivate::toplevel_windows;
	RKWardApplicationPrivate::updateToplevelWindowList ();
	QList<WId> candidate_windows = RKWardApplicationPrivate::toplevel_windows;

	detect_x11_creations = false;

	// remove all windows that existed before the call to startWindowCreationDetection
	for (int i = 0; i < old_windows.size (); ++i) {
		candidate_windows.removeAll (old_windows[i]);
	}
	// ideally we have a single candidate remaining, now

	// we could do some more checking, e.g. based on whether the window belongs to our
	// own process, and whether it appears to be of a sane size, but for now, we keep
	// things simple.

	if (candidate_windows.size ()) {
		RK_ASSERT (candidate_windows.size () < 2);
		for (int i = 0; i < candidate_windows.size (); ++i) {
			qDebug ("candidate: %d", i);
			RKWardApplicationPrivate::showWindowChildren (candidate_windows[i]);
		}
		return candidate_windows[0];
	}	// else
	return 0;
#else	//Q_WS_WIN
	if (!created_window) {
		// we did not see the window, yet? Maybe the event simply hasn't been processed, yet.
		syncX ();
		processEvents ();
	}

	detect_x11_creations = false;
	XSelectInput (QX11Info::display (), QX11Info::appRootWindow (), NoEventMask);
	return created_window;
#endif	//Q_WS_WIN
}

void RKWardApplication::registerNameWatcher (WId watched, RKMDIWindow *watcher) {
	RK_TRACE (APP);
	RK_ASSERT (!name_watchers_list.contains (watched));

#ifndef Q_WS_WIN
	XSelectInput (QX11Info::display (), watched, PropertyChangeMask);
#endif	//nQ_WS_WIN
	name_watchers_list.insert (watched, watcher);
}

void RKWardApplication::unregisterNameWatcher (WId watched) {
	RK_TRACE (APP);
	RK_ASSERT (name_watchers_list.contains (watched));

#ifndef Q_WS_WIN
	XSelectInput (QX11Info::display (), watched, NoEventMask);
#endif	//nQ_WS_WIN
	name_watchers_list.remove (watched);
}

#ifndef Q_WS_WIN
bool RKWardApplication::x11EventFilter (XEvent *e) {
	if (detect_x11_creations) {
		if (e->type == CreateNotify) {
			if (e->xcreatewindow.parent == QX11Info::appRootWindow ()) {
				KWindowInfo info = KWindowInfo (e->xcreatewindow.window, NET::WMName | NET::WMWindowType);
				// at this point, we used to check, whether this window has some name or another. This heuristic allowed to sieve out helper windows of the window manager. However, since R 2.8.0, sometimes the window is mapped, before it has been given a name.
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
#endif	//nQ_WS_WIN
