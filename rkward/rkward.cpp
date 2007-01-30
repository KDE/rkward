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

// include files for KDE
#include <kaboutapplication.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <kfiledialog.h>
#include <kmenubar.h>
#include <kstatusbar.h>
#include <klocale.h>
#include <kconfig.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kstdaction.h>
#include <kinputdialog.h>
#include <kdockwidget.h>
#include <kmultitabbar.h>
#include <dcopclient.h>

// include files for the kate part. Some may not be useful
#include <ktexteditor/configinterface.h>
#include <ktexteditor/sessionconfiginterface.h>
#include <ktexteditor/viewcursorinterface.h>
#include <ktexteditor/printinterface.h>
#include <ktexteditor/encodinginterface.h>
#include <ktexteditor/editorchooser.h>
#include <ktexteditor/popupmenuinterface.h>

// application specific includes
#include "rkward.h"
#include "core/rkmodificationtracker.h"
#include "plugin/rkcomponentmap.h"
#include "plugin/rkcomponentcontext.h"
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
#include "agents/rksaveagent.h"
#include "agents/rkloadagent.h"
#include "agents/rkquitagent.h"
#include "windows/rcontrolwindow.h"
#include "windows/rkhtmlwindow.h"
#include "windows/rkworkplaceview.h"
#include "windows/rkworkplace.h"
#include "windows/rkcommandlog.h"
#include "khelpdlg.h"
#include "rkconsole.h"
#include "debug.h"

#include "agents/showedittextfileagent.h"	// TODO: see below: needed purely for linking!
#include "dialogs/rkreadlinedialog.h"	// TODO: see below: needed purely for linking!
#include "windows/detachedwindowcontainer.h"	// TODO: see below: needed purely for linking!
#include "windows/qxembedcopy.h"	// TODO: see below: needed purely for linking!
#include "dataeditor/rkeditordataframepart.h"	// TODO: see below: needed purely for linking!

// This nevers gets called. It's needed to trick ld into linking correctly. Nothing else.
void bogusCalls () {
	ShowEditTextFileAgent::showEditFiles (0);		// TODO: AAAAAAAARGGGH!!!! It won't link without this bogus line!!!
	RKReadLineDialog::readLine (0, QString(), QString(), 0, 0);	// TODO: see above
	new RKEditorDataFramePart (0);
	DetachedWindowContainer (0);
	new RKWorkplaceView (0);
	new QXEmbedCopy (0);
}

//static
RKWardMainWindow *RKWardMainWindow::rkward_mainwin = 0;

RKWardMainWindow::RKWardMainWindow (KURL *load_url) : DCOPObject ("rkwardapp"), KMdiMainFrm (0, 0, KMdi::IDEAlMode) {
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

	///////////////////////////////////////////////////////////////////
	// build the interface

	setHelpMenuEnabled (false);
	setXMLFile ("rkwardui.rc");
	createShellGUI (true);

	connect (this, SIGNAL (childWindowCloseRequest (KMdiChildView *)), this, SLOT (slotChildWindowCloseRequest (KMdiChildView *)));

	RKGlobals::mtracker = new RKModificationTracker (this);
	RKComponentMap::initialize ();

	initial_url = load_url;

	QTimer::singleShot (50, this, SLOT (doPostInit ()));

	part_manager = new KParts::PartManager( this );
	// When the manager says the active part changes,
	// the builder updates (recreates) the GUI
	connect (partManager (), SIGNAL (activePartChanged (KParts::Part *)), this, SLOT (createGUI (KParts::Part *)));
	connect (partManager (), SIGNAL (partAdded (KParts::Part *)), this, SLOT (partAdded (KParts::Part *)));
	connect (partManager (), SIGNAL (partRemoved (KParts::Part *)), this, SLOT (partRemoved (KParts::Part *)));

	KMdiChildView *dummy = new KMdiChildView (this);
	QVBoxLayout *layout = new QVBoxLayout (dummy);
	addWindow (dummy);
	new RKWorkplace (dummy);
	layout->addWidget (RKWorkplace::mainWorkplace ()->view ());
	connect (RKWorkplace::mainWorkplace ()->view (), SIGNAL (captionChanged (const QString &)), this, SLOT (setCaption (const QString &)));

	if (!kapp->dcopClient ()->isRegistered ()) {
		kapp->dcopClient ()->registerAs ("rkward");
		kapp->dcopClient ()->setDefaultObject (objId ());
	}
}

