/***************************************************************************
                          rkward.cpp  -  description
                             -------------------
    begin                : Tue Oct 29 20:06:08 CET 2002
    copyright            : (C) 2002 by Thomas Friedrichsmeier 
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
#include "rkeditormanager.h"
#include "rkdocmanager.h" 
#include "core/rkmodificationtracker.h"
#include "rkwatch.h"
#include "misc/rkmenu.h"
#include "misc/rkmenulist.h"
#include "plugin/rkcomponentmap.h"
//#include "rkoutputwindow.h"
#include "settings/rksettings.h"
#include "settings/rksettingsmoduleplugins.h"
#include "settings/rksettingsmodulelogfiles.h"
#include "settings/rksettingsmoduleoutput.h"
#include "rbackend/rinterface.h"
#include "core/robjectlist.h"
#include "rkglobals.h"
#include "robjectbrowser.h"
#include "dialogs/startupdialog.h"
#include "dialogs/rkloadlibsdialog.h"
#include "agents/rksaveagent.h"
#include "agents/rkloadagent.h"
#include "windows/rkcommandeditorwindow.h"
#include "windows/rkhtmlwindowpart.h"
#include "khelpdlg.h"
#include "rkconsole.h"
#include "debug.h"

#define ID_STATUS_MSG 1
#define ID_R_STATUS_MSG 2

#include "agents/showedittextfileagent.h"	// TODO: see below: needed purely for linking!

RKwardApp::RKwardApp (KURL *load_url) : DCOPObject ("rkwardapp"), KMdiMainFrm (0, 0, KMdi::IDEAlMode) {
	ShowEditTextFileAgent::showEditFiles (0);		// TODO: AAAAAAAARGGGH!!!! It won't link without this bogus line!!!

	RK_TRACE (APP);
	RKGlobals::app = this;
	RKGlobals::rinter = 0;
	RKGlobals::list = 0;
	RKSettings::settings_tracker = new RKSettingsTracker (this);
	
	KGlobal::dirs()->addResourceType("plugins", KStandardDirs::kde_default("data") + "rkward/plugins/");
	
	// Nice docks a la Kdevelop.
	setToolviewStyle(KMultiTabBar::KDEV3ICON);
	
	
	
	///////////////////////////////////////////////////////////////////
	// call inits to invoke all other construction parts
	initStatusBar();
	initActions();

	///////////////////////////////////////////////////////////////////
	// build the interface

	// use the absolute path to your rkwardui.rc file for testing purpose in createGUI();
	setXMLFile( "rkwardui.rc" );
	createShellGUI ( true );

	RKGlobals::manager = new RKEditorManager ();

	connect (this, SIGNAL (childWindowCloseRequest (KMdiChildView *)), this, SLOT (slotChildWindowCloseRequest (KMdiChildView *)));

	RKGlobals::mtracker = new RKModificationTracker (this);
	RKGlobals::cmap = new RKComponentMap ();

	initial_url = load_url;

	startup_timer = new QTimer (this);
	startup_timer->start (50);
	connect (startup_timer, SIGNAL (timeout ()), this, SLOT (doPostInit ()));
	

	m_manager = new KParts::PartManager( this );
	// When the manager says the active part changes,
	// the builder updates (recreates) the GUI
	connect (m_manager, SIGNAL (activePartChanged (KParts::Part *)), this, SLOT (createGUI (KParts::Part *)));

	if (!kapp->dcopClient ()->isRegistered ()) {
		kapp->dcopClient ()->registerAs ("rkward");
		kapp->dcopClient ()->setDefaultObject (objId ());
	}
}

void RKwardApp::activateGUI (KParts::Part *part) {
	RK_TRACE (APP);
	createGUI (part);
}

RKwardApp::~RKwardApp() {
	RK_TRACE (APP);
	closeAllViews ();
	delete RKGlobals::rInterface ();
	delete RKGlobals::rObjectList ();
	delete object_browser;
	delete RKGlobals::tracker ();
	delete RKGlobals::editorManager ();
}

void RKwardApp::doPostInit () {
	RK_TRACE (APP);
	delete startup_timer;

	readOptions();
	object_browser = new RObjectBrowser ();
	
	QString dummy = i18n("Before you start bashing at it: please note that this is merely a technology preview release. You might actually be able to use it for some very simple tasks, but chances are it's of hardly any practical value so far. It does not do much good. It might do some very bad things (don't let it touch valuable data!). It's lacking in many respects. If you would like to help improve it, or simply get in contact, visit:\nhttp://rkward.sourceforge.net\nAll comments are welcome.");
	KMessageBox::information (this, dummy, i18n("Before you complain..."), "state_of_rkward");
	
	startR ();

	// create handle for menu bar and register standard menus
	menu_list = new RKMenuList (menuBar ());
	QMenuItem* item = menuBar ()->findItem (menuBar ()->idAt (0));
	if (item && item->popup ()) menu_list->registerMenu (item->popup (), "file");
	item = menuBar ()->findItem (menuBar ()->idAt (1));
	if (item && item->popup ()) menu_list->registerMenu (item->popup (), "edit");
	item = menuBar ()->findItem (menuBar ()->idAt (2));
	if (item && item->popup ()) menu_list->registerMenu (item->popup (), "workspace");
	item = menuBar ()->findItem (menuBar ()->idAt (3));
	if (item && item->popup ()) menu_list->registerMenu (item->popup (), "output");
	item = menuBar ()->findItem (menuBar ()->idAt (4));
	if (item && item->popup ()) menu_list->registerMenu (item->popup (), "run");
	item = menuBar ()->findItem (menuBar ()->idAt (5));
	if (item && item->popup ()) menu_list->registerMenu (item->popup (), "settings");
	item = menuBar ()->findItem (menuBar ()->idAt (6));
	if (item && item->popup ()) menu_list->registerMenu (item->popup (), "help");

	initPlugins ();
	
	//It's necessary to give a different name to all tool windows, or they won't be properly displayed
	object_browser->setName("workspace"); 
	object_browser->setIcon(SmallIcon("view_tree"));
	addToolWindow(object_browser,KDockWidget::DockLeft, getMainDockWidget(), 30 , i18n ("Existing objects in your workspace.") , i18n ("Workspace"));
	
	RKGlobals::rInterface ()->watch->setName("Command log");
	RKGlobals::rInterface ()->watch->setIcon(SmallIcon("text_block"));
	addToolWindow(RKGlobals::rInterface ()->watch,KDockWidget::DockBottom, getMainDockWidget(), 10);

	console = new RKConsole(0);
	console->setIcon(SmallIcon("konsole"));
	console->setName("r_console");
	addToolWindow(console,KDockWidget::DockBottom, getMainDockWidget(), 10);
	
	RKGlobals::helpdlg = new KHelpDlg(0);
	RKGlobals::helpDialog ()->setIcon(SmallIcon("help"));
	addToolWindow(RKGlobals::helpDialog (), KDockWidget::DockBottom, getMainDockWidget(), 10);

	if (initial_url) {
		openWorkspace (*initial_url);
		delete initial_url;
	} else {
		setCaption(i18n ("Untitled"));
		
		StartupDialog::StartupDialogResult *result = StartupDialog::getStartupAction (this, fileOpenRecentWorkspace);
		if (result->result == StartupDialog::EmptyWorkspace) {
		} else if (result->result == StartupDialog::OpenFile) {
			openWorkspace (result->open_url);
		} else if (result->result == StartupDialog::ChoseFile) {
			slotFileOpenWorkspace ();
		} else if (result->result == StartupDialog::EmptyTable) {
			RObject *object = RKGlobals::rObjectList ()->createNewChild (i18n ("my.data"), 0, true, true);
			// usually an explicit call to activateView should not be necessary. Somehow however, here, it is.
			RKGlobals::editorManager ()->editObject (object, true);
		}
		delete result;
	}
	
	show ();
}

void RKwardApp::initPlugins () {
	RK_TRACE (APP);
	slotStatusMsg(i18n("Setting up plugins..."));
	
	RKGlobals::componentMap ()->clear ();

	QStringList list = RKSettingsModulePlugins::pluginMaps ();
	int counter = 0;
	for (QStringList::const_iterator it = RKSettingsModulePlugins::pluginMaps ().begin (); it != RKSettingsModulePlugins::pluginMaps ().end (); ++it) {
		counter += RKGlobals::componentMap ()->addPluginMap ((*it));
	}

	if (counter < 1) {
		KMessageBox::information (0, i18n ("Plugins are needed: you may manage these through \"Settings->Configure RKWard\".\n"), i18n ("No (valid) plugins found"));
	}

	slotStatusReady ();
}

void RKwardApp::startR () {
	RK_TRACE (APP);
	RK_ASSERT (!RKGlobals::rInterface ());
	
	QDir dir (RKSettingsModuleLogfiles::filesPath());
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
	RKGlobals::list = new RObjectList ();
	
	object_browser->initialize ();
}

void RKwardApp::slotConfigure () {
	RKSettings::configureSettings (RKSettings::NoPage, this);
}

void RKwardApp::initActions()
{  
	RK_TRACE (APP);
	// TODO: is there a way to insert actions between standard actions without having to give all standard actions custom ids?
	new_data_frame = new KAction (i18n ("Dataset"), 0, 0, this, SLOT (slotNewDataFrame ()), actionCollection (), "new_data_frame");
	new_data_frame->setIcon("spreadsheet");
	new_command_editor = KStdAction::openNew(this, SLOT(slotNewCommandEditor()), actionCollection(), "new_command_editor");
	new_command_editor->setText (i18n ("Command File"));
	new_command_editor->setIcon("source");
	
	fileOpen = KStdAction::open(this, SLOT(slotOpenCommandEditor()), actionCollection(), "file_openy");
	fileOpen->setText (i18n ("Open Command File"));
	fileOpenRecent = KStdAction::openRecent(this, SLOT(slotOpenCommandEditor (const KURL&)), actionCollection(), "file_open_recenty");
	
	fileOpenWorkspace = KStdAction::open(this, SLOT(slotFileOpenWorkspace()), actionCollection(), "file_openx");
	fileOpenWorkspace->setText (i18n ("Open Workspace"));
	fileOpenRecentWorkspace = KStdAction::openRecent(this, SLOT(slotFileOpenRecentWorkspace(const KURL&)), actionCollection(), "file_open_recentx");
	fileSaveWorkspace = KStdAction::save(this, SLOT(slotFileSaveWorkspace()), actionCollection(), "file_savex");
	fileSaveWorkspace->setText (i18n ("Save Workspace"));
	fileSaveWorkspaceAs = KStdAction::saveAs(this, SLOT(slotFileSaveWorkspaceAs()), actionCollection(), "file_save_asx");
	fileSaveWorkspaceAs->setText (i18n ("Save Workspace As"));

	filePrint = KStdAction::print(this, SLOT(slotFilePrint()), actionCollection(), "file_printx");
	filePrint->setEnabled (false);
	fileQuit = KStdAction::quit(this, SLOT(slotFileQuit()), actionCollection(), "file_quitx");
	file_load_libs = new KAction (i18n ("Configure Packages"), 0, 0, this, SLOT (slotFileLoadLibs ()), actionCollection (), "file_load_libs");	


	viewToolBar = KStdAction::showToolbar(this, SLOT (slotViewToolBar()), actionCollection());
	viewStatusBar = KStdAction::showStatusbar(this, SLOT (slotViewStatusBar()), actionCollection());
	
	interruptCommand = new KAction (i18n ("Interrupt running command"), 0, 0, this, SLOT (slotInterruptCommand ()), actionCollection (), "interrupt");
	interruptCommand->setIcon("player_stop");

	close_all_editors = new KAction (i18n ("Close All Data"), 0, 0, this, SLOT (slotCloseAllEditors ()), actionCollection (), "close_all_editors");
	window_close = new KAction (i18n ("Close"), 0, KShortcut ("Ctrl+W"), this, SLOT (slotCloseWindow ()), actionCollection (), "window_close");
	window_close_all = new KAction (i18n ("Close All"), 0, 0, this, SLOT (slotCloseAllWindows ()), actionCollection (), "window_close_all");
	window_detach = new KAction (i18n ("Detach"), 0, 0, this, SLOT (slotDetachWindow ()), actionCollection (), "window_detach");
	outputShow= new KAction (i18n ("Show &Output"), 0, 0, this, SLOT (slotOutputShow ()), actionCollection (), "output_show");

	configure = new KAction (i18n ("Configure RKWard"), 0, 0, this, SLOT (slotConfigure ()), actionCollection (), "configure");
	
	new_data_frame->setStatusText (i18n ("Creates new empty dataset and opens it for editing"));
	fileOpenWorkspace->setStatusText(i18n("Opens an existing document"));
	fileOpenRecentWorkspace->setStatusText(i18n("Opens a recently used file"));
	fileSaveWorkspace->setStatusText(i18n("Saves the actual document"));
	fileSaveWorkspaceAs->setStatusText(i18n("Saves the actual document as..."));
	close_all_editors->setStatusText (i18n ("Closes all open data editors"));
	filePrint ->setStatusText(i18n("Prints out the actual document"));
	fileQuit->setStatusText(i18n("Quits the application"));
	viewToolBar->setStatusText(i18n("Enables/disables the toolbar"));
	viewStatusBar->setStatusText(i18n("Enables/disables the statusbar"));
}


void RKwardApp::initStatusBar()
{
	RK_TRACE (APP);
	///////////////////////////////////////////////////////////////////
	// STATUSBAR
	// TODO: add your own items you need for displaying current application status.
	statusBar()->insertItem(i18n("Ready."), ID_STATUS_MSG);
	statusBar()->insertItem(i18n("starting R engine"), ID_R_STATUS_MSG);
}

void RKwardApp::openWorkspace (const KURL &url) {
	RK_TRACE (APP);
	new RKLoadAgent (url, false);
	fileOpenRecentWorkspace->addURL (url);
}

void RKwardApp::saveOptions () {	
	RK_TRACE (APP);
	KConfig *config = kapp->config ();

	config->setGroup("General Options");
	config->writeEntry("Geometry", size());
	config->writeEntry("Show Toolbar", viewToolBar->isChecked());
	config->writeEntry("Show Statusbar",viewStatusBar->isChecked());
	config->writeEntry("ToolBarPos", (int) toolBar("mainToolBar")->barPos());
	config->writeEntry("EditBarPos", (int) toolBar("editToolBar")->barPos());
	config->writeEntry("RunBarPos", (int) toolBar("runToolBar")->barPos());

	RKSettings::saveSettings (config);
	
	fileOpenRecentWorkspace->saveEntries(config, "Recent Files");
	fileOpenRecent->saveEntries(config, "Recent Command Files");
}


void RKwardApp::readOptions () {
	RK_TRACE (APP);
	KConfig *config = kapp->config ();
	
	config->setGroup("General Options");
	
	// bar status settings
	viewToolBar->setChecked (config->readBoolEntry ("Show Toolbar", true));
	slotViewToolBar ();

	viewStatusBar->setChecked (config->readBoolEntry ("Show Statusbar", true));
	slotViewStatusBar();

	// bar position settings
	toolBar("mainToolBar")->setBarPos ((KToolBar::BarPosition) config->readNumEntry ("ToolBarPos", KToolBar::Top));
	toolBar("editToolBar")->setBarPos ((KToolBar::BarPosition) config->readNumEntry ("EditBarPos", KToolBar::Top));
	toolBar("runToolBar")->setBarPos ((KToolBar::BarPosition) config->readNumEntry("RunBarPos", KToolBar::Top));
	
	QSize size=config->readSizeEntry("Geometry");
	if(!size.isEmpty ()) {
		resize (size);
	}
	
	// initialize the recent file list
	fileOpenRecentWorkspace->loadEntries (config,"Recent Files");
	fileOpenRecent->loadEntries (config,"Recent Command Files");
	
	// do this last, since we may be setting some different config-group(s) in the process
	RKSettings::loadSettings (config);
}

void RKwardApp::saveProperties(KConfig *_cfg)
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


void RKwardApp::readProperties(KConfig* _cfg)
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
      setCaption(_url.fileName(),true);
      QFile::remove(tempname);
    }
  }
  else
  {
    if(!filename.isEmpty())
    {
//      doc->openDocument(url);
      setCaption(url.fileName(),false);
    }
  } */
}		

