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

#include <qlayout.h>
#include <qvbox.h>

#include <kmessagebox.h>
#include <klocale.h>
#include <kwin.h>

#include <X11/X.h>
#include <X11/Xlib.h>

#include "../rkwardapplication.h"
#include "../windows/qxembedcopy.h"
#include "../debug.h"

RKWindowCatcher::RKWindowCatcher (QWidget *parent) : QWidget (parent) {
}

RKWindowCatcher::~RKWindowCatcher () {
}

void RKWindowCatcher::start (int prev_cur_device) {
	RK_DO (qDebug ("Window Catcher activated"), RBACKEND, DL_DEBUG);

	RKWardApplication::getApp ()->startWindowCreationDetection ();
	last_cur_device = prev_cur_device;
}

void RKWindowCatcher::stop (int new_cur_device) {
	RK_DO (qDebug ("Window Catcher deactivated"), RBACKEND, DL_DEBUG);

	if (new_cur_device != last_cur_device) {
		WId w = RKWardApplication::getApp ()->endWindowCreationDetection ();

		qDebug ("Window id is: %x", w);
		if (w) {
			QXEmbedCopy *capture = new QXEmbedCopy (0, 0, Qt::WDestructiveClose);
			capture->setProtocol (QXEmbedCopy::XPLAIN);
			connect (capture, SIGNAL (embeddedWindowDestroyed ()), capture, SLOT (deleteLater ()));

			KWin::WindowInfo wininfo = KWin::windowInfo (w);
			capture->setGeometry (wininfo.frameGeometry ());
			capture->embed (w);

			capture->show ();
		} else {
			KMessageBox::sorry (0, i18n ("You have created a new X11 device window in R. Usually, RKWard tries to detect such windows, to take control of them, and add a menu-bar to them. This time, however, RKWard failed to detect, which window was created, and so can not embed it.\nIf you created the window on a different screen or X11 display, that is to be expected. You might want to consider changing options(\"display\"), then.\nIf you can see the X11 window on the same screen as this message, then RKWard should do better. In this case, please contact us at rkward-devel@lists.sourceforge.net with details on your setup, so we can try to fix this in future versions of RKWard."), i18n ("Could not embed R X11 window"));
		}
	}
	last_cur_device = new_cur_device;
}

#include "rkwindowcatcher.moc"

#endif // DISABLE_RKWINDOWCATCHER
