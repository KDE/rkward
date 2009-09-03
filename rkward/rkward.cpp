/***************************************************************************
                          rkward.cpp  -  description
                             -------------------
    begin                : Tue Oct 29 20:06:08 CET 2002
    copyright            : (C) 2002, 2005, 2006, 2007, 2008, 2009 by Thomas Friedrichsmeier 
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
#include <kfiledialog.h>
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

// application specific includes
#include "rkward.h"
#include "core/rkmodificationtracker.h"
#include "plugin/rkcomponentmap.h"
#include "settings/rksettings.h"
#include "settings/rksettingsmoduleplugins.h"
#include "settings/rksettingsmodulegeneral.h"
#include "settings/rksettingsmoduleoutput.h"
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
#include "rkconsole.h"
#include "debug.h"
#include "version.h"


#include "agents/showedittextfileagent.h"	// TODO: see below: needed purely for linking!
#include "dialogs/rkreadlinedialog.h"	// TODO: see below: needed purely for linking!
#include "windows/detachedwindowcontainer.h"	// TODO: see below: needed purely for linking!
#include "dataeditor/rkeditordataframe.h"	// TODO: see below: needed purely for linking!
#include "agents/rkeditobjectagent.h"	// TODO: see below: needed purely for linking!

// This nevers gets called. It's needed to trick ld into linking correctly. Nothing else.
void bogusCalls () {
	ShowEditTextFileAgent::showEditFiles (0);		// TODO: AAAAAAAARGGGH!!!! It won't link without this bogus line!!!
	RKReadLineDialog::readLine (0, QString(), QString(), 0, 0);	// TODO: see above
	new RKEditorDataFrame (0, 0);
	DetachedWindowContainer (0);
	new RKWorkplaceView (0);
	new RKEditObjectAgent (QStringList (), 0);
}

/** Main window **/

//static
RKWardMainWindow *RKWardMainWindow::rkward_mainwin = 0;

RKWardMainWindow::RKWardMainWindow (RKWardStartupOptions *options) : KParts::MainWindow ((QWidget *)0, (Qt::WindowFlags) KDE_DEFAULT_WINDOWFLAGS) {
	RK_TRACE (APP);
	RK_ASSERT (rkward_mainwin == 0);

	rkward_mainwin = this;
	RKGlobals::rinter = 0;
	RKSettings::settings_tracker = new RKSettingsTracker (this);

	///////////////////////////////////////////////////////////////////
	// call inits to invoke all other construction parts
	RKStandardIcons::initIcons ();
	initActions();
	initStatusBar();

	new RKWorkplace (this);
	RKWorkplace::mainWorkplace ()->initActions (actionCollection (), "prev_window", "next_window", "left_window", "right_window");
	setCentralWidget (RKWorkplace::mainWorkplace ());
	connect (RKWorkplace::mainWorkplace ()->view (), SIGNAL (captionChanged (const QString &)), this, SLOT (setCaption (const QString &)));

	///////////////////////////////////////////////////////////////////
	// build the interface

	setHelpMenuEnabled (false);
	setXMLFile ("rkwardui.rc");
	insertChildClient (toplevel_actions = new RKTopLevelWindowGUI (this));
	createShellGUI (true);
	RKXMLGUISyncer::self ()->watchXMLGUIClientUIrc (this);

	RKGlobals::mtracker = new RKModificationTracker (this);
	RKComponentMap::initialize ();

	if (options) {
		startup_options = options;
	} else {
		startup_options = new RKWardStartupOptions;
		startup_options->no_stack_check = false;
		startup_options->initial_url = KUrl();
	}

	QTimer::singleShot (50, this, SLOT (doPostInit ()));

	part_manager = new KParts::PartManager (this);
	// When the manager says the active part changes,
	// the builder updates (recreates) the GUI
	connect (partManager (), SIGNAL (activePartChanged (KParts::Part *)), this, SLOT (partChanged (KParts::Part *)));
}

