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
#include <qregexp.h>

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
#include "twintable.h"

#define RK_DATA_PREFIX	"rk."

RKwardDoc::RKwardDoc(RKwardApp *parent, const char *name) : QObject(parent, name)
{
	app = parent;
	inter = &app->r_inter;
	output_is = Nothing;
	tablename = RK_DATA_PREFIX;
	tablename.append ("data");
	command_separator = "-------------this is a separator---------------";
	connect (inter, SIGNAL (receivedReply (QString)), this, SLOT (processROutput (QString)));
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

	setURL (tmpfile);
	output_is = Loaded;
	inter->issueCommand ("load (\"" + doc_url.path () + "\")");

  KIO::NetAccess::removeTempFile( tmpfile );

  modified=false;
  return true;
}

bool RKwardDoc::saveDocument(const KURL& url, const char *format /*=0*/)
{
  /////////////////////////////////////////////////
  // TODO: Add your document saving code here
  /////////////////////////////////////////////////

	pushTable (view, tablename);	

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

void RKwardDoc::pushTable (TwinTable *ttable, QString name) {
	QString command;

	// first push the data-table
	TwinTableMember *table = ttable->dataview;
	command = name;
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

	// now push the meta-table (point-reflected at bottom-left corner)
	table = ttable->varview;
	command = name;
	command.append (".meta");
	command.append (" <- data.frame (");

	for (int row=0; row < table->numRows (); row++) {
		// TODO: add labels
		vector.setNum (row);
		vector.prepend ("meta");
		vector.append ("=c (");
		for (int col=0; col < table->numCols (); col++) {
			vector.append (table->rText (row, col));
			if (col < (table->numCols ()-1)) {
				vector.append (", ");
			}
		}
		vector.append (")");
		if (row < (table->numRows ()-1)) {
			vector.append (", ");
		}
		command.append (vector);
	}
	command.append (")");

	inter->issueAsyncCommand (command);
}

void RKwardDoc::pullTable () {
	QString command;

	command = tablename + ".meta\n";
	command.append ("print (\"" + command_separator + "\")\n");
	command.append (tablename);
	output_is = WholeTables;
	inter->issueCommand (command);

	// since communication is asynchronous, the rest is done inside
	// processROutput!
}

void RKwardDoc::processROutput (QString output) {
	if (output_is == Loaded) {
		pullTable ();
	} else if (output_is == WholeTables) {
		int output_line = 0;
		QString line = output.section ("\n", output_line, output_line);
		QString tsv;

		// first process the meta-table:

		// determine width of columns
		QValueList<int> fieldends;
		int pos;
		pos = line.find (QRegExp ("[^ ]"), 0);		// skip whitespace at top-left
		while ((pos = line.find (QRegExp ("[^ ] "), pos+1)) >= 0) {
			fieldends.append (pos+1);
		}
		fieldends.append (line.length ());		// now keeps a list of where each column ends

		++output_line;
		line = output.section ("\n", output_line, output_line);
        do  {
			pos = line.find (QRegExp ("[^ ] "), 0);	// strip case numbering
			unsigned col = 0;
			while (col < fieldends.count ()) {
				++pos;
				QString value = line.mid (pos, fieldends[col] - pos);
				value = value.stripWhiteSpace ();
// TODO: somehow distinguish between literal "NA" and NA vs <NA> in R-output
				if ((value == "NA") || (value == "<NA>")) {
					value = "";
				}
				tsv.append (value);
				pos = fieldends[col];
				if (++col < fieldends.count ()) {
					tsv.append ("\t");
				}
			}
			tsv.append ("\n");
			++output_line;
			line = output.section ("\n", output_line, output_line);
		}
		while (line.length () && !line.contains (command_separator));

		view->dataview->clearSelection (false);
		QTableSelection topleft;
		topleft.init (0, 0);
		topleft.expandTo (0, 0);
		view->varview->clearSelection (false);
		view->setPasteMode (TwinTable::PasteEverywhere);
		view->varview->addSelection (topleft);
		view->pasteEncodedFlipped (tsv.local8Bit ());


		// now process the data-table:
		// TODO: cleanup code-dublication!!!

		// advance past separator
		++output_line;
		line = output.section ("\n", output_line, output_line);
		// start out empty
		tsv = "";
		// determine width of columns
		fieldends.clear ();
		pos = line.find (QRegExp ("[^ ]"), 0);		// skip whitespace at top-left
		while ((pos = line.find (QRegExp ("[^ ] "), pos+1)) >= 0) {
			fieldends.append (pos+1);
		}
		fieldends.append (line.length ());		// now keeps a list of where each column ends

		++output_line;
		line = output.section ("\n", output_line, output_line);
        do  {
			pos = line.find (QRegExp ("[^ ] "), 0);	// strip case numbering
			unsigned col = 0;
			while (col < fieldends.count ()) {
				++pos;
				QString value = line.mid (pos, fieldends[col] - pos);
				value = value.stripWhiteSpace ();
// TODO: somehow distinguish between literal "NA" and NA vs <NA> in R-output
				if ((value == "NA") || (value == "<NA>")) {
					value = "";
				}
				tsv.append (value);
				pos = fieldends[col];
				if (++col < fieldends.count ()) {
					tsv.append ("\t");
				}
			}
			tsv.append ("\n");
			++output_line;
			line = output.section ("\n", output_line, output_line);
		}
		while (line.length () && !line.contains (command_separator));

		view->varview->clearSelection (false);
		view->dataview->clearSelection (false);
		view->setPasteMode (TwinTable::PasteEverywhere);
		view->dataview->addSelection (topleft);
		view->pasteEncoded (tsv.local8Bit ());

/*		view->varview->update ();
		view->dataview->update (); */

		// next output is not for us!
		output_is = Nothing;
	}
}
