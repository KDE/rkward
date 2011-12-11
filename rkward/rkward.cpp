/***************************************************************************
                          rkward.cpp  -  description
                             -------------------
    begin                : Tue Oct 29 20:06:08 CET 2002
    copyright            : (C) 2002, 2005, 2006, 2007, 2008, 2009, 2011 by Thomas Friedrichsmeier 
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

// include files for QT
#include <qprinter.h>
#include <qpainter.h>
#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qtimer.h>
#include <QDBusConnection>
#include <QDesktopWidget>
#include <QLabel>
#include <QCloseEvent>

// include files for KDE
#include <kiconloader.h>
#include <kmessagebox.h>
#include <kencodingfiledialog.h>
#include <kmenubar.h>
#include <kstatusbar.h>
#include <klocale.h>
#include <kconfig.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kstandardaction.h>
#include <kinputdialog.h>
#include <kmultitabbar.h>
#include <ksqueezedtextlabel.h>
#include <kparts/partmanager.h>
#include <kxmlguifactory.h>
#include <kactioncollection.h>
#include <krecentfilesaction.h>
#include <khbox.h>
#include <ktoolbar.h>
#include <kactionmenu.h>

// application specific includes
#include "rkward.h"
#include "core/rkmodificationtracker.h"
#include "plugin/rkcomponentmap.h"
#include "settings/rksettings.h"
#include "settings/rksettingsmoduleplugins.h"
#include "settings/rksettingsmodulegeneral.h"
#include "settings/rksettingsmoduleoutput.h"
#include "settings/rksettingsmodulecommandeditor.h"
#include "rbackend/rinterface.h"
#include "core/robjectlist.h"
#include "core/renvironmentobject.h"
#include "misc/rkstandardicons.h"
#include "misc/rkcommonfunctions.h"
#include "misc/rkxmlguisyncer.h"
#include "rkglobals.h"
#include "dialogs/startupdialog.h"
#include "dialogs/rkloadlibsdialog.h"
#include "dialogs/rkimportdialog.h"
#include "dialogs/rkrecoverdialog.h"
#include "agents/rksaveagent.h"
#include "agents/rkloadagent.h"
#include "agents/rkquitagent.h"
#include "windows/robjectbrowser.h"
#include "windows/rcontrolwindow.h"
#include "windows/rkhtmlwindow.h"
#include "windows/rkworkplaceview.h"
#include "windows/rkworkplace.h"
#include "windows/rkcommandlog.h"
#include "windows/rkhelpsearchwindow.h"
#include "windows/rktoplevelwindowgui.h"
#include "windows/rkfilebrowser.h"
#include "windows/rktoolwindowlist.h"
#include "windows/rkdebugconsole.h"
#include "windows/rkcallstackviewer.h"
#include "rkconsole.h"
#include "debug.h"
#include "version.h"


#include "agents/showedittextfileagent.h"	// TODO: see below: needed purely for linking!
#include "dialogs/rkreadlinedialog.h"	// TODO: see below: needed purely for linking!
#include "dialogs/rkselectlistdialog.h"	// TODO: see below: needed purely for linking!
#include "windows/detachedwindowcontainer.h"	// TODO: see below: needed purely for linking!
#include "dataeditor/rkeditordataframe.h"	// TODO: see below: needed purely for linking!
#include "agents/rkeditobjectagent.h"	// TODO: see below: needed purely for linking!
#include "agents/rkprintagent.h"	// TODO: see below: needed purely for linking!

// This nevers gets called. It's needed to trick ld into linking correctly. Nothing else.
void bogusCalls () {
	ShowEditTextFileAgent::showEditFiles (0);		// TODO: AAAAAAAARGGGH!!!! It won't link without this bogus line!!!
	RKReadLineDialog::readLine (0, QString(), QString(), 0, 0);	// TODO: see above
	RKSelectListDialog::doSelect (0, QString(), QStringList(), QStringList(), false);	// TODO: see above
	new RKEditorDataFrame (0, 0);
	DetachedWindowContainer (0);
	new RKWorkplaceView (0);
	new RKEditObjectAgent (QStringList (), 0);
	RKPrintAgent::printPostscript (QString (), false);
}

/** Main window **/

//static
RKWardMainWindow *RKWardMainWindow::rkward_mainwin = 0;

