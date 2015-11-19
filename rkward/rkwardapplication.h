/***************************************************************************
                          rkwardapplication  -  description
                             -------------------
    begin                : Sun Nov 26 2006
    copyright            : (C) 2006, 2009 by Thomas Friedrichsmeier
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

#ifndef RKWARDAPPLICATION_H
#define RKWARDAPPLICATION_H

#include <QApplication>
#include <netwm_def.h>

#include <qmap.h>

class RKMDIWindow;

/** The purpose of subclassing KApplication as RKWardApplication, is to obtain raw access to X11 events. This is needed to detect the creation of new toplevel windows (R X11 windows), and changed in the caption of those windows.
 * 
 * KF5 TODO: This class should no longer be needed, but should be moved to window catcher (if the code that is in it now, works)
*/

class RKWardApplication : public QApplication {
	Q_OBJECT
public:
	RKWardApplication (int argc, char *argv[]);
	~RKWardApplication ();

	/** like KApplication::kApplication () (and actually, this should always return the same pointer), but without the need to cast */
	static RKWardApplication *getApp ();

	/** start looking for new top-level windows created on the screen */
	void startWindowCreationDetection ();
	/** stop looking for new top-level windows created on the screen
	@returns the window id of the last top-level window created after the last call to startWindowCreation, hoping it was only one. 0 if no window was created/detected. */
	WId endWindowCreationDetection ();

	/** watch the given window for changes in its WM_NAME property (i.e. changes in caption). When a change is detected, the caption will be set on watcher. WARNING: Do not use to watch windows managed by Qt! WARNING: Remember to call unregisterNameWatcher, when watcher is deleted! */
	void registerNameWatcher (WId watched, RKMDIWindow *watcher);
	/** remove a watch created with registerNameWatcher */
	void unregisterNameWatcher (WId watched);
public slots:
	void windowAdded (WId id);
	void windowChanged (WId id, NET::Properties properties, NET::Properties2 properties2);
private:
	static RKWardApplication *rkapp;
	WId created_window;

	QMap<WId, RKMDIWindow*> name_watchers_list;
};

#endif
