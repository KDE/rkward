/***************************************************************************
                          rwindowcatcher.cpp  -  description
                             -------------------
    begin                : Wed May 4 2005
    copyright            : (C) 2005, 2006, 2007, 2009, 2010 by Thomas Friedrichsmeier
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

#include <kmessagebox.h>
#include <klocale.h>

#include "../rkwardapplication.h"
#include "rkworkplace.h"
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

	WId w = RKWardApplication::getApp ()->endWindowCreationDetection ();
	if (new_cur_device != last_cur_device) {
		if (w) {
			RKWorkplace::mainWorkplace ()->newX11Window (w, new_cur_device);
			//new RKCaughtX11Window (w, new_cur_device);
		} else {
			KMessageBox::information (0, i18n ("You have created a new X11 device window in R. Usually, RKWard tries to detect such windows, to take control of them, and add a menu-bar to them. This time, however, RKWard failed to detect, which window was created, and so can not embed it.\nIf you created the window on a different screen or X11 display, that is to be expected. You might want to consider changing options(\"display\"), then.\nIf you can see the X11 window on the same screen as this message, then RKWard should do better. In this case, please contact us at rkward-devel@lists.sourceforge.net with details on your setup, so we can try to fix this in future versions of RKWard."), i18n ("Could not embed R X11 window"), "failure_to_detect_x11_device");
		}
	}
	last_cur_device = new_cur_device;
}

///////////////////////////////// END RKWindowCatcher //////////////////////////////////
/**************************************************************************************/
//////////////////////////////// BEGIN RKCaughtX11Window //////////////////////////////


#include <QScrollArea>
#include <qlabel.h>
#ifdef Q_WS_WIN
#	include "../qwinhost/qwinhost.h"
#	include <windows.h>
#elif defined Q_WS_X11
#	include <QX11EmbedContainer>
#endif
#include <QTimer>

#include <ktoggleaction.h>
#include <kdialog.h>
#include <knuminput.h>
#include <kvbox.h>
#include <kwindowsystem.h>
#include <kactioncollection.h>

#include "../rkglobals.h"
#include "../rbackend/rinterface.h"
#include "../core/robject.h"
#include "../misc/rkprogresscontrol.h"
#include "../misc/rksaveobjectchooser.h"
#include "../plugin/rkcomponentcontext.h"

RKCaughtX11Window::RKCaughtX11Window (WId window_to_embed, int device_number) : RKMDIWindow (0, X11Window) {
	RK_TRACE (MISC);

	embedded = window_to_embed;
	RKCaughtX11Window::device_number = device_number;

	error_dialog = new RKProgressControl (0, i18n ("An error occurred"), i18n ("An error occurred"), RKProgressControl::DetailedError);
	setPart (new RKCaughtX11WindowPart (this));
	initializeActivationSignals ();
	setFocusPolicy (Qt::ClickFocus);

	QVBoxLayout *layout = new QVBoxLayout (this);
	layout->setContentsMargins (0, 0, 0, 0);
	box_widget = new KVBox (this);
	layout->addWidget (box_widget);
	scroll_widget = new QScrollArea (this);
	scroll_widget->hide ();
	layout->addWidget (scroll_widget);

	xembed_container = new KVBox (box_widget);	// QX11EmbedContainer can not be reparented (between the box_widget, and the scroll_widget) directly. Therefore we place it into a container, and reparent that instead

#ifdef Q_WS_WIN
	// unfortunately, trying to get KWindowInfo as below hangs on windows (KDElibs 4.2.3)
	WINDOWINFO wininfo;
	wininfo.cbSize = sizeof (WINDOWINFO);
	GetWindowInfo (embedded, &wininfo);

	// clip off the window frame and menubar
	xembed_container->setContentsMargins (wininfo.rcWindow.left - wininfo.rcClient.left, wininfo.rcWindow.top - wininfo.rcClient.top,
				wininfo.rcClient.right - wininfo.rcWindow.right, wininfo.rcClient.bottom - wininfo.rcWindow.bottom);
	// set a fixed size until the window is shown
	xembed_container->setFixedSize (wininfo.rcClient.right - wininfo.rcClient.left, wininfo.rcClient.bottom - wininfo.rcClient.top);
	move (wininfo.rcClient.left, wininfo.rcClient.top);
#elif defined Q_WS_X11
	KWindowInfo wininfo = KWindowSystem::windowInfo (embedded, NET::WMName | NET::WMGeometry);
	RK_ASSERT (wininfo.valid ());

	// set a fixed size until the window is shown
	xembed_container->setFixedSize (wininfo.geometry ().width (), wininfo.geometry ().height ());
	move (wininfo.geometry ().topLeft());
	setCaption (wininfo.name ());
#endif
	dynamic_size = false;
	dynamic_size_action->setChecked (false);

	// somehow in Qt 4.4.3, when the RKCaughtWindow is reparented the first time, the QX11EmbedContainer may kill its client. Hence we delay the actual embedding until after the window was shown.
	// In some previous version of Qt, this was not an issue, but I did not track the versions.
	QTimer::singleShot (0, this, SLOT (doEmbed()));
}