RKWardMainWindow::RKWardMainWindow (RKWardStartupOptions *options) : KParts::MainWindow ((QWidget *)0, (Qt::WindowFlags) KDE_DEFAULT_WINDOWFLAGS) {
	RK_TRACE (APP);
	RK_ASSERT (rkward_mainwin == 0);

	gui_rebuild_locked = true;
	rkward_mainwin = this;
	RKGlobals::rinter = 0;
	RKSettings::settings_tracker = new RKSettingsTracker (this);

	///////////////////////////////////////////////////////////////////
	// call inits to invoke all other construction parts
	RKStandardIcons::initIcons ();
	initActions();
	initStatusBar();

	new RKWorkplace (this);
	RKWorkplace::mainWorkplace ()->initActions (actionCollection (), "left_window", "right_window");
	setCentralWidget (RKWorkplace::mainWorkplace ());
	connect (RKWorkplace::mainWorkplace ()->view (), SIGNAL (captionChanged (const QString &)), this, SLOT (setCaption (const QString &)));

	part_manager = new KParts::PartManager (this);
	// When the manager says the active part changes,
	// the builder updates (recreates) the GUI
	connect (partManager (), SIGNAL (activePartChanged (KParts::Part *)), this, SLOT (partChanged (KParts::Part *)));

	readOptions();
	RKGlobals::mtracker = new RKModificationTracker (this);
	initToolViewsAndR ();

	///////////////////////////////////////////////////////////////////
	// build the interface

	setHelpMenuEnabled (false);
	setXMLFile ("rkwardui.rc");
	insertChildClient (toplevel_actions = new RKTopLevelWindowGUI (this));
	createShellGUI (true);
	RKXMLGUISyncer::self ()->watchXMLGUIClientUIrc (this);

	proxy_import->setMenu (dynamic_cast<QMenu*>(guiFactory ()->container ("import", this)));
	proxy_export->setMenu (dynamic_cast<QMenu*>(guiFactory ()->container ("export", this)));

	RKComponentMap::initialize ();

	startup_options = options;

	// stuff which should wait until the event loop is running
	QTimer::singleShot (0, this, SLOT (doPostInit ()));
}

RKWardMainWindow::~RKWardMainWindow() {
	RK_TRACE (APP);

	// these would not be strictly necessary, as we're exiting the app, anyway.
	delete RObjectList::getObjectList ();
	delete RObjectBrowser::mainBrowser ();
	delete RKCommandLog::getLog ();
	delete RKConsole::mainConsole ();
	delete RKHelpSearchWindow::mainHelpSearch ();
	delete RKGlobals::tracker ();
	delete RKGlobals::rInterface ();
	delete RControlWindow::getControl ();
}

void RKWardMainWindow::closeEvent (QCloseEvent *e) {
	RK_TRACE (APP);

	if (RKQuitAgent::quittingInProgress ()) {
		KParts::MainWindow::closeEvent (e);
		return;
	}

	e->ignore ();
	if (doQueryQuit ()) {
		emit (aboutToQuitRKWard());
		new RKQuitAgent (this);
	}
}

void RKWardMainWindow::doPostInit () {
	RK_TRACE (APP);

	// Check installation first
	QFile resource_ver (RKCommonFunctions::getRKWardDataDir () + "resource.ver");
	if (!(resource_ver.open (QIODevice::ReadOnly) && (resource_ver.read (100).trimmed () == RKWARD_VERSION))) {
		KMessageBox::error (this, i18n ("<p>RKWard either could not find its resource files at all, or only an old version of those files. The most likely cause is that the last installation failed to place the files in the correct place. This can lead to all sorts of problems, from single missing features to complete failure to function.</p><p><b>You should quit RKWard, now, and fix your installation</b>. For help with that, see <a href=\"http://p.sf.net/rkward/compiling\">http://p.sf.net/rkward/compiling</a>.</p>"), i18n ("Broken installation"), KMessageBox::Notify | KMessageBox::AllowLink);
	}

	// startup options will be deleted from the R thread (TODO correct this!), so we need to copy the initial_url here, or run into race conditions
	KUrl open_url = startup_options ? startup_options->initial_url : KUrl ();
	QString evaluate_code = startup_options ? startup_options->evaluate : QString ();
	delete startup_options;
	startup_options = 0;

	initPlugins ();
	gui_rebuild_locked = false;

	show ();
#ifdef Q_WS_WIN
	// detect and disable the buggy "native" file dialogs
	KConfigGroup cg = KGlobal::config ().data ()->group ("KFileDialog Settings");
	if (cg.readEntry ("Native", true)) {
		int res = KMessageBox::questionYesNo (this, i18n ("Your installation of KDE is configured to use \"native\" file dialogs. This is known to cause issues in some cases, and we recommend to disable \"native\" file dialogs.\nShould \"native\" file dialogs be disabled in RKWard?"),
							i18n ("Potential problem with your configuration"), KGuiItem (i18n ("Yes, disable")), KGuiItem (i18n ("No, use \"native\" file dialogs")), "windows_native_kfiledialog");
		if (res != KMessageBox::No) {
			cg.writeEntry ("Native", false);
			cg.sync ();
		}
	}
#endif

	KUrl recover_url = RKRecoverDialog::checkRecoverCrashedWorkspace ();
	if (!recover_url.isEmpty ()) open_url = recover_url;

	if (!open_url.isEmpty()) {
		openWorkspace (open_url);
	} else {
		StartupDialog::StartupDialogResult result = StartupDialog::getStartupAction (this, fileOpenRecentWorkspace);
		if (!result.open_url.isEmpty ()) {
			openWorkspace (result.open_url);
		} else {
			if (result.result == StartupDialog::ChoseFile) {
				fileOpenNoSave (KUrl());
			} else if (result.result == StartupDialog::EmptyTable) {
				RKWorkplace::mainWorkplace ()->editNewDataFrame (i18n ("my.data"));
			}
		}
	}

	if (RKSettingsModuleGeneral::workplaceSaveMode () == RKSettingsModuleGeneral::SaveWorkplaceWithSession) {
		RKWorkplace::mainWorkplace ()->restoreWorkplace (RKSettingsModuleGeneral::getSavedWorkplace (KGlobal::config ().data ()).split ('\n'));
	}

	if (RKSettingsModuleGeneral::showHelpOnStartup ()) {
		toplevel_actions->showRKWardHelp ();
	}

	if (!evaluate_code.isEmpty ()) RKConsole::pipeUserCommand (evaluate_code);

	setCaption (QString ());	// our version of setCaption takes care of creating a correct caption, so we do not need to provide it here
}

