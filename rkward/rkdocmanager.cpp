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
 
#include "rkdocmanager.h"

#include <kate/document.h>
#include <kate/view.h>

#include <ktexteditor/configinterface.h>
#include <ktexteditor/sessionconfiginterface.h>
#include <ktexteditor/viewcursorinterface.h>
#include <ktexteditor/printinterface.h>
#include <ktexteditor/encodinginterface.h>
#include <ktexteditor/editorchooser.h>
#include <ktexteditor/popupmenuinterface.h>

#include "debug.h"

RKDocManager::RKDocManager(QObject *parent, const char *name)
 : QObject(parent, name)
{
  m_factory = (KParts::Factory *) KLibLoader::self()->factory ("libkatepart");


  
  createDoc();
}


RKDocManager::~RKDocManager()
{
	KTextEditor::Document *doc;
	for ( doc = m_docList.first(); doc; doc = m_docList.next() ) {
		m_docList.remove (doc);
		delete doc;
	}
}



KTextEditor::Document *RKDocManager::createDoc ()
{
  KTextEditor::Document *doc; /*= (KTextEditor::Document *) m_factory->createPart (0, "", this, "", "KTextEditor::Document");*/
  m_docList.append(doc);

  return doc;
}

void RKDocManager::deleteDoc (Kate::Document *doc)
{
  m_docList.remove (doc);
  delete doc;
}


KTextEditor::Document *RKDocManager::activeDocument ()
{
  return m_docList.current();
}


KTextEditor::Document *RKDocManager::firstDocument ()
{
  return m_docList.first();
}

KTextEditor::Document *RKDocManager::nextDocument ()
{
  return m_docList.next();
}


int RKDocManager::findDocument (Kate::Document *doc)
{
  return m_docList.find (doc);
}

uint RKDocManager::documents ()
{
  return m_docList.count ();
}



#include "rkdocmanager.moc"