RKWardMainWindow::~RKWardMainWindow() {
	RK_TRACE (APP);
	closeAllViews ();
	delete RKGlobals::rInterface ();
	delete RObjectList::getObjectList ();
	delete object_browser;
	delete RKGlobals::tracker ();
}

void RKWardMainWindow::closeEvent (QCloseEvent *e) {
	RK_TRACE (APP);

	if (RKQuitAgent::quittingInProgress ()) {
		KMdiMainFrm::closeEvent (e);
		return;
	}

	if (doQueryQuit ()) {
		new RKQuitAgent (this);
	}
}

void RKWardMainWindow::doPostInit () {
	RK_TRACE (APP);

	readOptions();
	object_browser = new RObjectBrowser ();

	RControlWindowPart *rcpart = new RControlWindowPart ();		// the control window needs to be initialized before startR () is called.
	RKGlobals::rcontrol = static_cast<RControlWindow *> (rcpart->widget ());

	startR ();

	QString dummy = i18n ("Please note that RKWard is still far from being finished. We feel it is already helpful for a number of tasks, but many features are lacking or buggy. You can help us by filing bug reports, feature requests, or providing feedback in any other form. Please visit http://rkward.sourceforge.net for contact information.");
	KMessageBox::information (this, dummy, i18n("What to expect of RKWard"), "state_of_rkward");
	
	initPlugins ();

	//It's necessary to give a different name to all tool windows, or they won't be properly displayed
	object_browser->setName("workspace"); 
	object_browser->setIcon(SmallIcon("view_tree"));
	addToolWindow(object_browser,KDockWidget::DockLeft, getMainDockWidget(), 30 , i18n ("Existing objects in your workspace.") , i18n ("Workspace"));
	
	RKCommandLog::getLog ()->setName ("Command log");		// TODO: move to RKCommandLog
	RKCommandLog::getLog ()->setIcon (SmallIcon ("text_block"));	// TODO: move to RKCommandLog
	watch_view = addToolWindow (RKCommandLog::getLog (), KDockWidget::DockBottom, getMainDockWidget (), 10);
	connect (RKCommandLog::getLog (), SIGNAL (raiseWindow ()), this, SLOT (raiseWatch ()));
	partManager ()->addPart (RKCommandLog::getLog ()->getPart (), false);

	RKGlobals::rcontrol->setCaption (i18n ("Pending Jobs"));
	RKGlobals::rcontrol->setName ("rcontrol");
	addToolWindow (RKGlobals::rcontrol, KDockWidget::DockBottom, getMainDockWidget (), 10);
	RKGlobals::rcontrol->hide ();		// this line is important! RControlWindow must do some initializations on first show, and be hidden until then.

	RKConsole *console = new RKConsole ();
	console->setIcon (SmallIcon ("konsole"));
	console->setName ("r_console");
	console_view = addToolWindow (console, KDockWidget::DockBottom, getMainDockWidget (), 10);
	connect (console, SIGNAL (raiseWindow ()), this, SLOT (raiseConsole ()));
	partManager ()->addPart (console->getPart (), false);
	RKConsole::setMainConsole (console);
	
	RKGlobals::helpdlg = new KHelpDlg (0);
	RKGlobals::helpDialog ()->setIcon (SmallIcon ("help"));
	search_help_view = addToolWindow (RKGlobals::helpDialog (), KDockWidget::DockBottom, getMainDockWidget (), 10);

	RKOutputWindow::initialize ();

	if (initial_url) {
		openWorkspace (*initial_url);
		delete initial_url;
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
		RKWorkplace::mainWorkplace ()->restoreWorkplace (RKSettingsModuleGeneral::getSavedWorkplace (kapp->config ()));
	}

	if (RKSettingsModuleGeneral::showHelpOnStartup ()) {
		showRKWardHelp ();
	}

	setCaption (QString::null);	// our version of setCaption takes care of creating a correct caption, so we do not need to provide it here
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
		QDir current (dir.currentDirPath ());
		current.mkdir (dir.path (), true);
	}
	dir = dir.filePath (".packagetemp");
	if (!dir.exists ()) {
		QDir current (dir.currentDirPath ());
		current.mkdir (dir.path (), true);
	}
	
	RKGlobals::rinter = new RInterface ();
	new RObjectList ();

	RKGlobals::rInterface ()->startThread ();

	object_browser->initialize ();
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
	new_command_editor = KStdAction::openNew(this, SLOT(slotNewCommandEditor()), actionCollection(), "new_command_editor");
	new_command_editor->setText (i18n ("Script File"));
	new_command_editor->setIcon ("source");

	fileOpen = KStdAction::open(this, SLOT(slotOpenCommandEditor()), actionCollection(), "file_openy");
	fileOpen->setText (i18n ("Open R Script File"));
	fileOpenRecent = KStdAction::openRecent(this, SLOT(slotOpenCommandEditor (const KURL&)), actionCollection(), "file_open_recenty");
	KAction *import_data = new KAction (i18n ("Import Data"), 0, 0, this, SLOT (importData ()), actionCollection (), "import_data");

	fileOpenWorkspace = KStdAction::open(this, SLOT(slotFileOpenWorkspace()), actionCollection(), "file_openx");
	fileOpenWorkspace->setText (i18n ("Open Workspace"));
	fileOpenWorkspace->setShortcut (KShortcut ("Ctrl+Shift+O"));
	fileOpenRecentWorkspace = KStdAction::openRecent(this, SLOT(slotFileOpenRecentWorkspace(const KURL&)), actionCollection(), "file_open_recentx");
	fileSaveWorkspace = KStdAction::save(this, SLOT(slotFileSaveWorkspace()), actionCollection(), "file_savex");
	fileSaveWorkspace->setText (i18n ("Save Workspace"));
	fileSaveWorkspace->setShortcut (KShortcut ("Ctrl+Shift+S"));
	fileSaveWorkspaceAs = KStdAction::saveAs(this, SLOT(slotFileSaveWorkspaceAs()), actionCollection(), "file_save_asx");
	fileSaveWorkspaceAs->setText (i18n ("Save Workspace As"));

	fileQuit = KStdAction::quit(this, SLOT(close ()), actionCollection(), "file_quitx");
	file_load_libs = new KAction (i18n ("Configure Packages"), 0, 0, this, SLOT (slotFileLoadLibs ()), actionCollection (), "file_load_libs");	

	setStandardToolBarMenuEnabled (true);
	createStandardStatusBarAction ();

	close_all_editors = new KAction (i18n ("Close All Data"), 0, 0, this, SLOT (slotCloseAllEditors ()), actionCollection (), "close_all_editors");
	window_close = new KAction (i18n ("Close"), 0, KShortcut ("Ctrl+W"), this, SLOT (slotCloseWindow ()), actionCollection (), "window_close");
	window_close_all = new KAction (i18n ("Close All"), 0, 0, this, SLOT (slotCloseAllWindows ()), actionCollection (), "window_close_all");
	window_detach = new KAction (i18n ("Detach"), 0, 0, this, SLOT (slotDetachWindow ()), actionCollection (), "window_detach");
	outputShow= new KAction (i18n ("Show &Output"), 0, 0, this, SLOT (slotOutputShow ()), actionCollection (), "output_show");
	configure = new KAction (i18n ("Configure RKWard"), 0, 0, this, SLOT (slotConfigure ()), actionCollection (), "configure");

	makeRKWardHelpMenu (this, actionCollection ());

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

