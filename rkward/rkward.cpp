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
#include <qdir.h>
#include <qprinter.h>
#include <qpainter.h>
#include <qclipboard.h>
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
#include "dataeditor/rkeditor.h"
#include "dataeditor/rkdrag.h"
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
#include "windows/rkhelpwindow.h"
#include "khelpdlg.h"
#include "rkconsole.h"
#include "debug.h"

#define ID_STATUS_MSG 1
#define ID_R_STATUS_MSG 2

#include "agents/showedittextfileagent.h"	// TODO: see below: needed purely for linking!

RKwardApp::RKwardApp (KURL *load_url, QWidget* , const char* name) : KMdiMainFrm (0, name, KMdi::IDEAlMode) {
	ShowEditTextFileAgent::showEditFiles (0);		// TODO: AAAAAAAARGGGH!!!! It won't link without this bogus line!!!

	RK_TRACE (APP);
	RKGlobals::app = this;
	RKGlobals::rinter = 0;
	RKGlobals::list = 0;
	RKSettings::settings_tracker = new RKSettingsTracker (this);
	
	config=kapp->config();
	
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

	setEnabledActions(true);

	RKGlobals::manager = new RKEditorManager ();
	KMdiChildView * editorManagerView = createWrapper(RKGlobals::editorManager (), i18n( "Data editor"), i18n( "Data editor"));
	editorManagerView->setIcon(SmallIcon("spreadsheet"));
	editorManagerView->setName("dataeditor");
	addWindow( editorManagerView );
	
	

	
	connect (this, SIGNAL (childWindowCloseRequest (KMdiChildView *)), this, SLOT (slotChildWindowCloseRequest (KMdiChildView *)));
	connect (this, SIGNAL (viewActivated (KMdiChildView *)), this, SLOT (slotViewActivated (KMdiChildView *)));


	connect (RKGlobals::editorManager (), SIGNAL (editorClosed ()), this, SLOT (slotEditorsChanged ()));
	connect (RKGlobals::editorManager (), SIGNAL (editorOpened ()), this, SLOT (slotEditorsChanged ()));
	RKGlobals::mtracker = new RKModificationTracker (this);
	RKGlobals::cmap = new RKComponentMap ();

	initial_url = load_url;
  
	startup_timer = new QTimer (this);
	startup_timer->start (50);
	connect (startup_timer, SIGNAL (timeout ()), this, SLOT (doPostInit ()));
	

	m_manager = new KParts::PartManager( this );
	// When the manager says the active part changes,
	// the builder updates (recreates) the GUI
	connect( m_manager, SIGNAL( activePartChanged( KParts::Part * ) ), this, SLOT( createGUI( KParts::Part * ) ) );
}

