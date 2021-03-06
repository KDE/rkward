/***************************************************************************
                          rktoplevelwindowgui  -  description
                             -------------------
    begin                : Tue Apr 24 2007
    copyright            : (C) 2007-2020 by Thomas Friedrichsmeier
    email                : thomas.friedrichsmeier@kdemail.net
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

#include <KLocalizedString>
#include <kmessagebox.h>
#include <KAboutData>
#include <kaboutapplicationdialog.h>
#include <kactioncollection.h>
#include <kxmlguifactory.h>
#include <kshortcutsdialog.h>
#include <KHelpMenu>

#include <QWhatsThis>
#include <QDomDocument>
#include <QDomElement>

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
#include "../misc/rkprogresscontrol.h"
#include "../misc/rkcommonfunctions.h"
#include "../plugin/rkcomponentmap.h"
#include "../dialogs/rkerrordialog.h"
#include "../rbackend/rkrinterface.h"
#include "../rkglobals.h"
#include "../rkward.h"

#include "../debug.h"

RKTopLevelWindowGUI::RKTopLevelWindowGUI(KXmlGuiWindow *for_window) : QObject(for_window), KXMLGUIClient(), help_menu_dummy(nullptr) {
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

	actionCollection()->addAction(KStandardAction::AboutApp, "about_app", this, SLOT(showAboutApplication()));
	actionCollection()->addAction(KStandardAction::WhatsThis, "whats_this", this, SLOT(startWhatsThis()));
	actionCollection()->addAction(KStandardAction::ReportBug, "report_bug", this, SLOT(reportRKWardBug()));
	actionCollection()->addAction(KStandardAction::SwitchApplicationLanguage, "switch_application_language", this, SLOT(showSwitchApplicationLanguage()));

	help_invoke_r_help->setStatusTip (i18n ("Shows the R help index"));
	show_help_search->setStatusTip (i18n ("Shows/raises the R Help Search window"));
	show_rkward_help->setStatusTip (i18n ("Show help on RKWard"));

	// window menu
	// NOTE: enabling / disabling the prev/next actions is not a good idea. It will cause the script windows to "accept" their shortcuts, when disabled
	prev_action = actionCollection ()->addAction ("prev_window", this, SLOT (previousWindow()));
	prev_action->setText (i18n ("Previous Window"));
	prev_action->setIcon (QIcon (RKCommonFunctions::getRKWardDataDir () + "icons/window_back.png"));
	actionCollection ()->setDefaultShortcut (prev_action, Qt::ControlModifier + Qt::Key_Tab);
	next_action = actionCollection ()->addAction ("next_window", this, SLOT (nextWindow()));
	next_action->setText (i18n ("Next Window"));
	next_action->setIcon (QIcon (RKCommonFunctions::getRKWardDataDir () + "icons/window_forward.png"));
	actionCollection ()->setDefaultShortcut (next_action, Qt::ControlModifier + Qt::ShiftModifier + Qt::Key_Tab);

	QAction* action = actionCollection ()->addAction ("window_activate_docview", this, SLOT(activateDocumentView()));
	action->setText (i18n ("Activate Document view"));
	actionCollection ()->setDefaultShortcut (action, Qt::AltModifier + Qt::Key_0);

	action = actionCollection ()->addAction ("output_show", this, SLOT (slotOutputShow()));
	action->setText (i18n ("Show &Output"));
	action->setIcon (RKStandardIcons::getIcon (RKStandardIcons::WindowOutput));

	// settings
	KStandardAction::keyBindings (this, SLOT (configureShortcuts()), actionCollection ());
	KStandardAction::configureToolbars (this, SLOT (configureToolbars()), actionCollection ());
}

RKTopLevelWindowGUI::~RKTopLevelWindowGUI () {
	RK_TRACE (APP);
	delete help_menu_dummy;
}

void RKTopLevelWindowGUI::initToolWindowActions () {
	RK_TRACE (APP);

	// Tool window actions
	QString action_tag ("Action");
	QString name_attr ("name");
	QDomDocument doc = xmlguiBuildDocument ();
	if  (doc.documentElement ().isNull ()) doc = domDocument ();
	QDomElement menu = doc.elementsByTagName("Menu").at (1).toElement (); // NOTE: this is known to be the "Windows"-Menu
	QDomElement ref = menu.firstChildElement (action_tag);
	while (!ref.isNull() && ref.attribute (name_attr) != QLatin1String ("window_show_PLACEHOLDER")) {
		ref = ref.nextSiblingElement (action_tag);
	}
	QAction *action;
	foreach (const RKToolWindowList::ToolWindowRepresentation& rep, RKToolWindowList::registeredToolWindows ()) {
		QString id = QLatin1String ("window_show_") + rep.id;
		action = actionCollection ()->addAction (id, this, SLOT (toggleToolView()));
		action->setText (i18n ("Show/Hide %1", rep.window->shortCaption ()));
		action->setIcon (rep.window->windowIcon ());
		actionCollection ()->setDefaultShortcut (action, rep.default_shortcut);
		action->setProperty ("rk_toolwindow_id", rep.id);
		QDomElement e = doc.createElement (action_tag);
		e.setAttribute (name_attr, id);
		menu.insertBefore (e, ref);
	}
	setXMLGUIBuildDocument (doc);
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
	RKErrorDialog::reportBug (for_window);
}

void RKTopLevelWindowGUI::showAboutApplication () {
	RK_TRACE (APP);

	KAboutApplicationDialog about (KAboutData::applicationData ());
	about.exec ();
}

void RKTopLevelWindowGUI::showSwitchApplicationLanguage() {
	RK_TRACE (APP);

	// Uggh. No direct or static access to KSwitchLanguageDialog...
	if (!help_menu_dummy) help_menu_dummy = new KHelpMenu(for_window, QString(), false);
	help_menu_dummy->switchApplicationLanguage();
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

	RKMDIWindow *win = RKToolWindowList::findToolWindowById (act->property ("rk_toolwindow_id").toString ());
	RK_ASSERT (win);
	toggleToolView (win);
}

void RKTopLevelWindowGUI::showHelpSearch () {
	RK_TRACE (APP);

	RKHelpSearchWindow::mainHelpSearch ()->activate ();
}

void RKTopLevelWindowGUI::showRKWardHelp () {
	RK_TRACE (APP);

	RKWorkplace::mainWorkplace ()->openHelpWindow (QUrl("rkward://page/rkward_welcome"), true);
}

void RKTopLevelWindowGUI::activateDocumentView () {
	RK_TRACE (APP);

	RKMDIWindow *window = RKWorkplace::mainWorkplace ()->view ()->activePage ();
	if (window) window->activate ();
}

void RKTopLevelWindowGUI::slotOutputShow () {
	RK_TRACE (APP);

	RKWorkplace::mainWorkplace ()->openOutputWindow (QUrl ());
}

void RKTopLevelWindowGUI::nextWindow () {
	RK_TRACE (APP);

	// well, this is sort of cumbersome, but the switcher widget gets keyboard focus, and so we need to register the window switching actions with it.
	RKWorkplace::getHistory ()->next (prev_action, next_action);
}

void RKTopLevelWindowGUI::previousWindow () {
	RK_TRACE (APP);

	RKWorkplace::getHistory ()->prev (prev_action, next_action);
}

