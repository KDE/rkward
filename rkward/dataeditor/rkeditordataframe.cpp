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
#include "typeselectcell.h"
#include "../core/robject.h"
#include "../core/rkvariable.h"
#include "../core/rcontainerobject.h"

#include "../debug.h"

#define GET_NAMES_COMMAND 1
// warning! numbers above GET_DATA_OFFSET are used to determine, which row, the data should go to!
#define GET_DATA_OFFSET 10

RKEditorDataFrame::RKEditorDataFrame (QWidget *parent) : TwinTable (parent) {
	open_chain = 0;
	
	connect (varview, SIGNAL (valueChanged (int, int)), this, SLOT (metaValueChanged (int, int)));
	connect (dataview, SIGNAL (valueChanged (int, int)), this, SLOT (dataValueChanged (int, int)));
}

RKEditorDataFrame::~RKEditorDataFrame () {
}

void RKEditorDataFrame::syncToR (RCommandChain *sync_chain) {
	if (getObject ()->needsSyncToR ()) {
		pushTable (sync_chain);
	}
}

void RKEditorDataFrame::openObject (RObject *object) {
	flushEdit ();
	RKEditor::object = object;

	open_chain = RKGlobals::rInterface ()->startChain (open_chain);
	
	// actually, given the object, we already know the child-names. We don't know their order, however, so we better fetch the name-row again.
	RCommand *command = new RCommand ("names (" + object->getFullName () + ")", RCommand::Sync | RCommand::GetStringVector, "", this, GET_NAMES_COMMAND);

	RKGlobals::rInterface ()->issueCommand (command, open_chain);
	// since communication is asynchronous, the rest is done inside
	// processROutput!
}

void RKEditorDataFrame::rCommandDone (RCommand *command) {
	if (command->getFlags () == GET_NAMES_COMMAND) {
		// set the names and meta-information
		for (int i = 0; i < command->stringVectorLength (); ++i) {
			if (numCols () <= i) {
				insertNewColumn ();
			}
			// TODO: make clean
			RKVariable *current_child = static_cast<RKVariable *> (static_cast <RContainerObject*> (getObject ())->findChild (command->getStringVector ()[i]));
			setColObject (i, current_child);
			varview->setText (NAME_ROW, i, command->getStringVector ()[i]);
			varview->setText (TYPE_ROW, i, current_child->getVarTypeString ());
			varview->setText (LABEL_ROW, i, current_child->getLabel ());
		
			// ok, now get the data
			RCommand *rcom = new RCommand ("as.vector (" + current_child->getFullName() + ")", RCommand::Sync | RCommand::GetStringVector, "", this, GET_DATA_OFFSET + i);
			RKGlobals::rInterface ()->issueCommand (rcom, open_chain);
		}
		
		open_chain = RKGlobals::rInterface ()->closeChain (open_chain);

	} else if (command->getFlags () >= GET_DATA_OFFSET) {
		int col = command->getFlags () - GET_DATA_OFFSET;
		setColumn (dataview, col, 0, command->stringVectorLength () - 1, command->getStringVector ());
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

	// now store the meta-data
	getObject ()->writeMetaData (sync_chain, true);
}

void RKEditorDataFrame::metaValueChanged (int row, int col) {
	RK_ASSERT (getColObject (col));
	// for now:
	if (!getColObject (col)) return;
	
	if (row == LABEL_ROW) {
		getColObject (col)->setLabel (varview->rText (row, col));
	} else if (row == NAME_ROW) {
		getColObject (col)->rename (varview->rText (row, col));
	} else if (row == TYPE_ROW) {
		static_cast<RKVariable *> (getColObject (col))->setVarType (static_cast<TypeSelectCell *> (varview->item (row, col))->type ());
	}
}

void RKEditorDataFrame::dataValueChanged (int, int col) {
	RK_ASSERT (getColObject (col));
	// for now:
	if (!getColObject (col)) return;
	
	getColObject (col)->setDataModified ();
}

void RKEditorDataFrame::setColObject (int column, RObject *object) {
	col_map.insert (column, object);
}

RObject *RKEditorDataFrame::getColObject (int col) {
	ColMap::iterator it = col_map.find (col);
	if (it != col_map.end ()) {
		return it.data ();
	}
	return 0;
}

