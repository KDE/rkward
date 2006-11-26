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

#ifndef RKWARDAPPLICATION_H
#define RKWARDAPPLICATION_H

#include <kapplication.h>

#include <qmap.h>

/** The purpose of subclassing KApplication as RKWardApplication, is to obtain raw access to X11 events. This is needed to detect the creation of new toplevel windows (R X11 windows), and changed in the caption of those windows. */

class RKWardApplication : public KApplication {
public:
	RKWardApplication ();
	~RKWardApplication ();

	/** like KApplication::kApplication () (and actually, this should always return the same pointer), but without the need to cast */
	static RKWardApplication *getApp ();

	/** reimplemented from KApplication to look for CreateNotify and PropertyNotify events */
	bool x11EventFilter (XEvent *e);

	/** start looking for new top-level windows created on the screen */
	void startWindowCreationDetection ();
	/** stop looking for new top-level windows created on the screen
	@returns the window id of the last top-level window created after the last call to startWindowCreation, hoping it was only one. 0 if no window was created/detected. */
	WId endWindowCreationDetection ();

	/** watch the given window for changes in its WM_NAME property (i.e. changes in caption). When a change is detected, the caption will be set on watcher. WARNING: Do not use to watch windows managed by Qt! Will override the event mask for this window (within qt_xdisplay ()). WARNING: Remember to call unregisterNameWatcher, when watcher is deleted! */
	void registerNameWatcher (WId watched, QWidget *watcher);
	/** remove a watch created with registerNameWatcher */
	void unregisterNameWatcher (WId watched);
private:
	static RKWardApplication *rkapp;
	bool detect_x11_creations;
	WId created_window;

	QMap<WId, QWidget*> name_watchers_list;
};

#endif