bool RKwardApp::queryClose () {
	RK_TRACE (APP);

	if (!RKGlobals::rObjectList ()->isEmpty ()) {
		int res;
		res = KMessageBox::questionYesNoCancel (this, i18n ("Quitting RKWard: Do you want to save the workspace?\nPress Cancel, if you do not want to quit"), i18n ("Save Workspace?"));
		if (res == KMessageBox::Yes) {
			new RKSaveAgent (RKGlobals::rObjectList ()->getWorkspaceURL (), false, RKSaveAgent::Quit);
		} else if (res != KMessageBox::No) {
			return false;
		}
	}

	QValueList<KMdiChildView *> child_copy;
	for(KMdiChildView *w = m_pDocumentViews->first ();w;w= m_pDocumentViews->next ()){
		child_copy.append (w);
	}
	QValueListIterator<KMdiChildView *> childIt;
	for (childIt = child_copy.begin (); childIt != child_copy.end (); ++childIt) {
		if (!(*childIt)->close ()) {
			// If a child refuses to close, we return false.
			return false;
		}
	}

	return true;
}

bool RKwardApp::queryExit()
{
	RK_TRACE (APP);
	saveOptions();
	return true;
}

/////////////////////////////////////////////////////////////////////
// SLOT IMPLEMENTATION
/////////////////////////////////////////////////////////////////////