void RKCaughtX11Window::doEmbed () {
	RK_TRACE (MISC);

#ifdef Q_WS_WIN
	capture = new QWinHost (xembed_container);
	capture->setWindow (embedded);
	capture->setFocusPolicy (Qt::ClickFocus);
	capture->setAutoDestruct (true);
	connect (capture, SIGNAL (clientDestroyed()), this, SLOT (deleteLater()), Qt::QueuedConnection);
	connect (capture, SIGNAL (clientTitleChanged(const QString&)), this, SLOT (setCaption(const QString&)), Qt::QueuedConnection);

	setCaption (capture->getClientTitle ());
#elif defined Q_WS_X11
	capture = new QX11EmbedContainer (xembed_container);
	capture->embedClient (embedded);
	connect (capture, SIGNAL (clientClosed ()), this, SLOT (deleteLater ()));

	RKWardApplication::getApp ()->registerNameWatcher (embedded, this);
#endif
	// make xembed_container resizable, again, now that it actually has a content
	dynamic_size_action->setChecked (true);
	fixedSizeToggled ();
}

RKCaughtX11Window::~RKCaughtX11Window () {
	RK_TRACE (MISC);

	capture->close ();
#ifdef Q_WS_X11
	RKWardApplication::getApp ()->unregisterNameWatcher (embedded);
#endif
	error_dialog->autoDeleteWhenDone ();
}

void RKCaughtX11Window::prepareToBeAttached () {
	RK_TRACE (MISC);

	dynamic_size_action->setChecked (false);
	fixedSizeToggled ();
	dynamic_size_action->setEnabled (false);
}

void RKCaughtX11Window::prepareToBeDetached () {
	RK_TRACE (MISC);

	dynamic_size_action->setEnabled (true);
}

void RKCaughtX11Window::fixedSizeToggled () {
	RK_TRACE (MISC);

	if (dynamic_size == dynamic_size_action->isChecked ()) return;
	dynamic_size = dynamic_size_action->isChecked ();

	if (dynamic_size_action->isChecked ()) {
		scroll_widget->takeWidget ();
		xembed_container->setParent (box_widget);
		xembed_container->show ();
		scroll_widget->hide ();
		box_widget->show ();
		xembed_container->setMinimumSize (5, 5);
		xembed_container->setMaximumSize (32767, 32767);
	} else {
		xembed_container->setFixedSize (xembed_container->size ());
		scroll_widget->setWidget (xembed_container);
		box_widget->hide ();
		scroll_widget->show ();
	}
}

void RKCaughtX11Window::setFixedSize1 () {
	RK_TRACE (MISC);

	dynamic_size_action->setChecked (false);
	fixedSizeToggled ();		// apparently KToggleAction::setChecked () does not invoke the slot!
	xembed_container->setFixedSize (500, 500);
}

void RKCaughtX11Window::setFixedSize2 () {
	RK_TRACE (MISC);

	dynamic_size_action->setChecked (false);
	fixedSizeToggled ();		// see setFixedSize1 () above
	xembed_container->setFixedSize (1000, 1000);
}

void RKCaughtX11Window::setFixedSize3 () {
	RK_TRACE (MISC);

	dynamic_size_action->setChecked (false);
	fixedSizeToggled ();		// see setFixedSize1 () above
	xembed_container->setFixedSize (2000, 2000);
}

