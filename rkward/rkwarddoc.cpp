/***************************************************************************
                          rkwarddoc.cpp  -  description
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

// include files for Qt
#include <qdir.h>
#include <qwidget.h>

// include files for KDE
#include <klocale.h>
#include <kmessagebox.h>
#include <kio/job.h>
#include <kio/netaccess.h>

// application specific includes
#include "rkwarddoc.h"
#include "rkward.h"
#include "rkwardview.h"
#include "twintablemember.h"

#define RK_DATA_PREFIX	"rk."

RKwardDoc::RKwardDoc(RKwardApp *parent, const char *name) : QObject(parent, name)
{
	app = parent;
	inter = &app->r_inter;
}

RKwardDoc::~RKwardDoc()
{
}

void RKwardDoc::setView(RKwardView *view)
{
	RKwardDoc::view = view;
}

void RKwardDoc::removeView(RKwardView *view)
{
//  pViewList->remove(view);
}
void RKwardDoc::setURL(const KURL &url)
{
  doc_url=url;
}

const KURL& RKwardDoc::URL() const
{
  return doc_url;
}

void RKwardDoc::slotUpdateAllViews(RKwardView *sender)
{

// TODO: discard
/*  RKwardView *w;
  if(pViewList)
  {
    for(w=pViewList->first(); w!=0; w=pViewList->next())
    {
      if(w!=sender)
        w->repaint();
    }
  }
  */
}

bool RKwardDoc::saveModified()
{
  bool completed=true;

  if(modified)
  {
    RKwardApp *win=(RKwardApp *) parent();
    int want_save = KMessageBox::warningYesNoCancel(win,
                                         i18n("The current file has been modified.\n"
                                              "Do you want to save it?"),
                                         i18n("Warning"));
    switch(want_save)
    {
      case KMessageBox::Yes:
           if (doc_url.fileName() == i18n("Untitled"))
           {
             win->slotFileSaveAs();
           }
           else
           {
             saveDocument(URL());
       	   };

       	   deleteContents();
           completed=true;
           break;

      case KMessageBox::No:
           setModified(false);
           deleteContents();
           completed=true;
           break;

      case KMessageBox::Cancel:
           completed=false;
           break;

      default:
           completed=false;
           break;
    }
  }

  return completed;
}

void RKwardDoc::closeDocument()
{
  deleteContents();
}

bool RKwardDoc::newDocument()
{
  /////////////////////////////////////////////////
  // TODO: Add your document initialization code here
  /////////////////////////////////////////////////
  modified=false;
  doc_url.setFileName(i18n("Untitled"));

  return true;
}

bool RKwardDoc::openDocument(const KURL& url, const char *format /*=0*/)
{
  QString tmpfile;
  KIO::NetAccess::download( url, tmpfile );
  /////////////////////////////////////////////////
  // TODO: Add your document opening code here
  /////////////////////////////////////////////////

  KIO::NetAccess::removeTempFile( tmpfile );

  modified=false;
  return true;
}

bool RKwardDoc::saveDocument(const KURL& url, const char *format /*=0*/)
{
  /////////////////////////////////////////////////
  // TODO: Add your document saving code here
  /////////////////////////////////////////////////

	pushTable (view->dataview, "data");	
	pushTable (view->varview, "data.meta");

	inter->issueAsyncCommand ("save.image (\"" + url.path () + "\")");

	setURL (url);
  modified=false;
  return true;
}

void RKwardDoc::deleteContents()
{
  /////////////////////////////////////////////////
  // TODO: Add implementation to delete the document contents
  /////////////////////////////////////////////////

}

void RKwardDoc::pushTable (TwinTableMember *table, QString name) {
	QString command;
	command.append (RK_DATA_PREFIX);
	command.append (name);
	command.append (" <- data.frame (");

	QString vector;
	for (int col=0; col < table->numCols (); col++) {
		vector = table->varTable ()->rText (NAME_ROW, col) + "=c (";
		for (int row=0; row < table->numRows (); row++) {
			vector.append (table->rText (row, col));
			if (row < (table->numRows ()-1)) {
				vector.append (", ");
			}
		}
		vector.append (")");
		if (col < (table->numCols ()-1)) {
			vector.append (", ");
		}
		command.append (vector);
	}
	command.append (")");

	inter->issueAsyncCommand (command);
}

void RKwardDoc::pullTable (TwinTableMember *table, QString name) {
}
