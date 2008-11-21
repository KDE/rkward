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

/** This is a simple helper class helping in catching R X11 device windows. The start () and stop () functions are called from RInterface, and then this class takes care of handling those.

The main difficulty to overcome in this context, is to find out, when an R X11 window is created, and what is its X Window id. The notes below are some thoughts on that matter. Probably mostly obsolete, now (the current approach is basically Plan C, and seems to work ok), but maybe Plans A or B or something similar will become necessary some day:

Catch R X11 device windows
- Plan A:
	- initialization function seems to be in_do_X11
	- it might be possible to put a wrapper around this using R_setX11Routines
	- this wrapper could watch the list of devices (curDevice, numDevices), see also addDevice to find out how the list is kept internally
	- if a new device gets added grab its winId and capture
- Plan B:
	- it looks like there's no way to get access to R_setX11Routines or at least the needed struct R_X11Routines. (?)
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
	- we may catch this using KApplication::x11EventFilter
	- event filter should only be active during the wrapper (Plan A-C)
	- event filter should probably do some sanity checking
	- this should give us the window id corresponding to the x11-call

@author Thomas Friedrichsmeier
*/
class RKWindowCatcher {
public:
/** ctor. Probably you'll only ever need one instance of RKWindowCatcher. */
	RKWindowCatcher ();
/** dtor */
	~RKWindowCatcher ();

/** call this function to start looking out for new R X11 device windows.
@param prev_cur_device the device number that was active before a new device window was (potentially) created */
	void start (int prev_cur_device);
/** end looking out for new R X11 windows. If a new window was in fact created, this is captured by creating an RKCaughtX11Window
@param new_cur_device the new active device number, i.e. the device number of the created window */
	void stop (int new_cur_device);
private:
	int last_cur_device;
};


#include "rkmdiwindow.h"

class RKCaughtX11WindowPart;
class KToggleAction;
class QXEmbedCopy;
class QScrollArea;
class KVBox;
class RKProgressControl;
class QX11EmbedContainer;

/** An R X11 device window managed by rkward. */
class RKCaughtX11Window : public RKMDIWindow {
	Q_OBJECT
public:
/** ctor
@param window_to_embed the Window id of the R X11 device window to embed
@param device_number the device number corresponding to that window */
	RKCaughtX11Window (WId window_to_embed, int device_number);
/** dtor */
	~RKCaughtX11Window ();
/** TODO */
	QString getDescription () { return "x11:unimplemented"; };
/** TODO? */
	bool isModified () { return false; };

/** reimplemented from RKMDIWindow to switch to fixed size mode, and disable the dynamic_size_action */
	void prepareToBeAttached ();
/** see prepareToBeAttached (). Reenable the dynamic_size_action */
	void prepareToBeDetached ();
public slots:
/** Fixed size action was (potentially) toggled. Update to the new state */
	void fixedSizeToggled ();
/** Switch to fixed size mode, and set size1 (currently 500*500) */
	void setFixedSize1 ();
/** Switch to fixed size mode, and set size2 (currently 1000*1000) */
	void setFixedSize2 ();
/** Switch to fixed size mode, and set size3 (currently 2000*2000) */
	void setFixedSize3 ();
/** Switch to fixed size mode, and set user specified size (size read from a dialog) */
	void setFixedSizeManual ();

	void activateDevice ();
	void copyDeviceToOutput ();
	void printDevice ();
	void copyDeviceToRObject ();
	void duplicateDevice ();

private slots:
	void doEmbed ();
private:
	friend class RKCaughtX11WindowPart;	// needs access to the actions
	int device_number;
	WId embedded;
	KVBox *xembed_container;
	QScrollArea *scroll_widget;
	KVBox *box_widget;
	RKProgressControl *error_dialog;
	QX11EmbedContainer *capture;

	bool dynamic_size;
	KToggleAction *dynamic_size_action;
};

/** Provides a KPart interface for RKCaughtX11Window. */
class RKCaughtX11WindowPart : public KParts::Part {
public:
/** constructor.
@param window The RKCatehdX11Window for this part */
	explicit RKCaughtX11WindowPart (RKCaughtX11Window *window);
/** destructor */
	~RKCaughtX11WindowPart ();
private:
	RKCaughtX11Window *window;
};

#endif //DISABLE_RKWINDOWCATCHER
#endif
