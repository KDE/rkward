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

// application specific includes
#include "rkward.h"
#include "rkeditormanager.h"
#include "dataeditor/rkeditor.h"
#include "dataeditor/rkdrag.h"
#include "rkwatch.h"
#include "misc/rkmenu.h"
#include "plugin/rkpluginhandle.h"
#include "rkoutputwindow.h"
#include "settings/rksettings.h"
#include "settings/rksettingsmoduleplugins.h"
#include "settings/rksettingsmodulelogfiles.h"
#include "settings/rksettingsmoduleoutput.h"
#include "rbackend/rinterface.h"
#include "core/robjectlist.h"
#include "rkglobals.h"
#include "robjectbrowser.h"
#include "dialogs/startupdialog.h"

#include "debug.h"

#define ID_STATUS_MSG 1
#define ID_R_STATUS_MSG 2

RKwardApp::RKwardApp(KURL *load_url, QWidget* , const char* name):KMainWindow(0, name)
{
	RKGlobals::app = this;
	RKGlobals::rinter = 0;
	RKGlobals::list = 0;
	
  config=kapp->config();

	KGlobal::dirs()->addResourceType("plugins", KStandardDirs::kde_default("data") + "rkward/plugins/");

  ///////////////////////////////////////////////////////////////////
  // call inits to invoke all other construction parts
  initStatusBar();
  initActions();

  ///////////////////////////////////////////////////////////////////
  // disable actions at startup   // why?
/*  fileSave->setEnabled(false);
  fileSaveAs->setEnabled(false);
  filePrint->setEnabled(false);
  editCut->setEnabled(false);
  editCopy->setEnabled(false);
  editPaste->setEnabled(false); */

  
	RKGlobals::manager = new RKEditorManager (this);
	setCentralWidget (RKGlobals::editorManager ());

	initial_url = load_url;
  
	startup_timer = new QTimer (this);
	startup_timer->start (50);
	connect (startup_timer, SIGNAL (timeout ()), this, SLOT (doPostInit ()));
}

RKwardApp::~RKwardApp() {
	delete RKGlobals::rInterface ();
	delete RKGlobals::rObjectList ();
	delete object_browser;
}

void RKwardApp::doPostInit () {
	delete startup_timer;
	
	show ();
	readOptions();
	
	object_browser = new RObjectBrowser ();
	object_browser->show ();

	output = new RKOutputWindow (0);
	output->showMaximized ();
	output->hide ();

    QString dummy = "Before you start bashing at it: Please note that this is merely a technology preview release. You might acutally be able to use it for some very simple tasks, but chances are it's of hardly any practical value so far. It does not do much good. It might do some very bad things (don't let it touch valuable data!). It's lacking in many respects. If you would like to help improve it, or simply get in contact, visit:\nhttp://rkward.sourceforge.net\nAll comments welcome.";
	KMessageBox::information (this, dummy, "Before you complain...", "state_of_rkward");

	startR ();
	initPlugins ();
	// just to initialize the window-actions accordingly
	slotToggleWindowClosed ();
	
	if (initial_url) {
		openWorkspace (*initial_url);
		delete initial_url;
	} else {
		setCaption(i18n ("Untitled"));
		
		StartupDialog::StartupDialogResult *result = StartupDialog::getStartupAction (this, fileOpenRecent);
		if (result->result == StartupDialog::EmptyWorkspace) {
		} else if (result->result == StartupDialog::OpenFile) {
			openWorkspace (result->open_url);
		} else if (result->result == StartupDialog::ChoseFile) {
			slotFileOpen ();
		} else if (result->result == StartupDialog::EmptyTable) {
			RObject *object = RKGlobals::rObjectList ()->createNewChild ("my.data", true, true);
			RKEditor *editor = RKGlobals::editorManager ()->editObject (object);
			editor->syncToR (0);
		}
		delete result;
	}
}

void RKwardApp::initPlugins () {
	slotStatusMsg(i18n("Setting up plugins..."));

	if (!initPluginDir (RKSettingsModulePlugins::pluginDir (), 0)) {
		KMessageBox::information (0, i18n ("Could not find any plugins!\nRKWard is pretty useless without plugins, so you should use Settings->Plugins to import some.\n"), i18n ("No plugins found"));
	}
	
	slotStatusMsg(i18n("Ready."));
}