RKwardApp::~RKwardApp() {
	RK_TRACE (APP);
	slotCloseAllEditors ();
	delete RKGlobals::rInterface ();
	delete RKGlobals::rObjectList ();
	delete object_browser;
	delete RKGlobals::tracker ();
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
			RKGlobals::editorManager ()->editObject (object, true);
		}
		delete result;
	}
	
	show ();
	
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
	
	helpDlg = new KHelpDlg(0);
	helpDlg->setIcon(SmallIcon("help"));
	addToolWindow(helpDlg,KDockWidget::DockBottom, getMainDockWidget(), 10);
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

	slotStatusMsg(i18n("Ready."));
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
	fileOpenRecent = KStdAction::openRecent(this, SLOT(slotOpenRecentCommandEditor(const KURL&)), actionCollection(), "file_open_recenty");
	fileSave = KStdAction::save(this, SLOT(slotSaveCommandEditor()), actionCollection(), "file_savey");
	fileSave->setText (i18n ("Save Command File"));
	fileSaveAs = KStdAction::saveAs(this, SLOT(slotSaveCommandEditorAs()), actionCollection(), "file_save_asy");
	fileSaveAs->setText (i18n ("Save Command File As"));
	
	fileOpenWorkspace = KStdAction::open(this, SLOT(slotFileOpenWorkspace()), actionCollection(), "file_openx");
	fileOpenWorkspace->setText (i18n ("Open Workspace"));
	fileOpenRecentWorkspace = KStdAction::openRecent(this, SLOT(slotFileOpenRecentWorkspace(const KURL&)), actionCollection(), "file_open_recentx");
	fileSaveWorkspace = KStdAction::save(this, SLOT(slotFileSaveWorkspace()), actionCollection(), "file_savex");
	fileSaveWorkspace->setText (i18n ("Save Workspace"));
	fileSaveWorkspaceAs = KStdAction::saveAs(this, SLOT(slotFileSaveWorkspaceAs()), actionCollection(), "file_save_asx");
	fileSaveWorkspaceAs->setText (i18n ("Save Workspace As"));
	//file_load_libs = new KAction (i18n ("Libraries"), 0, 0, this, SLOT (slotFileLoadLibs ()), actionCollection (), "file_load_libs");
	close_editor = KStdAction::close (this, SLOT(slotCloseEditor ()), actionCollection(), "editor_close");
	close_editor->setText (i18n ("Close current editor"));
	close_editor->setEnabled (false);
	close_all_editors = new KAction (i18n ("Close All Editors"), 0, 0, this, SLOT(slotCloseAllEditors ()), actionCollection (), "close_all_editors");
	close_all_editors->setEnabled (false);
	filePrint = KStdAction::print(this, SLOT(slotFilePrint()), actionCollection(), "file_printx");
	filePrint->setEnabled (false);
	fileQuit = KStdAction::quit(this, SLOT(slotFileQuit()), actionCollection(), "file_quitx");
	
	editCut = KStdAction::cut(this, SLOT(slotEditCut()), actionCollection(), "cut");
	editCopy = KStdAction::copy(this, SLOT(slotEditCopy()), actionCollection(), "copy");
	editPaste = KStdAction::paste(this, SLOT(slotEditPaste()), actionCollection(), "paste");
	editPasteToTable = new KAction(i18n("Paste inside Table"), 0, 0, this, SLOT(slotEditPasteToTable()), actionCollection(), "paste_to_table");
	editPasteToTable->setIcon("frame_spreadsheet");
	editPasteToSelection = new KAction(i18n("Paste inside Selection"), 0, 0, this, SLOT(slotEditPasteToSelection()), actionCollection(), "paste_to_selection");
	editPasteToSelection->setIcon("frame_edit");


	outputShow= new KAction (i18n ("&Show / Refresh"), 0, 0, this, SLOT (slotOutputShow ()), actionCollection (), "output_show");
	outputFlush= new KAction (i18n ("&Flush"), 0, 0, this, SLOT (slotOutputFlush ()), actionCollection (), "output_flush");


	viewToolBar = KStdAction::showToolbar(this, SLOT(slotViewToolBar()), actionCollection());
	viewStatusBar = KStdAction::showStatusbar(this, SLOT(slotViewStatusBar()), actionCollection());
	showRKWatch = new KToggleAction (i18n ("Console"), 0, 0, this, SLOT(slotShowRKWatch ()), actionCollection(), "windows_rkwatch");
	showRKOutput = new KToggleAction (i18n ("Output"), 0, 0, this, SLOT(slotShowRKOutput ()), actionCollection(), "windows_rkoutput");
	showRObjectBrowser = new KToggleAction (i18n ("Workspace"), 0, 0, this, SLOT(slotShowRObjectBrowser ()), actionCollection(), "windows_robjectbrowser");
	
	runAll = new KAction (i18n ("Run all"), KShortcut ("F9"), this, SLOT (slotRunAll ()), actionCollection (), "run_all");
	runAll->setIcon("player_fwd");
	runSelection = new KAction (i18n ("Run selection"), KShortcut ("F8"), this, SLOT (slotRunSelection ()), actionCollection (), "run_selection");
	runSelection->setIcon("player_play");
	runLine = new KAction (i18n ("Run current line"), KShortcut ("Ctrl+L"), this, SLOT (slotRunLine ()), actionCollection (), "run_line");
	runLine->setIcon("player_play");
	interruptCommand = new KAction (i18n ("Interrupt running command"), 0, 0, this, SLOT (slotInterruptCommand ()), actionCollection (), "interrupt");
	interruptCommand->setIcon("player_stop");



	file_load_libs = new KAction (i18n ("Configure Packages"), 0, 0, this, SLOT (slotFileLoadLibs ()), actionCollection (), "file_load_libs");	
	configure = new KAction (i18n ("Configure RKWard"), 0, 0, this, SLOT(slotConfigure ()), actionCollection(), "configure");
	
	helpFunction = new KAction (i18n ("&Function reference"), KShortcut ("F2"), this, SLOT(slotFunctionReference ()), actionCollection(), "function_reference");
	
	new_data_frame->setStatusText (i18n ("Creates new empty dataset and opens it for editing"));
	fileOpenWorkspace->setStatusText(i18n("Opens an existing document"));
	fileOpenRecentWorkspace->setStatusText(i18n("Opens a recently used file"));
	fileSaveWorkspace->setStatusText(i18n("Saves the actual document"));
	fileSaveWorkspaceAs->setStatusText(i18n("Saves the actual document as..."));
	close_editor->setStatusText (i18n ("Closes the current data editor"));
	close_all_editors->setStatusText (i18n ("Closes all open data editors"));
	filePrint ->setStatusText(i18n("Prints out the actual document"));
	fileQuit->setStatusText(i18n("Quits the application"));
	editCut->setStatusText(i18n("Cuts the selected section and puts it to the clipboard"));
	editCopy->setStatusText(i18n("Copies the selected section to the clipboard"));
	editPaste->setStatusText(i18n("Pastes the clipboard contents to actual position"));
	editPasteToTable->setStatusText(i18n("Pastes the clipboard contents to actual position, but not beyond the table's boundaries"));
	editPasteToSelection->setStatusText(i18n("Pastes the clipboard contents to actual position, but not beyond the boundaries of the current selection"));
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

