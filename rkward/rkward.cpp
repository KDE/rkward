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

// application specific includes
#include "rkward.h"
#include "rkwarddoc.h"
#include "rkdrag.h"
#include "rkwatch.h"
#include "rkmenu.h"
#include "rkplugin.h"
#include "rkoutputwindow.h"
#include "rksettings.h"
#include "rksettingsmoduleplugins.h"
#include "rksettingsmodulelogfiles.h"
#include "rksettingsmoduleoutput.h"
#include "rinterface.h"

#define ID_STATUS_MSG 1
#define ID_R_STATUS_MSG 2

RKwardApp::RKwardApp(QWidget* , const char* name):KMainWindow(0, name)
{
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

  initDocument();
  initView();
  readOptions();

	startup_timer = new QTimer (this);
	startup_timer->start (50);
	connect (startup_timer, SIGNAL (timeout ()), this, SLOT (doPostInit ()));
	
	output = new RKOutputWindow (0);
	output->showMaximized ();
	output->hide ();
	
	r_inter = 0;
}

RKwardApp::~RKwardApp() {
	delete r_inter;
}

void RKwardApp::doPostInit () {
	delete startup_timer;
	
    QString dummy = "Before you start bashing at it: Please note that is is merely ";
	dummy.append ("a very early proof-of-concept release. It does not do much good. It might do some ");
	dummy.append ("very bad things. It's really only targeted at people who might be interested in helping with the development.\n");
	dummy.append ("So before you start finding all the bugs/missing features/design flaws, enter some ");
	dummy.append ("data in the table, save the table, if you like, select Analyse->Means->T-Tests->Independent ");
	dummy.append ("samples T-Test. Watch what happens in the RKWatch-window.\n");
	dummy.append ("Note that the T-Test-dialog and all the entries in the Analyse-menu were created ");
	dummy.append ("at run-time and on the fly from a simple XML-file located in the plugin-directory.");
	KMessageBox::information (this, dummy, "Before you complain...", "state_of_rkward");

	startR ();
	initPlugins ();
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
		qDebug ("Could not open file for reading: " + description.name ());
		description.close ();
		return 0;
	}
	if (!doc.setContent(&description, false, &error_message, &error_line, &error_column)) {
		description.close();
		qDebug ("parsing-error in: " + description.name ());
		qDebug ("Message: " + error_message);
		qDebug ("Line: %d", error_line);
		qDebug ("Column: %d", error_column);
		return 0;
	}
	description.close();

	QDomElement element = doc.documentElement ();
	QDomNodeList children = element.elementsByTagName("entry");
	element = children.item (0).toElement ();
	
	RKMenu *menu = 0;
	if (element.attribute ("type") == "menu") {
		if (!parent) {
			menu = new RKMenu (menuBar (), element.attribute ("id"), element.attribute ("label", "untitled"), this);
			rkmenus.insert (element.attribute ("id"), menu);
			menuBar ()->insertItem (menu->label (), menu);
		} else {
			menu = new RKMenu (parent, element.attribute ("id"), element.attribute ("label", "untitled"), this);
			parent->addSubMenu (element.attribute ("id"), menu);
		}
	} else {
		if (!parent) {
			qDebug ("can't add plugin on the top-level menu");
		} else {
			parent->addEntry (element.attribute ("id"), new RKPlugin (this, element.attribute ("label", "untitled"), description.name ()));
			num_plugins++;
		}
	}

	if (menu) {
		// get subdirectories if applicable
		for (unsigned int i = 0; i < dir.count (); i++) {
			qDebug (dir[i]);
			if ((dir[i] != ".") && (dir[i] != "..")) {
				num_plugins += initPluginDir (dir.filePath (dir[i]), menu);
			}
		}
	}
	
	return num_plugins;
}

void RKwardApp::startR () {
	if (r_inter) qDebug ("Trying to start R-Interface a second time. Not good.");
	
	QDir dir (RKSettingsModuleLogfiles::filesPath());
	if (!dir.exists ()) {
		QDir current (dir.currentDirPath ());
		current.mkdir (dir.path (), true);
	}
	
	r_inter = new RInterface (this);
}

void RKwardApp::slotConfigure () {
	RKSettings *settings = new RKSettings (this);
	settings->show ();
}