int RKwardApp::initPluginDir (const QString & dirname, RKMenu *parent) {
	int num_plugins = 0;
	
	// list directories only
	QDir dir = QDir (dirname, QString::null, QDir::Name, QDir::Dirs);
	
	// first get and parse the description for the current directory
	int error_line, error_column;
	QString error_message, dummy;
	QDomDocument doc;

	QFile description (dir.filePath ("description.xml"));
	if (!description.open (IO_ReadOnly)) {
		RK_DO (qDebug ("Could not open file for reading: %s", description.name ().latin1 ()), APP | PLUGIN, DL_WARNING);
		description.close ();
		return 0;
	}
	if (!doc.setContent(&description, false, &error_message, &error_line, &error_column)) {
		description.close();
		RK_DO (qDebug ("parsing-error in: %s\nMessage: %s, Line: %d, Column: %d", description.name ().latin1 (), error_message.latin1 (), error_line, error_column), APP | PLUGIN, DL_WARNING);
		return 0;
	}
	description.close();

	QDomElement element = doc.documentElement ();
	QDomNodeList children = element.elementsByTagName("entry");
	element = children.item (0).toElement ();
	
	RKMenu *menu = 0;
	if (element.attribute ("type") == "menu") {
		if (!parent) {
			menu = new RKMenu (menuBar (), element.attribute ("id"), element.attribute ("label", "untitled"));
			rkmenus.insert (element.attribute ("id"), menu);
			menuBar ()->insertItem (menu->label (), menu);
		} else {
			menu = new RKMenu (parent, element.attribute ("id"), element.attribute ("label", "untitled"));
			parent->addSubMenu (element.attribute ("id"), menu);
		}
	} else {
		if (!parent) {
			RK_DO (qDebug ("%s", "can't add plugin on the top-level menu"), APP | PLUGIN, DL_WARNING);
		} else {
			parent->addEntry (element.attribute ("id"), new RKPluginHandle (this, description.name ()), element.attribute ("label", "untitled"));
			num_plugins++;
		}
	}

	if (menu) {
		// get subdirectories if applicable
		for (unsigned int i = 0; i < dir.count (); i++) {
			RK_DO (qDebug ("%s", dir[i].latin1 ()), APP | PLUGIN, DL_DEBUG);
			if ((dir[i] != ".") && (dir[i] != "..")) {
				num_plugins += initPluginDir (dir.filePath (dir[i]), menu);
			}
		}
	}
	
	return num_plugins;
}

