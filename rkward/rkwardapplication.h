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

/** The purpose of subclassing KApplication as RKWardApplication, is to obtain raw access to X11 events. This is needed to detect the creation of new toplevel windows (R X11 windows). */

class RKWardApplication : public KApplication {
public:
	RKWardApplication ();
	~RKWardApplication ();

	/** like KApplication::kApplication () (and actually, this should always return the same pointer), but without the need to cast */
	static RKWardApplication *getApp ();

	/** reimplemented from KApplication to look for CreateNotify events */
	bool x11EventFilter (XEvent *e);

	void startWindowCreationDetection ();
	WId endWindowCreationDetection ();
private:
	static RKWardApplication *rkapp;
	bool detect_x11_creations;
	WId created_window;
};

#endif