void RKWardMainWindow::initPlugins () {
	RK_TRACE (APP);
	slotSetStatusBarText(i18n("Setting up plugins..."));
	
	factory ()->removeClient (RKComponentMap::getMap ());
	RKComponentMap::clearAll ();

	QStringList list = RKSettingsModulePlugins::pluginMaps ();
	int counter = 0;
	for (QStringList::const_iterator it = RKSettingsModulePlugins::pluginMaps ().begin (); it != RKSettingsModulePlugins::pluginMaps ().end (); ++it) {
		counter += RKComponentMap::addPluginMap ((*it));
	}

	if (counter < 1) {
		KMessageBox::information (0, i18n ("Plugins are needed: you may manage these through \"Settings->Configure RKWard\".\n"), i18n ("No (valid) plugins found"));
	}

	factory ()->addClient (RKComponentMap::getMap ());

	slotSetStatusReady ();
}

void RKWardMainWindow::startR () {
	RK_TRACE (APP);
	RK_ASSERT (!RKGlobals::rInterface ());

	// make sure our general purpose files directory exists
	bool ok = QDir ().mkpath (RKSettingsModuleGeneral::filesPath());
	RK_ASSERT (ok);

	RKGlobals::rinter = new RInterface ();
	new RObjectList ();
	connect (RObjectList::getObjectList (), SIGNAL (workspaceUrlChanged(const KUrl&)), this, SLOT (addWorkspaceUrl(const KUrl&)));

	RObjectBrowser::mainBrowser ()->unlock ();
}

void RKWardMainWindow::slotConfigure () {
	RK_TRACE (APP);
	RKSettings::configureSettings (RKSettings::NoPage, this);
}

void RKWardMainWindow::slotCancelAllCommands () {
	RK_TRACE (APP);
	RK_ASSERT (RKGlobals::rInterface ());
	RKGlobals::rInterface ()->cancelAll ();
}

void RKWardMainWindow::configureCarbonCopy () {
	RK_TRACE (APP);

	KDialog *dialog = new KDialog ();
	dialog->setCaption (i18n ("Carbon Copy Settings"));
	RKCarbonCopySettings *settings = new RKCarbonCopySettings (dialog);
	dialog->setMainWidget (settings);
	dialog->setButtons (KDialog::Ok | KDialog::Apply | KDialog::Cancel);
	dialog->setAttribute (Qt::WA_DeleteOnClose);
	connect (dialog, SIGNAL (okClicked()), settings, SLOT (applyChanges ())); 
	connect (dialog, SIGNAL (applyClicked()), settings, SLOT (applyChanges ())); 
	dialog->show ();
}

void RKWardMainWindow::initToolViewsAndR () {
	RK_TRACE (APP);

	RObjectBrowser::object_browser = new RObjectBrowser (0, true);
	RObjectBrowser::mainBrowser ()->setCaption (i18n ("Workspace"));
	RKToolWindowList::registerToolWindow (RObjectBrowser::mainBrowser (), "workspace", RKToolWindowList::Left, Qt::AltModifier + Qt::Key_1);

	RKCommandLog::rkcommand_log = new RKCommandLog (0, true);
	RKToolWindowList::registerToolWindow (RKCommandLog::rkcommand_log, "commandlog", RKToolWindowList::Bottom, Qt::AltModifier + Qt::Key_3);

	startR ();

	RKFileBrowser::main_browser = new RKFileBrowser (0, true);
	RKFileBrowser::main_browser->setCaption (i18n ("Files"));
	RKToolWindowList::registerToolWindow (RKFileBrowser::main_browser, "filebrowser", RKToolWindowList::Left, Qt::AltModifier + Qt::Key_2);

	RControlWindow::control_window = new RControlWindow (0, true);
	RControlWindow::getControl ()->setCaption (i18n ("Pending Jobs"));
	RKToolWindowList::registerToolWindow (RControlWindow::getControl (), "pendingjobs", RKToolWindowList::Nowhere, Qt::AltModifier + Qt::Key_4);

	RKConsole *console = new RKConsole (0, true);
	RKConsole::setMainConsole (console);
	RKToolWindowList::registerToolWindow (console, "console", RKToolWindowList::Bottom, Qt::AltModifier + Qt::Key_5);

	RKHelpSearchWindow *help_search = new RKHelpSearchWindow (0, true);
	RKHelpSearchWindow::main_help_search = help_search;
	RKToolWindowList::registerToolWindow (help_search, "helpsearch", RKToolWindowList::Bottom, Qt::AltModifier + Qt::Key_6);

	RKCallstackViewer::_instance = new RKCallstackViewer (0, true);
	RKCallstackViewer::instance ()->setCaption (i18n ("Debugger Frames"));
	RKToolWindowList::registerToolWindow (RKCallstackViewer::instance (), "debugframes", RKToolWindowList::Right, Qt::AltModifier + Qt::Key_8);

	// HACK: Creating this _after_ the callstackviewer is important, so the debug console will end up the active window when entering a debug context
	RKDebugConsole::_instance = new RKDebugConsole (0, true);
	RKDebugConsole::instance ()->setCaption (i18n ("Debugger Console"));
	RKToolWindowList::registerToolWindow (RKDebugConsole::instance (), "debugconsole", RKToolWindowList::Nowhere, Qt::AltModifier + Qt::Key_7);

	RKWorkplace::mainWorkplace ()->placeToolWindows ();
}

