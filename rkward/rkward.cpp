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
#include "rsettings.h"
#include "pluginsettings.h"
#include "rkmenu.h"

#define ID_STATUS_MSG 1

RKwardApp::RKwardApp(QWidget* , const char* name):KMainWindow(0, name)
{
  config=kapp->config();

	KGlobal::dirs()->addResourceType("plugins", KStandardDirs::kde_default("data") + "rkward/plugins/");
	plugin_dir = KGlobal::dirs()->findResourceDir("plugins", "50.50.50.t.test.rkward");
	if (plugin_dir == "") {
		// try our luck with a relative path
		plugin_dir = "plugins/";
	}

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
	initPlugins ();
	startR ();
}

RKwardApp::~RKwardApp()
{
}

void RKwardApp::initPlugins () {
	slotStatusMsg(i18n("Setting up plugins..."));

	QDir dir = QDir (plugin_dir, "*.rkward", QDir::Name, QDir::Files | QDir::Readable);

	for (unsigned int i = 0; i < dir.count (); i++) {
		qDebug (dir[i]);
		initPlugin (plugin_dir + dir[i]);
	}

	if (!dir.count ()) {
		KMessageBox::information (0, i18n ("Could not find any plugins!\nRKWard is pretty useless without plugins, so you should use Settings->Plugins to import some.\n"),
			    i18n ("No plugins found"));
	}

	slotStatusMsg(i18n("Ready."));
}

void RKwardApp::initPlugin (const QString & filename) {
	int error_line, error_column;
	QString error_message, dummy;
	QDomDocument doc;
	QFile f(filename);
	if (!f.open(IO_ReadOnly))
		qDebug ("Could not open file for reading: " + filename);
	if (!doc.setContent(&f, false, &error_message, &error_line, &error_column)) {
		f.close();
		qDebug ("parsing-error in: " + filename);
		qDebug ("Message: " + error_message);
		qDebug ("Line: %d", error_line);
		qDebug ("Column: %d", error_column);
		return;
	}
	f.close();

	QDomElement element = doc.documentElement ();
	QDomNodeList children = element.elementsByTagName("location");
	element = children.item (0).toElement ();

	if (!rkmenus.contains (element.attribute ("tag"))) {
		RKMenu *sub = new RKMenu (menuBar (), element.attribute ("tag"), element.attribute ("label", "untitled"), this);
		rkmenus.insert (element.attribute ("tag"), sub);
		menuBar ()->insertItem (sub->label (), sub);
	}
	rkmenus[element.attribute ("tag")]->place (element, filename);
}

void RKwardApp::startR () {
	r_inter.shutdown ();

	QStrList r_line;
	r_line.append (path_to_r);
	if (opt_r_nosave) {
		r_line.append ("--no-save");
	}
	if (opt_r_slave) {
		r_line.append ("--slave");
	}
	r_inter.startR (r_line);
}

void RKwardApp::slotConfigureR () {
	RSettings *settings = new RSettings (this);
	settings->show ();
}

void RKwardApp::fetchRSettings (RSettings *from, bool apply) {
	if (apply) {
		opt_r_nosave = from->no_save->isChecked ();
		opt_r_slave = from->slave->isChecked ();
		path_to_r = from->path_to_r->text ();
		saveOptions ();
		startR ();
	}
	delete from;
}

void RKwardApp::slotConfigurePlugins () {
	PluginSettings *settings = new PluginSettings (this);
	settings->show ();
}

void RKwardApp::fetchPluginSettings (PluginSettings *from, bool apply) {
	if (apply) {
		plugin_dir = from->pathEdit->text ();
		saveOptions ();
		initPlugins ();
	}
	delete from;
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
	showRKWatch = new KAction (i18n ("Toggle RKWatch-Window"), 0, 0, this, SLOT(slotShowRKWatch ()), actionCollection(), "settings_rkwatch");
	configureR = new KAction (i18n ("R Settings"), 0, 0, this, SLOT(slotConfigureR ()), actionCollection(), "settings_r");
	configurePlugins = new KAction (i18n ("Plugin Settings"), 0, 0, this, SLOT(slotConfigurePlugins ()), actionCollection(), "settings_plugins");

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

	config->setGroup ("R-Settings");
	config->writeEntry ("Option --no-save", opt_r_nosave);
	config->writeEntry ("Option --slave", opt_r_slave);
	config->writeEntry ("Path to R", path_to_r);

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
	
	config->setGroup ("R-Settings");
	opt_r_nosave = config->readBoolEntry ("Option --no-save", true);
	opt_r_slave = config->readBoolEntry ("Option --slave", true);
	path_to_r = config->readEntry ("Path to R", "/usr/bin/R");

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
	if (r_inter.watch->isVisible ()) {
		r_inter.watch->hide ();
	} else {
		r_inter.watch->show ();
	}
}