void RKwardApp::saveOptions()
{	
	RK_TRACE (APP);
	config->setGroup("General Options");
	config->writeEntry("Geometry", size());
	config->writeEntry("Show Toolbar", viewToolBar->isChecked());
	config->writeEntry("Show Statusbar",viewStatusBar->isChecked());
	config->writeEntry("ToolBarPos", (int) toolBar("mainToolBar")->barPos());
	config->writeEntry("EditBarPos", (int) toolBar("editToolBar")->barPos());
	config->writeEntry("RunBarPos", (int) toolBar("runToolBar")->barPos());


	RKSettings::saveSettings (config);
	
	fileOpenRecentWorkspace->saveEntries(config,"Recent Files");
	fileOpenRecent->saveEntries(config,"Recent Command Files");
}


void RKwardApp::readOptions ()
{
	RK_TRACE (APP);
	
	config->setGroup("General Options");
	
	// bar status settings
	bool bViewToolbar = config->readBoolEntry("Show Toolbar", true);
	viewToolBar->setChecked(bViewToolbar);
	slotViewToolBar();
	
	bool bViewStatusbar = config->readBoolEntry("Show Statusbar", true);
	viewStatusBar->setChecked(bViewStatusbar);
	slotViewStatusBar();
	
	
	// bar position settings
	KToolBar::BarPosition toolBarPos;
	toolBarPos=(KToolBar::BarPosition) config->readNumEntry("ToolBarPos", KToolBar::Top);
	toolBar("mainToolBar")->setBarPos(toolBarPos);
	
	KToolBar::BarPosition editBarPos;
	editBarPos=(KToolBar::BarPosition) config->readNumEntry("EditBarPos", KToolBar::Top);
	toolBar("editToolBar")->setBarPos(editBarPos);
	
	KToolBar::BarPosition runBarPos;
	runBarPos=(KToolBar::BarPosition) config->readNumEntry("RunBarPos", KToolBar::Top);
	toolBar("runToolBar")->setBarPos(runBarPos);
	
	QSize size=config->readSizeEntry("Geometry");
	if(!size.isEmpty ()) {
		resize (size);
	}
	
	// initialize the recent file list
	fileOpenRecentWorkspace->loadEntries(config,"Recent Files");
	fileOpenRecent->loadEntries(config,"Recent Command Files");
	
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
	
	QValueList<KMdiChildView *> children;
	for(KMdiChildView *w = m_pDocumentViews->first();w;w= m_pDocumentViews->next()){
		children.append(w);
	}
	QValueListIterator<KMdiChildView *> childIt;
	for (childIt = children.begin(); childIt != children.end(); ++childIt) {
		if ((*childIt)->inherits("RKCommandEditorWindow") ) {
			if (! (*childIt)->close()) {
				//If a child refuses to close, we return false.
				return false;
			}
		}
	}
	
	
	if (RKGlobals::rObjectList ()->isEmpty ()) return true;

	int res;
	res = KMessageBox::questionYesNoCancel (this, i18n ("Do you want to save the workspace?"), i18n ("Save Workspace?"));
	if (res == KMessageBox::No) return true;
	if (res == KMessageBox::Yes) new RKSaveAgent (RKGlobals::rObjectList ()->getWorkspaceURL (), false, RKSaveAgent::Quit);

	return false;
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

void RKwardApp::slotEditorsChanged () {
	RK_TRACE (APP);
	close_editor->setEnabled (RKGlobals::editorManager ()->numEditors ());
	close_all_editors->setEnabled (RKGlobals::editorManager ()->numEditors ());
}

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
	slotStatusMsg(i18n("Opening workspace..."));
	KURL lurl = url;
	if (lurl.isEmpty ()) {
		lurl = KFileDialog::getOpenURL (QString::null, i18n("*|All files"), this, i18n("Open File..."));
	}
	if (!lurl.isEmpty ()) {
		openWorkspace (lurl);
	}
	slotStatusMsg(i18n("Ready."));
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

void RKwardApp::slotCloseEditor () {
	RK_TRACE (APP);
	RKGlobals::editorManager ()->closeEditor (RKGlobals::editorManager ()->currentEditor ());
}

void RKwardApp::slotCloseAllEditors () {
	RK_TRACE (APP);
	while (RKGlobals::editorManager ()->numEditors ()) {
		RKGlobals::editorManager ()->closeEditor (RKGlobals::editorManager ()->currentEditor ());
	}
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
	
	slotStatusMsg(i18n("Ready."));
}

void RKwardApp::slotFileQuit () {
	RK_TRACE (APP);
	slotStatusMsg(i18n("Exiting..."));
	saveOptions();
	close ();
}

void RKwardApp::slotEditCut()
{
	RK_TRACE (APP);
	
	if ((QString) activeWindow()->name()!="dataeditor"){
		return;
	}
	
	slotStatusMsg(i18n("Cutting selection..."));
	slotEditCopy ();
	RKGlobals::editorManager ()->currentEditor ()->clearSelected ();
	slotStatusMsg(i18n("Ready."));
	
}

void RKwardApp::slotEditCopy() {
	RK_TRACE (APP);
	
	if ((QString) activeWindow()->name()!="dataeditor"){
		return;
	}
	
	slotStatusMsg(i18n("Copying selection to clipboard..."));
	QApplication::clipboard()->setData(RKGlobals::editorManager ()->currentEditor ()->makeDrag ());
	slotStatusMsg(i18n("Ready."));
	
}

void RKwardApp::doPaste () {
	RK_TRACE (APP);
	
	if ((QString) activeWindow()->name()!="dataeditor"){
		return;
	}
	
	
	slotStatusMsg(i18n("Inserting clipboard contents..."));

	// actually, we don't care, whether tsv or plain gets pasted - it's both
	// treated the same. We should however encourage external senders to
	// provided the two in order.
	if (QApplication::clipboard()->data()->provides ("text/tab-separated-values")) {
		qDebug ("paste tsv");
		RKGlobals::editorManager ()->currentEditor ()->paste (QApplication::clipboard()->data()->encodedData ("text/tab-separated-values"));
	} else if (QApplication::clipboard()->data()->provides ("text/plain")) {
		qDebug ("paste plain");
		RKGlobals::editorManager ()->currentEditor ()->paste (QApplication::clipboard()->data()->encodedData ("text/plain"));
	}

	slotStatusMsg(i18n("Ready."));
}

void RKwardApp::slotEditPaste() {
	if ((QString) activeWindow()->name()!="dataeditor"){
		return;
	}
	
	RK_TRACE (APP);
	RKGlobals::editorManager ()->currentEditor ()->setPasteMode (RKEditor::PasteEverywhere);
 	doPaste ();
	

}

void RKwardApp::slotEditPasteToTable() {
	RK_TRACE (APP);
	if ((QString) activeWindow()->name()!="dataeditor"){
		return;
	}
		
	
	RKGlobals::editorManager ()->currentEditor ()->setPasteMode (RKEditor::PasteToTable);
	doPaste();
}
void RKwardApp::slotEditPasteToSelection() {
	RK_TRACE (APP);
	
	if ((QString) activeWindow()->name()!="dataeditor"){
		return;
	}

	RKGlobals::editorManager ()->currentEditor ()->setPasteMode (RKEditor::PasteToSelection);
	doPaste();
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

  slotStatusMsg(i18n("Ready."));
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

  slotStatusMsg(i18n("Ready."));
}


void RKwardApp::slotStatusMsg(const QString &text)
{
	RK_TRACE (APP);
  ///////////////////////////////////////////////////////////////////
  // change status message permanently
  statusBar()->clear();
  statusBar()->changeItem(text, ID_STATUS_MSG);
}

void RKwardApp::slotShowRKWatch () {
	RK_TRACE (APP);
	//RKGlobals::rInterface ()->watch->setShown (showRKWatch->isChecked ());
}

void RKwardApp::slotShowRKOutput () {
	RK_TRACE (APP);
	//output->setShown (showRKOutput->isChecked ());
}

void RKwardApp::slotShowRObjectBrowser () {
	RK_TRACE (APP);
	//object_browser->setShown (showRObjectBrowser->isChecked ());
}

void RKwardApp::slotToggleWindowClosed () {
	RK_TRACE (APP);
	/*showRKWatch->setChecked (RKGlobals::rInterface ()->watch->isShown ());
	showRKOutput->setChecked (output->isShown ());
	showRObjectBrowser->setChecked (object_browser->isShown ());*/
}

void RKwardApp::newOutput () {
	RK_TRACE (APP);
	refreshOutput (RKSettingsModuleOutput::autoShow (), RKSettingsModuleOutput::autoRaise ());
}

void RKwardApp::setRStatus (bool busy) {
	RK_TRACE (APP);
	if (busy) {
		statusBar()->changeItem(i18n("R engine busy"), ID_R_STATUS_MSG);
	} else {
		statusBar ()->changeItem (i18n ("R engine idle"), ID_R_STATUS_MSG);
	}
}



void RKwardApp::slotNewCommandEditor(){
	RKCommandEditorWindow *editor = new RKCommandEditorWindow;
	editor->setIcon(SmallIcon("source"));
	editor->name("RCEditor");
	addWindow(editor);
	
}


void RKwardApp::slotOpenURL(const KURL &url){
	RKCommandEditorWindow *editor;
	
	if (!url.isLocalFile())
	{
		KMessageBox::messageBox(this,KMessageBox::Information,
				i18n ("You specified a file that is not on local system"), i18n ("Cannot open file"));
		return;
	}
	
	editor = new RKCommandEditorWindow;
	
	if (!editor->openURL(url))
	{
		QString errstr = i18n ("Unable to open ");
		
		errstr += url.prettyURL();
		
		KMessageBox::messageBox(this,KMessageBox::Error,
				errstr, i18n ("Error!"));
		delete editor;
		return;
	}
	
	fileOpenRecent->addURL (url);
	editor->setIcon(SmallIcon("source"));
	editor->name("RCEditor");
	addWindow(editor);
	

};

void RKwardApp::slotOpenCommandEditor(){
	KURL::List urls;
	KURL::List::const_iterator it;
	
	KFileDialog dlg(QString("."),QString("*.R *.r"),this,i18n("Open file"),true);
	
	dlg.setOperationMode(KFileDialog::Opening);
	dlg.setMode(KFile::Files);
	if (dlg.exec() == QDialog::Rejected)
		return;
	
	urls = dlg.selectedURLs();
	for (it = urls.begin() ; it != urls.end() ; ++it){
		slotOpenURL(*it);
	}

};


void RKwardApp::slotSaveCommandEditor(){

	if (! activeWindow()->inherits("RKCommandEditorWindow"))
		return;


	RKCommandEditorWindow *editor;
	KURL url;
	QString strURL;
	
	editor = (RKCommandEditorWindow *)activeWindow();
	if (!editor)
		return;
	url = editor->url();
	strURL = url.url();
	
	if (strURL.length() == 0) // was a new document
		saveAsProcedure(editor);
	else
	{
		if (!editor->save())
			KMessageBox::messageBox(this,KMessageBox::Information,i18n("Could not save document"),i18n("Warning"));
	}
};
void RKwardApp::slotSaveCommandEditorAs(){
	if (! activeWindow()->inherits("RKCommandEditorWindow"))
		return;
		
	RKCommandEditorWindow *editor;
	
	editor = (RKCommandEditorWindow *)activeWindow();
	if (editor == 0)
		return;
	saveAsProcedure(editor);
};

void RKwardApp::slotCloseCommandEditor(){
	if (! activeWindow()->inherits("RKCommandEditorWindow"))
		return;
	
	activeWindow()->close();
};


void RKwardApp::saveAsProcedure(RKCommandEditorWindow *editor)
{
	KFileDialog dlg(QString("."),QString("*.R *.r"),this,i18n("Save file"),true);
	KURL url;
	QString fname;
		
	dlg.setOperationMode(KFileDialog::Saving);
	if (dlg.exec() == QDialog::Rejected)
		return;
	url = dlg.selectedURL();
	if (editor->saveAs(url))
	{
		if (getFilenameAndPath(url,&fname))
			editor->setTabCaption(fname);	
		else
			editor->setTabCaption(url.prettyURL());
	}
	else
		KMessageBox::messageBox(this,KMessageBox::Information,i18n("Could not save document"),i18n("Warning"));
}


bool RKwardApp::getFilenameAndPath(const KURL &url,QString *fname)
{
	QString fullpath = url.path();
	int i,length,fnamepos;
	bool done;
	
	if ((length = (int)fullpath.length()) == 0)
		return false;

	fnamepos = 0;
	for (i = length-1,done = false ; i >= 0 && !done ; i--)
	{
		if (fullpath[i] == '/')
		{
			done = true;
			fnamepos = i+1;
		}
	}
 
	if (!done)
		return false;
	
	if (fnamepos >= length)
		return false;

	if (fname)
		*fname = fullpath.right(length-fnamepos);

		
	return true;
}



void RKwardApp::slotChildWindowCloseRequest (KMdiChildView * window) {
	//If it's an unsaved command editor window, there is a warning.
	if (window->inherits("RKCommandEditorWindow")) {
		RKCommandEditorWindow * editor = (RKCommandEditorWindow*) window;
		if (editor->isModified()) {
			int status = KMessageBox::warningYesNo(this,i18n("The document \"%1\" has been modified. Close it anyway?").arg(editor->tabCaption()),i18n("File not saved"));
	
			if (status == KMessageBox::Yes) {
				closeWindow(window);
			}
		}
		else {
			closeWindow(window);
		}
	}
	else if (window->inherits("RKHelpWindow"))
	{
		closeWindow(window);
	}
}




void RKwardApp::slotRunSelection() {
	if (! activeWindow()->inherits("RKCommandEditorWindow"))
		return;

	if(((RKCommandEditorWindow*) activeWindow())->getSelection().isEmpty() || ((RKCommandEditorWindow*) activeWindow())->getSelection().isNull())
		return;

	RKGlobals::rInterface ()->issueCommand (new RCommand ( ((RKCommandEditorWindow*) activeWindow())->getSelection(), RCommand::User, ""));
}

void RKwardApp::slotRunLine() {
	if (! activeWindow()->inherits("RKCommandEditorWindow"))
		return;

	if(((RKCommandEditorWindow*) activeWindow())->getLine().isEmpty() || ((RKCommandEditorWindow*) activeWindow())->getLine().isNull())
		return;

	RKGlobals::rInterface ()->issueCommand (new RCommand ( ((RKCommandEditorWindow*) activeWindow())->getLine(), RCommand::User, ""));
}


void RKwardApp::slotRunAll() {
	if (! activeWindow()->inherits("RKCommandEditorWindow"))
		return;
	if(((RKCommandEditorWindow*) activeWindow())->getText().isEmpty() || ((RKCommandEditorWindow*) activeWindow())->getText().isNull())
		return;
		
	RKGlobals::rInterface ()->issueCommand (new RCommand ( ((RKCommandEditorWindow*) activeWindow())->getText(), RCommand::User, ""));
}


void RKwardApp::slotViewActivated (KMdiChildView * window)
{
	if ((QString) activeWindow()->name()=="dataeditor"){
		m_manager->setActivePart(0L);
	}
	setEnabledActions((QString) activeWindow()->name()=="dataeditor");
	setCaption(window->caption());
}



void RKwardApp::slotOpenRecentCommandEditor(const KURL& url)
{
	slotOpenURL(url);
}

void RKwardApp::slotInterruptCommand()
{

}



void RKwardApp::setEnabledActions(bool objectEditor)
{
	if (objectEditor) {
  		editCut->setEnabled(true);
  		editCopy->setEnabled(true);
  		editPaste->setEnabled(true);
    		editPasteToSelection->setEnabled(true);
    		editPasteToTable->setEnabled(true);
		fileSave->setEnabled(false);
		fileSaveAs->setEnabled(false);
		runAll->setEnabled(false);
		runSelection->setEnabled(false);
		runLine->setEnabled(false);
		interruptCommand->setEnabled(false);
	}
	else{
		editCut->setEnabled(false);
		editCopy->setEnabled(false);
		editPaste->setEnabled(false);
		editPasteToSelection->setEnabled(false);
		editPasteToTable->setEnabled(false);
		fileSave->setEnabled(true);
		fileSaveAs->setEnabled(true);
		runAll->setEnabled(true);
		runSelection->setEnabled(true);
		runLine->setEnabled(true);
		interruptCommand->setEnabled(true);
	}
}

void RKwardApp::openHTML(KURL url)
{
	RKHelpWindow *help = new RKHelpWindow(this,"help");
	help->openURL (url);	
	help->setIcon(SmallIcon("help"));
	addWindow( help );
}



void RKwardApp::slotFunctionReference()
{
	if (! activeWindow()->inherits("RKCommandEditorWindow"))
		return;
	((RKCommandEditorWindow*) activeWindow())->showHelp();
}



/*!
    \fn RKwardApp::slotOutputShow()
	Show html output.
 */
void RKwardApp::slotOutputShow () {
	refreshOutput (true, true);
}


/*!
    \fn RKwardApp::slotOutputFlush()
	Empties output.
 */
void RKwardApp::slotOutputFlush () {
	int res = KMessageBox::questionYesNo (this, i18n ("Do you really want to flush the ouput? It won't be possible to restore it."), i18n ("Flush output?"));
	if (res==KMessageBox::Yes) {
		QFile out_file (RKSettingsModuleLogfiles::filesPath () + "/rk_out.html");
		out_file.remove ();
		refreshOutput (false, false);
	}
}

void RKwardApp::refreshOutput (bool show, bool raise) {
	KMdiChildView* outp = outputView();
	if (outp){
		if (raise) {
			activateView (outp);
		}
		// do we need this if?
		if (outp->inherits ("RKHelpWindow")) static_cast<RKHelpWindow*> (outp)->refresh ();
	} else {
		if (show) {
			RKHelpWindow *out = new RKHelpWindow (this,"output",true);
			KURL url (RKSettingsModuleLogfiles::filesPath () + "/rk_out.html");
			out->openURL (url);	
			out->setIcon (SmallIcon ("text_block"));
			addWindow (out);
		}
	}
}

/*!
    \fn RKwardApp::outputView()
	Returns a pointer to the output MDI child view.
 */
KMdiChildView* RKwardApp::outputView()
{
	KMdiChildView* result = 0;
	KMdiIterator<KMdiChildView*>* it = createIterator();
	for ( it->first(); !it->isDone(); it->next() )
	{
		if ((QString)it->currentItem()->name() == "output" && it->currentItem() != 0){
			result = it->currentItem();
		}
	}
	deleteIterator(it);

	return(result);
}