void RKWardMainWindow::makeRKWardHelpMenu (QWidget *for_window, KActionCollection *ac) {
	KAction *help_invoke_r_help = new KAction (i18n ("Help on R"), 0, 0, this, SLOT (invokeRHelp ()), ac, "invoke_r_help");
	KAction *show_help_search = new KAction (i18n ("Search R Help"), 0, 0, this, SLOT (showHelpSearch ()), ac, "show_help_search");
	KAction *show_rkward_help = new KAction (i18n ("Help on RKWard"), 0, 0, this, SLOT (showRKWardHelp ()), ac, "rkward_help");

	KStdAction::helpContents (this, SLOT (appHelpActivated ()), ac);
	KStdAction::aboutApp (this, SLOT (showAboutApplication ()), ac);
	KStdAction::whatsThis (for_window, SLOT (whatsThis ()), ac);
	KStdAction::reportBug (this, SLOT (reportRKWardBug ()), ac);

	help_invoke_r_help->setStatusText (i18n ("Shows the R help index"));
	show_help_search->setStatusText (i18n ("Shows/raises the R Help Search window"));
	show_rkward_help->setStatusText (i18n ("Show help on RKWard"));
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

	r_status_label = new QLabel (i18n ("starting R engine"), statusBar ());
	r_status_label->setPaletteBackgroundColor (QColor (255, 255, 0));
	statusBar ()->addWidget (r_status_label, 0, true);

	connect (actionCollection (), SIGNAL (actionStatusText (const QString &)), this, SLOT (slotSetStatusBarText (const QString &)));
	connect (actionCollection (), SIGNAL (clearStatusText ()), this, SLOT (slotSetStatusReady ()));
}

