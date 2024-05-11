/*
detachedwindowcontainer - This file is part of RKWard (https://rkward.kde.org). Created: Wed Oct 21 2005
SPDX-FileCopyrightText: 2005-2020 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "detachedwindowcontainer.h"

#include <KLocalizedString>
#include <kactioncollection.h>
#include <ktoolbar.h>
#include <kxmlguifactory.h>

#include <qlayout.h>
#include <qwidget.h>
#include <QCloseEvent>
#include <QMenu>
#include <QStatusBar>

#include "rktoplevelwindowgui.h"
#include "../rkward.h"
#include "../misc/rkstandardicons.h"
#include "../misc/rkxmlguisyncer.h"
#include "rkworkplace.h"

#include "../debug.h"

/* Warning! Do not pass a parent widget to the KParts::MainWindow. Otherwise there will be strange crahes while removing the KXMLGUIClients! (In this case: Open a help window, and detach it. Open another help window attached. Close the detached one, then close the attached one -> crash; KDE 3.5.5) */
DetachedWindowContainer::DetachedWindowContainer (RKMDIWindow *widget_to_capture, bool copy_geometry) : KParts::MainWindow  () {
	RK_TRACE (APP);

	actionCollection()->addAction(KStandardAction::Close, "dwindow_close", this, &DetachedWindowContainer::close);

	QAction *reattach = actionCollection()->addAction("dwindow_attach", this, &DetachedWindowContainer::slotReattach);
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
	const auto toolbars = RKWardMainWindow::getMain ()->toolBars ();
	for (KToolBar *bar : toolbars) {
		main_window_toolbar_styles.insert (bar->objectName (), bar->toolButtonStyle ());
	}

// capture widget
	if (copy_geometry) {
		setGeometry (widget_to_capture->frameGeometry ());
		if (!widget_to_capture->isWindow ()) move (widget_to_capture->mapToGlobal (widget_to_capture->pos ()));
#ifdef Q_OS_WIN
		// fix for detached tool windows positioned with the frame outside the screen
		ensurePolished ();
		QPoint adjust = pos ();
		if (adjust.x () < 0) adjust.setX (0);
		if (adjust.y () < 0) adjust.setY (0);
		if (adjust != pos ()) move (adjust);
#endif
	}
	widget_to_capture->setParent (this);
	setCentralWidget (widget_to_capture);
	widget_to_capture->show ();
	createGUI(widget_to_capture->getPart());
//	if (widget_to_capture->uiBuddy()) factory()->addClient(widget_to_capture->uiBuddy());
	captured = widget_to_capture;
	// Special case for graph windows: We don't want to touch their default size before showing. So tell the toolbars to step back, if needed.
	if (widget_to_capture->isType(RKMDIWindow::X11Window)) {
		const auto bars = toolBars();
		for (KToolBar *bar : bars) {
			bar->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);
		}
	}

	hideEmptyMenus ();
	// hide empty menus now, and after any reloads
	connect (guiFactory (), &KXMLGUIFactory::makingChanges, this, &DetachedWindowContainer::hideEmptyMenus);

// sanitize toolbars
	const auto bars = toolBars();
	for (KToolBar *bar : bars) {
		if (main_window_toolbar_styles.contains (bar->objectName ())) {
			bar->setToolButtonStyle (main_window_toolbar_styles[bar->objectName ()]);
		} else {
			RK_ASSERT (false);
		}
	}

// should self-destruct, when child widget is destroyed
	connect (widget_to_capture, &QObject::destroyed, this, &DetachedWindowContainer::viewDestroyed);
	connect (widget_to_capture, &RKMDIWindow::captionChanged, this, &DetachedWindowContainer::updateCaption);
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
	menu_names << "file" << "device" << "history" << "edit" << "run" << "view" << "settings";
	for (const QString& name : std::as_const(menu_names)) {
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
	ntext.replace ("<qt>", QString ());	// WORKAROUND: what the ?!? is going on? The KTHMLPart seems to post such messages.

	statusBar ()->showMessage (ntext);
	statusBar ()->show ();
}

void DetachedWindowContainer::slotReattach () {
	RK_TRACE (APP);

	RKWorkplace::mainWorkplace ()->attachWindow (captured);
	captured->activate ();
}

void DetachedWindowContainer::closeEvent (QCloseEvent *e) {
	RK_TRACE (APP);

	if (captured->close (RKMDIWindow::AutoAskSaveModified)) {
		e->accept ();
	} else {
		e->ignore ();
	}
}

