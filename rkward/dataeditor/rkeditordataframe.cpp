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

#include "../debug.h"

#define GET_NAMES_COMMAND 1
// warning! numbers above GET_DATA_OFFSET are used to determine, which row, the data should go to!
#define GET_DATA_OFFSET 10

RKEditorDataFrame::RKEditorDataFrame (QWidget *parent) : TwinTable (parent) {
	open_chain = 0;
	
	connect (varview, SIGNAL (valueChanged (int, int)), this, SLOT (metaValueChanged (int, int)));
	connect (dataview, SIGNAL (valueChanged (int, int)), this, SLOT (dataValueChanged (int, int)));
	connect (this, SIGNAL (aboutToDeleteColumn (int)), this, SLOT (columnDeleted (int)));
}

RKEditorDataFrame::~RKEditorDataFrame () {
}

void RKEditorDataFrame::syncToR (RCommandChain *sync_chain) {
	if (getObject ()->needsSyncToR ()) {
		RK_DO (qDebug ("pushing table: data_modified %d, meta_modified %d, children_modified %d", getObject ()->isDataModified (), getObject ()->isMetaModified (), getObject ()->hasModifiedChildren ()), EDITOR, DL_DEBUG);
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
		// this is just a quick and dirty fix. The real fix will be not to start the editor with a set of empty variables, but to implement a better way of adding
		// variables "as you type", instead.
		for (int i=command->stringVectorLength (); i < numCols (); ++i) {
			setColObject (i, static_cast<RContainerObject *> (getObject ())->createNewChild (varview->text (NAME_ROW, i)));
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

	} else if (command->getFlags () >= GET_DATA_OFFSET) {
		int col = command->getFlags () - GET_DATA_OFFSET;
		setColumn (dataview, col, 0, command->stringVectorLength () - 1, command->getStringVector ());
	}
}

void RKEditorDataFrame::modifyObjectMeta (RKVariable *object, int column) {
	disconnect (varview, SIGNAL (valueChanged (int, int)), this, SLOT (metaValueChanged (int, int)));
	varview->setText (NAME_ROW, column, object->getShortName ());
	varview->setText (TYPE_ROW, column, object->getVarTypeString ());
	varview->setText (LABEL_ROW, column, object->getLabel ());
	connect (varview, SIGNAL (valueChanged (int, int)), this, SLOT (metaValueChanged (int, int)));
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
	
	getObject ()->setDataSynced ();

	RK_DO (qDebug ("table-state: data_modified %d, meta_modified %d, children_modified %d", getObject ()->isDataModified (), getObject ()->isMetaModified (), getObject ()->hasModifiedChildren ()), EDITOR, DL_DEBUG);
}

void RKEditorDataFrame::metaValueChanged (int row, int col) {
	RK_ASSERT (getColObject (col));
	// for now:
	if (!getColObject (col)) return;
	
	if (row == LABEL_ROW) {
		getColObject (col)->setLabel (varview->text (row, col));
	} else if (row == NAME_ROW) {
		getColObject (col)->rename (varview->text (row, col));
	} else if (row == TYPE_ROW) {
		static_cast<RKVariable *> (getColObject (col))->setVarType (static_cast<TypeSelectCell *> (varview->item (row, col))->type ());
	}
}

void RKEditorDataFrame::dataValueChanged (int, int col) {
	RObject *obj = getColObject (col);
	RK_ASSERT (obj);
	// for now:
	if (!obj) return;
	
	obj->setDataModified ();
}

void RKEditorDataFrame::columnDeleted (int col) {
	RObject *obj = getColObject (col);
	RK_ASSERT (obj);
	// for now:
	if (!obj) return;

	obj->remove ();
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

int RKEditorDataFrame::getObjectCol (RObject *object) {
	for (ColMap::iterator it = col_map.begin (); it != col_map.end (); ++it) {
		if (it.data () == object) return it.key ();
	}
	
	RK_ASSERT (false);
	return -1;
}

void RKEditorDataFrame::objectDeleted (RObject *object) {
	if (object == getObject ()) {
		// self destruct
		RKGlobals::editorManager ()->closeEditor (this, false);
		return;
	}
	
	// we don't want any notification on this
	disconnect (this, SIGNAL (aboutToDeleteColumn (int)), this, SLOT (columnDeleted (int)));
	deleteColumn (getObjectCol (object));
	connect (this, SIGNAL (aboutToDeleteColumn (int)), this, SLOT (columnDeleted (int)));
}

void RKEditorDataFrame::objectMetaModified (RObject *object) {
	if (object == getObject ()) {
		RKGlobals::editorManager ()->setEditorName (this, object->getShortName ());
		// nothing to do for now
		return;
	}

	modifyObjectMeta (static_cast<RKVariable*> (object), getObjectCol (object));
}
