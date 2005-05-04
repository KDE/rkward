/***************************************************************************
                          rwindowcatcher.h  -  description
                             -------------------
    begin                : Wed May 4 2005
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

#ifndef RKWINDOWCATCHER_H
#define RKWINDOWCATCHER_H

#include <qwidget.h>

/** This class will be used to find out, when R opens a new X11-device, find out the id of that device and embed it into a QWidget.

Unfortunately this does not work, yet. Right when trying to embed the window, there is a crash. Therefore this class is currently deactivated.
To reactivate it, modify the corresponding Makefile.am, uncomment the x11-override in rpackages/rkward/R/internal.R, and uncomment the
window_catcher/RKWindowCatcher lines in rinterface.h and rinterface.cpp. Maybe I'll add some #ifdefs instead...
@author Thomas Friedrichsmeier
*/
class RKWindowCatcher : public QWidget {
	Q_OBJECT
public:
	RKWindowCatcher (QWidget *parent);

	~RKWindowCatcher ();
	
	void start (int prev_cur_device);
	void stop (int new_cur_device);
private:
	int last_cur_device;
};

#endif