void RKWardMainWindow::initActions() {  
	RK_TRACE (APP);
	KAction *action;

	// TODO: is there a way to insert actions between standard actions without having to give all standard actions custom ids?
	new_data_frame = actionCollection ()->addAction ("new_data_frame", this, SLOT (slotNewDataFrame ()));
	new_data_frame->setText (i18n ("Dataset"));
	new_data_frame->setIcon (RKStandardIcons::getIcon (RKStandardIcons::WindowDataFrameEditor));
	new_data_frame->setStatusTip (i18n ("Creates new empty dataset and opens it for editing"));

	new_command_editor = actionCollection ()->addAction (KStandardAction::New, "new_command_editor", this, SLOT(slotNewCommandEditor()));
	new_command_editor->setText (i18n ("Script File"));
	new_command_editor->setIcon (RKStandardIcons::getIcon (RKStandardIcons::WindowCommandEditor));

	fileOpen = actionCollection ()->addAction (KStandardAction::Open, "file_openy", this, SLOT(slotOpenCommandEditor()));
	fileOpen->setText (i18n ("Open R Script File..."));

	fileOpenRecent = static_cast<KRecentFilesAction*> (actionCollection ()->addAction (KStandardAction::OpenRecent, "file_open_recenty", this, SLOT(slotOpenCommandEditor (const KUrl&))));
	fileOpenRecent->setText (i18n ("Open Recent R Script File"));

#ifdef Q_WS_WIN
	// TODO: find the cause and fix it! http://sourceforge.net/tracker/?func=detail&aid=2848341&group_id=50231&atid=459007
#	warning TODO: import data dialog is disabled on windows due to bug in kdelibs
#else
	action = actionCollection ()->addAction ("import_data", this, SLOT (importData()));
	action->setText (i18n ("Import Data"));
	action->setStatusTip (i18n ("Import data from a variety of file formats"));
#endif

	fileOpenWorkspace = actionCollection ()->addAction (KStandardAction::Open, "file_openx", this, SLOT(slotFileOpenWorkspace()));
	fileOpenWorkspace->setText (i18n ("Open Workspace..."));
	fileOpenWorkspace->setShortcut (Qt::ControlModifier + Qt::ShiftModifier + Qt::Key_O);
	fileOpenWorkspace->setStatusTip (i18n ("Opens an existing document"));

	fileOpenRecentWorkspace = static_cast<KRecentFilesAction*> (actionCollection ()->addAction (KStandardAction::OpenRecent, "file_open_recentx", this, SLOT(slotFileOpenRecentWorkspace(const KUrl&))));
	fileOpenRecentWorkspace->setText (i18n ("Open Recent Workspace"));
	fileOpenRecentWorkspace->setStatusTip (i18n ("Opens a recently used file"));

	fileSaveWorkspace = actionCollection ()->addAction (KStandardAction::Save, "file_savex", this, SLOT(slotFileSaveWorkspace()));
	fileSaveWorkspace->setText (i18n ("Save Workspace"));
	fileSaveWorkspace->setShortcut (Qt::ControlModifier + Qt::ShiftModifier + Qt::Key_S);
	fileSaveWorkspace->setStatusTip (i18n ("Saves the actual document"));

	fileSaveWorkspaceAs = actionCollection ()->addAction (KStandardAction::SaveAs, "file_save_asx", this, SLOT(slotFileSaveWorkspaceAs()));
	fileSaveWorkspaceAs->setText (i18n ("Save Workspace As"));
	fileSaveWorkspaceAs->setStatusTip (i18n ("Saves the actual document as..."));

	fileQuit = actionCollection ()->addAction (KStandardAction::Quit, "file_quitx", this, SLOT(close()));
	fileQuit->setStatusTip (i18n ("Quits the application"));

	interrupt_all_commands = actionCollection ()->addAction ("cancel_all_commands", this, SLOT (slotCancelAllCommands()));
	interrupt_all_commands->setText (i18n ("Interrupt all commands"));
	interrupt_all_commands->setShortcut (Qt::ShiftModifier + Qt::Key_Escape);
	interrupt_all_commands->setIcon (RKStandardIcons::getIcon (RKStandardIcons::ActionInterrupt));
	interrupt_all_commands->setEnabled (false);		// enabled from within setRStatus()

	action = actionCollection ()->addAction ("carbon_copy", this, SLOT (configureCarbonCopy ()));
	action->setText (i18n ("CC commands to output..."));

	// These two currently do the same thing
	action = actionCollection ()->addAction ("load_unload_libs", this, SLOT (slotFileLoadLibs()));
	action->setText (i18n ("Manage R packages..."));
	action->setIcon (RKStandardIcons::getIcon (RKStandardIcons::ActionConfigurePackages));

	action = actionCollection ()->addAction ("configure_packages", this, SLOT (slotFileLoadLibs()));
	action->setText (i18n ("Manage R packages..."));
	action->setIcon (RKStandardIcons::getIcon (RKStandardIcons::ActionConfigurePackages));

	setStandardToolBarMenuEnabled (true);
	createStandardStatusBarAction ();

	close_all_editors = actionCollection ()->addAction ("close_all_editors", this, SLOT (slotCloseAllEditors()));
	close_all_editors->setText (i18n ("Close All Data"));
	close_all_editors->setStatusTip (i18n ("Closes all open data editors"));

	action = actionCollection ()->addAction (KStandardAction::Close, "window_close", this, SLOT (slotCloseWindow()));

	window_close_all = actionCollection ()->addAction ("window_close_all", this, SLOT (slotCloseAllWindows()));
	window_close_all->setText (i18n ("Close All"));

	window_detach = actionCollection ()->addAction ("window_detach", this, SLOT (slotDetachWindow()));
	window_detach->setIcon (RKStandardIcons::getIcon (RKStandardIcons::ActionDetachWindow));
	window_detach->setText (i18n ("Detach"));

	configure = actionCollection ()->addAction (KStandardAction::Preferences, "options_configure", this, SLOT (slotConfigure()));

	edit_menu_dummy = actionCollection ()->addAction ("edit_menu_dummy", this);
	edit_menu_dummy->setText (i18n ("[No actions available for current view]"));
	edit_menu_dummy->setEnabled (false);
	view_menu_dummy = actionCollection ()->addAction ("view_menu_dummy", this);
	view_menu_dummy->setText (edit_menu_dummy->text ());
	view_menu_dummy->setEnabled (false);

	// collections for the toolbar:
	KActionMenu* open_any_action = new KActionMenu (KIcon ("document-open-folder"), i18n ("Open..."), this);
	open_any_action->setDelayed (false);
	actionCollection ()->addAction ("open_any", open_any_action);

	open_any_action->addAction (fileOpenWorkspace);
	open_any_action->addAction (fileOpenRecentWorkspace);
	open_any_action->addSeparator ();
	open_any_action->addAction (fileOpen);
	open_any_action->addAction (fileOpenRecent);
	open_any_action->addSeparator ();
	proxy_import = new KAction (i18n ("Import"), this);
	open_any_action->addAction (proxy_import);

	KActionMenu* new_any_action = new KActionMenu (KIcon ("document-new"), i18n ("Create..."), this);
	new_any_action->setDelayed (false);
	actionCollection ()->addAction ("new_any", new_any_action);

	new_any_action->addAction (new_data_frame);
	new_any_action->addAction (new_command_editor);

	save_any_action = new KActionMenu (KIcon ("document-save"), i18n ("Save..."), this);
	save_any_action->setDelayed (false);
	actionCollection ()->addAction ("save_any", save_any_action);

	proxy_export = new KAction (i18n ("Export"), this);
	save_any_action->addAction (fileSaveWorkspace);
	save_any_action->addAction (fileSaveWorkspaceAs);
	save_any_action->addSeparator ();
// TODO: A way to add R-script-save actions, dynamically, would be nice
	save_actions_plug_point = save_any_action->addSeparator ();
	save_any_action->addAction (proxy_export);
}