void RKwardApp::slotNewDataFrame () {
	RK_TRACE (APP);
	bool ok;

	QString name = KInputDialog::getText (i18n ("New dataset"), i18n ("Enter name for the new dataset"), "my.data", &ok, this);

	if (ok) {
		QString valid = RKGlobals::rObjectList ()->validizeName (name);
		if (valid != name) KMessageBox::sorry (this, i18n ("The name you specified was already in use or not valid. Renamed to %1").arg (valid), i18n ("Invalid Name"));
		RObject *object = RKGlobals::rObjectList ()->createNewChild (valid, 0, true, true);
		RKGlobals::editorManager ()->editObject (object, true);
	}
	
}

void RKwardApp::fileOpenNoSave (const KURL &url) {
	RK_TRACE (APP);

	slotCloseAllEditors ();

	slotStatusMsg(i18n("Opening workspace..."));
	KURL lurl = url;
	if (lurl.isEmpty ()) {
		lurl = KFileDialog::getOpenURL (QString::null, i18n("*|All files"), this, i18n("Open File..."));
	}
	if (!lurl.isEmpty ()) {
		openWorkspace (lurl);
	}
	slotStatusReady ();
}

void RKwardApp::fileOpenAskSave (const KURL &url) {
	RK_TRACE (APP);
	if (RKGlobals::rObjectList ()->isEmpty ()) {
		fileOpenNoSave (url);
		return;
	}
	
	int res;
	res = KMessageBox::questionYesNoCancel (this, i18n ("Do you want to save the current workspace?"), i18n ("Save Workspace?"));
	if (res == KMessageBox::No) {
		fileOpenNoSave (url);
	} else if (res == KMessageBox::Yes) {
		new RKSaveAgent (RKGlobals::rObjectList ()->getWorkspaceURL (), false, RKSaveAgent::Load, url);
	}
	// else: cancel. Don't do anything
}

