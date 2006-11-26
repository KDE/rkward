/***************************************************************************
                          rwindowcatcher.cpp  -  description
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

#include "rkwindowcatcher.h"

#ifndef DISABLE_RKWINDOWCATCHER

#include <qlayout.h>
#include <qvbox.h>

#include <kmessagebox.h>
#include <klocale.h>
#include <kwin.h>

#include "../rkwardapplication.h"
#include "rkworkplace.h"
#include "qxembedcopy.h"
#include "../debug.h"

RKWindowCatcher::RKWindowCatcher (QWidget *parent) : QWidget (parent) {
	RK_TRACE (MISC);
}

RKWindowCatcher::~RKWindowCatcher () {
	RK_TRACE (MISC);
}

void RKWindowCatcher::start (int prev_cur_device) {
	RK_TRACE (MISC);
	RK_DO (qDebug ("Window Catcher activated"), RBACKEND, DL_DEBUG);

	RKWardApplication::getApp ()->startWindowCreationDetection ();
	last_cur_device = prev_cur_device;
}

void RKWindowCatcher::stop (int new_cur_device) {
	RK_TRACE (MISC);
	RK_DO (qDebug ("Window Catcher deactivated"), RBACKEND, DL_DEBUG);

	if (new_cur_device != last_cur_device) {
		WId w = RKWardApplication::getApp ()->endWindowCreationDetection ();

		if (w) {
			RKWorkplace::mainWorkplace ()->newX11Window (w, new_cur_device);
			//new RKCatchedX11Window (w, new_cur_device);
		} else {
			KMessageBox::sorry (0, i18n ("You have created a new X11 device window in R. Usually, RKWard tries to detect such windows, to take control of them, and add a menu-bar to them. This time, however, RKWard failed to detect, which window was created, and so can not embed it.\nIf you created the window on a different screen or X11 display, that is to be expected. You might want to consider changing options(\"display\"), then.\nIf you can see the X11 window on the same screen as this message, then RKWard should do better. In this case, please contact us at rkward-devel@lists.sourceforge.net with details on your setup, so we can try to fix this in future versions of RKWard."), i18n ("Could not embed R X11 window"));
		}
	}
	last_cur_device = new_cur_device;
}




RKCatchedX11Window::RKCatchedX11Window (WId window_to_embed, int device_number) : RKMDIWindow (0, X11Window) {
	RK_TRACE (MISC);

	part = new RKCatchedX11WindowPart (this);
	setFocusPolicy (QWidget::ClickFocus);

	embedded = window_to_embed;
	RKCatchedX11Window::device_number = device_number;

	QVBoxLayout *layout = new QVBoxLayout (this);
	QXEmbedCopy *capture = new QXEmbedCopy (this);
	capture->setProtocol (QXEmbedCopy::XPLAIN);
	connect (capture, SIGNAL (embeddedWindowDestroyed ()), this, SLOT (deleteLater ()));
	layout->addWidget (capture);

	KWin::WindowInfo wininfo = KWin::windowInfo (window_to_embed);
	setGeometry (wininfo.frameGeometry ());
	setCaption (wininfo.name ());
	capture->embed (window_to_embed);

	RKWardApplication::getApp ()->registerNameWatcher (window_to_embed, this);

	show ();
}

RKCatchedX11Window::~RKCatchedX11Window () {
	RK_TRACE (MISC);

	RKWardApplication::getApp ()->unregisterNameWatcher (embedded);
}

KParts::Part *RKCatchedX11Window::getPart () {
	RK_TRACE (MISC);

	return part;
}




RKCatchedX11WindowPart::RKCatchedX11WindowPart (RKCatchedX11Window *window) : KParts::Part (0) {
	RK_TRACE (MISC);

	KInstance* instance = new KInstance ("rkward");
	setInstance (instance);

	setWidget (window);
	RKCatchedX11WindowPart::window = window;

	setXMLFile ("rkcatchedx11windowpart.rc");
}

RKCatchedX11WindowPart::~RKCatchedX11WindowPart () {
	RK_TRACE (MISC);
}

#include "rkwindowcatcher.moc"

#endif // DISABLE_RKWINDOWCATCHER