/*
// debug code: prints out all current actions
void printActionsRecursive (QAction* action, const QString &prefix) {
	if (action->menu ()) {
		foreach (QAction *a, action->menu ()->actions ()) printActionsRecursive (a, prefix + action->text () + "->");
	} else {
		qDebug ("%s", qPrintable (prefix + action->text ()));
	}
}
*/

void updateEmptyMenuIndicator (QAction* indicator, const QMenu *menu) {
	if (!menu) {
		indicator->setVisible (false);
		return;
	}

	// NOTE: QMenu::isEmpty () does not work, here
	QList<QAction *> actions = menu->actions ();
	for (int i = 0; i < actions.size (); ++i) {
		if (actions[i] == indicator) continue;
		if (actions[i]->isSeparator ()) continue;
		if (actions[i]->isVisible ()) {
			indicator->setVisible (false);
			return;
		}
	}

	indicator->setVisible (true);
}

void RKWardMainWindow::partChanged (KParts::Part *part) {
	RK_TRACE (APP);

	if (gui_rebuild_locked) return;
	createGUI (part);

	if (!guiFactory ()) {
		RK_ASSERT (false);
		return;
	}

	updateEmptyMenuIndicator (edit_menu_dummy, dynamic_cast<QMenu*>(guiFactory ()->container ("edit", this)));
	updateEmptyMenuIndicator (view_menu_dummy, dynamic_cast<QMenu*>(guiFactory ()->container ("view", this)));

	// plug save file actions into the toolbar collections
	RK_ASSERT (save_any_action);
	for (int i = 0; i < plugged_save_actions.size (); ++i) {
		QAction* a = plugged_save_actions[i].data ();
		if (a) save_any_action->removeAction (a);
	}
	plugged_save_actions.clear ();

	RKMDIWindow *w = RKWorkplace::mainWorkplace ()->activeWindow (RKMDIWindow::Attached);
	if (w && (w->isType (RKMDIWindow::CommandEditorWindow))) {
		QAction *a = static_cast<RKCommandEditorWindow*>(w)->fileSaveAction ();
		if (a) plugged_save_actions.append (a);
		a = static_cast<RKCommandEditorWindow*>(w)->fileSaveAsAction ();
		if (a) plugged_save_actions.append (a);
	}
	for (int i = 0; i < plugged_save_actions.size (); ++i) {
		save_any_action->insertAction (save_actions_plug_point, plugged_save_actions[i]);
	}
/*
	// debug code: prints out all current actions
	foreach (QAction *action, menuBar ()->actions ()) printActionsRecursive (action, QString ());
*/
}