void RKwardApp::slotFileOpenWorkspace () {
	RK_TRACE (APP);
	fileOpenAskSave ("");
}

void RKwardApp::slotFileOpenRecentWorkspace(const KURL& url)
{
	RK_TRACE (APP);
	fileOpenAskSave (url);
}

void RKwardApp::slotFileLoadLibs () {
	RK_TRACE (APP);
	RKLoadLibsDialog *dial = new RKLoadLibsDialog (this, 0);
	dial->show ();
}

void RKwardApp::slotFileSaveWorkspace () {
	RK_TRACE (APP);
	new RKSaveAgent (RKGlobals::rObjectList ()->getWorkspaceURL ());
}

void RKwardApp::slotFileSaveWorkspaceAs () {
	RK_TRACE (APP);
	new RKSaveAgent (RKGlobals::rObjectList ()->getWorkspaceURL (), true);
}

void RKwardApp::slotFilePrint()
{
		RK_TRACE (APP);
	slotStatusMsg(i18n("Printing..."));
	
	QPrinter printer;
	if (printer.setup(this))
	{
	//    view->print(&printer);
	}
	
	slotStatusReady ();
}

void RKwardApp::slotFileQuit () {
	RK_TRACE (APP);
	slotStatusMsg(i18n("Exiting..."));
	saveOptions();
	close ();
}

