/***************************************************************************
                          rkward.cpp  -  description
                             -------------------
    begin                : Tue Oct 29 20:06:08 CET 2002
    copyright            : (C) 2002, 2005, 2006, 2007 by Thomas Friedrichsmeier 
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
//Added by qt3to4:
#include <QLabel>
#include <Q3VBoxLayout>
#include <QCloseEvent>

// include files for KDE
#include <k3aboutapplication.h>
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
#include <k3dockwidget.h>
#include <kmultitabbar.h>
#include <ksqueezedtextlabel.h>
#include <dcopclient.h>

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
#include "rkglobals.h"
#include "robjectbrowser.h"
#include "dialogs/startupdialog.h"
#include "dialogs/rkloadlibsdialog.h"
#include "dialogs/rkimportdialog.h"
#include "agents/rksaveagent.h"
#include "agents/rkloadagent.h"
#include "agents/rkquitagent.h"
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

#include "agents/showedittextfileagent.h"	// TODO: see below: needed purely for linking!
#include "dialogs/rkreadlinedialog.h"	// TODO: see below: needed purely for linking!
#include "windows/detachedwindowcontainer.h"	// TODO: see below: needed purely for linking!
#include "windows/qxembedcopy.h"	// TODO: see below: needed purely for linking!
#include "dataeditor/rkeditordataframepart.h"	// TODO: see below: needed purely for linking!
#include "agents/rkeditobjectagent.h"	// TODO: see below: needed purely for linking!

// This nevers gets called. It's needed to trick ld into linking correctly. Nothing else.
void bogusCalls () {
	ShowEditTextFileAgent::showEditFiles (0);		// TODO: AAAAAAAARGGGH!!!! It won't link without this bogus line!!!
	RKReadLineDialog::readLine (0, QString(), QString(), 0, 0);	// TODO: see above
	new RKEditorDataFramePart (0);
	DetachedWindowContainer (0);
	new RKWorkplaceView (0);
	new QXEmbedCopy (0);
	new RKEditObjectAgent (QStringList (), 0);
}

//static
RKWardMainWindow *RKWardMainWindow::rkward_mainwin = 0;

RKWardMainWindow::RKWardMainWindow (RKWardStartupOptions *options) : DCOPObject ("rkwardapp"), KMdiMainFrm (0, 0, KMdi::IDEAlMode) {
	RK_TRACE (APP);
	RK_ASSERT (rkward_mainwin == 0);

	rkward_mainwin = this;
	RKGlobals::rinter = 0;
	RKSettings::settings_tracker = new RKSettingsTracker (this);

#if !KDE_IS_VERSION(3,3,0)
	setIDEAlModeStyle (KMultiTabBar::KDEV3);
#else
	// Nice docks a la Kdevelop.
	setToolviewStyle (KMultiTabBar::KDEV3ICON);
#endif

	///////////////////////////////////////////////////////////////////
	// call inits to invoke all other construction parts
	initActions();
	initStatusBar();

	KMdiChildView *dummy = new KMdiChildView (this);
	Q3VBoxLayout *layout = new Q3VBoxLayout (dummy);
	addWindow (dummy);
	new RKWorkplace (dummy);
	RKWorkplace::mainWorkplace ()->initActions (actionCollection (), "prev_window", "next_window", "left_window", "right_window");
	layout->addWidget (RKWorkplace::mainWorkplace ()->view ());
	connect (RKWorkplace::mainWorkplace ()->view (), SIGNAL (captionChanged (const QString &)), this, SLOT (setCaption (const QString &)));

	///////////////////////////////////////////////////////////////////
	// build the interface

	setHelpMenuEnabled (false);
	setXMLFile ("rkwardui.rc");
	insertChildClient (toplevel_actions = new RKTopLevelWindowGUI (this));
	connect (toplevel_actions->actionCollection (), SIGNAL (actionStatusText (const QString &)), this, SLOT (slotSetStatusBarText (const QString &)));
	connect (toplevel_actions->actionCollection (), SIGNAL (clearStatusText ()), this, SLOT (slotSetStatusReady ()));
	createShellGUI (true);

	connect (this, SIGNAL (childWindowCloseRequest (KMdiChildView *)), this, SLOT (slotChildWindowCloseRequest (KMdiChildView *)));

	RKGlobals::mtracker = new RKModificationTracker (this);
	RKComponentMap::initialize ();

	if (options) {
		startup_options = options;
	} else {
		startup_options = new RKWardStartupOptions;
		startup_options->no_stack_check = false;
		startup_options->initial_url = 0;
	}

	QTimer::singleShot (50, this, SLOT (doPostInit ()));

	part_manager = new KParts::PartManager( this );
	// When the manager says the active part changes,
	// the builder updates (recreates) the GUI
	connect (partManager (), SIGNAL (activePartChanged (KParts::Part *)), this, SLOT (createGUI (KParts::Part *)));
	connect (partManager (), SIGNAL (partAdded (KParts::Part *)), this, SLOT (partAdded (KParts::Part *)));
	connect (partManager (), SIGNAL (partRemoved (KParts::Part *)), this, SLOT (partRemoved (KParts::Part *)));

	if (!kapp->dcopClient ()->isRegistered ()) {
		kapp->dcopClient ()->registerAs ("rkward");
		kapp->dcopClient ()->setDefaultObject (objId ());
	}
}

RKWardMainWindow::~RKWardMainWindow() {
	RK_TRACE (APP);
	closeAllViews ();

	// these would not be strictly necessary, as we're exiting the app, anyway.
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
		KMdiMainFrm::closeEvent (e);
		return;
	}

	if (doQueryQuit ()) {
		emit (aboutToQuitRKWard());
		new RKQuitAgent (this);
	}
}

void RKWardMainWindow::doPostInit () {
	RK_TRACE (APP);

	setUpdatesEnabled (false);

	readOptions();
	//It's necessary to give a different name to all tool windows, or they won't be properly displayed
	RObjectBrowser::object_browser = new RObjectBrowser (0, true, "workspace");

	RControlWindow::control_window = new RControlWindow (0, true, "rcontrol");		// the control window needs to be initialized before startR () is called.

	RKCommandLog *log = new RKCommandLog (0, true, "Command log");
	log->setIcon (SmallIcon ("text_block"));
	log->setToolWrapper (addToolWindow (log, KDockWidget::DockBottom, getMainDockWidget (), 10));
	RKWorkplace::mainWorkplace ()->registerToolWindow (log);
	RKCommandLog::rkcommand_log = log;

	startR ();

	QString dummy = i18n ("RKWard has made great progress in the past few months and it is already helpful for many tasks, but some features may be lacking. You can help us by filing bug reports, feature requests, or providing feedback in any other form. Please visit http://rkward.sourceforge.net for more information.");
	KMessageBox::information (this, dummy, i18n("What to expect of RKWard"), "state_of_rkward");
	
	initPlugins ();

	RObjectBrowser::mainBrowser ()->setIcon(SmallIcon("view_tree"));
	RObjectBrowser::mainBrowser ()->setToolWrapper (addToolWindow(RObjectBrowser::mainBrowser (), KDockWidget::DockLeft, getMainDockWidget(), 30, i18n ("Existing objects in your workspace."), i18n ("Workspace")));
	RKWorkplace::mainWorkplace ()->registerToolWindow (RObjectBrowser::mainBrowser ());

	RKFileBrowser::main_browser = new RKFileBrowser (0, true, "file_browser");
	RKFileBrowser::main_browser->setIcon (SmallIcon ("fileopen"));
	RKFileBrowser::main_browser->setToolWrapper (addToolWindow (RKFileBrowser::main_browser, KDockWidget::DockLeft, getMainDockWidget (), 10, i18n ("File-system Browser"), i18n ("Files")));
	RKWorkplace::mainWorkplace ()->registerToolWindow (RKFileBrowser::main_browser);

	RControlWindow::getControl ()->setCaption (i18n ("Pending Jobs"));
	RControlWindow::getControl ()->setToolWrapper (addToolWindow (RControlWindow::getControl (), KDockWidget::DockBottom, getMainDockWidget (), 10));
	RKWorkplace::mainWorkplace ()->registerToolWindow (RControlWindow::getControl ());

	RKConsole *console = new RKConsole (0, true, "r_console");
	console->setIcon (SmallIcon ("konsole"));
	console->setToolWrapper (addToolWindow (console, KDockWidget::DockBottom, getMainDockWidget (), 10));
	RKWorkplace::mainWorkplace ()->registerToolWindow (console);
	RKConsole::setMainConsole (console);

	RKHelpSearchWindow *help_search = new RKHelpSearchWindow (0, true, "r_help");
	help_search->setIcon (SmallIcon ("help"));
	help_search->setToolWrapper (addToolWindow (help_search, KDockWidget::DockBottom, getMainDockWidget (), 10));
	RKWorkplace::mainWorkplace ()->registerToolWindow (help_search);
	RKHelpSearchWindow::main_help_search = help_search;

	setUpdatesEnabled (true);

	RKOutputWindow::initialize ();
	RControlWindow::getControl ()->initialize ();

	if (startup_options->initial_url) {
		openWorkspace (*(startup_options->initial_url));
		delete (startup_options->initial_url);
		startup_options->initial_url = 0;
	} else {
		StartupDialog::StartupDialogResult *result = StartupDialog::getStartupAction (this, fileOpenRecentWorkspace);
		if (result->result == StartupDialog::EmptyWorkspace) {
		} else if (result->result == StartupDialog::OpenFile) {
			openWorkspace (result->open_url);
		} else if (result->result == StartupDialog::ChoseFile) {
			slotFileOpenWorkspace ();
		} else if (result->result == StartupDialog::EmptyTable) {
			RObject *object = RObjectList::getObjectList ()->createNewChild (i18n ("my.data"), 0, true, true);
			// usually an explicit call to activateView should not be necessary. Somehow however, here, it is.
			RKWorkplace::mainWorkplace ()->editObject (object, true);
		}
		delete result;
	}

	if (RKSettingsModuleGeneral::workplaceSaveMode () == RKSettingsModuleGeneral::SaveWorkplaceWithSession) {
		RKWorkplace::mainWorkplace ()->restoreWorkplace (RKSettingsModuleGeneral::getSavedWorkplace (KGlobal::config ()));
	}

	if (RKSettingsModuleGeneral::showHelpOnStartup ()) {
		toplevel_actions->showRKWardHelp ();
	}

	setCaption (QString::null);	// our version of setCaption takes care of creating a correct caption, so we do not need to provide it here

	delete startup_options;
	startup_options = 0;
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
	
	QDir dir (RKSettingsModuleGeneral::filesPath());
	if (!dir.exists ()) {
		QDir current (dir.currentPath ());
		current.mkdir (dir.path (), true);
	}
	dir = dir.filePath (".packagetemp");
	if (!dir.exists ()) {
		QDir current (dir.currentPath ());
		current.mkdir (dir.path (), true);
	}
	
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
	// TODO: is there a way to insert actions between standard actions without having to give all standard actions custom ids?
	new_data_frame = new KAction (i18n ("Dataset"), 0, 0, this, SLOT (slotNewDataFrame ()), actionCollection (), "new_data_frame");
	new_data_frame->setIcon("spreadsheet");
	new_command_editor = KStandardAction::openNew(this, SLOT(slotNewCommandEditor()), actionCollection(), "new_command_editor");
	new_command_editor->setText (i18n ("Script File"));
	new_command_editor->setIcon ("source");

	fileOpen = KStandardAction::open(this, SLOT(slotOpenCommandEditor()), actionCollection(), "file_openy");
	fileOpen->setText (i18n ("Open R Script File"));
	fileOpenRecent = KStandardAction::openRecent(this, SLOT(slotOpenCommandEditor (const KUrl&)), actionCollection(), "file_open_recenty");
	KAction *import_data = new KAction (i18n ("Import Data"), 0, 0, this, SLOT (importData ()), actionCollection (), "import_data");

	fileOpenWorkspace = KStandardAction::open(this, SLOT(slotFileOpenWorkspace()), actionCollection(), "file_openx");
	fileOpenWorkspace->setText (i18n ("Open Workspace"));
	fileOpenWorkspace->setShortcut (KShortcut ("Ctrl+Shift+O"));
	fileOpenRecentWorkspace = KStandardAction::openRecent(this, SLOT(slotFileOpenRecentWorkspace(const KUrl&)), actionCollection(), "file_open_recentx");
	fileSaveWorkspace = KStandardAction::save(this, SLOT(slotFileSaveWorkspace()), actionCollection(), "file_savex");
	fileSaveWorkspace->setText (i18n ("Save Workspace"));
	fileSaveWorkspace->setShortcut (KShortcut ("Ctrl+Shift+S"));
	fileSaveWorkspaceAs = KStandardAction::saveAs(this, SLOT(slotFileSaveWorkspaceAs()), actionCollection(), "file_save_asx");
	fileSaveWorkspaceAs->setText (i18n ("Save Workspace As"));

	fileQuit = KStandardAction::quit(this, SLOT(close ()), actionCollection(), "file_quitx");
	file_load_libs = new KAction (i18n ("Configure Packages"), 0, 0, this, SLOT (slotFileLoadLibs ()), actionCollection (), "file_load_libs");	

	setStandardToolBarMenuEnabled (true);
	createStandardStatusBarAction ();

	close_all_editors = new KAction (i18n ("Close All Data"), 0, 0, this, SLOT (slotCloseAllEditors ()), actionCollection (), "close_all_editors");
	new KAction (i18n ("Close"), 0, KShortcut ("Ctrl+W"), this, SLOT (slotCloseWindow ()), actionCollection (), "window_close");
	window_close_all = new KAction (i18n ("Close All"), 0, 0, this, SLOT (slotCloseAllWindows ()), actionCollection (), "window_close_all");
	window_detach = new KAction (i18n ("Detach"), 0, 0, this, SLOT (slotDetachWindow ()), actionCollection (), "window_detach");

	configure = new KAction (i18n ("Configure RKWard"), 0, 0, this, SLOT (slotConfigure ()), actionCollection (), "configure");

	new_data_frame->setStatusText (i18n ("Creates new empty dataset and opens it for editing"));
	import_data->setStatusText (i18n ("Import data from a variety of file formats"));
	fileOpenWorkspace->setStatusText(i18n("Opens an existing document"));
	fileOpenRecentWorkspace->setStatusText(i18n("Opens a recently used file"));
	fileSaveWorkspace->setStatusText(i18n("Saves the actual document"));
	fileSaveWorkspaceAs->setStatusText(i18n("Saves the actual document as..."));
	close_all_editors->setStatusText (i18n ("Closes all open data editors"));
	fileQuit->setStatusText(i18n("Quits the application"));

	actionCollection ()->setHighlightingEnabled (true);
}

void RKWardMainWindow::partAdded (KParts::Part *part) {
	RK_TRACE (APP);

	if (!part->actionCollection ()) {
		RK_ASSERT (false);
		return;
	}

	part->actionCollection ()->setHighlightingEnabled (true);
	connect (part->actionCollection (), SIGNAL (actionStatusText (const QString &)), this, SLOT (slotSetStatusBarText (const QString &)));
	connect (part->actionCollection (), SIGNAL (clearStatusText ()), this, SLOT (slotSetStatusReady ()));
}

void RKWardMainWindow::partRemoved (KParts::Part *part) {
	RK_TRACE (APP);

	if (!part->actionCollection ()) {
		RK_ASSERT (false);
		return;
	}

	disconnect (part->actionCollection (), SIGNAL (actionStatusText (const QString &)), this, SLOT (slotSetStatusBarText (const QString &)));
	disconnect (part->actionCollection (), SIGNAL (clearStatusText ()), this, SLOT (slotSetStatusReady ()));
}

void RKWardMainWindow::initStatusBar () {
	RK_TRACE (APP);

	// why do we need this QHBox, when the statusbar already does horizontal layout?
	// Well, apparently the stretch factors do not survive a hide/show, so we need some way to work around this
	Q3HBox *statusbar_hbox = new Q3HBox (statusBar ());
	statusbar_action = new KSqueezedTextLabel (statusbar_hbox);
	statusbar_action->hide ();
	statusbar_ready = new QLabel (i18n ("Ready."), statusbar_hbox);
	statusbar_cwd = new KSqueezedTextLabel (statusbar_hbox);
	statusbar_cwd->setAlignment (QLabel::AlignRight);
	statusbar_hbox->setStretchFactor (statusbar_cwd, 1);
	updateCWD ();
	statusBar ()->addWidget (statusbar_hbox, 1);

	statusbar_r_status = new QLabel (i18n ("starting R engine"), statusBar ());
	statusbar_r_status->setPaletteBackgroundColor (QColor (255, 255, 0));
	statusbar_r_status->setFixedHeight (statusBar ()->fontMetrics ().height () + 2);
	statusBar ()->addWidget (statusbar_r_status, 0, true);

	connect (actionCollection (), SIGNAL (actionStatusText (const QString &)), this, SLOT (slotSetStatusBarText (const QString &)));
	connect (actionCollection (), SIGNAL (clearStatusText ()), this, SLOT (slotSetStatusReady ()));
}

void RKWardMainWindow::openWorkspace (const KUrl &url) {
	RK_TRACE (APP);
	if (url.isEmpty ()) return;

	new RKLoadAgent (url, false);
	fileOpenRecentWorkspace->addUrl (url);
}

void RKWardMainWindow::saveOptions () {
	RK_TRACE (APP);
	KConfig *config = KGlobal::config ();

	saveMainWindowSettings (config, "main window options");

	config->setGroup("General Options");
// TODO: WORKAROUND. See corresponding line in readOptions ()
	config->writeEntry("Geometry", size ());
	fileOpenRecentWorkspace->saveEntries(config, "Recent Files");
	fileOpenRecent->saveEntries(config, "Recent Command Files");

	RKSettings::saveSettings (config);

	config->sync ();
}


void RKWardMainWindow::readOptions () {
	RK_TRACE (APP);
	KConfig *config = KGlobal::config ();

#if !KDE_IS_VERSION(3,3,0)
	applyMainWindowSettings (KGlobal::config (), "main window options");
#else
	applyMainWindowSettings (KGlobal::config (), "main window options", true);
#endif

// TODO: WORKAROUND: Actually applyMainWindowSettings could/should do this, but apparently this just does not work for maximized windows. Therefore we use our own version instead.
	config->setGroup("General Options");
	QSize size = config->readSizeEntry ("Geometry");
	if (size.isEmpty ()) {
		size = QApplication::desktop ()->availableGeometry ().size ();
	}
	resize (size);

	// initialize the recent file list
	fileOpenRecentWorkspace->loadEntries (config,"Recent Files");
	fileOpenRecent->loadEntries (config,"Recent Command Files");

	// do this last, since we may be setting some different config-group(s) in the process
	RKSettings::loadSettings (config);

	// explicitly hide the KMdiTaskBar. It could get in the way (and not quite sure, why it shows up on some systems)
	KToolBar *mditask = (KToolBar*) child ("KMdiTaskBar", "KToolBar");
	if (mditask) mditask->hide ();
}

void RKWardMainWindow::saveProperties(KConfig *_cfg)
{
	RK_TRACE (APP);
/*  if(doc->URL().fileName()!=i18n("Untitled") && !doc->isModified())
  {
    // saving to tempfile not necessary

  }
  else */
  {
    //KUrl url=doc->URL();	
    //_cfg->writeEntry("filename", url.url());
    //_cfg->writeEntry("modified", doc->isModified());
    //QString tempname = kapp->tempSaveName(url.url());
    //QString tempurl= KUrl::encode_string(tempname);
    //KUrl _url(tempurl);
    //doc->saveDocument(_url);
  }
}


