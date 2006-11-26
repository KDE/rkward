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



#include <qscrollview.h>
#include <qvbox.h>
#include <qlabel.h>

#include <kactionclasses.h>
#include <kdialogbase.h>
#include <knuminput.h>

RKCatchedX11Window::RKCatchedX11Window (WId window_to_embed, int device_number) : RKMDIWindow (0, X11Window) {
	RK_TRACE (MISC);

	part = new RKCatchedX11WindowPart (this);
	setFocusPolicy (QWidget::ClickFocus);

	embedded = window_to_embed;
	RKCatchedX11Window::device_number = device_number;

	QVBoxLayout *layout = new QVBoxLayout (this);
	box_widget = new QVBox (this);
	layout->addWidget (box_widget);
	scroll_widget = new QScrollView (this);
	scroll_widget->hide ();
	layout->addWidget (scroll_widget);
	xembed_container = new QVBox (box_widget);
	dynamic_size = true;
	dynamic_size_action->setChecked (true);

	QXEmbedCopy *capture = new QXEmbedCopy (xembed_container);
	capture->setProtocol (QXEmbedCopy::XPLAIN);
	connect (capture, SIGNAL (embeddedWindowDestroyed ()), this, SLOT (deleteLater ()));

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

void RKCatchedX11Window::prepareToBeAttached () {
	RK_TRACE (MISC);

	dynamic_size_action->setChecked (false);
	toggleFixedSize ();
	dynamic_size_action->setEnabled (false);
}

void RKCatchedX11Window::prepareToBeDetached () {
	RK_TRACE (MISC);

	dynamic_size_action->setEnabled (true);
}

void RKCatchedX11Window::toggleFixedSize () {
	RK_TRACE (MISC);

	if (dynamic_size == dynamic_size_action->isChecked ()) return;
	dynamic_size = dynamic_size_action->isChecked ();

	if (dynamic_size_action->isChecked ()) {
		scroll_widget->removeChild (xembed_container);
		xembed_container->reparent (box_widget, QPoint (0, 0), true);
		scroll_widget->hide ();
		box_widget->show ();
		xembed_container->setMinimumSize (5, 5);
		xembed_container->setMaximumSize (32767, 32767);
	} else {
		xembed_container->setFixedSize (xembed_container->size ());
		xembed_container->reparent (scroll_widget->viewport (), QPoint (0, 0), true);
		scroll_widget->addChild (xembed_container);
		box_widget->hide ();
		scroll_widget->show ();
	}
}

void RKCatchedX11Window::setFixedSize1 () {
	RK_TRACE (MISC);

	dynamic_size_action->setChecked (false);
	toggleFixedSize ();		// apparently KToggleAction::setChecked () does not invoke the slot!
	xembed_container->setFixedSize (500, 500);
}

void RKCatchedX11Window::setFixedSize2 () {
	RK_TRACE (MISC);

	dynamic_size_action->setChecked (false);
	toggleFixedSize ();		// see setFixedSize1 () above
	xembed_container->setFixedSize (1000, 1000);
}

void RKCatchedX11Window::setFixedSize3 () {
	RK_TRACE (MISC);

	dynamic_size_action->setChecked (false);
	toggleFixedSize ();		// see setFixedSize1 () above
	xembed_container->setFixedSize (2000, 2000);
}

void RKCatchedX11Window::setFixedSizeManual () {
	RK_TRACE (MISC);

// TODO: not very pretty, yet
	KDialogBase *dialog = new KDialogBase (this, 0, true, i18n ("Specify fixed size"), KDialogBase::Ok|KDialogBase::Cancel);
	QVBox *page = dialog->makeVBoxMainWidget ();

	QLabel *label = new QLabel (i18n ("Width"), page);
	KIntSpinBox *width = new KIntSpinBox (5, 32767, 1, xembed_container->width (), 10, page);

	label = new QLabel (i18n ("Height"), page);
	KIntSpinBox *height = new KIntSpinBox (5, 32767, 1, xembed_container->height (), 10, page);

	dialog->exec ();

	if (dialog->result () == QDialog::Accepted) {
		dynamic_size_action->setChecked (false);
		toggleFixedSize ();		// see setFixedSize1 () above

		xembed_container->setFixedSize (width->value (), height->value ());
	}

	delete dialog;
}




RKCatchedX11WindowPart::RKCatchedX11WindowPart (RKCatchedX11Window *window) : KParts::Part (0) {
	RK_TRACE (MISC);

	KInstance* instance = new KInstance ("rkward");
	setInstance (instance);

	setWidget (window);
	RKCatchedX11WindowPart::window = window;

	setXMLFile ("rkcatchedx11windowpart.rc");

	window->dynamic_size_action = new KToggleAction (i18n ("Draw area follows size of window"), 0, window, SLOT (toggleFixedSize ()), actionCollection (), "toggle_fixed_size");

	new KAction (i18n ("Set fixed size 500x500"), 0, window, SLOT (setFixedSize1 ()), actionCollection (), "set_fixed_size_1");
	new KAction (i18n ("Set fixed size 1000x1000"), 0, window, SLOT (setFixedSize2 ()), actionCollection (), "set_fixed_size_2");
	new KAction (i18n ("Set fixed size 2000x2000"), 0, window, SLOT (setFixedSize3 ()), actionCollection (), "set_fixed_size_3");
	new KAction (i18n ("Set specified fixed size..."), 0, window, SLOT (setFixedSizeManual ()), actionCollection (), "set_fixed_size_manual");

}

RKCatchedX11WindowPart::~RKCatchedX11WindowPart () {
	RK_TRACE (MISC);
}

#include "rkwindowcatcher.moc"

#endif // DISABLE_RKWINDOWCATCHER
