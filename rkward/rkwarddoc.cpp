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
#include "dataeditor/twintablemember.h"
#include "dataeditor/twintable.h"

#include "rkglobals.h"

#define RK_DATA_PREFIX	"rk."

#define RLOAD_COMMAND 1
#define RPULL_COMMAND 2
#define RMETA_TABLE 4
#define RDONE_WITH_TABLE 8
#define RPULL_BOTH_TABLES 16

RKwardDoc::RKwardDoc(RKwardApp *parent, const char *name) : TwinTable (parent, name)
{
	app = parent;
//	output_is = Nothing;
	tablename = RK_DATA_PREFIX;
	tablename.append ("data");
	command_chain = 0;
}

RKwardDoc::~RKwardDoc()
{
}

void RKwardDoc::setURL(const KURL &url)
{
  doc_url=url;
}

const KURL& RKwardDoc::URL() const
{
  return doc_url;
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
	KIO::NetAccess::download (url, tmpfile, app);
	
	setURL (tmpfile);
//	output_is = Loaded;
	RCommand *command = new RCommand ("load (\"" + doc_url.path () + "\")", RCommand::App, "", this, SLOT (processROutput (RCommand *)), RLOAD_COMMAND);
	RKGlobals::rInterface ()->issueCommand (command);
	pullTable ();
	
	modified=false;
	return true;
}

bool RKwardDoc::saveDocument(const KURL& url, const char *format /*=0*/)
{
  /////////////////////////////////////////////////
  // TODO: Add your document saving code here
  /////////////////////////////////////////////////

	syncToR ();

	RKGlobals::rInterface ()->issueCommand (new RCommand ("save.image (\"" + url.path () + "\")", RCommand::App));

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

void RKwardDoc::syncToR () {
	// TODO: only push changed rows/columns
	pushTable (this, tablename);
}

void RKwardDoc::pushTable (TwinTable *ttable, QString name) {
	flushEdit ();
	QString command;

	// first push the data-table
	TwinTableMember *table = dataview;
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

	command_chain = RKGlobals::rInterface ()->startChain (command_chain);
	RKGlobals::rInterface ()->issueCommand (new RCommand (command, RCommand::Sync), command_chain);

	// now push the meta-table (point-reflected at bottom-left corner)
	table = varview;
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

	RKGlobals::rInterface ()->issueCommand (new RCommand (command, RCommand::Sync), command_chain);
	command_chain = RKGlobals::rInterface ()->closeChain (command_chain);
}

void RKwardDoc::pullTable () {
	QString command, dummy;

	command_chain = RKGlobals::rInterface ()->startChain (command_chain);
	for (int i=0; i < 5; ++i) {
		command = "as.vector (" + tablename + ".meta[[" + dummy.setNum (i+1) + "]])";
		
		PullCommandIdentifier *pci = new PullCommandIdentifier;
		pci->table = varview;
		pci->as_column = false;
		pci->offset_col = 0;
		pci->offset_row = i;
		pci->length = -1;
// TODO: use the RCommand-flags instead
		pci->get_data_table_next = (i == 4);
		RCommand *rcom = new RCommand (command, RCommand::Sync | RCommand::GetStringVector, "", this, SLOT (processROutput (RCommand *)), RPULL_COMMAND);
		pull_map.insert (rcom, pci);
		
		RKGlobals::rInterface ()->issueCommand (rcom, command_chain);
	}
	// since communication is asynchronous, the rest is done inside
	// processROutput!
}

void RKwardDoc::processROutput (RCommand *command) {
	if (command->getFlags () == RPULL_COMMAND) {
		PullMap::const_iterator it = pull_map.find (command);
		if (it == pull_map.end ()) return;	// TODO: ASSERT
		PullCommandIdentifier *pci = it.data ();
	
		if (pci->length == -1) {
			pci->length = command->stringVectorLength ();
		}
	
		if (command->stringVectorLength () < pci->length) return; // TODO: ASSERT
		if (pci->as_column) {
			setColumn (pci->table, pci->offset_col, pci->offset_row, pci->offset_row + pci->length - 1, command->getStringVector());
		} else {
			setRow (pci->table, pci->offset_row, pci->offset_col, pci->offset_col + pci->length - 1, command->getStringVector());
		}
		
		// seems we reached the end of the meta-table. Next, get the data-table.
		if (pci->get_data_table_next) {
			QString command_string, dummy;

			for (int i=0; i < numCols (); ++i) {
				command_string = "as.vector (" + tablename + "[[" + dummy.setNum (i+1) + "]])";
		
				PullCommandIdentifier *datapci = new PullCommandIdentifier;
				datapci->table = dataview;
				datapci->as_column = true;
				datapci->offset_col = i;
				datapci->offset_row = 0;
				datapci->length = -1;
				datapci->get_data_table_next = false;
				RCommand *rcom = new RCommand (command_string, RCommand::Sync | RCommand::GetStringVector, "", this, SLOT (processROutput (RCommand *)), RPULL_COMMAND);
				pull_map.insert (rcom, datapci);
		
				RKGlobals::rInterface ()->issueCommand (rcom, command_chain);
			}
			
			command_chain = RKGlobals::rInterface ()->closeChain (command_chain);
		}

		delete pci;
		pull_map.remove (command);
	} else if (command->getFlags () == RLOAD_COMMAND) {
		KIO::NetAccess::removeTempFile (tmpfile);
	}
}
