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
#include "twintabledatamember.h"
#include "twintablemetamember.h"
#include "../core/robject.h"
#include "../core/rkvariable.h"
#include "../core/rcontainerobject.h"
#include "../rkeditormanager.h"
#include "../core/rkmodificationtracker.h"
#include "rkeditordataframepart.h"

#include "../debug.h"

#define GET_NAMES_COMMAND 1
#define LOAD_COMPLETE_COMMAND 2
// warning! numbers above GET_DATA_OFFSET are used to determine, which row, the data should go to!
#define GET_DATA_OFFSET 10

RKEditorDataFrame::RKEditorDataFrame (QWidget *parent) : TwinTable (parent) {
	RK_TRACE (EDITOR);

	part = new RKEditorDataFramePart (parent, this);

	open_chain = 0;
}

void RKEditorDataFrame::enableEditing (bool on) {
	if (on) {
		connect (this, SIGNAL (deleteColumnRequest (int)), this, SLOT (columnDeletionRequested (int)));
		connect (this, SIGNAL (addedColumn (int)), this, SLOT (columnAdded (int)));
		connect (this, SIGNAL (dataAddingRow (int)), this, SLOT (aboutToAddRow (int)));
		connect (this, SIGNAL (dataRemovingRow (int)), this, SLOT (aboutToRemoveRow (int)));
		varview->setEnabled (true);
		dataview->setEnabled (true);
	} else {
		varview->setEnabled (false);
		dataview->setEnabled (false);
		disconnect (this, SIGNAL (deleteColumnRequest (int)), this, SLOT (columnDeletionRequested (int)));
		disconnect (this, SIGNAL (addedColumn (int)), this, SLOT (columnAdded (int)));
		disconnect (this, SIGNAL (dataAddingRow (int)), this, SLOT (aboutToAddRow (int)));
		disconnect (this, SIGNAL (dataRemovingRow (int)), this, SLOT (aboutToRemoveRow (int)));
	}
}

RKEditorDataFrame::~RKEditorDataFrame () {
	RK_TRACE (EDITOR);
	object->setObjectOpened (this, false);
}

void RKEditorDataFrame::flushChanges () {
	RK_TRACE (EDITOR);
	flushEdit ();
}

void RKEditorDataFrame::openObject (RObject *object, bool initialize_to_empty) {
	RK_TRACE (EDITOR);
	flushEdit ();
	RKEditor::object = object;
	object->setObjectOpened (this, true);

	enableEditing (false);
	open_chain = RKGlobals::rInterface ()->startChain (open_chain);
	if (initialize_to_empty) {
		for (int i=0; i < numTrueCols (); ++i) {
			RObject *obj = static_cast<RContainerObject *> (getObject ())->createNewChild (static_cast<RContainerObject *> (getObject ())->validizeName ("var"), this);
			if (obj->isVariable ()) {
				static_cast<RKVariable*> (obj)->setLength (dataview->numTrueRows ());
				setColObject (i, static_cast<RKVariable*> (obj));
				obj->setCreatedInEditor (this);
			} else {
				RK_ASSERT (false);
			}
		}
		pushTable (open_chain);
	}

	// actually, given the object, we already know the child-names. We don't know their order, however, so we better fetch the name-row again.
	RCommand *command = new RCommand ("names (" + object->getFullName () + ")", RCommand::Sync | RCommand::GetStringVector, QString::null, this, GET_NAMES_COMMAND);
	RKGlobals::rInterface ()->issueCommand (command, open_chain);

	// since communication is asynchronous, the rest is done inside
	// processROutput!
}

void RKEditorDataFrame::rCommandDone (RCommand *command) {
	RK_TRACE (EDITOR);

	if (command->getFlags () == GET_NAMES_COMMAND) {
		while (command->stringVectorLength () < numTrueCols ()) {	// get rid of superficial columns
			deleteColumn (0);
		}

		// this is really just a very preliminary HACK. If stringVectorLength () is 0, this can not be a data.frame any longer.
		// abort in order to avoid crash.
		// TODO: actually, we should have a true check for object type each time before opening an object.
		if (!command->stringVectorLength ()) {
			open_chain = RKGlobals::rInterface ()->closeChain (open_chain);
			RKGlobals::editorManager ()->closeEditor (this);
			return;
		}

		// set the names and meta-information
		for (int i = 0; i < command->stringVectorLength (); ++i) {
			qDebug ("alive %d of %d", i+1, command->stringVectorLength ());
			if (numTrueCols () <= i) {
				insertNewColumn ();
			}
			// TODO: make clean
			RKVariable *current_child = static_cast<RKVariable *> (static_cast <RContainerObject*> (getObject ())->findChild (command->getStringVector ()[i]));
			// this is really just a very preliminary HACK. If we find new children now exist, create them now in order to avoid crash..
			// TODO: actually, we should have a true check for object type/structure each time before opening an object.
			if (!current_child) {
				current_child = static_cast<RKVariable *> (static_cast<RContainerObject *> (getObject ())->createNewChild (command->getStringVector ()[i], this));
			}
			if (current_child->isVariable ()) {
				if (!getColObject (i)) {		// if we initialized the table to empty, the object may already exist in our map
					setColObject (i, current_child);
					current_child->setObjectOpened (this, true);
				} else {
					RK_ASSERT (getColObject (i) == current_child);
				}
			} else {
				RK_ASSERT (false);
			}
		}

		open_chain = RKGlobals::rInterface ()->closeChain (open_chain);
		/* make sure enough rows are displayed. Note: Calling QTable::insertRows, since no data should be juggled around, only the number of visible rows is to be changed. */
		if (dataview->numTrueRows () < getColObject (0)->getLength ()) {
			dataview->QTable::insertRows (0, getColObject (0)->getLength () - dataview->numTrueRows ());
		}
		enableEditing (true);
	}
}