void RKwardApp::startR () {
	RK_ASSERT (!RKGlobals::rInterface ());
	
	QDir dir (RKSettingsModuleLogfiles::filesPath());
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
  fileOpen = KStdAction::open(this, SLOT(slotFileOpen()), actionCollection());
  fileOpenRecent = KStdAction::openRecent(this, SLOT(slotFileOpenRecent(const KURL&)), actionCollection());
  fileSave = KStdAction::save(this, SLOT(slotFileSave()), actionCollection());
  fileSaveAs = KStdAction::saveAs(this, SLOT(slotFileSaveAs()), actionCollection());
  fileClose = KStdAction::close(this, SLOT(slotFileClose()), actionCollection());
	fileClose->setEnabled (false);
  filePrint = KStdAction::print(this, SLOT(slotFilePrint()), actionCollection());
	filePrint->setEnabled (false);
  fileQuit = KStdAction::quit(this, SLOT(slotFileQuit()), actionCollection());
	dataClose = new KAction (i18n ("Close current editor"), 0, 0, this, SLOT (slotDataClose ()), actionCollection (), "data_close_editor");
	dataNewDataFrame = new KAction (i18n ("data.frame"), 0, 0, this, SLOT (slotDataNewDataFrame ()), actionCollection (), "data_new_data_frame");
  editCut = KStdAction::cut(this, SLOT(slotEditCut()), actionCollection());
  editCopy = KStdAction::copy(this, SLOT(slotEditCopy()), actionCollection());
  editPaste = KStdAction::paste(this, SLOT(slotEditPaste()), actionCollection());
  editPasteToTable = new KAction(i18n("Paste inside Table"), 0, 0, this, SLOT(slotEditPasteToTable()), actionCollection(), "paste_to_table");
  editPasteToSelection = new KAction(i18n("Paste inside Selection"), 0, 0, this, SLOT(slotEditPasteToSelection()), actionCollection(), "paste_to_selection");
  viewToolBar = KStdAction::showToolbar(this, SLOT(slotViewToolBar()), actionCollection());
  viewStatusBar = KStdAction::showStatusbar(this, SLOT(slotViewStatusBar()), actionCollection());
	showRKWatch = new KToggleAction (i18n ("Show RKWatch-Window"), 0, 0, this, SLOT(slotShowRKWatch ()), actionCollection(), "windows_rkwatch");
	showRKOutput = new KToggleAction (i18n ("Show RKOutput-Window"), 0, 0, this, SLOT(slotShowRKOutput ()), actionCollection(), "windows_rkoutput");
	showRObjectBrowser = new KToggleAction (i18n ("Show RObjectBrowser-Window"), 0, 0, this, SLOT(slotShowRObjectBrowser ()), actionCollection(), "windows_robjectbrowser");
	configure = new KAction (i18n ("Configure Settings"), 0, 0, this, SLOT(slotConfigure ()), actionCollection(), "configure");

  fileOpen->setStatusText(i18n("Opens an existing document"));
  fileOpenRecent->setStatusText(i18n("Opens a recently used file"));
  fileSave->setStatusText(i18n("Saves the actual document"));
  fileSaveAs->setStatusText(i18n("Saves the actual document as..."));
  fileClose->setStatusText(i18n("Closes the actual document"));
  filePrint ->setStatusText(i18n("Prints out the actual document"));
  fileQuit->setStatusText(i18n("Quits the application"));
	dataClose->setStatusText (i18n ("Closes the currently active editor window"));
	dataNewDataFrame->setStatusText (i18n ("Creates a new empty data.frame and opens it for editing"));
  editCut->setStatusText(i18n("Cuts the selected section and puts it to the clipboard"));
  editCopy->setStatusText(i18n("Copies the selected section to the clipboard"));
  editPaste->setStatusText(i18n("Pastes the clipboard contents to actual position"));
  editPasteToTable->setStatusText(i18n("Pastes the clipboard contents to actual position, but not beyond the table's boundaries"));
  editPasteToSelection->setStatusText(i18n("Pastes the clipboard contents to actual position, but not beyond the boundaries of the current selection"));
  viewToolBar->setStatusText(i18n("Enables/disables the toolbar"));
  viewStatusBar->setStatusText(i18n("Enables/disables the statusbar"));

  // use the absolute path to your rkwardui.rc file for testing purpose in createGUI();
  createGUI("rkwardui.rc");

}


void RKwardApp::initStatusBar()
{
  ///////////////////////////////////////////////////////////////////
  // STATUSBAR
  // TODO: add your own items you need for displaying current application status.
  statusBar()->insertItem(i18n("Ready."), ID_STATUS_MSG);
	statusBar()->insertItem(i18n("starting R-processor"), ID_R_STATUS_MSG);
}

void RKwardApp::openWorkspace (const KURL &url) {
	RKGlobals::rObjectList ()->loadWorkspace(url);
	setCaption(url.fileName(), false);
	fileOpenRecent->addURL (url);
}


void RKwardApp::saveOptions()
{	
  config->setGroup("General Options");
  config->writeEntry("Geometry", size());
  config->writeEntry("Show Toolbar", viewToolBar->isChecked());
  config->writeEntry("Show Statusbar",viewStatusBar->isChecked());
  config->writeEntry("ToolBarPos", (int) toolBar("mainToolBar")->barPos());

	RKSettings::saveSettings (config);
	
  fileOpenRecent->saveEntries(config,"Recent Files");
}


