/***************************************************************************
                          rkeditordataframe  -  description
                             -------------------
    begin                : Fri Aug 20 2004
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
#include "rkeditordataframe.h"

#include "../rbackend/rinterface.h"
#include "../rkglobals.h"
#include "twintable.h"
#include "twintablemember.h"
#include "../core/robject.h"

#define RLOAD_COMMAND 1
#define RPULL_COMMAND 2
#define RMETA_TABLE 4
#define RDONE_WITH_TABLE 8
#define RPULL_BOTH_TABLES 16

RKEditorDataFrame::RKEditorDataFrame (QWidget *parent) : TwinTable (parent) {
	open_chain = 0;
}

RKEditorDataFrame::~RKEditorDataFrame () {
}

void RKEditorDataFrame::syncToR (RCommandChain *sync_chain) {
	pushTable (sync_chain);
}

void RKEditorDataFrame::openObject (RObject *object) {
	RKEditor::object = object;
	QString command, dummy;

	open_chain = RKGlobals::rInterface ()->startChain (open_chain);
	for (int i=0; i < 5; ++i) {
		command = "as.vector (" + object->getFullName () + ".meta[[" + dummy.setNum (i+1) + "]])";
		
		PullCommandIdentifier *pci = new PullCommandIdentifier;
		pci->table = varview;
		pci->as_column = false;
		pci->offset_col = 0;
		pci->offset_row = i;
		pci->length = -1;
// TODO: use the RCommand-flags instead
		pci->get_data_table_next = (i == 4);
		RCommand *rcom = new RCommand (command, RCommand::Sync | RCommand::GetStringVector, "", this, RPULL_COMMAND);
		pull_map.insert (rcom, pci);
		
		RKGlobals::rInterface ()->issueCommand (rcom, open_chain);
	}
	// since communication is asynchronous, the rest is done inside
	// processROutput!
}

void RKEditorDataFrame::rCommandDone (RCommand *command) {
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
				command_string = "as.vector (" + getObject ()->getFullName () + "[[" + dummy.setNum (i+1) + "]])";
		
				PullCommandIdentifier *datapci = new PullCommandIdentifier;
				datapci->table = dataview;
				datapci->as_column = true;
				datapci->offset_col = i;
				datapci->offset_row = 0;
				datapci->length = -1;
				datapci->get_data_table_next = false;
				RCommand *rcom = new RCommand (command_string, RCommand::Sync | RCommand::GetStringVector, "", this, RPULL_COMMAND);
				pull_map.insert (rcom, datapci);
		
				RKGlobals::rInterface ()->issueCommand (rcom, open_chain);
			}
			
			open_chain = RKGlobals::rInterface ()->closeChain (open_chain);
		}

		delete pci;
		pull_map.remove (command);
	}
}

void RKEditorDataFrame::pushTable (RCommandChain *sync_chain) {
	flushEdit ();
	QString command;

	// first push the data-table
	TwinTableMember *table = dataview;
	command = getObject ()->getFullName ();
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

	RKGlobals::rInterface ()->issueCommand (new RCommand (command, RCommand::Sync), sync_chain);

/// TODO: store meta data entirely differently.
	// now push the meta-table (point-reflected at bottom-left corner)
	table = varview;
	command = getObject ()->getFullName ();
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

	RKGlobals::rInterface ()->issueCommand (new RCommand (command, RCommand::Sync), sync_chain);
}

