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


#ifndef RKDOCMANAGER_H
#define RKDOCMANAGER_H

#include <qobject.h>
#include <kate/document.h>
#include <kparts/factory.h>

/**
* This class handles Kate documents.
*
* @author Pierre Ecochard
*/
class RKDocManager : public QObject
{
Q_OBJECT
public:
    RKDocManager(QObject *parent = 0, const char *name = 0);

    ~RKDocManager();
    
    KTextEditor::Document *createDoc ();
    void deleteDoc (Kate::Document *doc);


    KTextEditor::Document *activeDocument ();

    KTextEditor::Document *firstDocument ();
    KTextEditor::Document *nextDocument ();    
    
    int findDocument (Kate::Document *doc);
    uint documents ();    
    
    
private:
    QPtrList<KTextEditor::Document> m_docList;
    KParts::Factory *m_factory;
};

#endif