void RKwardApp::slotViewToolBar()
{
	RK_TRACE (APP);
  slotStatusMsg(i18n("Toggling toolbar..."));
  ///////////////////////////////////////////////////////////////////
  // turn Toolbar on or off
  if(!viewToolBar->isChecked())
  {
    toolBar("mainToolBar")->hide();
    toolBar("runToolBar")->hide();
    toolBar("editToolBar")->hide();
  }
  else
  {
    toolBar("mainToolBar")->show();
    toolBar("runToolBar")->show();
    toolBar("editToolBar")->show();
  }		

  slotStatusReady ();
}

void RKwardApp::slotViewStatusBar()
{
	RK_TRACE (APP);
  slotStatusMsg(i18n("Toggling statusbar..."));
  ///////////////////////////////////////////////////////////////////
  //turn Statusbar on or off
  if(!viewStatusBar->isChecked())
  {
    statusBar()->hide();
  }
  else
  {
    statusBar()->show();
  }

  slotStatusReady ();
}


void RKwardApp::slotStatusMsg(const QString &text) {
	RK_TRACE (APP);

  ///////////////////////////////////////////////////////////////////
  // change status message permanently
  statusBar()->clear();
  statusBar()->changeItem(text, ID_STATUS_MSG);
}

void RKwardApp::slotStatusReady () {
	RK_TRACE (APP);

	slotStatusMsg (i18n ("Ready"));
}

