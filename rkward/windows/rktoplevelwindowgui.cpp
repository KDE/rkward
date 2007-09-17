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
#include <kaboutapplication.h>

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
	KAction *help_invoke_r_help = new KAction (i18n ("Help on R"), 0, 0, this, SLOT (invokeRHelp ()), actionCollection(), "invoke_r_help");
	KAction *show_help_search = new KAction (i18n ("Search R Help"), 0, 0, this, SLOT (showHelpSearch ()), actionCollection(), "show_help_search");
	KAction *show_rkward_help = KStdAction::helpContents (this, SLOT (showRKWardHelp ()), actionCollection(), "rkward_help");
	show_rkward_help->setText (i18n ("Help on RKWard"));

	KStdAction::aboutApp (this, SLOT (showAboutApplication ()), actionCollection(), "about_app");
	KStdAction::whatsThis (for_window, SLOT (whatsThis ()), actionCollection(), "whats_this");
	KStdAction::reportBug (this, SLOT (reportRKWardBug ()), actionCollection(), "report_bug");

	help_invoke_r_help->setStatusText (i18n ("Shows the R help index"));
	show_help_search->setStatusText (i18n ("Shows/raises the R Help Search window"));
	show_rkward_help->setStatusText (i18n ("Show help on RKWard"));

	// window menu
	new KAction (i18n ("Show/Hide Workspace Browser"), 0, KShortcut ("Alt+1"), this, SLOT (toggleWorkspace()), actionCollection (), "window_show_workspace");
	new KAction (i18n ("Show/Hide Filesystem Browser"), 0, KShortcut ("Alt+2"), this, SLOT (toggleFilebrowser()), actionCollection (), "window_show_filebrowser");
	new KAction (i18n ("Show/Hide Command Log"), 0, KShortcut ("Alt+3"), this, SLOT (toggleCommandLog()), actionCollection (), "window_show_commandlog");
	new KAction (i18n ("Show/Hide Pending Jobs"), 0, KShortcut ("Alt+4"), this, SLOT (togglePendingJobs()), actionCollection (), "window_show_pendingjobs");
	new KAction (i18n ("Show/Hide Console"), 0, KShortcut ("Alt+5"), this, SLOT (toggleConsole()), actionCollection (), "window_show_console");
	new KAction (i18n ("Show/Hide R Help Search"), 0, KShortcut ("Alt+6"), this, SLOT (toggleHelpSearch()), actionCollection (), "window_show_helpsearch");
	new KAction (i18n ("Activate Document view"), 0, KShortcut ("Alt+0"), this, SLOT (activateDocumentView()), actionCollection (), "window_activate_docview");

	new KAction (i18n ("Show &Output"), 0, 0, this, SLOT (slotOutputShow ()), actionCollection (), "output_show");

	actionCollection ()->setWidget (for_window);
	actionCollection ()->setHighlightingEnabled (true);
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

	KAboutApplication *about = new KAboutApplication ();
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

	RKWorkplace::mainWorkplace ()->openHelpWindow ("rkward://page/rkward_welcome", true);
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

	RKWorkplace::mainWorkplace ()->openOutputWindow (KURL ());
}

#include "rktoplevelwindowgui.moc"
