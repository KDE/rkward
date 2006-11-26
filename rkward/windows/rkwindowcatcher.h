/***************************************************************************
                          rwindowcatcher.h  -  description
                             -------------------
    begin                : Wed May 4 2005
    copyright            : (C) 2005, 2006 by Thomas Friedrichsmeier
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

//#define DISABLE_RKWINDOWCATCHER
#ifndef DISABLE_RKWINDOWCATCHER

#include <qwidget.h>

/** This class will be used to find out, when R opens a new X11-device, find out the id of that device and embed it into a QWidget.

Unfortunately this does not work, yet. Right when trying to embed the window, there is a crash (R segfaults; this also happens when trying to catch a window from another separate R process, so it really seems to be R's fault!). Therefore this class is currently deactivated.
To reactivate it, modify the corresponding Makefile.am, uncomment the x11-override in rpackages/rkward/R/internal.R, and uncomment the
window_catcher/RKWindowCatcher lines in rinterface.h and rinterface.cpp. Maybe I'll add some #ifdefs instead...

Given the problems with fetching the window Id (currently only possible by searching for the window title), it's probably a lot easier to instead create a single R call: .rk.catch.window (title="R Graphics: Device...", graph_device_number) that will directly trigger catching.


Here are some more notes I have taken on the subject of catching R's x11 windows. Current approach is Plan C for simplicity.
- Plan A:
	- initialization function seems to be in_do_X11
	- it might be possible to put a wrapper around this using R_setX11Routines
	- this wrapper could watch the list of devices (curDevice, numDevices), see also addDevice to find out how the list is kept internally
	- if a new device gets added grab its winId and capture (using QXEmbed?)!
- Plan B:
	- it looks like there's no way to get acces to R_setX11Routines or at least the needed struct R_X11Routines. (?)
	- the level above that seems to be do_X11
	- maybe we can modify the mapping from .Internal (X11) to do_X11 and insert wrapper from Plan A -> R_FunTab
	- proceed like in Plan A
	- less preferable as C-plugins might be able to call do_X11 directly (can they?)
- Plan C:
	- modify at R level (override X11 ())
		- notify app right before device is created
		- notify app right after device is created
	- least preferable solution as we can not be sure we catch every use.
		- but definitely most. This is dispatched via CurrentDevice ()->options("device"), and then evalued in R_GlobalEnv
- remaining problem: how to get the window id given the device id?
	- http://tronche.com/gui/x/xlib/events/window-state-change/create.html#XCreateWindowEvent
	- for active / inactive: XPropertyEvent WM_NAME
	- we may catch this using KApplication::installX11EventFilter
		- XSelectInput -> QWidget::x11Event()?
	- event filter should only be active during the wrapper (Plan A-C)
	- event filter should probably do some sanity checking
	- this should give us the window id corresponding to the x11-call

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

#include "rkmdiwindow.h"

class RKCatchedX11WindowPart;

/** An R X11 device window managed by rkward */
class RKCatchedX11Window : public RKMDIWindow {
	Q_OBJECT
public:
	RKCatchedX11Window (WId window_to_embed, int device_number);

	~RKCatchedX11Window ();

	KParts::Part *getPart ();
	QString getRDescription () { return "unimplemtend"; };
	bool isModified () { return false; };
private:
	int device_number;
	WId embedded;
	RKCatchedX11WindowPart *part;
};

class RKCatchedX11WindowPart : public KParts::Part {
	Q_OBJECT
public:
/** constructor.
@param console The console for this part */
	RKCatchedX11WindowPart (RKCatchedX11Window *window);
/** destructor */
	~RKCatchedX11WindowPart ();
public slots:
// TODO
private:
	RKCatchedX11Window *window;
};

#endif //DISABLE_RKWINDOWCATCHER
#endif