void RKCaughtX11Window::setFixedSizeManual () {
	RK_TRACE (MISC);

// TODO: not very pretty, yet
	KDialog *dialog = new KDialog (this);
	dialog->setButtons (KDialog::Ok|KDialog::Cancel);
	dialog->setCaption (i18n ("Specify fixed size"));
	dialog->setModal (true);

	KVBox *page = new KVBox (dialog);
	dialog->setMainWidget (page);

	QLabel *label = new QLabel (i18n ("Width"), page);
	KIntSpinBox *width = new KIntSpinBox (5, 32767, 1, xembed_container->width (), page, 10);
	width->setEditFocus (true);

	label = new QLabel (i18n ("Height"), page);
	KIntSpinBox *height = new KIntSpinBox (5, 32767, 1, xembed_container->height (), page, 10);

	dialog->exec ();

	if (dialog->result () == QDialog::Accepted) {
		dynamic_size_action->setChecked (false);
		fixedSizeToggled ();		// see setFixedSize1 () above

		xembed_container->setFixedSize (width->value (), height->value ());
	}

	delete dialog;
}

void RKCaughtX11Window::activateDevice () {
	RK_TRACE (MISC);

	RKGlobals::rInterface ()->issueCommand ("dev.set (" + QString::number (device_number) + ')', RCommand::App, i18n ("Activate graphics device number %1", device_number), error_dialog);
}

void RKCaughtX11Window::copyDeviceToOutput () {
	RK_TRACE (MISC);

	RKGlobals::rInterface ()->issueCommand ("dev.set (" + QString::number (device_number) + ")\ndev.copy (device=rk.graph.on)\nrk.graph.off ()", RCommand::App | RCommand::DirectToOutput, i18n ("Copy contents of graphics device number %1 to output", device_number), error_dialog);
}

void RKCaughtX11Window::printDevice () {
	RK_TRACE (MISC);

	RKGlobals::rInterface ()->issueCommand ("dev.set (" + QString::number (device_number) + ")\ndev.print ()", RCommand::App, i18n ("Print contents of graphics device number %1", device_number), error_dialog);
}

void RKCaughtX11Window::copyDeviceToRObject () {
	RK_TRACE (MISC);

// TODO: not very pretty, yet
	KDialog *dialog = new KDialog (this);
	dialog->setButtons (KDialog::Ok|KDialog::Cancel);
	dialog->setCaption (i18n ("Specify R object"));
	dialog->setModal (true);
	KVBox *page = new KVBox (dialog);
	dialog->setMainWidget (page);

	RKSaveObjectChooser *chooser = new RKSaveObjectChooser (page, "my.plot", i18n ("Specify the R object name, you want to save the graph to"));
	connect (chooser, SIGNAL (okStatusChanged (bool)), dialog, SLOT (enableButtonOk (bool)));
	if (!chooser->isOk ()) dialog->enableButtonOk (false);

	dialog->exec ();

	if (dialog->result () == QDialog::Accepted) {
		RK_ASSERT (chooser->isOk ());

		QString name = chooser->validizedSelectedObjectName ();

		RKGlobals::rInterface ()->issueCommand ("dev.set (" + QString::number (device_number) + ")\n" + RObject::rQuote (name) + " <- recordPlot ()", RCommand::App | RCommand::ObjectListUpdate, i18n ("Save contents of graphics device number %1 to object '%2'", device_number, name), error_dialog);
	}

	delete dialog;
}

void RKCaughtX11Window::duplicateDevice () {
	RK_TRACE (MISC);

// 	RKGlobals::rInterface ()->issueCommand ("dev.set (" + QString::number (device_number) + ")\ndev.copy (device=x11)", RCommand::App, i18n ("Duplicate graphics device number %1", device_number), error_dialog);
	RKGlobals::rInterface ()->issueCommand ("rk.duplicate.device (" + QString::number (device_number) + ")", RCommand::App, i18n ("Duplicate graphics device number %1", device_number), error_dialog);
}

void RKCaughtX11Window::nextPlot () {
	RK_TRACE (MISC);

	RKGlobals::rInterface ()->issueCommand ("rk.next.plot (" + QString::number (device_number) + ')', RCommand::App, i18n ("Load next plot in device number %1", device_number), error_dialog);
}