void RKWardMainWindow::openWorkspace (const KURL &url) {
	RK_TRACE (APP);
	if (url.isEmpty ()) return;

	new RKLoadAgent (url, false);
	fileOpenRecentWorkspace->addURL (url);
}

void RKWardMainWindow::saveOptions () {
	RK_TRACE (APP);
	KConfig *config = kapp->config ();

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
	KConfig *config = kapp->config ();

#if !KDE_IS_VERSION(3,3,0)
	applyMainWindowSettings (kapp->config (), "main window options");
#else
	applyMainWindowSettings (kapp->config (), "main window options", true);
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
    //KURL url=doc->URL();	
    //_cfg->writeEntry("filename", url.url());
    //_cfg->writeEntry("modified", doc->isModified());
    //QString tempname = kapp->tempSaveName(url.url());
    //QString tempurl= KURL::encode_string(tempname);
    //KURL _url(tempurl);
    //doc->saveDocument(_url);
  }
}


void RKWardMainWindow::readProperties(KConfig* _cfg)
{
	RK_TRACE (APP);
/*  QString filename = _cfg->readEntry("filename", "");
  KURL url(filename);
  bool modified = _cfg->readBoolEntry("modified", false);
  if(modified)
  {
    bool canRecover;
    QString tempname = kapp->checkRecoverFile(filename, canRecover);
    KURL _url(tempname);
  	
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
		RKSettingsModuleGeneral::setSavedWorkplace (RKWorkplace::mainWorkplace ()->makeWorkplaceDescription (), kapp->config ());
	}

	if (!RObjectList::getGlobalEnv ()->isEmpty ()) {
		int res;
		res = KMessageBox::questionYesNoCancel (this, i18n ("Quitting RKWard: Do you want to save the workspace?\nRKWard will remain open if you press Cancel"), i18n ("Save Workspace?"));
		if (res == KMessageBox::Yes) {
			new RKSaveAgent (RObjectList::getObjectList ()->getWorkspaceURL (), false, RKSaveAgent::DoNothing);
		} else if (res != KMessageBox::No) {
			slotSetStatusReady ();
			return false;
		}
	}

	RKWorkplace::RKWorkplaceObjectList map = RKWorkplace::mainWorkplace ()->getObjectList ();
	for (RKWorkplace::RKWorkplaceObjectList::const_iterator it = map.constBegin (); it != map.constEnd (); ++it){
		if (!(*it)->close ()) {
			// If a child refuses to close, we return false.
			slotSetStatusReady ();
			return false;
		}
	}

	return true;
}

void RKWardMainWindow::raiseWatch () {
	RK_TRACE (APP);
	watch_view->show ();
	KMdiChildView *window = activeWindow ();
	if (window) {
		window->activate ();
	}
}

void RKWardMainWindow::raiseConsole () {
	RK_TRACE (APP);
	console_view->show ();
	KMdiChildView *window = activeWindow ();
	if (window) {
		window->activate ();
	}
}

void RKWardMainWindow::invokeRHelp () {
	RK_TRACE (APP);

	RKGlobals::rInterface ()->issueCommand ("help.start ()", RCommand::App);
	topLevelWidget ()->raise ();
}

void RKWardMainWindow::reportRKWardBug () {
	RK_TRACE (APP);

// TOOD: something pretty
	KMessageBox::information (this, i18n ("Please submit your bug reports or wishes at http://sourceforge.net/tracker/?group_id=50231&atid=459007 or send email to rkward-devel@lists.sourceforge.net"));
}

void RKWardMainWindow::showAboutApplication () {
	RK_TRACE (APP);

	KAboutApplication *about = new KAboutApplication ();
	about->exec ();
	delete about;
}

void RKWardMainWindow::showHelpSearch () {
	RK_TRACE (APP);

	search_help_view->show ();
	search_help_view->wrapperWidget ()->topLevelWidget ()->raise ();
}

void RKWardMainWindow::showRKWardHelp () {
	RK_TRACE (APP);

	RKWorkplace::mainWorkplace ()->openHelpWindow ("rkward://page/rkward_welcome", true);
	topLevelWidget ()->raise ();
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

void RKWardMainWindow::fileOpenNoSave (const KURL &url) {
	RK_TRACE (APP);

	slotCloseAllEditors ();

	slotSetStatusBarText(i18n("Opening workspace..."));
	KURL lurl = url;
	if (lurl.isEmpty ()) {
		lurl = KFileDialog::getOpenURL (":<rfiles>", i18n("*|All files"), this, i18n("Open File..."));
	}
	if (!lurl.isEmpty ()) {
		openWorkspace (lurl);
	}
	slotSetStatusReady ();
}

void RKWardMainWindow::fileOpenAskSave (const KURL &url) {
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

void RKWardMainWindow::slotFileOpenRecentWorkspace(const KURL& url)
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

void RKWardMainWindow::slotSetStatusBarText (const QString &text) {
	RK_TRACE (APP);

	QString ntext = text.stripWhiteSpace ();
	ntext.replace ("<qt>", "");	// WORKAROUND: what the ?!? is going on? The KTHMLPart seems to post such messages.
	if (ntext.isEmpty ()) {
		statusBar ()->message (i18n ("Ready."));
	} else {
		statusBar ()->message (ntext);
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

	RKWorkplace::mainWorkplace ()->detachWindow (RKWorkplace::mainWorkplace ()->activeAttachedWindow ());
}

void RKWardMainWindow::setRStatus (bool busy) {
	RK_TRACE (APP);
	if (busy) {
		r_status_label->setText (i18n ("R engine busy"));
		r_status_label->setPaletteBackgroundColor (QColor (255, 0, 0));
	} else {
		r_status_label->setText (i18n ("R engine idle"));
		r_status_label->setPaletteBackgroundColor (QColor (0, 255, 0));
	}
}

void RKWardMainWindow::importData () {
	RK_TRACE (APP);

	RKContextMap *import_context = RKComponentMap::getContext ("import");
	if (!import_context) {
		KMessageBox::sorry (this, i18n ("No import plugins defined"));
		return;
	}

	QStringList ids = import_context->components ();
	QString formats = "*|" + i18n ("All Files") + " (*)\n";
	for (QStringList::const_iterator it = ids.constBegin (); it != ids.constEnd (); ++it) {
		RKComponentHandle *handle = RKComponentMap::getComponentHandle (*it);
		if (!handle) {
			RK_ASSERT (false);
			continue;
		}
		QString filter = handle->getAttributeValue ("format");
		formats.append (filter + '|' + handle->getAttributeLabel ("format") + " (" + filter + ")\n");
	}

	// this is not correct! we need a customized class for this, as we need to select the import filter to use at the same time
	QString filename = KFileDialog::getOpenFileName (QString::null, formats, this);
#warning TODO
}

void RKWardMainWindow::slotNewCommandEditor () {
	RK_TRACE (APP);

	slotOpenCommandEditor (KURL ());
}

void RKWardMainWindow::slotOpenCommandEditor (const KURL &url) {
	RK_TRACE (APP);

	if (RKWorkplace::mainWorkplace ()->openScriptEditor (url)) {
		if (!url.isEmpty ()) fileOpenRecent->addURL (url);
	}
};

void RKWardMainWindow::slotOpenCommandEditor () {
	RK_TRACE (APP);
	KURL::List urls;
	KURL::List::const_iterator it;
	
	urls = KFileDialog::getOpenURLs (":<rfiles>", "*.R *.r *.S *.s *.q|R Script Files (*.R *.r *.S *.s *.q)\n*.*|All Files (*.*)", this, i18n ("Open command file(s)"));

	for (it = urls.begin() ; it != urls.end() ; ++it) {
		slotOpenCommandEditor (*it);
	}
};

void RKWardMainWindow::slotChildWindowCloseRequest (KMdiChildView * window) {
	RK_TRACE (APP);

	closeWindow (window);
}

void RKWardMainWindow::openHTML (const KURL &url) {
	RK_TRACE (APP);

	RKWorkplace::mainWorkplace ()->openHelpWindow (url);
}

void RKWardMainWindow::openHTMLHelp (const QString & url) {
	RK_TRACE (APP);
	openHTML (url);
}

void RKWardMainWindow::slotOutputShow () {
	RK_TRACE (APP);

	RKWorkplace::mainWorkplace ()->openOutputWindow (KURL ());
}

void RKWardMainWindow::setCaption (const QString &) {
	RK_TRACE (APP);

	QString wcaption = RObjectList::getObjectList ()->getWorkspaceURL ().fileName ();
	if (wcaption.isEmpty ()) wcaption = RObjectList::getObjectList ()->getWorkspaceURL ().prettyURL ();
	if (wcaption.isEmpty ()) wcaption = i18n ("[Unnamed Workspace]");
	wcaption.append (" - " + RKWorkplace::mainWorkplace ()->view ()->activeCaption ());
	KMdiMainFrm::setCaption (wcaption);
}

#include "rkward.moc"
