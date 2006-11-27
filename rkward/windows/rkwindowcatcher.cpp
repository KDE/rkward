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

RKWindowCatcher::RKWindowCatcher () {
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
			KMessageBox::information (0, i18n ("You have created a new X11 device window in R. Usually, RKWard tries to detect such windows, to take control of them, and add a menu-bar to them. This time, however, RKWard failed to detect, which window was created, and so can not embed it.\nIf you created the window on a different screen or X11 display, that is to be expected. You might want to consider changing options(\"display\"), then.\nIf you can see the X11 window on the same screen as this message, then RKWard should do better. In this case, please contact us at rkward-devel@lists.sourceforge.net with details on your setup, so we can try to fix this in future versions of RKWard."), i18n ("Could not embed R X11 window"), "failure_to_detect_x11_device");
		}
	}
	last_cur_device = new_cur_device;
}

///////////////////////////////// END RKWindowCatcher //////////////////////////////////
/**************************************************************************************/
//////////////////////////////// BEGIN RKCatchedX11Window //////////////////////////////


#include <qscrollview.h>
#include <qvbox.h>
#include <qlabel.h>

#include <kactionclasses.h>
#include <kdialogbase.h>
#include <knuminput.h>

#include "../rkglobals.h"
#include "../rbackend/rinterface.h"
#include "../core/robject.h"
#include "../misc/rkerrordialog.h"
#include "../misc/rksaveobjectchooser.h"

RKCatchedX11Window::RKCatchedX11Window (WId window_to_embed, int device_number) : RKMDIWindow (0, X11Window) {
	RK_TRACE (MISC);

	error_dialog = new RKRErrorDialog (i18n ("An error occurred"), i18n ("An error occurred"));
	part = new RKCatchedX11WindowPart (this);
	setFocusPolicy (QWidget::ClickFocus);

	embedded = window_to_embed;
	RKCatchedX11Window::device_number = device_number;

	QVBoxLayout *layout = new QVBoxLayout (this);
	box_widget = new QVBox (this);
	layout->addWidget (box_widget);
	scroll_widget = new QScrollView (this);
	scroll_widget->setFrameStyle (QFrame::NoFrame);
	scroll_widget->hide ();
	layout->addWidget (scroll_widget);
	xembed_container = new QVBox (box_widget);	// QXEmbed can not be reparented (between the box_widget, and the scroll_widget) directly. Therefore we place it into a container, and reparent that instead
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
}

RKCatchedX11Window::~RKCatchedX11Window () {
	RK_TRACE (MISC);

	RKWardApplication::getApp ()->unregisterNameWatcher (embedded);
	error_dialog->autoDeleteWhenDone ();
}

KParts::Part *RKCatchedX11Window::getPart () {
	RK_TRACE (MISC);

	return part;
}

void RKCatchedX11Window::prepareToBeAttached () {
	RK_TRACE (MISC);

	dynamic_size_action->setChecked (false);
	fixedSizeToggled ();
	dynamic_size_action->setEnabled (false);
}

void RKCatchedX11Window::prepareToBeDetached () {
	RK_TRACE (MISC);

	dynamic_size_action->setEnabled (true);
}

void RKCatchedX11Window::fixedSizeToggled () {
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
	fixedSizeToggled ();		// apparently KToggleAction::setChecked () does not invoke the slot!
	xembed_container->setFixedSize (500, 500);
}

void RKCatchedX11Window::setFixedSize2 () {
	RK_TRACE (MISC);

	dynamic_size_action->setChecked (false);
	fixedSizeToggled ();		// see setFixedSize1 () above
	xembed_container->setFixedSize (1000, 1000);
}

void RKCatchedX11Window::setFixedSize3 () {
	RK_TRACE (MISC);

	dynamic_size_action->setChecked (false);
	fixedSizeToggled ();		// see setFixedSize1 () above
	xembed_container->setFixedSize (2000, 2000);
}

void RKCatchedX11Window::setFixedSizeManual () {
	RK_TRACE (MISC);

// TODO: not very pretty, yet
	KDialogBase *dialog = new KDialogBase (this, 0, true, i18n ("Specify fixed size"), KDialogBase::Ok|KDialogBase::Cancel);
	QVBox *page = dialog->makeVBoxMainWidget ();

	QLabel *label = new QLabel (i18n ("Width"), page);
	KIntSpinBox *width = new KIntSpinBox (5, 32767, 1, xembed_container->width (), 10, page);
	width->setEditFocus (true);

	label = new QLabel (i18n ("Height"), page);
	KIntSpinBox *height = new KIntSpinBox (5, 32767, 1, xembed_container->height (), 10, page);

	dialog->exec ();

	if (dialog->result () == QDialog::Accepted) {
		dynamic_size_action->setChecked (false);
		fixedSizeToggled ();		// see setFixedSize1 () above

		xembed_container->setFixedSize (width->value (), height->value ());
	}

	delete dialog;
}