void RKwardApp::initActions()
{
  fileNewWindow = new KAction(i18n("New &Window"), 0, 0, this, SLOT(slotFileNewWindow()), actionCollection(),"file_new_window");
	fileNewWindow->setEnabled (false);
  fileNew = KStdAction::openNew(this, SLOT(slotFileNew()), actionCollection());
	fileNew->setEnabled (false);
  fileOpen = KStdAction::open(this, SLOT(slotFileOpen()), actionCollection());
  fileOpenRecent = KStdAction::openRecent(this, SLOT(slotFileOpenRecent(const KURL&)), actionCollection());
  fileSave = KStdAction::save(this, SLOT(slotFileSave()), actionCollection());
  fileSaveAs = KStdAction::saveAs(this, SLOT(slotFileSaveAs()), actionCollection());
  fileClose = KStdAction::close(this, SLOT(slotFileClose()), actionCollection());
	fileClose->setEnabled (false);
  filePrint = KStdAction::print(this, SLOT(slotFilePrint()), actionCollection());
	filePrint->setEnabled (false);
  fileQuit = KStdAction::quit(this, SLOT(slotFileQuit()), actionCollection());
  editCut = KStdAction::cut(this, SLOT(slotEditCut()), actionCollection());
  editCopy = KStdAction::copy(this, SLOT(slotEditCopy()), actionCollection());
  editPaste = KStdAction::paste(this, SLOT(slotEditPaste()), actionCollection());
  editPasteToTable = new KAction(i18n("Paste inside Table"), 0, 0, this, SLOT(slotEditPasteToTable()), actionCollection(), "paste_to_table");
  editPasteToSelection = new KAction(i18n("Paste inside Selection"), 0, 0, this, SLOT(slotEditPasteToSelection()), actionCollection(), "paste_to_selection");
  viewToolBar = KStdAction::showToolbar(this, SLOT(slotViewToolBar()), actionCollection());
  viewStatusBar = KStdAction::showStatusbar(this, SLOT(slotViewStatusBar()), actionCollection());
	showRKWatch = new KToggleAction (i18n ("Show RKWatch-Window"), 0, 0, this, SLOT(slotShowRKWatch ()), actionCollection(), "windows_rkwatch");
	showRKOutput = new KToggleAction (i18n ("Show RKOutput-Window"), 0, 0, this, SLOT(slotShowRKOutput ()), actionCollection(), "windows_rkoutput");
	configure = new KAction (i18n ("Configure Settings"), 0, 0, this, SLOT(slotConfigure ()), actionCollection(), "configure");

  fileNewWindow->setStatusText(i18n("Opens a new application window"));
  fileNew->setStatusText(i18n("Creates a new document"));
  fileOpen->setStatusText(i18n("Opens an existing document"));
  fileOpenRecent->setStatusText(i18n("Opens a recently used file"));
  fileSave->setStatusText(i18n("Saves the actual document"));
  fileSaveAs->setStatusText(i18n("Saves the actual document as..."));
  fileClose->setStatusText(i18n("Closes the actual document"));
  filePrint ->setStatusText(i18n("Prints out the actual document"));
  fileQuit->setStatusText(i18n("Quits the application"));
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
	statusBar()->insertItem(i18n("R-process busy"), ID_R_STATUS_MSG);
}

void RKwardApp::initDocument()
{
	doc = new RKwardDoc(this);
	doc->newDocument();
	setCentralWidget (doc);
/*	editCopy->setEnabled(true);
	editPaste->setEnabled(true);
	editCut->setEnabled(true);
	fileSave->setEnabled(true);
	fileSaveAs->setEnabled(true); */
}

void RKwardApp::initView()
{ 
  ////////////////////////////////////////////////////////////////////
  // create the main widget here that is managed by KTMainWindow's view-region and
  // connect the widget to your document to display document contents.

/*  view = new RKwardView(this);
  doc->setView(view);
  setCentralWidget(view);	*/
  setCaption(doc->URL().fileName(),false);

}

void RKwardApp::openDocumentFile(const KURL& url)
{
  slotStatusMsg(i18n("Opening file..."));

//  doc->openDocument( url);
  fileOpenRecent->addURL( url );
  slotStatusMsg(i18n("Ready."));
}


RKwardDoc *RKwardApp::getDocument() const
{
  return doc;
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


void RKwardApp::readOptions()
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
	
	RKSettings::loadSettings (config);

  // initialize the recent file list
  fileOpenRecent->loadEntries(config,"Recent Files");

  QSize size=config->readSizeEntry("Geometry");
  if(!size.isEmpty())
  {
    resize(size);
  }
}