void RKCaughtX11Window::currentPlot () {
	RK_TRACE (MISC);

	RKGlobals::rInterface ()->issueCommand ("rk.current.plot (" + QString::number (device_number) + ')', RCommand::App, i18n ("Add current plot in device number %1", device_number), error_dialog);
}

void RKCaughtX11Window::previousPlot () {
	RK_TRACE (MISC);

	RKGlobals::rInterface ()->issueCommand ("rk.previous.plot (" + QString::number (device_number) + ')', RCommand::App, i18n ("Load previous plot in device number %1", device_number), error_dialog);
}

///////////////////////////////// END RKCaughtX11Window ///////////////////////////////
/**************************************************************************************/
//////////////////////////////// BEGIN RKCaughtX11WindowPart //////////////////////////


RKCaughtX11WindowPart::RKCaughtX11WindowPart (RKCaughtX11Window *window) : KParts::Part (0) {
	RK_TRACE (MISC);

	setComponentData (KGlobal::mainComponent ());

	setWidget (window);
	RKCaughtX11WindowPart::window = window;

	setXMLFile ("rkcatchedx11windowpart.rc");

	window->dynamic_size_action = new KToggleAction (i18n ("Draw area follows size of window"), window);
	connect (window->dynamic_size_action, SIGNAL (triggered()), window, SLOT (fixedSizeToggled()));
	actionCollection ()->addAction ("toggle_fixed_size", window->dynamic_size_action);

	QAction *action;
	action = actionCollection ()->addAction ("set_fixed_size_1", window, SLOT (setFixedSize1()));
	action->setText (i18n ("Set fixed size 500x500"));
	action = actionCollection ()->addAction ("set_fixed_size_2", window, SLOT (setFixedSize2()));
	action->setText (i18n ("Set fixed size 1000x1000"));
	action = actionCollection ()->addAction ("set_fixed_size_3", window, SLOT (setFixedSize3()));
	action->setText (i18n ("Set fixed size 2000x2000"));
	action = actionCollection ()->addAction ("set_fixed_size_manual", window, SLOT (setFixedSizeManual()));
	action->setText (i18n ("Set specified fixed size..."));

	action = actionCollection ()->addAction ("plot_prev", window, SLOT (previousPlot()));
// 	action->setText (i18n ("Restore previous plot"));
	action->setText (i18n ("<"));
	action = actionCollection ()->addAction ("plot_curr", window, SLOT (currentPlot()));
// 	action->setText (i18n ("Add current plot"));
	action->setText (i18n ("+"));
	action = actionCollection ()->addAction ("plot_next", window, SLOT (nextPlot()));
// 	action->setText (i18n ("Advance to next plot"));
	action->setText (i18n (">"));

	action = actionCollection ()->addAction ("device_activate", window, SLOT (activateDevice()));
	action->setText (i18n ("Make active"));
	action = actionCollection ()->addAction ("device_copy_to_output", window, SLOT (copyDeviceToOutput()));
	action->setText (i18n ("Copy to output"));
	action = actionCollection ()->addAction (KStandardAction::Print, "device_print", window, SLOT (printDevice()));
	action = actionCollection ()->addAction ("device_copy_to_r_object", window, SLOT (copyDeviceToRObject()));
	action->setText (i18n ("Store as R object..."));
	action = actionCollection ()->addAction ("device_duplicate", window, SLOT (duplicateDevice()));
	action->setText (i18n ("Duplicate"));

	// initialize context for plugins
	RKContextMap *context = RKComponentMap::getContext ("x11");
	if (!context) return;
	RKContextHandler *context_handler = context->makeContextHandler (this);
	insertChildClient (context_handler);
	RKComponentPropertyInt *devnum_property = new RKComponentPropertyInt (this, false, 0);
	devnum_property->setIntValue (window->device_number);
	context_handler->addChild ("devnum", devnum_property);
}

RKCaughtX11WindowPart::~RKCaughtX11WindowPart () {
	RK_TRACE (MISC);
}

#include "rkwindowcatcher.moc"

#endif // DISABLE_RKWINDOWCATCHER