void RKWardMainWindow::readProperties(KConfig* _cfg)
{
	RK_TRACE (APP);
/*  QString filename = _cfg->readEntry("filename", "");
  KUrl url(filename);
  bool modified = _cfg->readBoolEntry("modified", false);
  if(modified)
  {
    bool canRecover;
    QString tempname = kapp->checkRecoverFile(filename, canRecover);
    KUrl _url(tempname);
  	
    if(canRecover)
    {
//      doc->openDocument(_url);
      doc->setModified();
      QFile::remove(tempname);
    }
  }
  else
  {
    if(!filename.isEmpty())
    {
//      doc->openDocument(url);
    }
  } */
}

bool RKWardMainWindow::doQueryQuit () {
	RK_TRACE (APP);

	slotSetStatusBarText (i18n ("Exiting..."));
	saveOptions ();
	if (RKSettingsModuleGeneral::workplaceSaveMode () == RKSettingsModuleGeneral::SaveWorkplaceWithSession) {
		RKSettingsModuleGeneral::setSavedWorkplace (RKWorkplace::mainWorkplace ()->makeWorkplaceDescription ("\n", false), KGlobal::config ());
	}

//	if (!RObjectList::getGlobalEnv ()->isEmpty ()) {
	int res;
	res = KMessageBox::questionYesNoCancel (this, i18n ("Quitting RKWard: Do you want to save the workspace?\nRKWard will remain open if you press Cancel"), i18n ("Save Workspace?"));
	if (res == KMessageBox::Yes) {
		new RKSaveAgent (RObjectList::getObjectList ()->getWorkspaceURL (), false, RKSaveAgent::DoNothing);
	} else if (res != KMessageBox::No) {
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

	if (ok) {
		QString valid = RObjectList::getObjectList ()->validizeName (name);
		if (valid != name) KMessageBox::sorry (this, i18n ("The name you specified was already in use or not valid. Renamed to %1").arg (valid), i18n ("Invalid Name"));
		RObject *object = RObjectList::getObjectList ()->createNewChild (valid, 0, true, true);
		RKWorkplace::mainWorkplace ()->editObject (object, true);
	}
	
}

void RKWardMainWindow::fileOpenNoSave (const KUrl &url) {
	RK_TRACE (APP);

	slotCloseAllEditors ();

	slotSetStatusBarText(i18n("Opening workspace..."));
	KUrl lurl = url;
	if (lurl.isEmpty ()) {
		lurl = KFileDialog::getOpenUrl (":<rfiles>", i18n("*|All files"), this, i18n("Open File..."));
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
	fileOpenAskSave (QString::null);
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

	QString ntext = text.trimmed ();
	ntext.replace ("<qt>", "");	// WORKAROUND: what the ?!? is going on? The KTHMLPart seems to post such messages.
	if (ntext.isEmpty ()) {
		statusbar_action->hide ();
		statusbar_ready->show ();
		statusbar_cwd->show ();
	} else {
		statusbar_action->show ();
		statusbar_ready->hide ();
		statusbar_cwd->hide ();
		statusbar_action->setText (ntext);
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

void RKWardMainWindow::setRStatus (bool busy) {
	RK_TRACE (APP);
	if (busy) {
		statusbar_r_status->setText (i18n ("R engine busy"));
		statusbar_r_status->setPaletteBackgroundColor (QColor (255, 0, 0));
	} else {
		statusbar_r_status->setText (i18n ("R engine idle"));
		statusbar_r_status->setPaletteBackgroundColor (QColor (0, 255, 0));
	}
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
	
	urls = KFileDialog::getOpenUrls (":<rfiles>", "*.R *.r *.S *.s *.q|R Script Files (*.R *.r *.S *.s *.q)\n*.*|All Files (*.*)", this, i18n ("Open command file(s)"));

	for (it = urls.begin() ; it != urls.end() ; ++it) {
		slotOpenCommandEditor (*it);
	}
};

void RKWardMainWindow::slotChildWindowCloseRequest (KMdiChildView * window) {
	RK_TRACE (APP);

	closeWindow (window);
}

void RKWardMainWindow::openHTML (const KUrl &url) {
	RK_TRACE (APP);

	RKWorkplace::mainWorkplace ()->openHelpWindow (url);
}

void RKWardMainWindow::openHTMLHelp (const QString & url) {
	RK_TRACE (APP);
	openHTML (url);
}

void RKWardMainWindow::setCaption (const QString &) {
	RK_TRACE (APP);

	QString wcaption = RObjectList::getObjectList ()->getWorkspaceURL ().fileName ();
	if (wcaption.isEmpty ()) wcaption = RObjectList::getObjectList ()->getWorkspaceURL ().prettyUrl ();
	if (wcaption.isEmpty ()) wcaption = i18n ("[Unnamed Workspace]");
	RKMDIWindow *window = RKWorkplace::mainWorkplace ()->activeWindow (RKMDIWindow::Attached);
	if (window) wcaption.append (" - " + window->fullCaption ());
	KMdiMainFrm::setCaption (wcaption);
}

#include "rkward.moc"