void RKwardApp::saveProperties(KConfig *_cfg)
{
  if(doc->URL().fileName()!=i18n("Untitled") && !doc->isModified())
  {
    // saving to tempfile not necessary

  }
  else
  {
    KURL url=doc->URL();	
    _cfg->writeEntry("filename", url.url());
    _cfg->writeEntry("modified", doc->isModified());
    QString tempname = kapp->tempSaveName(url.url());
    QString tempurl= KURL::encode_string(tempname);
    KURL _url(tempurl);
    doc->saveDocument(_url);
  }
}


void RKwardApp::readProperties(KConfig* _cfg)
{
  QString filename = _cfg->readEntry("filename", "");
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
  }
}		

bool RKwardApp::queryClose()
{
  return doc->saveModified();
}

bool RKwardApp::queryExit()
{
  saveOptions();
  return true;
}

/////////////////////////////////////////////////////////////////////
// SLOT IMPLEMENTATION
/////////////////////////////////////////////////////////////////////

void RKwardApp::slotFileNewWindow()
{
  slotStatusMsg(i18n("Opening a new application window..."));
	
  RKwardApp *new_window= new RKwardApp();
  new_window->show();

  slotStatusMsg(i18n("Ready."));
}

void RKwardApp::slotFileNew()
{
  slotStatusMsg(i18n("Creating new document..."));

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
}

void RKwardApp::slotFileOpen()
{
  slotStatusMsg(i18n("Opening file..."));
	
  if(!doc->saveModified())
  {
     // here saving wasn't successful

  }
  else
  {	
    KURL url=KFileDialog::getOpenURL(QString::null,
        i18n("*|All files"), this, i18n("Open File..."));
    if(!url.isEmpty())
    {
      doc->openDocument(url);
      setCaption(url.fileName(), false);
      fileOpenRecent->addURL( url );
    }
  }
  slotStatusMsg(i18n("Ready."));
}

void RKwardApp::slotFileOpenRecent(const KURL& url)
{
  slotStatusMsg(i18n("Opening file..."));
	
  if(!doc->saveModified())
  {
     // here saving wasn't successful
  }
  else
  {
    doc->openDocument(url);
    setCaption(url.fileName(), false);
  }

  slotStatusMsg(i18n("Ready."));
}

void RKwardApp::slotFileSave()
{
	if (doc->URL ().fileName() == i18n("Untitled")) {
		slotFileSaveAs ();
		return;
	}

  slotStatusMsg(i18n("Saving file..."));
	
  doc->saveDocument(doc->URL());

  slotStatusMsg(i18n("Ready."));
}

void RKwardApp::slotFileSaveAs()
{
  slotStatusMsg(i18n("Saving file with a new filename..."));

  KURL url=KFileDialog::getSaveURL(QDir::currentDirPath(),
        i18n("*|All files"), this, i18n("Save as..."));
  if(!url.isEmpty())
  {
    doc->saveDocument(url);
    fileOpenRecent->addURL(url);
    setCaption(url.fileName(),doc->isModified());
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

void RKwardApp::slotEditCut()
{
	slotStatusMsg(i18n("Cutting selection..."));
	slotEditCopy ();
	doc->clearSelected();
	slotStatusMsg(i18n("Ready."));
}

void RKwardApp::slotEditCopy() {

	slotStatusMsg(i18n("Copying selection to clipboard..."));

	QApplication::clipboard()->setData(new RKDrag(doc));

	slotStatusMsg(i18n("Ready."));
}

void RKwardApp::doPaste () {
	slotStatusMsg(i18n("Inserting clipboard contents..."));

	// actually, we don't care, whether tsv or plain gets pasted - it's both
	// treated the same. We should however encourage external senders to
	// provided the two in order.
	if (QApplication::clipboard()->data()->provides ("text/tab-separated-values")) {
		qDebug ("paste tsv");
		doc->pasteEncoded (QApplication::clipboard()->data()->encodedData ("text/tab-separated-values"));
	} else if (QApplication::clipboard()->data()->provides ("text/plain")) {
		qDebug ("paste plain");
		doc->pasteEncoded (QApplication::clipboard()->data()->encodedData ("text/plain"));
	}

	slotStatusMsg(i18n("Ready."));
}

void RKwardApp::slotEditPaste() {
	doc->setPasteMode (TwinTable::PasteEverywhere);
 	doPaste ();
}

void RKwardApp::slotEditPasteToTable() {
	doc->setPasteMode (TwinTable::PasteToTable);
	doPaste();
}
void RKwardApp::slotEditPasteToSelection() {
	doc->setPasteMode (TwinTable::PasteToSelection);
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
	r_inter->watch->setShown (showRKWatch->isChecked ());
}

void RKwardApp::slotShowRKOutput () {
	output->setShown (showRKOutput->isChecked ());
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