void RKwardApp::readOptions ()
{
	
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

  QSize size=config->readSizeEntry("Geometry");
  if(!size.isEmpty ())
  {
    resize (size);
  }
  
  // initialize the recent file list
  fileOpenRecent->loadEntries(config,"Recent Files");

	// do this last, since we may be setting some different config-group(s) in the process
	RKSettings::loadSettings (config);  
}

void RKwardApp::saveProperties(KConfig *_cfg)
{
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

bool RKwardApp::queryClose()
{
// TODO: wait until document is saved?!
	//RKGlobals::rObjectList ()->saveWorkspace ();
	return true;
}

bool RKwardApp::queryExit()
{
  saveOptions();
  return true;
}

/////////////////////////////////////////////////////////////////////
// SLOT IMPLEMENTATION
/////////////////////////////////////////////////////////////////////

/*void RKwardApp::slotFileNew()
{
  slotStatusMsg(i18n("Creating new workspace..."));

  if(!doc->saveModified())
  {
     // here saving wasn't successful

  }
  else
  {	
    doc->newDocument();		
    setCaption(doc->URL().fileName(), false);
  }

  slotStatusMsg(i18n("Ready."));
}*/

void RKwardApp::slotFileOpen()
{
  slotStatusMsg(i18n("Opening workspace..."));
	
  //if(!doc->saveModified())
  if (false)
  {
     // here saving wasn't successful

  }
  else
  {	
    KURL url=KFileDialog::getOpenURL(QString::null,
        i18n("*|All files"), this, i18n("Open File..."));
    if(!url.isEmpty())
    {
		openWorkspace (url);
    }
  }
  slotStatusMsg(i18n("Ready."));
}

void RKwardApp::slotFileOpenRecent(const KURL& url)
{
  slotStatusMsg(i18n("Opening file..."));
	
//  if(!doc->saveModified())
  if (false)
  {
     // here saving wasn't successful
  }
  else
  {
		openWorkspace (url);
  }

  slotStatusMsg(i18n("Ready."));
}

void RKwardApp::slotFileSave()
{
	if (RKGlobals::rObjectList ()->getWorkspaceURL ().isEmpty ()) {
		slotFileSaveAs ();
		return;
	}

	RKGlobals::rObjectList ()->saveWorkspace (RKGlobals::rObjectList ()->getWorkspaceURL ());
}

void RKwardApp::slotFileSaveAs()
{
  slotStatusMsg(i18n("Saving workspace with a new filename..."));

  KURL url=KFileDialog::getSaveURL(QDir::currentDirPath(),
        i18n("*|All files"), this, i18n("Save as..."));
  if(!url.isEmpty())
  {
    RKGlobals::rObjectList ()->saveWorkspace (url);
    fileOpenRecent->addURL(url);
    setCaption(url.fileName(), false);
  }

  slotStatusMsg(i18n("Ready."));
}

void RKwardApp::slotFileClose()
{
  slotStatusMsg(i18n("Closing file..."));
	
  close();

  slotStatusMsg(i18n("Ready."));
}

void RKwardApp::slotFilePrint()
{
  slotStatusMsg(i18n("Printing..."));

  QPrinter printer;
  if (printer.setup(this))
  {
//    view->print(&printer);
  }

  slotStatusMsg(i18n("Ready."));
}

void RKwardApp::slotFileQuit()
{
  slotStatusMsg(i18n("Exiting..."));
  saveOptions();
  // close the first window, the list makes the next one the first again.
  // This ensures that queryClose() is called on each window to ask for closing
  KMainWindow* w;
  if(memberList)
  {
    for(w=memberList->first(); w!=0; w=memberList->first())
    {
      // only close the window if the closeEvent is accepted. If the user presses Cancel on the saveModified() dialog,
      // the window and the application stay open.
      if(!w->close())
	break;
    }
  }	
}

void RKwardApp::slotDataClose () {
	RKGlobals::editorManager ()->closeEditor (RKGlobals::editorManager ()->currentEditor ());
}

void RKwardApp::slotDataNewDataFrame () {
	bool ok;

	QString name = KInputDialog::getText (i18n ("Create new data.frame"), i18n ("Enter name for the new object (make it a valid one - no checks so far)"), "my.data", &ok, this);
	
	if (ok) {
		RObject *object = RKGlobals::rObjectList ()->createNewChild (name, true, true);
		RKEditor *editor = RKGlobals::editorManager ()->editObject (object);
		editor->syncToR (0);
	}
}

void RKwardApp::slotEditCut()
{
	slotStatusMsg(i18n("Cutting selection..."));
	slotEditCopy ();
	RKGlobals::editorManager ()->currentEditor ()->clearSelected ();
	slotStatusMsg(i18n("Ready."));
}

void RKwardApp::slotEditCopy() {

	slotStatusMsg(i18n("Copying selection to clipboard..."));

	QApplication::clipboard()->setData(RKGlobals::editorManager ()->currentEditor ()->makeDrag ());

	slotStatusMsg(i18n("Ready."));
}

void RKwardApp::doPaste () {
	slotStatusMsg(i18n("Inserting clipboard contents..."));

	// actually, we don't care, whether tsv or plain gets pasted - it's both
	// treated the same. We should however encourage external senders to
	// provided the two in order.
	if (QApplication::clipboard()->data()->provides ("text/tab-separated-values")) {
		qDebug ("paste tsv");
		RKGlobals::editorManager ()->currentEditor ()->pasteEncoded (QApplication::clipboard()->data()->encodedData ("text/tab-separated-values"));
	} else if (QApplication::clipboard()->data()->provides ("text/plain")) {
		qDebug ("paste plain");
		RKGlobals::editorManager ()->currentEditor ()->pasteEncoded (QApplication::clipboard()->data()->encodedData ("text/plain"));
	}

	slotStatusMsg(i18n("Ready."));
}

void RKwardApp::slotEditPaste() {
	RKGlobals::editorManager ()->currentEditor ()->setPasteMode (RKEditor::PasteEverywhere);
 	doPaste ();
}

void RKwardApp::slotEditPasteToTable() {
	RKGlobals::editorManager ()->currentEditor ()->setPasteMode (RKEditor::PasteToTable);
	doPaste();
}
void RKwardApp::slotEditPasteToSelection() {
	RKGlobals::editorManager ()->currentEditor ()->setPasteMode (RKEditor::PasteToSelection);
	doPaste();
}

void RKwardApp::slotViewToolBar()
{
  slotStatusMsg(i18n("Toggling toolbar..."));
  ///////////////////////////////////////////////////////////////////
  // turn Toolbar on or off
  if(!viewToolBar->isChecked())
  {
    toolBar("mainToolBar")->hide();
  }
  else
  {
    toolBar("mainToolBar")->show();
  }		

  slotStatusMsg(i18n("Ready."));
}

void RKwardApp::slotViewStatusBar()
{
  slotStatusMsg(i18n("Toggle the statusbar..."));
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
  ///////////////////////////////////////////////////////////////////
  // change status message permanently
  statusBar()->clear();
  statusBar()->changeItem(text, ID_STATUS_MSG);
}

void RKwardApp::slotShowRKWatch () {
	RKGlobals::rInterface ()->watch->setShown (showRKWatch->isChecked ());
}

void RKwardApp::slotShowRKOutput () {
	output->setShown (showRKOutput->isChecked ());
}

void RKwardApp::slotShowRObjectBrowser () {
	object_browser->setShown (showRObjectBrowser->isChecked ());
}

void RKwardApp::slotToggleWindowClosed () {
	showRKWatch->setChecked (RKGlobals::rInterface ()->watch->isShown ());
	showRKOutput->setChecked (output->isShown ());
	showRObjectBrowser->setChecked (object_browser->isShown ());
}

void RKwardApp::newOutput () {
	output->checkNewInput ();
	if (RKSettingsModuleOutput::autoShow ()) {
		output->show ();
		showRKOutput->setChecked (true);
		if (RKSettingsModuleOutput::autoRaise ()) {
			output->raise ();
		}
	}
}

void RKwardApp::setRStatus (bool busy) {
	if (busy) {
		statusBar()->changeItem(i18n("R-process busy"), ID_R_STATUS_MSG);
	} else {
		statusBar ()->changeItem (i18n ("R-process idle"), ID_R_STATUS_MSG);
	}
}
