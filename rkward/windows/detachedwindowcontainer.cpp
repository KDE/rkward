/***************************************************************************
                          detachedwindowcontainer  -  description
                             -------------------
    begin                : Wed Oct 21 2005
    copyright            : (C) 2005, 2007 by Thomas Friedrichsmeier
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

#include "detachedwindowcontainer.h"

#include <klocale.h>
#include <kactioncollection.h>
#include <kstatusbar.h>

#include <qlayout.h>
#include <qwidget.h>
#include <QCloseEvent>

#include "rktoplevelwindowgui.h"
#include "../rkward.h"
#include "../misc/rkstandardicons.h"
#include "rkworkplace.h"
#include "../rkglobals.h"
#include "../debug.h"

/* Warning! Do not pass a parent widget to the KParts::MainWindow. Otherwise there will be strange crahes while removing the KXMLGUIClients! (In this case: Open a help window, and detach it. Open another help window attached. Close the detached one, then close the attached one -> crash; KDE 3.5.5) */
DetachedWindowContainer::DetachedWindowContainer (RKMDIWindow *widget_to_capture) : KParts::MainWindow  () {
	RK_TRACE (APP);

	setHelpMenuEnabled (false);
// create own GUI
	setXMLFile ("detachedwindowcontainer.rc");
	actionCollection ()->addAction (KStandardAction::Close, "dwindow_close", this, SLOT(close()));

	QAction *reattach = actionCollection ()->addAction ("dwindow_attach", this, SLOT(slotReattach()));
	reattach->setText (i18n ("Attach to main window"));
	reattach->setIcon (RKStandardIcons::getIcon (RKStandardIcons::ActionAttachWindow));

	RKTopLevelWindowGUI *toplevel_actions = new RKTopLevelWindowGUI (this);
	insertChildClient (toplevel_actions);
	statusBar ()->hide ();
	createShellGUI ();

// capture widget
	setGeometry (widget_to_capture->frameGeometry ());
	widget_to_capture->setParent (this);
	setCentralWidget (widget_to_capture);
	widget_to_capture->show ();
	createGUI (widget_to_capture->getPart ());
	captured = widget_to_capture;

// should self-destruct, when child widget is destroyed
	connect (widget_to_capture, SIGNAL (destroyed (QObject *)), this, SLOT (viewDestroyed (QObject *)));
	connect (widget_to_capture, SIGNAL (captionChanged (RKMDIWindow *)), this, SLOT (updateCaption (RKMDIWindow *)));
	setCaption (widget_to_capture->fullCaption ());	// has to come after createGUI!
}

DetachedWindowContainer::~DetachedWindowContainer () {
	RK_TRACE (APP);
}

void DetachedWindowContainer::viewDestroyed (QObject *) {
	RK_TRACE (APP);

	hide ();
	deleteLater ();
}

void DetachedWindowContainer::updateCaption (RKMDIWindow *widget) {
	RK_TRACE (APP);
	RK_ASSERT (widget == captured);

	setCaption (widget->fullCaption ());
}

void DetachedWindowContainer::slotSetStatusBarText (const QString &text) {
	RK_TRACE (APP);

	QString ntext = text.trimmed ();
	ntext.replace ("<qt>", "");	// WORKAROUND: what the ?!? is going on? The KTHMLPart seems to post such messages.

	statusBar ()->message (ntext);
	statusBar ()->show ();
}

void DetachedWindowContainer::slotReattach () {
	RK_TRACE (APP);

// we will not handle any more signals from the window
	disconnect (captured, SIGNAL (destroyed (QObject *)), this, SLOT (viewDestroyed (QObject *)));
	disconnect (captured, SIGNAL (captionChanged (RKMDIWindow *)), this, SLOT (updateCaption (RKMDIWindow *)));

	captured->setParent (0);
	RKWorkplace::mainWorkplace ()->attachWindow (captured);

	hide ();
	deleteLater ();
}

void DetachedWindowContainer::closeEvent (QCloseEvent *e) {
	RK_TRACE (APP);

	if (captured->close (true)) {
		e->accept ();
	} else {
		e->ignore ();
	}
}

#include "detachedwindowcontainer.moc"
