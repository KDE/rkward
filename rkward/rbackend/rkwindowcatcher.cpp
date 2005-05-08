/***************************************************************************
                          rwindowcatcher.cpp  -  description
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

#include "rkwindowcatcher.h"

#ifndef DISABLE_RKWINDOWCATCHER

#include <kapplication.h>
#include <qxembed.h>

#include <X11/X.h>
#include <X11/Xlib.h>

#include "../debug.h"

// function below is a slightly adapted copy from http://lists.trolltech.com/qt-interest/1999-10/msg00224.html
// this could be tweaked a little more, since for instance we know we're only looking for toplevel windows
Window Window_With_Name (Display *dp, Window top, const char *name) {
    Window *children, dummy;
    unsigned int nchildren;
    int i;
    Window w=0;
    char *window_name;

    if (XFetchName(dp, top, &window_name)) {
		if (QString (window_name).startsWith (name)) return(top);
	}

    if (!XQueryTree(dp, top, &dummy, &dummy, &children, &nchildren))
        return(0);

    for (i=0; i<nchildren; i++) {
        w = Window_With_Name(dp, children[i], name);
        if (w)
            break;
    }
    if (children) XFree ((char *)children);
    return(w);
}


RKWindowCatcher::RKWindowCatcher (QWidget *parent) : QWidget (parent) {
}

RKWindowCatcher::~RKWindowCatcher () {
}

void RKWindowCatcher::catchWindow (const QString &title_start, int corresponding_device_number) {
	Window w = Window_With_Name (qt_xdisplay (), qApp->desktop ()->winId (), title_start.latin1 ());
	qDebug ("Window id is: %x", w);
	if (w) {
		QXEmbed *capture = new QXEmbed (); // (0, 0, Qt::WDestructiveClose);
		capture->setProtocol (QXEmbed::XPLAIN);
// (trying to) work around buggy R X11 event handling (see http://www.ens.gu.edu.au/robertk/R/devel/01a/0077.html)
		extern Display *Rf_getX11Display (); // now, how to link this?

		qDebug ("qt disp: %x r disp: %x", qt_xdisplay (), Rf_getX11Display ());
		
		Atom *old_prots;
		int num_old_prots;
		if (!XGetWMProtocols (Rf_getX11Display (), w, &old_prots, &num_old_prots)) qDebug ("fail 1");
		if (!XSetWMProtocols (Rf_getX11Display (), w, 0, 0)) qDebug ("fail2");
//		capture->embed (w);
		XEvent dummy;
		while (XPending (Rf_getX11Display ())) {
			XNextEvent (Rf_getX11Display (), &dummy);
			qDebug ("event for window: %x", dummy.xany.window);
		}
		XSetWMProtocols (Rf_getX11Display (), w, old_prots, num_old_prots);
		capture->show ();
	}
}

/*
void RKWindowCatcher::start (int prev_cur_device) {
	RK_DO (qDebug ("Window Catcher activated"), RBACKEND, DL_DEBUG);

	last_cur_device = prev_cur_device;
}

void RKWindowCatcher::stop (int new_cur_device) {
	RK_DO (qDebug ("Window Catcher deactivated"), RBACKEND, DL_DEBUG);

	if (new_cur_device != last_cur_device) {
		QString dummy = "R Graphics: Device ";
		//dummy.append (QString::number (new_cur_device));
		//dummy.append (" ");
		Window w = Window_With_Name (qt_xdisplay (), qApp->desktop ()->winId (), dummy.latin1 ());
		qDebug ("Window id is: %x", w);
		if (w) {
			QXEmbed *capture = new QXEmbed (); // (0, 0, Qt::WDestructiveClose);
			capture->setProtocol (QXEmbed::XPLAIN);
			capture->embed (w);
			capture->show ();
		}
	}
	last_cur_device = new_cur_device;
} */

#include "rkwindowcatcher.moc"

#endif // DISABLE_RKWINDOWCATCHER