void RKWardMainWindow::lockGUIRebuild (bool lock) {
	RK_TRACE (APP);

	if (lock) {
		RK_ASSERT (!gui_rebuild_locked);
		gui_rebuild_locked = true;
	} else {
		gui_rebuild_locked = false;
		partChanged (part_manager->activePart ());
	}
}

void RKWardMainWindow::initStatusBar () {
	RK_TRACE (APP);

	statusbar_ready = new QLabel (i18n ("Ready."), statusBar ());
	statusBar ()->addWidget (statusbar_ready);
	statusbar_cwd = new KSqueezedTextLabel (statusBar ());
	statusbar_cwd->setAlignment (Qt::AlignRight);
	statusbar_cwd->setToolTip (i18n ("Current working directory"));
	statusBar ()->addWidget (statusbar_cwd, 10);
	updateCWD ();

	KHBox *box = new KHBox (statusBar ());
	box->setSpacing (0);
	statusbar_r_status = new QLabel ("&nbsp;<b>R</b>&nbsp;", box);
	statusbar_r_status->setFixedHeight (statusBar ()->fontMetrics ().height () + 2);

	QToolButton* dummy = new QToolButton (box);
	dummy->setDefaultAction (interrupt_all_commands);
	dummy->setFixedHeight (statusbar_r_status->height ());
	dummy->setAutoRaise (true);

	statusBar ()->addPermanentWidget (box, 0);
	setRStatus (Starting);
}

void RKWardMainWindow::openWorkspace (const KUrl &url) {
	RK_TRACE (APP);
	if (url.isEmpty ()) return;

	new RKLoadAgent (url, false);
}

void RKWardMainWindow::saveOptions () {
	RK_TRACE (APP);
	KConfig *config = KGlobal::config ().data ();

	saveMainWindowSettings (config->group ("main window options"));

	KConfigGroup cg = config->group ("General Options");
// TODO: WORKAROUND. See corresponding line in readOptions ()
	cg.writeEntry("Geometry", size ());

	fileOpenRecentWorkspace->saveEntries (config->group ("Recent Files"));
	fileOpenRecent->saveEntries (config->group ("Recent Command Files"));

	RKSettings::saveSettings (config);

	config->sync ();
}


void RKWardMainWindow::readOptions () {
	RK_TRACE (APP);
	// first make sure to give the global defaults a chance, if needed.
	// TODO: Why don't the toolbars honor the global style automatically?
	QList<KToolBar*> tool_bars = toolBars ();
	for (int i=0; i < tool_bars.size (); ++i) {
		tool_bars[i]->setToolButtonStyle (KToolBar::toolButtonStyleSetting ());
	}

	KConfig *config = KGlobal::config ().data ();

	applyMainWindowSettings (config->group ("main window options"), true);

// TODO: WORKAROUND: Actually applyMainWindowSettings could/should do this, but apparently this just does not work for maximized windows. Therefore we use our own version instead.
// KDE4: still needed?
	KConfigGroup cg = config->group ("General Options");
	QSize size = cg.readEntry ("Geometry", QSize ());
	if (size.isEmpty ()) {
		size = QApplication::desktop ()->availableGeometry ().size ();
	}
	resize (size);

	RKSettings::loadSettings (config);
	RK_ASSERT (config == KGlobal::config ().data ());	// not messing with config groups

	// initialize the recent file list
	fileOpenRecentWorkspace->loadEntries (config->group ("Recent Files"));
	fileOpenRecent->setMaxItems (RKSettingsModuleCommandEditor::maxNumRecentFiles ());
	fileOpenRecent->loadEntries (config->group ("Recent Command Files"));
}

