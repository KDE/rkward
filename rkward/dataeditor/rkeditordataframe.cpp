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
#include "../rkeditormanager.h"
#include "../core/rkmodificationtracker.h"

#include "../debug.h"

#define GET_NAMES_COMMAND 1
// warning! numbers above GET_DATA_OFFSET are used to determine, which row, the data should go to!
#define GET_DATA_OFFSET 10

RKEditorDataFrame::RKEditorDataFrame (QWidget *parent) : TwinTable (parent) {
	RK_TRACE (EDITOR);
	open_chain = 0;
	
	connect (varview, SIGNAL (valueChanged (int, int)), this, SLOT (metaValueChanged (int, int)));
	connect (dataview, SIGNAL (valueChanged (int, int)), this, SLOT (dataValueChanged (int, int)));
	connect (this, SIGNAL (deleteColumnRequest (int)), this, SLOT (columnDeletionRequested (int)));
	connect (this, SIGNAL (addedColumn (int)), this, SLOT (columnAdded (int)));
}

RKEditorDataFrame::~RKEditorDataFrame () {
	RK_TRACE (EDITOR);
}

void RKEditorDataFrame::flushChanges () {
	RK_TRACE (EDITOR);
	flushEdit ();
}

void RKEditorDataFrame::openObject (RObject *object, bool initialize_to_empty) {
	RK_TRACE (EDITOR);
	flushEdit ();
	RKEditor::object = object;

	open_chain = RKGlobals::rInterface ()->startChain (open_chain);
	if (initialize_to_empty) {
		pushTable (open_chain);
		for (int i=0; i < numCols (); ++i) {
			static_cast<RContainerObject *> (getObject ())->createNewChild (varview->text (NAME_ROW, i), this);
		}
	}
		
	// actually, given the object, we already know the child-names. We don't know their order, however, so we better fetch the name-row again.
	RCommand *command = new RCommand ("names (" + object->getFullName () + ")", RCommand::Sync | RCommand::GetStringVector, "", this, GET_NAMES_COMMAND);
	RKGlobals::rInterface ()->issueCommand (command, open_chain);

	// since communication is asynchronous, the rest is done inside
	// processROutput!
}

void RKEditorDataFrame::rCommandDone (RCommand *command) {
	RK_TRACE (EDITOR);
	if (command->getFlags () == GET_NAMES_COMMAND) {
		disconnect (varview, SIGNAL (valueChanged (int, int)), this, SLOT (metaValueChanged (int, int)));
		disconnect (this, SIGNAL (addedColumn (int)), this, SLOT (columnAdded (int)));
		// this is just a quick and dirty fix. The real fix will be not to start the editor with a set of empty variables, but to implement a better way of adding
		// variables "as you type", instead.
		for (int i=command->stringVectorLength (); i < numCols (); ++i) {
			setColObject (i, static_cast<RContainerObject *> (getObject ())->createNewChild (varview->text (NAME_ROW, i), this));
		}
	
		// set the names and meta-information
		for (int i = 0; i < command->stringVectorLength (); ++i) {
			if (numCols () <= i) {
				insertNewColumn ();
			}
			// TODO: make clean
			RKVariable *current_child = static_cast<RKVariable *> (static_cast <RContainerObject*> (getObject ())->findChild (command->getStringVector ()[i]));
			setColObject (i, current_child);
			modifyObjectMeta (current_child, i);
					
			// ok, now get the data
			RCommand *rcom = new RCommand ("as.vector (" + current_child->getFullName() + ")", RCommand::Sync | RCommand::GetStringVector, "", this, GET_DATA_OFFSET + i);
			RKGlobals::rInterface ()->issueCommand (rcom, open_chain);
		}

		open_chain = RKGlobals::rInterface ()->closeChain (open_chain);
		connect (this, SIGNAL (addedColumn (int)), this, SLOT (columnAdded (int)));
		connect (varview, SIGNAL (valueChanged (int, int)), this, SLOT (metaValueChanged (int, int)));

	} else if (command->getFlags () >= GET_DATA_OFFSET) {
		int col = command->getFlags () - GET_DATA_OFFSET;
		setColumn (dataview, col, 0, command->stringVectorLength () - 1, command->getStringVector ());
	}
}

void RKEditorDataFrame::modifyObjectMeta (RObject *object, int column) {
	RK_TRACE (EDITOR);
	if (object == getObject ()) {
		RKGlobals::editorManager ()->setEditorName (this, object->getShortName ());
		return;
	}
	RK_ASSERT (column >= 0);
	
	disconnect (varview, SIGNAL (valueChanged (int, int)), this, SLOT (metaValueChanged (int, int)));
	varview->setText (NAME_ROW, column, object->getShortName ());
	varview->setText (TYPE_ROW, column, static_cast<RKVariable *> (object)->getVarTypeString ());
	varview->setText (LABEL_ROW, column, object->getLabel ());
	connect (varview, SIGNAL (valueChanged (int, int)), this, SLOT (metaValueChanged (int, int)));
}