void RKCatchedX11Window::activateDevice () {
	RK_TRACE (MISC);

	RKGlobals::rInterface ()->issueCommand ("dev.set (" + QString::number (device_number) + ")", RCommand::App, i18n ("Activate graphics device number %1").arg (QString::number (device_number)), error_dialog);
}

void RKCatchedX11Window::copyDeviceToOutput () {
	RK_TRACE (MISC);

	RKGlobals::rInterface ()->issueCommand ("dev.set (" + QString::number (device_number) + ")\ndev.copy (device=rk.graph.on)\nrk.graph.off ()", RCommand::App | RCommand::DirectToOutput, i18n ("Copy contents of graphics device number %1 to output").arg (QString::number (device_number)), error_dialog);
}

void RKCatchedX11Window::printDevice () {
	RK_TRACE (MISC);

	RKGlobals::rInterface ()->issueCommand ("dev.set (" + QString::number (device_number) + ")\ndev.print ()", RCommand::App, i18n ("Print contents of graphics device number %1").arg (QString::number (device_number)), error_dialog);
}

void RKCatchedX11Window::copyDeviceToRObject () {
	RK_TRACE (MISC);

// TODO: not very pretty, yet
	KDialogBase *dialog = new KDialogBase (this, 0, true, i18n ("Specify R object"), KDialogBase::Ok|KDialogBase::Cancel);
	QVBox *page = dialog->makeVBoxMainWidget ();

	RKSaveObjectChooser *chooser = new RKSaveObjectChooser (page, true, "my.plot", i18n ("Specify the R object name, you want to save the graph to"));
	connect (chooser, SIGNAL (okStatusChanged (bool)), dialog, SLOT (enableButtonOK (bool)));
	if (!chooser->isOk ()) dialog->enableButtonOK (false);

	dialog->exec ();

	if (dialog->result () == QDialog::Accepted) {
		RK_ASSERT (chooser->isOk ());

		QString name = chooser->validizedSelectedObjectName ();

		RKGlobals::rInterface ()->issueCommand ("dev.set (" + QString::number (device_number) + ")\n" + RObject::rQuote (name) + " <- recordPlot ()", RCommand::App | RCommand::ObjectListUpdate, i18n ("Save contents of graphics device number %1 to object '%2'").arg (QString::number (device_number)).arg (name), error_dialog);
	}

	delete dialog;
}

void RKCatchedX11Window::duplicateDevice () {
	RK_TRACE (MISC);

	RKGlobals::rInterface ()->issueCommand ("dev.set (" + QString::number (device_number) + ")\ndev.copy (device=x11)", RCommand::App, i18n ("Duplicate graphics device number %1").arg (QString::number (device_number)), error_dialog);
}


///////////////////////////////// END RKCatchedX11Window ///////////////////////////////
/**************************************************************************************/
//////////////////////////////// BEGIN RKCatchedX11WindowPart //////////////////////////


RKCatchedX11WindowPart::RKCatchedX11WindowPart (RKCatchedX11Window *window) : KParts::Part (0) {
	RK_TRACE (MISC);

	KInstance* instance = new KInstance ("rkward");
	setInstance (instance);

	setWidget (window);
	RKCatchedX11WindowPart::window = window;

	setXMLFile ("rkcatchedx11windowpart.rc");

	window->dynamic_size_action = new KToggleAction (i18n ("Draw area follows size of window"), 0, window, SLOT (fixedSizeToggled ()), actionCollection (), "toggle_fixed_size");

	new KAction (i18n ("Set fixed size 500x500"), 0, window, SLOT (setFixedSize1 ()), actionCollection (), "set_fixed_size_1");
	new KAction (i18n ("Set fixed size 1000x1000"), 0, window, SLOT (setFixedSize2 ()), actionCollection (), "set_fixed_size_2");
	new KAction (i18n ("Set fixed size 2000x2000"), 0, window, SLOT (setFixedSize3 ()), actionCollection (), "set_fixed_size_3");
	new KAction (i18n ("Set specified fixed size..."), 0, window, SLOT (setFixedSizeManual ()), actionCollection (), "set_fixed_size_manual");

	new KAction (i18n ("Make active"), 0, window, SLOT (activateDevice ()), actionCollection (), "device_activate");
	new KAction (i18n ("Copy to output"), 0, window, SLOT (copyDeviceToOutput ()), actionCollection (), "device_copy_to_output");
	new KAction (i18n ("Print"), 0, window, SLOT (printDevice ()), actionCollection (), "device_print");
	new KAction (i18n ("Store as R object..."), 0, window, SLOT (copyDeviceToRObject ()), actionCollection (), "device_copy_to_r_object");
	new KAction (i18n ("Duplicate"), 0, window, SLOT (duplicateDevice ()), actionCollection (), "device_duplicate");
}

RKCatchedX11WindowPart::~RKCatchedX11WindowPart () {
	RK_TRACE (MISC);
}

#include "rkwindowcatcher.moc"

#endif // DISABLE_RKWINDOWCATCHER