bool RKWardMainWindow::doQueryQuit () {
	RK_TRACE (APP);

	slotSetStatusBarText (i18n ("Exiting..."));
	saveOptions ();
	if (RKSettingsModuleGeneral::workplaceSaveMode () == RKSettingsModuleGeneral::SaveWorkplaceWithSession) {
		RKSettingsModuleGeneral::setSavedWorkplace (RKWorkplace::mainWorkplace ()->makeWorkplaceDescription ().join ("\n"), KGlobal::config ().data ());
	}

//	if (!RObjectList::getGlobalEnv ()->isEmpty ()) {
	int res;
	res = KMessageBox::questionYesNoCancel (this, i18n ("Quitting RKWard: Do you want to save the workspace?"), i18n ("Save Workspace?"), KStandardGuiItem::save (), KStandardGuiItem::discard (), KGuiItem (i18n ("Don't quit")));
	if (res == KMessageBox::Yes) {
		new RKSaveAgent (RObjectList::getObjectList ()->getWorkspaceURL (), false, RKSaveAgent::DoNothing);
	} else if (res == KMessageBox::Cancel) {
		slotSetStatusReady ();
		return false;
	}
//	}

	RKWorkplace::RKWorkplaceObjectList map = RKWorkplace::mainWorkplace ()->getObjectList ();
	for (RKWorkplace::RKWorkplaceObjectList::const_iterator it = map.constBegin (); it != map.constEnd (); ++it) {
		lockGUIRebuild (true);
		if (!(*it)->close (true)) {
			if (!(*it)->isType (RKMDIWindow::X11Window)) {	// X11 windows have a delayed close
				// If a child refuses to close, we return false.
				slotSetStatusReady ();
				lockGUIRebuild (false);
				return false;
			}
		}
		lockGUIRebuild (false);
	}

	return true;
}

void RKWardMainWindow::slotNewDataFrame () {
	RK_TRACE (APP);
	bool ok;

	QString name = KInputDialog::getText (i18n ("New dataset"), i18n ("Enter name for the new dataset"), "my.data", &ok, this);

	if (ok) RKWorkplace::mainWorkplace ()->editNewDataFrame (name);
}

void RKWardMainWindow::fileOpenNoSave (const KUrl &url) {
	RK_TRACE (APP);

	slotCloseAllEditors ();

	slotSetStatusBarText(i18n("Opening workspace..."));
	KUrl lurl = url;
	if (lurl.isEmpty ()) {
#ifdef Q_WS_WIN
	// getOpenUrl(KUrl("kfiledialog:///<rfiles>"), ...) causes a hang on windows (KDElibs 4.2.3).
#	warning Track this bug down and/or report it
		lurl = KFileDialog::getOpenUrl (KUrl (), i18n("%1|R Workspace Files (%1)\n*|All files", RKSettingsModuleGeneral::workspaceFilenameFilter ()), this, i18n("Select workspace to open..."));
#else
		lurl = KFileDialog::getOpenUrl (KUrl ("kfiledialog:///<rfiles>"), i18n("%1|R Workspace Files (%1)\n*|All files", RKSettingsModuleGeneral::workspaceFilenameFilter ()), this, i18n("Select workspace to open..."));
#endif
	}
	if (!lurl.isEmpty ()) {
		openWorkspace (lurl);
	}
	slotSetStatusReady ();
}

void RKWardMainWindow::fileOpenAskSave (const KUrl &url) {
	RK_TRACE (APP);
	if (RObjectList::getObjectList ()->isEmpty ()) {
		fileOpenNoSave (url);
		return;
	}
	
	int res;
	res = KMessageBox::questionYesNoCancel (this, i18n ("Do you want to save the current workspace?"), i18n ("Save Workspace?"));
	if (res == KMessageBox::No) {
		fileOpenNoSave (url);
	} else if (res == KMessageBox::Yes) {
		new RKSaveAgent (RObjectList::getObjectList ()->getWorkspaceURL (), false, RKSaveAgent::Load, url);
	}
	// else: cancel. Don't do anything
}

void RKWardMainWindow::slotFileOpenWorkspace () {
	RK_TRACE (APP);
	fileOpenAskSave (KUrl ());
}

void RKWardMainWindow::slotFileOpenRecentWorkspace(const KUrl& url)
{
	RK_TRACE (APP);
	fileOpenAskSave (url);
}

void RKWardMainWindow::slotFileLoadLibs () {
	RK_TRACE (APP);
	RKLoadLibsDialog *dial = new RKLoadLibsDialog (this, 0);
	dial->show ();
}

void RKWardMainWindow::slotFileSaveWorkspace () {
	RK_TRACE (APP);
	new RKSaveAgent (RObjectList::getObjectList ()->getWorkspaceURL ());
}

void RKWardMainWindow::slotFileSaveWorkspaceAs () {
	RK_TRACE (APP);
	new RKSaveAgent (RObjectList::getObjectList ()->getWorkspaceURL (), true);
}

void RKWardMainWindow::addWorkspaceUrl (const KUrl &url) {
	RK_TRACE (APP);

	if (!url.isEmpty ()) fileOpenRecentWorkspace->addUrl (url);
	setCaption (QString::null);	// trigger update of caption
}