void RKwardApp::slotCloseWindow () {
	RK_TRACE (APP);

	closeActiveView ();
}

void RKwardApp::slotCloseAllWindows () {
	RK_TRACE (APP);

	closeAllViews ();
}

void RKwardApp::slotCloseAllEditors () {
	RK_TRACE (APP);

	RKGlobals::editorManager ()->closeAll ();
}

void RKwardApp::slotDetachWindow () {
	RK_TRACE (APP);

	if (activeWindow ()) {
		RK_TRACE (APP);
		detachWindow (activeWindow ());
	}
}


void RKwardApp::newOutput () {
	RK_TRACE (APP);
	RKHTMLWindowPart::refreshOutput (RKSettingsModuleOutput::autoShow (), RKSettingsModuleOutput::autoRaise ());
}

void RKwardApp::setRStatus (bool busy) {
	RK_TRACE (APP);
	if (busy) {
		statusBar()->changeItem(i18n("R engine busy"), ID_R_STATUS_MSG);
	} else {
		statusBar ()->changeItem (i18n ("R engine idle"), ID_R_STATUS_MSG);
	}
}


void RKwardApp::slotNewCommandEditor () {
	RK_TRACE (APP);
	RKCommandEditorWindow *editor = new RKCommandEditorWindow;
	editor->setIcon (SmallIcon("source"));
	addWindow (editor);
	editor->activate ();
}

void RKwardApp::slotOpenCommandEditor (const KURL &url) {
	RK_TRACE (APP);
	RKCommandEditorWindow *editor;

	editor = new RKCommandEditorWindow;

	if (!editor->openURL (url)) {
		QString errstr = i18n ("Unable to open \"%1\"").arg (url.prettyURL ());

		KMessageBox::messageBox (this, KMessageBox::Error, errstr, i18n ("Could not open command file"));
		delete editor;
		return;
	}

	fileOpenRecent->addURL (url);
	editor->setIcon (SmallIcon("source"));
	addWindow (editor);
	editor->activate ();
};

void RKwardApp::slotOpenCommandEditor (){
	RK_TRACE (APP);
	KURL::List urls;
	KURL::List::const_iterator it;
	
	urls = KFileDialog::getOpenURLs (QString("."), QString("*.R *.r"), this, i18n ("Open command file(s)"));

	for (it = urls.begin() ; it != urls.end() ; ++it){
		slotOpenCommandEditor (*it);
	}
};

void RKwardApp::slotChildWindowCloseRequest (KMdiChildView * window) {
	RK_TRACE (APP);
	//If it's an unsaved command editor window, there is a warning.
	// TODO: it's sort of ugly, having to handle this here. Can't we somehow handle it in RKCommandEditorWindow instead?
	if (window->inherits("RKCommandEditorWindow")) {
		RKCommandEditorWindow * editor = (RKCommandEditorWindow*) window;
		if (editor->isModified()) {
			int status = KMessageBox::warningYesNo(this,i18n("The document \"%1\" has been modified. Close it anyway?").arg(editor->tabCaption()),i18n("File not saved"));
	
			if (status != KMessageBox::Yes) {
				return;
			}
		}
	}

	closeWindow(window);
}

void RKwardApp::slotInterruptCommand () {
// TODO!
}

void RKwardApp::openHTML(const KURL &url) {
	RK_TRACE (APP);
	RKHTMLWindowPart::openHTML (url, false);
}

void RKwardApp::openHTMLHelp (const QString & url) {
	RK_TRACE (APP);
	openHTML (url);
}

void RKwardApp::slotOutputShow () {
	RK_TRACE (APP);
	RKHTMLWindowPart::refreshOutput (true, true);
}

