/***************************************************************************
                          rktoplevelwindowgui  -  description
                             -------------------
    begin                : Tue Apr 24 2007
    copyright            : (C) 2007 by Thomas Friedrichsmeier
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

#include "rktoplevelwindowgui.h"

#include <klocale.h>
#include <kmessagebox.h>
#include <k3aboutapplication.h>
#include <kactioncollection.h>

#include "../rkconsole.h"
#include "../robjectbrowser.h"
#include "../windows/rkfilebrowser.h"
#include "../windows/rcontrolwindow.h"
#include "../windows/rkhtmlwindow.h"
#include "../windows/rkworkplaceview.h"
#include "../windows/rkworkplace.h"
#include "../windows/rkcommandlog.h"
#include "../windows/rkhelpsearchwindow.h"
#include "../windows/rkmdiwindow.h"
#include "../rbackend/rinterface.h"
#include "../rkglobals.h"
#include "../rkward.h"

#include "../debug.h"

RKTopLevelWindowGUI::RKTopLevelWindowGUI (QWidget *for_window) : QObject (for_window), KXMLGUIClient () {
	RK_TRACE (APP);

	RKTopLevelWindowGUI::for_window = for_window;

	setXMLFile ("rktoplevelwindowgui.rc");

	// help menu
	QAction *help_invoke_r_help = actionCollection ()->addAction ("invoke_r_help", this, SLOT(invokeRHelp()));
	help_invoke_r_help->setText (i18n ("Help on R"));
	QAction *show_help_search = actionCollection ()->addAction ("show_help_search", this, SLOT(showHelpSearch()));
	show_help_search->setText (i18n ("Search R Help"));
	QAction *show_rkward_help = actionCollection ()->addAction (KStandardAction::HelpContents, "rkward_help", this, SLOT (showRKWardHelp()));
	show_rkward_help->setText (i18n ("Help on RKWard"));

	actionCollection ()->addAction (KStandardAction::AboutApp, "about_app", this, SLOT (showAboutApplication()));
	actionCollection ()->addAction (KStandardAction::WhatsThis, "whats_this", for_window, SLOT (whatsThis()));
	actionCollection ()->addAction (KStandardAction::ReportBug, "report_bug", this, SLOT (reportRKWardBug()));

	help_invoke_r_help->setStatusTip (i18n ("Shows the R help index"));
	show_help_search->setStatusTip (i18n ("Shows/raises the R Help Search window"));
	show_rkward_help->setStatusTip (i18n ("Show help on RKWard"));

	// window menu
	QAction *action;
	action = actionCollection ()->addAction ("window_show_workspace", this, SLOT(toggleWorkspace()));
	action->setText (i18n ("Show/Hide Workspace Browser"));
	action->setShortcut (Qt::AltModifier + Qt::Key_1);
	action = actionCollection ()->addAction ("window_show_filebrowser", this, SLOT(toggleFilebrowser()));
	action->setText (i18n ("Show/Hide Filesystem Browser"));
	action->setShortcut (Qt::AltModifier + Qt::Key_2);
	action = actionCollection ()->addAction ("window_show_commandlog", this, SLOT(toggleCommandLog()));
	action->setText (i18n ("Show/Hide Command Log"));
	action->setShortcut (Qt::AltModifier + Qt::Key_3);
	action = actionCollection ()->addAction ("window_show_pendingjobs", this, SLOT(togglePendingJobs()));
	action->setText (i18n ("Show/Hide Pending Jobs"));
	action->setShortcut (Qt::AltModifier + Qt::Key_4);
	action = actionCollection ()->addAction ("window_show_console", this, SLOT(toggleConsole()));
	action->setText (i18n ("Show/Hide Console"));
	action->setShortcut (Qt::AltModifier + Qt::Key_5);
	action = actionCollection ()->addAction ("window_show_helpsearch", this, SLOT(toggleHelpSearch()));
	action->setText (i18n ("Show/Hide R Help Search"));
	action->setShortcut (Qt::AltModifier + Qt::Key_6);
	action = actionCollection ()->addAction ("window_activate_docview", this, SLOT(activateDocumentView()));
	action->setText (i18n ("Activate Document view"));
	action->setShortcut (Qt::AltModifier + Qt::Key_0);

	action = actionCollection ()->addAction ("output_show", this, SLOT (slotOutputShow()));
	action->setText (i18n ("Show &Output"));

	actionCollection ()->setAssociatedWidget (for_window);
}

RKTopLevelWindowGUI::~RKTopLevelWindowGUI () {
	RK_TRACE (APP);
}

void RKTopLevelWindowGUI::invokeRHelp () {
	RK_TRACE (APP);

	RKGlobals::rInterface ()->issueCommand ("help.start ()", RCommand::App);
	RKWardMainWindow::getMain ()->topLevelWidget ()->raise ();
}

void RKTopLevelWindowGUI::reportRKWardBug () {
	RK_TRACE (APP);

// TOOD: something pretty
	KMessageBox::information (for_window, i18n ("Please submit your bug reports or wishes at http://sourceforge.net/tracker/?group_id=50231&atid=459007 or send email to rkward-devel@lists.sourceforge.net"));
}

void RKTopLevelWindowGUI::showAboutApplication () {
	RK_TRACE (APP);

	K3AboutApplication *about = new K3AboutApplication ();
	about->exec ();
	delete about;
}

void RKTopLevelWindowGUI::toggleToolView (RKMDIWindow *tool_window) {
	RK_TRACE (APP);
	RK_ASSERT (tool_window);

	if (tool_window->isActive ()) {
		tool_window->close (false);
		activateDocumentView ();
	} else {
		tool_window->activate (true);
	}
}

void RKTopLevelWindowGUI::showHelpSearch () {
	RK_TRACE (APP);

	RKHelpSearchWindow::mainHelpSearch ()->activate ();
}

void RKTopLevelWindowGUI::showRKWardHelp () {
	RK_TRACE (APP);

	RKWorkplace::mainWorkplace ()->openHelpWindow (KUrl ("rkward://page/rkward_welcome"), true);
}

void RKTopLevelWindowGUI::toggleHelpSearch () {
	RK_TRACE (APP);

	toggleToolView (RKHelpSearchWindow::mainHelpSearch ());
}

void RKTopLevelWindowGUI::toggleConsole () {
	RK_TRACE (APP);

	toggleToolView (RKConsole::mainConsole ());
}

void RKTopLevelWindowGUI::toggleCommandLog () {
	RK_TRACE (APP);

	toggleToolView (RKCommandLog::getLog ());
}

void RKTopLevelWindowGUI::togglePendingJobs () {
	RK_TRACE (APP);

	toggleToolView (RControlWindow::getControl ());
}

void RKTopLevelWindowGUI::toggleWorkspace () {
	RK_TRACE (APP);

	toggleToolView (RObjectBrowser::mainBrowser ());
}

void RKTopLevelWindowGUI::toggleFilebrowser () {
	RK_TRACE (APP);

	toggleToolView (RKFileBrowser::getMainBrowser ());
}

void RKTopLevelWindowGUI::activateDocumentView () {
	RK_TRACE (APP);

	RKMDIWindow *window = RKWorkplace::mainWorkplace ()->view ()->activePage ();
	if (window) window->activate ();
}

void RKTopLevelWindowGUI::slotOutputShow () {
	RK_TRACE (APP);

	RKWorkplace::mainWorkplace ()->openOutputWindow (KUrl ());
}

#include "rktoplevelwindowgui.moc"
