/***************************************************************************
                          rkeditordataframe  -  description
                             -------------------
    begin                : Fri Aug 20 2004
    copyright            : (C) 2004, 2006 by Thomas Friedrichsmeier
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
#include "../core/rkmodificationtracker.h"
#include "rkeditordataframepart.h"
#include "../windows/rkworkplace.h"

#include "../debug.h"

#define GET_NAMES_COMMAND 1
#define LOAD_COMPLETE_COMMAND 2
// warning! numbers above GET_DATA_OFFSET are used to determine, which row, the data should go to!
#define GET_DATA_OFFSET 10

RKEditorDataFrame::RKEditorDataFrame (QWidget *parent, KParts::Part* part) : TwinTable (parent) {
	RK_TRACE (EDITOR);

	setPart (part);
	initializeActivationSignals ();

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
}

void RKEditorDataFrame::flushChanges () {
	RK_TRACE (EDITOR);
	flushEdit ();
}

void RKEditorDataFrame::openObject (RObject *object, bool initialize_to_empty) {
	RK_TRACE (EDITOR);
	flushEdit ();
	RKEditor::object = object;
	if (initialize_to_empty) {
		object->setCreatedInEditor (this);
	} else {
		object->setObjectOpened (this, true);
	}

	enableEditing (false);
	open_chain = RKGlobals::rInterface ()->startChain (open_chain);

	if (initialize_to_empty) {
		for (int i=0; i < numTrueCols (); ++i) {
			RObject *obj = static_cast<RContainerObject *> (getObject ())->createNewChild (static_cast<RContainerObject *> (getObject ())->validizeName ("var"), i, this);
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

	// KDE4 TODO: this is no longer needed, as objects can now be addressed by their position in the parent
	// actually, given the object, we already know the child-names. We don't know their order, however, so we better fetch the name-row again.
	object->markDataDirty ();
	object->updateFromR (open_chain);
	RCommand *command = new RCommand ("names (" + object->getFullName () + ')', RCommand::Sync | RCommand::GetStringVector, QString::null, this, GET_NAMES_COMMAND);
	RKGlobals::rInterface ()->issueCommand (command, open_chain);

	// since communication is asynchronous, the rest is done inside
	// processROutput!
}

void RKEditorDataFrame::rCommandDone (RCommand *command) {
	RK_TRACE (EDITOR);

	if (command->getFlags () == GET_NAMES_COMMAND) {
		int len = command->getDataLength ();
		while (len < numTrueCols ()) {	// get rid of superficial columns
			deleteColumn (0);
		}

		RK_ASSERT (len);

		// set the names and meta-information
		for (int col = 0; col < len; ++col) {
			if (numTrueCols () <= col) {
				insertNewColumn ();
			}
			RKVariable *current_child = static_cast<RKVariable *> (static_cast <RContainerObject*> (getObject ())->findChildByName (command->getStringVector ()[col]));
			RK_ASSERT (current_child);
			if (current_child->isVariable ()) {
				if (!getColObject (col)) {		// if we initialized the table to empty, the object may already exist in our map
					setColObject (col, current_child);
					current_child->setObjectOpened (this, true);
				} else {
					RK_ASSERT (getColObject (col) == current_child);
				}
			} else {
				RK_ASSERT (false);
			}
		}

		RKGlobals::rInterface ()->closeChain (open_chain);
		/* make sure enough rows are displayed. Note: Calling QTable::insertRows, since no data should be juggled around, only the number of visible rows is to be changed. */
		if (dataview->numTrueRows () < getColObject (0)->getLength ()) {
			dataview->Q3Table::insertRows (0, getColObject (0)->getLength () - dataview->numTrueRows ());
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
	
	QString na_vector = "=as.numeric (rep (NA, " + QString::number (getColObject (0)->getLength ()) + "))";
	for (int col=0; col < table->numTrueCols (); col++) {
		if (col != 0) command.append (", ");
		command.append (getColObject (col)->getShortName () + na_vector);
	}
	command.append (")");

	RKGlobals::rInterface ()->issueCommand (new RCommand (command, RCommand::Sync | RCommand::ObjectListUpdate), sync_chain);
	for (int col=0; col < table->numTrueCols (); col++) {
		getColObject (col)->restore (sync_chain);
	}

	// now store the meta-data
	getObject ()->writeMetaData (sync_chain);
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
	RObject *obj = static_cast<RContainerObject *> (getObject ())->createNewChild (static_cast<RContainerObject *> (getObject ())->validizeName (QString ()), col, this);
	RK_ASSERT (obj->isVariable ());
	RKGlobals::rInterface ()->issueCommand (new RCommand (".rk.data.frame.insert.column (" + getObject ()->getFullName () + ", \"" + obj->getShortName () + "\", " + QString::number (col+1) + ")", RCommand::App | RCommand::Sync));
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
	RKGlobals::rInterface ()->issueCommand (new RCommand (".rk.data.frame.insert.row (" + getObject ()->getFullName () + ", " + QString ().setNum (row+1) + ')', RCommand::App | RCommand::Sync));
}

void RKEditorDataFrame::aboutToRemoveRow (int row) {
	RK_TRACE (EDITOR);
	RKGlobals::rInterface ()->issueCommand (new RCommand (".rk.data.frame.delete.row (" + getObject ()->getFullName () + ", " + QString ().setNum (row+1) + ')', RCommand::App | RCommand::Sync));
}

void RKEditorDataFrame::removeObject (RObject *object) {
	RK_TRACE (EDITOR);
	if (object == getObject ()) {
		// self destruct
		delete this;
		return;
	}
	
	int col = getObjectCol (object);
	RK_ASSERT (col >= 0);
	// for now:
	if (col < 0) return;

	object->setObjectOpened (this, false);
	
	for (int i=(col+1); i < numTrueCols (); ++i) {
		setColObject (i-1, getColObject (i));
	}
	setColObject (numTrueCols () - 1, 0);
	deleteColumn (col);
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
		setCaption (object->getShortName ());
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

#include "rkeditordataframe.moc"
