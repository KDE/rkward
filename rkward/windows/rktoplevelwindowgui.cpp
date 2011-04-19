/***************************************************************************
                          rktoplevelwindowgui  -  description
                             -------------------
    begin                : Tue Apr 24 2007
    copyright            : (C) 2007, 2009, 2011 by Thomas Friedrichsmeier
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
#include <kaboutapplicationdialog.h>
#include <kcmdlineargs.h>
#include <kactioncollection.h>
#include <kxmlguifactory.h>
#include <kshortcutsdialog.h>

#include <QWhatsThis>

#include "../rkconsole.h"
#include "../windows/robjectbrowser.h"
#include "../windows/rkfilebrowser.h"
#include "../windows/rcontrolwindow.h"
#include "../windows/rkhtmlwindow.h"
#include "../windows/rkworkplaceview.h"
#include "../windows/rkworkplace.h"
#include "../windows/rkcommandlog.h"
#include "../windows/rkhelpsearchwindow.h"
#include "../windows/rkmdiwindow.h"
#include "../misc/rkstandardicons.h"
#include "../plugin/rkcomponentmap.h"
#include "../rbackend/rinterface.h"
#include "../rkglobals.h"
#include "../rkward.h"

#include "../debug.h"

RKTopLevelWindowGUI::RKTopLevelWindowGUI (KXmlGuiWindow *for_window) : QObject (for_window), KXMLGUIClient () {
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
	actionCollection ()->addAction (KStandardAction::WhatsThis, "whats_this", this, SLOT (startWhatsThis()));
	actionCollection ()->addAction (KStandardAction::ReportBug, "report_bug", this, SLOT (reportRKWardBug()));

	help_invoke_r_help->setStatusTip (i18n ("Shows the R help index"));
	show_help_search->setStatusTip (i18n ("Shows/raises the R Help Search window"));
	show_rkward_help->setStatusTip (i18n ("Show help on RKWard"));

	// window menu
	KAction *action;
	foreach (RKToolWindowList::ToolWindowRepresentation rep, RKToolWindowList::registeredToolWindows ()) {
		action = actionCollection ()->addAction ("window_show_" + rep.id, this, SLOT (toggleToolView()));
		action->setText (i18n ("Show/Hide %1", rep.window->shortCaption ()));
		action->setIcon (rep.window->windowIcon ());
		action->setShortcut (rep.default_shortcut);
		action->setData (rep.id);
	}
	action = actionCollection ()->addAction ("window_activate_docview", this, SLOT(activateDocumentView()));
	action->setText (i18n ("Activate Document view"));
	action->setShortcut (Qt::AltModifier + Qt::Key_0);

	action = actionCollection ()->addAction ("output_show", this, SLOT (slotOutputShow()));
	action->setText (i18n ("Show &Output"));
	action->setIcon (RKStandardIcons::getIcon (RKStandardIcons::WindowOutput));

	// settings
	KStandardAction::keyBindings (this, SLOT (configureShortcuts ()), actionCollection ());
	KStandardAction::configureToolbars (this, SLOT (configureToolbars()), actionCollection ());
}

RKTopLevelWindowGUI::~RKTopLevelWindowGUI () {
	RK_TRACE (APP);
}

void RKTopLevelWindowGUI::configureShortcuts () {
	RK_TRACE (APP);

	KMessageBox::information (for_window, i18n ("For technical reasons, the following dialog allows you to configure the keyboard shortcuts only for those parts of RKWard that are currently active.\n\nTherefore, if you want to configure keyboard shortcuts e.g. for use inside the script editor, you need to open a script editor window, and activate it."), i18n ("Note"), "configure_shortcuts_kparts");

	KShortcutsDialog dlg (KShortcutsEditor::AllActions, KShortcutsEditor::LetterShortcutsAllowed, qobject_cast<QWidget*> (parent()));
	foreach (KXMLGUIClient *client, factory ()->clients ()) {
		if (client && !client->xmlFile ().isEmpty ()) dlg.addCollection (client->actionCollection());
	}
	dlg.addCollection (RKComponentMap::getMap ()->actionCollection (), i18n ("RKWard Plugins"));
	dlg.configure (true);
}

void RKTopLevelWindowGUI::configureToolbars () {
	RK_TRACE (APP);

	KMessageBox::information (for_window, i18n ("For technical reasons, the following dialog allows you to configure the toolbar buttons only for those parts of RKWard that are currently active.\n\nTherefore, if you want to configure tool buttons e.g. for use inside the script editor, you need to open a script editor window, and activate it."), i18n ("Note"), "configure_toolbars_kparts");

	for_window->configureToolbars ();
}

void RKTopLevelWindowGUI::invokeRHelp () {
	RK_TRACE (APP);

	RKGlobals::rInterface ()->issueCommand ("help.start ()", RCommand::App);
	RKWardMainWindow::getMain ()->topLevelWidget ()->raise ();
}

void RKTopLevelWindowGUI::startWhatsThis () {
	RK_TRACE (APP);

	QWhatsThis::enterWhatsThisMode ();
}

void RKTopLevelWindowGUI::reportRKWardBug () {
	RK_TRACE (APP);

// TOOD: something pretty
	KMessageBox::information (for_window, i18n ("<p>Please submit your bug reports or wishes at <a href=\"%1\">%1</a> or send email to <a href=\"mailto:%2\">%2</a>.</p>"
							, QString ("http://sourceforge.net/tracker/?group_id=50231&atid=459007"), QString ("rkward-devel@lists.sourceforge.net")),
							i18n ("Reporting bugs in RKWard"), QString (), KMessageBox::Notify | KMessageBox::AllowLink);
}

void RKTopLevelWindowGUI::showAboutApplication () {
	RK_TRACE (APP);

	KAboutApplicationDialog about (KCmdLineArgs::aboutData ());
	about.exec ();
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

void RKTopLevelWindowGUI::toggleToolView () {
	RK_TRACE (APP);
	QAction *act = dynamic_cast<QAction*> (sender ());
	RK_ASSERT (act);

	RKMDIWindow *win = RKToolWindowList::findToolWindowById (act->data ().toString ());
	RK_ASSERT (win);
	toggleToolView (win);
}

void RKTopLevelWindowGUI::showHelpSearch () {
	RK_TRACE (APP);

	RKHelpSearchWindow::mainHelpSearch ()->activate ();
}

void RKTopLevelWindowGUI::showRKWardHelp () {
	RK_TRACE (APP);

	RKWorkplace::mainWorkplace ()->openHelpWindow (KUrl ("rkward://page/rkward_welcome"), true);
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
