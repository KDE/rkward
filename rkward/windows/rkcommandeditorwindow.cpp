/***************************************************************************
                          rkcommandeditorwindow  -  description
                             -------------------
    begin                : Mon Aug 30 2004
    copyright            : (C) 2004 by Thomas Friedrichsmeier
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
#include "rkcommandeditorwindow.h"

#include <kate/document.h>
#include <kate/view.h>

#include <ktexteditor/configinterface.h>
#include <ktexteditor/sessionconfiginterface.h>
#include <ktexteditor/viewcursorinterface.h>
#include <ktexteditor/printinterface.h>
#include <ktexteditor/encodinginterface.h>
#include <ktexteditor/editorchooser.h>
#include <ktexteditor/popupmenuinterface.h>


#include <qlayout.h>
#include <qpopupmenu.h>
#include <qapplication.h>

#include <klocale.h>
#include <kmenubar.h>
#include <kmessagebox.h>
#include <kfiledialog.h>
#include <kaction.h>
#include <kstdaction.h>
#include <kaccel.h>
#include <klibloader.h>


#include "../rbackend/rinterface.h"
#include "../rkeditormanager.h"
#include "../rkglobals.h"

#include "../debug.h"


RKCommandEditorWindow::RKCommandEditorWindow (QWidget *parent) : KParts::MainWindow (parent) {
	RK_TRACE (COMMANDEDITOR);
	connect (qApp, SIGNAL (aboutToQuit ()), this, SLOT (close ()));

	if ( !(m_doc = KTextEditor::EditorChooser::createDocument(0,"KTextEditor::Document")) ){
		KMessageBox::error(this, i18n("A KDE text-editor component could not be found;\nplease check your KDE installation."));
		delete this;
	}

	m_view = m_doc->createView (this, 0L);

	setCentralWidget(m_view);
	setAcceptDrops(true);
	
	setXMLFile( "rkcommandeditorwindowui.rc" );
	
	KAction * file_new = KStdAction::openNew (this, SLOT (newFile ()), actionCollection(), "file_new");
	file_new->setWhatsThis(i18n("Use this command to create a new document"));
	KAction * file_open = KStdAction::open (this, SLOT (openFile ()), actionCollection(), "file_open");
	file_open->setWhatsThis(i18n("Use this command to open an existing document for editing"));
	
	KAction * close_window = KStdAction::close (this, SLOT (close ()), actionCollection(), "file_close");
	close_window->setWhatsThis(i18n("Use this to close the current document"));

	KAction * run_all = new KAction (i18n ("Run all"), KShortcut ("Ctrl+R"), this, SLOT (run ()), actionCollection(), "run_all" );
	run_all->setWhatsThis(i18n("Use this to run the current document"));
	KAction * run_selection = new KAction (i18n ("Run selection"), KShortcut ("Ctrl+E"), this, SLOT (runSelection ()), actionCollection(), "run_selection" );
	run_selection->setWhatsThis(i18n("Use this to run the current selection"));
	KAction * interrupt = new KAction (i18n ("Interrupt command"), KShortcut ("Ctrl+I"), this, SLOT (interruptCommand ()), actionCollection(), "interrupt" );
	interrupt->setWhatsThis(i18n("Use this to interrupt the current command"));

	KLibFactory *factory = KLibLoader::self()->factory( "libkatepart" );
	createShellGUI( true );
	guiFactory()->addClient( m_view );
	
	setRHighlighting ();
	m_doc->setModified (false);
	
	
	setCaption (caption = i18n ("Command editor"));
	resize (minimumSizeHint ().expandedTo (QSize (640, 480)));
	
	show ();
}

RKCommandEditorWindow::~RKCommandEditorWindow () {
	RK_TRACE (COMMANDEDITOR);
	delete m_view;
	delete m_doc;
}

void RKCommandEditorWindow::closeEvent (QCloseEvent *e) {
	RK_TRACE (COMMANDEDITOR);
// TODO: call quit in order to try to save changes
	if (m_doc->closeURL ()) {
		e->accept ();
		delete this;
	}
}



void RKCommandEditorWindow::setRHighlighting () {
	// set syntax-highlighting for R
	int modes_count = highlightingInterface(m_doc)->hlModeCount ();
	bool found_mode = false;
	int i;
	RK_DO (qDebug ("%s", "Looking for syntax highlighting definition"), COMMANDEDITOR, DL_INFO);
	for (i = 0; i < modes_count; ++i) {
		RK_DO (qDebug ("%s", highlightingInterface(m_doc)->hlModeName(i).lower().latin1 ()), COMMANDEDITOR, DL_DEBUG);
		if (highlightingInterface(m_doc)->hlModeName(i).lower() == "r script") {
			found_mode = true;
			break;
		}
	}
	if (found_mode) {
		highlightingInterface(m_doc)->setHlMode(i);
	} else {
		RK_DO (qDebug ("%s", highlightingInterface(m_doc)->hlModeName(i).lower().latin1 ()), COMMANDEDITOR, DL_WARNING);
	}
}


// GUI slots below
void RKCommandEditorWindow::newWindow () {
	RK_TRACE (COMMANDEDITOR);
	new RKCommandEditorWindow (0);
}


void RKCommandEditorWindow::closeWindow () {
	RK_TRACE (COMMANDEDITOR);
	if (m_doc->closeURL ()) delete this;
}


void RKCommandEditorWindow::run () {
	RK_TRACE (COMMANDEDITOR);
	if ( ! editInterface(m_doc)->text().isEmpty() ) {
		RKGlobals::rInterface ()->issueCommand (new RCommand (editInterface(m_doc)->text(), RCommand::User, ""));
	}
}

void RKCommandEditorWindow::runSelection () {
	RK_TRACE (COMMANDEDITOR);
	if ( ! selectionInterface(m_doc)->selection().isEmpty() ) {
		RKGlobals::rInterface ()->issueCommand (new RCommand (selectionInterface(m_doc)->selection(), RCommand::User, ""));
	}
}

void RKCommandEditorWindow::runToCursor () {
	RK_TRACE (COMMANDEDITOR);
}

void RKCommandEditorWindow::runFromCursor () {
	RK_TRACE (COMMANDEDITOR);
}

void RKCommandEditorWindow::interruptCommand () {
	RK_TRACE (COMMANDEDITOR);
	RKGlobals::rInterface ()->cancelCommand (user_command);
}



void RKCommandEditorWindow::newFile()
{
  if (m_view->document()->isModified() || !m_view->document()->url().isEmpty())
    new RKCommandEditorWindow (0);
  else
    m_view->document()->openURL(KURL());
}

void RKCommandEditorWindow::openFile()
{
	RK_TRACE (COMMANDEDITOR);
	if (!m_doc->closeURL ()) return;
	
	KURL url = KFileDialog::getOpenURL (QString::null, "*.R", this);
	if (!url.isEmpty ()) {
		if (m_view->document()->openURL(url)) {
			setCaption (caption + " - " + url.filename ());
			setRHighlighting ();
		}
	}
}



#include "rkcommandeditorwindow.moc"