RKWardMainWindow::~RKWardMainWindow() {
	RK_TRACE (APP);

	// these would not be strictly necessary, as we're exiting the app, anyway.
	delete RControlWindow::getControl ();
	delete RKGlobals::rInterface ();
	delete RObjectList::getObjectList ();
	delete RObjectBrowser::mainBrowser ();
	delete RKCommandLog::getLog ();
	delete RKConsole::mainConsole ();
	delete RKHelpSearchWindow::mainHelpSearch ();
	delete RKGlobals::tracker ();
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
	if (!(resource_ver.open (QIODevice::ReadOnly) && (resource_ver.read (100).trimmed () == VERSION))) {
		KMessageBox::error (this, i18n ("<p>RKWard either could not find its resource files at all, or only an old version of those files. The most likely cause is that the last installation failed to place the files in the correct place. This can lead to all sorts of problems, from single missing features to complete failure to function.</p><p><b>You should quit RKWard, now, and fix your installation</b>. For help with that, see <a href=\"http://p.sf.net/rkward/compiling\">http://p.sf.net/rkward/compiling</a>.</p>"), i18n ("Broken installation"), KMessageBox::Notify | KMessageBox::AllowLink);
	}

#ifdef Q_WS_WIN
	KMessageBox::information (this, i18n ("<p>Please note that RKWard on is not well tested on Windows, yet.</p><p>There are all sorts of known issues, and issues yet to be reported. If you'd like to help: <a href=\"http://p.sf.net/rkward/contact\">http://p.sf.net/rkward/contact</a>.</p>"), i18n ("RKWard on Windows"), "rkward_on_windows", KMessageBox::Notify | KMessageBox::AllowLink);
#endif

	setUpdatesEnabled (false);

	readOptions();
	//It's necessary to give a different name to all tool windows, or they won't be properly displayed
	RObjectBrowser::object_browser = new RObjectBrowser (0, true, "workspace");

	RKCommandLog *log = new RKCommandLog (0, true, "Command log");
	RKWorkplace::mainWorkplace ()->placeInToolWindowBar (log, KMultiTabBar::Bottom);
	RKCommandLog::rkcommand_log = log;

	// startup options will be deleted from the R thread (TODO correct this!), so we need to copy the initial_url here, or run into race conditions
	KUrl open_url = startup_options->initial_url;
	QString evaluate_code = startup_options->evaluate;
	startR ();
	
	initPlugins ();

	RObjectBrowser::mainBrowser ()->setCaption (i18n ("Workspace"));
	RKWorkplace::mainWorkplace ()->placeInToolWindowBar (RObjectBrowser::mainBrowser (), KMultiTabBar::Left);

	RKFileBrowser::main_browser = new RKFileBrowser (0, true, "file_browser");
	RKFileBrowser::main_browser->setCaption (i18n ("Files"));
	RKWorkplace::mainWorkplace ()->placeInToolWindowBar (RKFileBrowser::main_browser, KMultiTabBar::Left);

	RControlWindow::control_window = new RControlWindow (0, true, "rcontrol");
	RControlWindow::getControl ()->setCaption (i18n ("Pending Jobs"));
	RKWorkplace::mainWorkplace ()->placeInToolWindowBar (RControlWindow::getControl (), KMultiTabBar::Bottom);

	RKConsole *console = new RKConsole (0, true, "r_console");
	RKConsole::setMainConsole (console);
	RKWorkplace::mainWorkplace ()->placeInToolWindowBar (console, KMultiTabBar::Bottom);

	RKHelpSearchWindow *help_search = new RKHelpSearchWindow (0, true, "r_help");
	RKHelpSearchWindow::main_help_search = help_search;
	RKWorkplace::mainWorkplace ()->placeInToolWindowBar (help_search, KMultiTabBar::Bottom);

	setUpdatesEnabled (true);
	show ();

	if (!open_url.isEmpty()) {
		openWorkspace (open_url);
	} else {
		StartupDialog::StartupDialogResult *result = StartupDialog::getStartupAction (this, fileOpenRecentWorkspace);
		if (result->result == StartupDialog::EmptyWorkspace) {
		} else if (result->result == StartupDialog::OpenFile) {
			openWorkspace (result->open_url);
		} else if (result->result == StartupDialog::ChoseFile) {
			slotFileOpenWorkspace ();
		} else if (result->result == StartupDialog::EmptyTable) {
			RKWorkplace::mainWorkplace ()->editNewDataFrame (i18n ("my.data"));
		}
		delete result;
	}

	if (RKSettingsModuleGeneral::workplaceSaveMode () == RKSettingsModuleGeneral::SaveWorkplaceWithSession) {
		RKWorkplace::mainWorkplace ()->restoreWorkplace (RKSettingsModuleGeneral::getSavedWorkplace (KGlobal::config ().data ()));
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

	RKGlobals::rInterface ()->startThread ();

	RObjectBrowser::mainBrowser ()->unlock ();
}

void RKWardMainWindow::slotConfigure () {
	RKSettings::configureSettings (RKSettings::NoPage, this);
}

void RKWardMainWindow::initActions()
{  
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
	fileOpen->setText (i18n ("Open R Script File"));

	fileOpenRecent = static_cast<KRecentFilesAction*> (actionCollection ()->addAction (KStandardAction::OpenRecent, "file_open_recenty", this, SLOT(slotOpenCommandEditor (const KUrl&))));

	action = actionCollection ()->addAction ("import_data", this, SLOT (importData()));
	action->setText (i18n ("Import Data"));
	action->setStatusTip (i18n ("Import data from a variety of file formats"));

	fileOpenWorkspace = actionCollection ()->addAction (KStandardAction::Open, "file_openx", this, SLOT(slotFileOpenWorkspace()));
	fileOpenWorkspace->setText (i18n ("Open Workspace"));
	fileOpenWorkspace->setShortcut (Qt::ControlModifier + Qt::ShiftModifier + Qt::Key_O);
	fileOpenWorkspace->setStatusTip (i18n ("Opens an existing document"));

	fileOpenRecentWorkspace = static_cast<KRecentFilesAction*> (actionCollection ()->addAction (KStandardAction::OpenRecent, "file_open_recentx", this, SLOT(slotFileOpenRecentWorkspace(const KUrl&))));
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

	// These two currently do the same thing
	action = actionCollection ()->addAction ("load_unload_libs", this, SLOT (slotFileLoadLibs()));
	action->setText (i18n ("Load / Unload Packages"));
	action->setIcon (RKStandardIcons::getIcon (RKStandardIcons::ActionConfigurePackages));

	action = actionCollection ()->addAction ("configure_packages", this, SLOT (slotFileLoadLibs()));
	action->setText (i18n ("Configure Packages"));
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

	configure = actionCollection ()->addAction ("configure", this, SLOT (slotConfigure()));
	configure->setText (i18n ("Configure RKWard"));

	edit_menu_dummy = actionCollection ()->addAction ("edit_menu_dummy", this);
	edit_menu_dummy->setText (i18n ("[No actions available for current view]"));
	edit_menu_dummy->setEnabled (false);
	view_menu_dummy = actionCollection ()->addAction ("view_menu_dummy", this);
	view_menu_dummy->setText (edit_menu_dummy->text ());
	view_menu_dummy->setEnabled (false);
	run_menu_dummy = actionCollection ()->addAction ("run_menu_dummy", this);
	run_menu_dummy->setText (edit_menu_dummy->text ());
	run_menu_dummy->setEnabled (false);
}

void RKWardMainWindow::partChanged (KParts::Part *part) {
	RK_TRACE (APP);

	createGUI (part);

	if (!guiFactory ()) {
		RK_ASSERT (false);
		return;
	}

	QMenu* menu = dynamic_cast<QMenu*>(guiFactory ()->container ("edit", this));
	edit_menu_dummy->setVisible (menu && (menu->actions ().size () <= 1));
	menu = dynamic_cast<QMenu*>(guiFactory ()->container ("view", this));
	view_menu_dummy->setVisible (menu && (menu->actions ().size () <= 1));
	menu = dynamic_cast<QMenu*>(guiFactory ()->container ("run", this));
	run_menu_dummy->setVisible (menu && (menu->actions ().size () <= 1));
}

void RKWardMainWindow::initStatusBar () {
	RK_TRACE (APP);

	statusbar_ready = new QLabel (i18n ("Ready."), statusBar ());
	statusBar ()->addWidget (statusbar_ready);
	statusbar_cwd = new KSqueezedTextLabel (statusBar ());
	statusbar_cwd->setAlignment (Qt::AlignRight);
	statusBar ()->addWidget (statusbar_cwd, 10);
	updateCWD ();

	statusbar_r_status = new QLabel (statusBar ());
	statusbar_r_status->setFixedHeight (statusBar ()->fontMetrics ().height () + 2);
	statusBar ()->addPermanentWidget (statusbar_r_status, 0);
	setRStatus (Starting);
}

void RKWardMainWindow::openWorkspace (const KUrl &url) {
	RK_TRACE (APP);
	if (url.isEmpty ()) return;

	new RKLoadAgent (url, false);
	fileOpenRecentWorkspace->addUrl (url);
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

	// initialize the recent file list
	fileOpenRecentWorkspace->loadEntries (config->group ("Recent Files"));
	fileOpenRecent->loadEntries (config->group ("Recent Command Files"));

	// do this last, since we may be setting some different config-group(s) in the process
	RKSettings::loadSettings (config);
}

bool RKWardMainWindow::doQueryQuit () {
	RK_TRACE (APP);

	slotSetStatusBarText (i18n ("Exiting..."));
	saveOptions ();
	if (RKSettingsModuleGeneral::workplaceSaveMode () == RKSettingsModuleGeneral::SaveWorkplaceWithSession) {
		RKSettingsModuleGeneral::setSavedWorkplace (RKWorkplace::mainWorkplace ()->makeWorkplaceDescription ("\n", false), KGlobal::config ().data ());
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
	for (RKWorkplace::RKWorkplaceObjectList::const_iterator it = map.constBegin (); it != map.constEnd (); ++it){
		if (!(*it)->close (true)) {
			// If a child refuses to close, we return false.
			slotSetStatusReady ();
			return false;
		}
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
		lurl = KFileDialog::getOpenUrl (KUrl (), i18n("*|All files"), this, i18n("Open File..."));
#else
		lurl = KFileDialog::getOpenUrl (KUrl ("kfiledialog:///<rfiles>"), i18n("*|All files"), this, i18n("Open File..."));
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
		statusbar_r_status->setText (i18n ("R engine busy"));
		status_color = QColor (255, 0, 0);
	} else if (status == Idle) {
		statusbar_r_status->setText (i18n ("R engine idle"));
		status_color = QColor (0, 255, 0);
	} else {
		statusbar_r_status->setText (i18n ("R engine starting"));
		status_color = QColor (255, 255, 0);
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

void RKWardMainWindow::slotOpenCommandEditor (const KUrl &url) {
	RK_TRACE (APP);

	if (RKWorkplace::mainWorkplace ()->openScriptEditor (url)) {
		if (!url.isEmpty ()) fileOpenRecent->addUrl (url);
	}
};

void RKWardMainWindow::slotOpenCommandEditor () {
	RK_TRACE (APP);
	KUrl::List urls;
	KUrl::List::const_iterator it;

#ifdef Q_WS_WIN
	// getOpenUrls(KUrl("kfiledialog:///<rfiles>"), ...) causes a hang on windows (KDElibs 4.2.3).
#	warning Track this bug down and/or report it
	urls = KFileDialog::getOpenUrls (KUrl (), "*.R *.r *.S *.s *.q|R Script Files (*.R *.r *.S *.s *.q)\n*.*|All Files (*.*)", this, i18n ("Open command file(s)"));
#else
	urls = KFileDialog::getOpenUrls (KUrl ("kfiledialog:///<rfiles>"), "*.R *.r *.S *.s *.q|R Script Files (*.R *.r *.S *.s *.q)\n*.*|All Files (*.*)", this, i18n ("Open command file(s)"));
#endif
	for (it = urls.begin() ; it != urls.end() ; ++it) {
		slotOpenCommandEditor (*it);
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
