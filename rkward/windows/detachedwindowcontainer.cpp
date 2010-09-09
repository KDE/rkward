/***************************************************************************
                          detachedwindowcontainer  -  description
                             -------------------
    begin                : Wed Oct 21 2005
    copyright            : (C) 2005, 2007, 2009, 2010 by Thomas Friedrichsmeier
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
#include <ktoolbar.h>
#include <kxmlguifactory.h>

#include <qlayout.h>
#include <qwidget.h>
#include <QCloseEvent>
#include <QMenu>

#include "rktoplevelwindowgui.h"
#include "../rkward.h"
#include "../misc/rkstandardicons.h"
#include "../misc/rkxmlguisyncer.h"
#include "rkworkplace.h"
#include "../rkglobals.h"
#include "../debug.h"

/* Warning! Do not pass a parent widget to the KParts::MainWindow. Otherwise there will be strange crahes while removing the KXMLGUIClients! (In this case: Open a help window, and detach it. Open another help window attached. Close the detached one, then close the attached one -> crash; KDE 3.5.5) */
DetachedWindowContainer::DetachedWindowContainer (RKMDIWindow *widget_to_capture) : KParts::MainWindow  () {
	RK_TRACE (APP);

	actionCollection ()->addAction (KStandardAction::Close, "dwindow_close", this, SLOT(close()));

	QAction *reattach = actionCollection ()->addAction ("dwindow_attach", this, SLOT(slotReattach()));
	reattach->setText (i18n ("Attach to main window"));
	reattach->setIcon (RKStandardIcons::getIcon (RKStandardIcons::ActionAttachWindow));

	setHelpMenuEnabled (false);
// create own GUI
	setXMLFile ("detachedwindowcontainer.rc");
	insertChildClient (toplevel_actions = new RKTopLevelWindowGUI (this));
	statusBar ()->hide ();
	createShellGUI (true);
	RKXMLGUISyncer::self ()->watchXMLGUIClientUIrc (this);

// copy main window toolbar settings
	QMap<QString, Qt::ToolButtonStyle> main_window_toolbar_styles;
	foreach (KToolBar *bar, RKWardMainWindow::getMain ()->toolBars ()) {
		main_window_toolbar_styles.insert (bar->objectName (), bar->toolButtonStyle ());
	}

// capture widget
// NOTE: If the window has not been shown, before, its geometry() is always 640*480, unconditionally.
	setGeometry (widget_to_capture->frameGeometry ());
	if (!widget_to_capture->isWindow ()) move (widget_to_capture->mapToGlobal (widget_to_capture->pos ()));
	widget_to_capture->setParent (this);
	setCentralWidget (widget_to_capture);
	widget_to_capture->show ();
	createGUI (widget_to_capture->getPart ());
	captured = widget_to_capture;

	hideEmptyMenus ();
	// hide empty menus now, and after any reloads
	// the signal is available since KDE 4.1.3, but we can tolerate a bit of aesthethic malfunction on earlier versions
	connect (guiFactory (), SIGNAL(makingChanges(bool)), this, SLOT(hideEmptyMenus(bool)));

// sanitize toolbars
	foreach (KToolBar *bar, toolBars ()) {
		if (main_window_toolbar_styles.contains (bar->objectName ())) {
			bar->setToolButtonStyle (main_window_toolbar_styles[bar->objectName ()]);
		} else {
			RK_ASSERT (false);
		}
	}

// should self-destruct, when child widget is destroyed
	connect (widget_to_capture, SIGNAL (destroyed (QObject *)), this, SLOT (viewDestroyed (QObject *)));
	connect (widget_to_capture, SIGNAL (captionChanged (RKMDIWindow *)), this, SLOT (updateCaption (RKMDIWindow *)));
	setCaption (widget_to_capture->fullCaption ());	// has to come after createGUI!
}

DetachedWindowContainer::~DetachedWindowContainer () {
	RK_TRACE (APP);
}

void DetachedWindowContainer::hideEmptyMenus (bool ignore) {
	if (ignore) return;
	RK_TRACE (APP);

	// remove empty menus (we had to define them in detachedwindowcontainer.rc in order to force a sane menu order)
	QStringList menu_names;
	menu_names << "file" << "device" << "edit" << "run" << "view" << "settings";
	foreach (QString name, menu_names) {
		QMenu* menu = dynamic_cast<QMenu*>(guiFactory ()->container (name, this));
		if (menu) menu->menuAction ()->setVisible (!menu->isEmpty ());
	}
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

	statusBar ()->showMessage (ntext);
	statusBar ()->show ();
}

void DetachedWindowContainer::slotReattach () {
	RK_TRACE (APP);

	RKWorkplace::mainWorkplace ()->attachWindow (captured);
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
