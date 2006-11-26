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
#include <qlayout.h>
#include <qvbox.h>

#include <X11/X.h>
#include <X11/Xlib.h>

#include "../windows/qxembedcopy.h"
#include "../debug.h"

static int get_parent(WId winid, Window *out_parent)
{
    Window root, *children=0;
    unsigned int nchildren;
    int st = XQueryTree(qt_xdisplay(), winid, &root, out_parent, &children, &nchildren);
    if (st && children) 
        XFree(children);
    return st;
}

// function below is a slightly adapted copy from http://lists.trolltech.com/qt-interest/1999-10/msg00224.html
// this could be tweaked a little more, since for instance we know we're only looking for toplevel windows
Window Window_With_Name (Display *dp, Window top, const char *name, int level=10) {
    if (level < 0) return (0);
    Window *children, dummy;
    unsigned int nchildren;
    int i;
    Window w=0;
    char *window_name;

    if (XFetchName(dp, top, &window_name)) {
		if (QString (window_name).startsWith (name)) {
			Window parent=0;
			if (get_parent (top, &parent)) {
				XFetchName (dp, parent, &window_name);
				qDebug ("parent %x, name: %s", parent, window_name);
			}
			return (top);
		}
	}

    if (!XQueryTree(dp, top, &dummy, &dummy, &children, &nchildren))
        return(0);

    for (i=0; i<nchildren; i++) {
        w = Window_With_Name(dp, children[i], name, level-1);
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

void RKWindowCatcher::windowLost () {
	qDebug ("lost");
	deleteLater ();
}

void RKWindowCatcher::catchWindow (const QString &title_start, int corresponding_device_number) {
	Window w = Window_With_Name (qt_xdisplay (), DefaultRootWindow (qt_xdisplay()), title_start.latin1 ());
	qDebug ("Window id is: %x (root: %x)", w, DefaultRootWindow (qt_xdisplay()));
	if (w) {
		QWidget *test1 = new QWidget (0);
		test1->setCaption ("1");
		QVBoxLayout *layout = new QVBoxLayout (test1);
		QXEmbedCopy *capture = new QXEmbedCopy (test1, 0, Qt::WDestructiveClose);
		connect (capture, SIGNAL (embeddedWindowDestroyed ()), this, SLOT (windowLost ()));
		capture->setProtocol (QXEmbedCopy::XPLAIN);
		capture->embed (w);
		capture->show ();
		layout->addWidget (capture);
		test1->show ();
		QVBox *test2 = new QVBox (0);
		test2->setCaption ("2");
		test1->reparent (test2, QPoint (0, 0), true);
		test2->show ();
	}
}

void RKWindowCatcher::start (int prev_cur_device) {
	RK_DO (qDebug ("Window Catcher activated"), RBACKEND, DL_DEBUG);

	last_cur_device = prev_cur_device;
	new_windows.clear ();
}

void RKWindowCatcher::stop (int new_cur_device) {
	RK_DO (qDebug ("Window Catcher deactivated"), RBACKEND, DL_DEBUG);
/*
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
	last_cur_device = new_cur_device; */
}

#include "rkwindowcatcher.moc"

#endif // DISABLE_RKWINDOWCATCHER
