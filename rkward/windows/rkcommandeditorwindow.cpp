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
#include <kparts/partmanager.h>

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
#include <qtabwidget.h>

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
#include "../rkward.h"

#include "../debug.h"


RKCommandEditorWindow::RKCommandEditorWindow (QWidget *parent) : KMdiChildView (parent) {
	RK_TRACE (COMMANDEDITOR);
	
	
	m_library = KLibLoader::self()->library("libkatepart");
	m_doc = (Kate::Document *) m_library->factory ()->create (0L,"kate","KTextEditor::Document");
	m_view = (Kate::View *)m_doc->createView(this);
	m_view->setName("Kate Part View");
	setRHighlighting(m_doc);
	pLayout = new QHBoxLayout( this, 0, -1, "layout");
	pLayout->addWidget(m_view);
 
	connect (this, SIGNAL (gotFocus (KMdiChildView *)), this, SLOT (slotGotFocus ()));
	connect (this, SIGNAL (lostFocus (KMdiChildView *)), this, SLOT (slotLostFocus ()));

}

RKCommandEditorWindow::~RKCommandEditorWindow () {
	RK_TRACE (COMMANDEDITOR);
	delete pLayout;
}


void RKCommandEditorWindow::setRHighlighting (Kate::Document *doc) {
	// set syntax-highlighting for R
	int modes_count = highlightingInterface(doc)->hlModeCount ();
	bool found_mode = false;
	int i;
	RK_DO (qDebug ("%s", "Looking for syntax highlighting definition"), COMMANDEDITOR, DL_INFO);
	for (i = 0; i < modes_count; ++i) {
		RK_DO (qDebug ("%s", highlightingInterface(doc)->hlModeName(i).lower().latin1 ()), COMMANDEDITOR, DL_DEBUG);
		if (highlightingInterface(doc)->hlModeName(i).lower() == "r script") {
			found_mode = true;
			break;
		}
	}
	if (found_mode) {
		highlightingInterface(doc)->setHlMode(i);
	} else {
		RK_DO (qDebug ("%s", highlightingInterface(doc)->hlModeName(i).lower().latin1 ()), COMMANDEDITOR, DL_WARNING);
	}
}






QString RKCommandEditorWindow::getSelection()
{
    	RK_TRACE (COMMANDEDITOR);
	return selectionInterface(m_doc)->selection();
}



QString RKCommandEditorWindow::getText()
{
	return editInterface(m_doc)->text();
}


#include "rkcommandeditorwindow.moc"



void RKCommandEditorWindow::slotGotFocus()
{

	//RKGlobals::rkApp()->guiFactory()->addClient( m_view );
	
	//RKGlobals::rkApp()->changeGUI((KParts::Part *) this );//createGUI((KParts::Part *) m_view );
}



void RKCommandEditorWindow::slotLostFocus()
{
	//RKGlobals::rkApp()->guiFactory()->removeClient(m_view);
}



bool RKCommandEditorWindow::openURL(const KURL &url){
	if (m_doc->openURL(url)){
		setRHighlighting(m_doc);
		
		updateTabCaption(url);
		
		return true;
	}
	return false;
	

}

bool RKCommandEditorWindow::getFilenameAndPath(const KURL &url,QString *fname){
	QString fullpath = url.path();
	int i,length,fnamepos;
	bool done;
	
	if ((length = (int)fullpath.length()) == 0)
		return false;

	fnamepos = 0;
	for (i = length-1,done = false ; i >= 0 && !done ; i--){
		if (fullpath[i] == '/'){
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





KURL RKCommandEditorWindow::url(){
    return m_doc->url();
}



bool RKCommandEditorWindow::save(){
    return m_doc->save();
}

bool RKCommandEditorWindow::saveAs(const KURL &url){
	return m_doc->saveAs(url);
	updateTabCaption(url); 
}



bool RKCommandEditorWindow::isModified() {
    return m_doc->isModified();
}


void RKCommandEditorWindow::cut(){
	m_view->cut();
}


void RKCommandEditorWindow::copy(){
	 m_view->copy();
}



void RKCommandEditorWindow::paste(){
	 m_view->paste();
}


void RKCommandEditorWindow::undo(){
	 m_doc->undo();
}


void RKCommandEditorWindow::redo(){
	 m_doc->redo();
}



void RKCommandEditorWindow::insertText(QString text)
{
	m_doc->insertText(m_view->cursorLine(),m_view->cursorColumn(),text);
	setFocus();
}



void RKCommandEditorWindow::updateTabCaption(const KURL &url)
{
	QString fname;
	if (getFilenameAndPath(url,&fname)){
		setTabCaption(fname);
	}
	else {
		setTabCaption(url.prettyURL());
	}
}