void RKEditorDataFrame::pushTable (RCommandChain *sync_chain) {
	RK_TRACE (EDITOR);
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
	getObject ()->writeMetaData (sync_chain);
}

void RKEditorDataFrame::metaValueChanged (int row, int col) {
	RK_TRACE (EDITOR);
	RObject *obj = getColObject (col);
	RK_ASSERT (obj);
	// for now:
	if (!obj) return;
	
	if (row == LABEL_ROW) {
		obj->setLabel (varview->text (row, col));
	} else if (row == NAME_ROW) {
		RKGlobals::tracker ()->renameObject (obj, varview->text (row, col), this);
	} else if (row == TYPE_ROW) {
		static_cast<RKVariable *> (obj)->setVarType (static_cast<TypeSelectCell *> (varview->item (row, col))->type ());
	}

	RKGlobals::tracker ()->objectMetaChanged (getColObject (col), this);
}

void RKEditorDataFrame::dataValueChanged (int row, int col) {
	RK_TRACE (EDITOR);
	RObject *obj = getColObject (col);
	RK_ASSERT (obj);
	// for now:
	if (!obj) return;
	
	RObject::ChangeSet *changes = new RObject::ChangeSet;
	changes->from_index = col;
	changes->to_index = col;
	
	RKGlobals::rInterface ()->issueCommand (new RCommand (obj->getFullName () + "[" + QString ().setNum (row+1) + "] <- " + dataview->rText (row, col), RCommand::App | RCommand::Sync));
	RKGlobals::tracker ()->objectDataChanged (obj, changes, this);
}

void RKEditorDataFrame::columnDeletionRequested (int col) {
	RK_TRACE (EDITOR);
	RObject *obj = getColObject (col);
	RK_ASSERT (obj);
	// for now:
	if (!obj) return;

	RKGlobals::tracker ()->removeObject (obj);
}

void RKEditorDataFrame::columnAdded (int col) {
	RK_TRACE (EDITOR);
	RObject *obj = static_cast<RContainerObject *> (getObject ())->createNewChild (varview->text (NAME_ROW, col), this);
	RKGlobals::rInterface ()->issueCommand (new RCommand (obj->getFullName () + " <- c (NA)", RCommand::App | RCommand::Sync));
	
	// TODO: find a nice way to update the list:
	ColMap new_map;
	for (int i=0; i < col; ++i) {
		new_map.insert (i, col_map[i]);
	}
	new_map.insert (col, obj);
	for (int i=(col+1); i < numCols (); ++i) {
		new_map.insert (i, col_map[i-1]);
	}
	col_map = new_map;
}

void RKEditorDataFrame::setColObject (int column, RObject *object) {
	RK_TRACE (EDITOR);
	col_map.insert (column, object);
}

RObject *RKEditorDataFrame::getColObject (int col) {
	RK_TRACE (EDITOR);
	ColMap::iterator it = col_map.find (col);
	if (it != col_map.end ()) {
		return it.data ();
	}
	return 0;
}

int RKEditorDataFrame::getObjectCol (RObject *object) {
	RK_TRACE (EDITOR);
	for (ColMap::iterator it = col_map.begin (); it != col_map.end (); ++it) {
		if (it.data () == object) return it.key ();
	}
	
	RK_ASSERT (false);
	return -1;
}

void RKEditorDataFrame::removeObject (RObject *object) {
	RK_TRACE (EDITOR);
	if (object == getObject ()) {
		// self destruct
		RKGlobals::editorManager ()->closeEditor (this);
		return;
	}
	
	int col = getObjectCol (object);
	RK_ASSERT (col >= 0);
	// for now:
	if (col < 0) return;

	deleteColumn (col);
	
	// TODO: find a nice way to update the list:
	ColMap new_map;
	for (int i=0; i < col; ++i) {
		new_map.insert (i, col_map[i]);
	}
	for (int i=(col+1); i < numCols (); ++i) {
		new_map.insert (i-1, col_map[i]);
	}
	col_map = new_map;
}

void RKEditorDataFrame::restoreObject (RObject *object) {
	RK_TRACE (EDITOR);
	// for now, simply sync the whole table unconditionally.
	pushTable (0);
}

void RKEditorDataFrame::renameObject (RObject *object) {
	RK_TRACE (EDITOR);
	int col = getObjectCol (object);
	modifyObjectMeta (object, col);
}

void RKEditorDataFrame::addObject (RObject *object) {
	RK_TRACE (EDITOR);
	insertNewColumn ();
	// columnAdded will be called in between!
	modifyObjectMeta (object, numCols () - 1);
	
	// TODO: get the data for the new object!
}

void RKEditorDataFrame::updateObjectMeta (RObject *object) {
	RK_TRACE (EDITOR);
	int col = getObjectCol (object);

	modifyObjectMeta (object, col);
}

void RKEditorDataFrame::updateObjectData (RObject *object, RObject::ChangeSet *changes) {
	RK_TRACE (EDITOR);
	int col = getObjectCol (object);
	RK_ASSERT (col >= 0);
	// for now:
	if (col < 0) return;

	RK_ASSERT (false);		// not yet implemented. Need this as soon as several editors may work on the same object.
}