void RKEditorDataFrame::pushTable (RCommandChain *sync_chain) {
	RK_TRACE (EDITOR);
	flushEdit ();
	QString command;

	// first push the data-table
	TwinTableMember *table = dataview;
	command = getObject ()->getFullName ();
	command.append (" <- data.frame (");
	
	QString na_vector = "=rep (NA, " + QString::number (getColObject (0)->getLength ()) + ")";
	for (int col=0; col < table->numTrueCols (); col++) {
		command.append (getColObject (col)->getShortName () + na_vector);
		if (col < (table->numTrueCols ()-1)) {
			command.append (", ");
		}
	}
	command.append (")");

	RKGlobals::rInterface ()->issueCommand (new RCommand (command, RCommand::Sync), sync_chain);
	for (int col=0; col < table->numTrueCols (); col++) {
		getColObject (col)->restore (sync_chain);
	}

	// now store the meta-data
	getObject ()->writeMetaData (sync_chain);
}

void RKEditorDataFrame::paste (QByteArray content) {
	RK_TRACE (EDITOR);

	pasteEncoded (content);
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
	RObject *obj = static_cast<RContainerObject *> (getObject ())->createNewChild (static_cast<RContainerObject *> (getObject ())->validizeName (QString::null), this);
	RK_ASSERT (obj->isVariable ());	
	RKGlobals::rInterface ()->issueCommand (new RCommand (".rk.data.frame.insert.column (" + getObject ()->getFullName () + ", \"" + obj->getShortName () + "\", " + QString ().setNum (col+1) + ")", RCommand::App | RCommand::Sync));
	static_cast<RKVariable*> (obj)->setLength (dataview->numTrueRows ());
	obj->setCreatedInEditor (this);

	// TODO: find a nice way to update the list:
	RK_ASSERT (col <= (numTrueCols () - 1));
	for (int i=numTrueCols () - 1; i > col; --i) {
		setColObject (i, getColObject (i-1));
	}
	if (obj->isVariable ()) {
		setColObject (col, static_cast<RKVariable*> (obj));
	} else {
		RK_ASSERT (false);
	}

}

void RKEditorDataFrame::aboutToAddRow (int row) {
	RK_TRACE (EDITOR);
	RKGlobals::rInterface ()->issueCommand (new RCommand (".rk.data.frame.insert.row (" + getObject ()->getFullName () + ", " + QString ().setNum (row+1) + ")", RCommand::App | RCommand::Sync));
}

void RKEditorDataFrame::aboutToRemoveRow (int row) {
	RK_TRACE (EDITOR);
	RKGlobals::rInterface ()->issueCommand (new RCommand (".rk.data.frame.delete.row (" + getObject ()->getFullName () + ", " + QString ().setNum (row+1) + ")", RCommand::App | RCommand::Sync));
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
	object->setObjectOpened (this, false);
	
	for (int i=(col+1); i < numTrueCols (); ++i) {
		setColObject (i-1, getColObject (i));
	}
	setColObject (numTrueCols (), 0);
}

void RKEditorDataFrame::restoreObject (RObject *object) {
	RK_TRACE (EDITOR);
	// for now, simply sync the whole table unconditionally.
	if (object == getObject ()) {
		pushTable (0);
	} else {
		RK_ASSERT (object->isVariable ());
		static_cast<RKVariable*> (object)->restore ();
	}
}

void RKEditorDataFrame::renameObject (RObject *object) {
	RK_TRACE (EDITOR);
	
	if (object == getObject ()) {
		RKGlobals::editorManager ()->setEditorName (this, object->getShortName ());
		return;
	}

	int col = getObjectCol (object);
	varview->updateCell (NAME_ROW, col);
}

void RKEditorDataFrame::addObject (RObject *object) {
	RK_TRACE (EDITOR);
	
	enableEditing (false);
	insertNewColumn ();
	if (object->isVariable ()) {
		setColObject (numTrueCols () - 1, static_cast<RKVariable*> (object));
		object->setObjectOpened (this, true);
	} else {
		RK_ASSERT (false);
	}

	enableEditing (true);
	
	updateObjectMeta (object);
	RObject::ChangeSet *set = new RObject::ChangeSet;
	set->from_index = -1;
	set->to_index = -1;
	RKGlobals::tracker ()->objectDataChanged (object, set);
}

void RKEditorDataFrame::updateObjectMeta (RObject *object) {
	RK_TRACE (EDITOR);
	if (object == getObject ()) return;	// for now: can't update meta on the table itself
	
	int col = getObjectCol (object);
	for (int i=0; i < varview->numTrueRows (); ++i) {
		varview->updateCell (i, col);
	}
}

void RKEditorDataFrame::updateObjectData (RObject *object, RObject::ChangeSet *changes) {
	RK_TRACE (EDITOR);
	
	if (object != getObject ()) {
		int col = getObjectCol (object);
		for (int i=0; i < dataview->numTrueRows (); ++i) {
			dataview->updateCell (i, col);
		}
	}
}