void RKWardMainWindow::updateCWD () {
	RK_TRACE (APP);

	statusbar_cwd->setText (QDir::currentPath ());
}

void RKWardMainWindow::slotSetStatusBarText (const QString &text) {
	RK_TRACE (APP);

//KDE4: still needed?
	QString ntext = text.trimmed ();
	ntext.replace ("<qt>", "");	// WORKAROUND: what the ?!? is going on? The KTHMLPart seems to post such messages.
	if (ntext.isEmpty ()) {
		statusBar ()->clearMessage ();
	} else {
		statusBar ()->showMessage (ntext);
	}
}

void RKWardMainWindow::slotCloseWindow () {
	RK_TRACE (APP);

	RKWorkplace::mainWorkplace ()->closeActiveWindow ();
}

void RKWardMainWindow::slotCloseAllWindows () {
	RK_TRACE (APP);

	RKWorkplace::mainWorkplace ()->closeAll ();
}

void RKWardMainWindow::slotCloseAllEditors () {
	RK_TRACE (APP);

	RKWorkplace::mainWorkplace ()->closeAll (RKMDIWindow::DataEditorWindow);
}

void RKWardMainWindow::slotDetachWindow () {
	RK_TRACE (APP);

	RKWorkplace::mainWorkplace ()->detachWindow (RKWorkplace::mainWorkplace ()->activeWindow (RKMDIWindow::Attached));
}

void RKWardMainWindow::setRStatus (RStatus status) {
	RK_TRACE (APP);

	QColor status_color;
	if (status == Busy) {
		status_color = QColor (255, 0, 0);
		statusbar_r_status->setToolTip (i18n ("The <b>R</b> engine is busy."));
		interrupt_all_commands->setEnabled (true);
	} else if (status == Idle) {
		status_color = QColor (0, 255, 0);
		statusbar_r_status->setToolTip (i18n ("The <b>R</b> engine is idle."));
		interrupt_all_commands->setEnabled (false);
	} else {
		status_color = QColor (255, 255, 0);
		statusbar_r_status->setToolTip (i18n ("The <b>R</b> engine is being initialized."));
	}
	QPalette palette = statusbar_r_status->palette ();
	palette.setBrush (statusbar_r_status->backgroundRole(), QBrush (status_color));
	statusbar_r_status->setAutoFillBackground (true);
	statusbar_r_status->setPalette (palette);
}

void RKWardMainWindow::importData () {
	RK_TRACE (APP);

	new RKImportDialog ("import", this);
}

void RKWardMainWindow::slotNewCommandEditor () {
	RK_TRACE (APP);

	slotOpenCommandEditor (KUrl ());
}

void RKWardMainWindow::addScriptUrl (const KUrl& url) {
	RK_TRACE (APP);

	if (!url.isEmpty ()) fileOpenRecent->addUrl (url);
}

void RKWardMainWindow::slotOpenCommandEditor (const KUrl &url, const QString &encoding) {
	RK_TRACE (APP);

	RKWorkplace::mainWorkplace ()->openScriptEditor (url, encoding);
}

void RKWardMainWindow::slotOpenCommandEditor () {
	RK_TRACE (APP);
	KEncodingFileDialog::Result res;
	KUrl::List::const_iterator it;

#ifdef Q_WS_WIN
	// getOpenUrls(KUrl("kfiledialog:///<rfiles>"), ...) causes a hang on windows (KDElibs 4.2.3).
#	warning Track this bug down and/or report it
	res = KEncodingFileDialog::getOpenUrlsAndEncoding (QString (), QString (), QString ("%1|R Script Files (%1)\n*|All Files (*)").arg (RKSettingsModuleCommandEditor::scriptFileFilter ()), this, i18n ("Open script file(s)"));
#else
	res = KEncodingFileDialog::getOpenUrlsAndEncoding (QString (), "kfiledialog:///<rfiles>", QString ("%1|R Script Files (%1)\n*|All Files (*)").arg (RKSettingsModuleCommandEditor::scriptFileFilter ()), this, i18n ("Open script file(s)"));
#endif
	for (it = res.URLs.begin() ; it != res.URLs.end() ; ++it) {
		slotOpenCommandEditor (*it, res.encoding);
	}
};

void RKWardMainWindow::openHTML (const KUrl &url) {
	RK_TRACE (APP);

	RKWorkplace::mainWorkplace ()->openHelpWindow (url);
}

void RKWardMainWindow::setCaption (const QString &) {
	RK_TRACE (APP);

	QString wcaption = RObjectList::getObjectList ()->getWorkspaceURL ().fileName ();
	if (wcaption.isEmpty ()) wcaption = RObjectList::getObjectList ()->getWorkspaceURL ().prettyUrl ();
	if (wcaption.isEmpty ()) wcaption = i18n ("[Unnamed Workspace]");
	RKMDIWindow *window = RKWorkplace::mainWorkplace ()->view ()->activePage ();
	if (window) wcaption.append (" - " + window->fullCaption ());
	KParts::MainWindow::setCaption (wcaption);
}

#include "rkward.moc"
