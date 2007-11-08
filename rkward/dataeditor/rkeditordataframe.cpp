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

#include <kmessagebox.h>
#include <klocale.h>

#include "../rbackend/rinterface.h"
#include "../rkglobals.h"
#include "twintable.h"
#include "twintablemember.h"
#include "twintabledatamember.h"
#include "twintablemetamember.h"
#include "../core/robject.h"
#include "../core/robjectlist.h"
#include "../core/rkvariable.h"
#include "../core/rcontainerobject.h"
#include "../core/rkmodificationtracker.h"
#include "rkeditordataframepart.h"
#include "../windows/rkworkplace.h"
#include "../misc/rkstandardicons.h"

#include "../debug.h"

#define LOAD_COMPLETE_COMMAND 1
// warning! numbers above GET_DATA_OFFSET are used to determine, which row, the data should go to!
#define GET_DATA_OFFSET 10

RKEditorDataFrame::RKEditorDataFrame (RObject* object, QWidget *parent) : TwinTable (parent) {
	RK_TRACE (EDITOR);

	commonInit ();

	RK_ASSERT (!object->isPending ());
	openObject (object);
}

RKEditorDataFrame::RKEditorDataFrame (const QString& new_object_name, QWidget* parent) : TwinTable (parent) {
	RK_TRACE (EDITOR);

	commonInit ();

	QString valid = RObjectList::getObjectList ()->validizeName (new_object_name);
	if (valid != new_object_name) KMessageBox::sorry (this, i18n ("The name you specified was already in use or not valid. Renamed to %1", valid), i18n ("Invalid Name"));
	RObject *object = RObjectList::getObjectList ()->createPendingChild (valid, -1, true, true);

// initialize the new object
#warning TODO: call model->insertColumns() instead.
	for (int i=0; i < numTrueCols (); ++i) {
		RObject *child = static_cast<RContainerObject *> (object)->createPendingChild (static_cast<RContainerObject *> (object)->validizeName (QString ()), i);
		if (child->isVariable ()) {
			static_cast<RKVariable*> (child)->setLength (dataview->numTrueRows ());
		} else {
			RK_ASSERT (false);
		}
	}
	pushTable (open_chain);

	openObject (object);
}

void RKEditorDataFrame::commonInit () {
	RK_TRACE (EDITOR);

	setPart (new RKEditorDataFramePart (parent (), this));
	initializeActivationSignals ();

	setCaption (object->getShortName ());
	setWindowIcon (RKStandardIcons::iconForWindow (this));

	open_chain = RKGlobals::rInterface ()->startChain (0);
}

void RKEditorDataFrame::enableEditing (bool on) {
	if (on) {
		connect (this, SIGNAL (deleteColumnRequest (int)), this, SLOT (columnDeletionRequested (int)));
		connect (this, SIGNAL (addedColumn (int)), this, SLOT (columnAdded (int)));
		varview->setEnabled (true);
		dataview->setEnabled (true);
	} else {
		varview->setEnabled (false);
		dataview->setEnabled (false);
		disconnect (this, SIGNAL (deleteColumnRequest (int)), this, SLOT (columnDeletionRequested (int)));
		disconnect (this, SIGNAL (addedColumn (int)), this, SLOT (columnAdded (int)));
	}
}

RKEditorDataFrame::~RKEditorDataFrame () {
	RK_TRACE (EDITOR);
}

void RKEditorDataFrame::flushChanges () {
	RK_TRACE (EDITOR);
	flushEdit ();
}

void RKEditorDataFrame::openObject (RObject *object) {
	RK_TRACE (EDITOR);
	flushEdit ();
	RKEditor::object = object;
	RK_ASSERT (object->isDataFrame ());

	enableEditing (false);

	// trigger fetching of the edit data
	object->markDataDirty ();
	object->updateFromR (open_chain);

	RCommand *command = new RCommand (QString (), RCommand::EmptyCommand | RCommand::Sync | RCommand::GetStringVector, QString (), this, LOAD_COMPLETE_COMMAND);
	RKGlobals::rInterface ()->issueCommand (command, open_chain);
}

void RKEditorDataFrame::rCommandDone (RCommand *command) {
	RK_TRACE (EDITOR);

	if (command->getFlags () == LOAD_COMPLETE_COMMAND) {
		RKGlobals::rInterface ()->closeChain (open_chain);
		open_chain = 0;

		enableEditing (true);
	}
}

void RKEditorDataFrame::pushTable (RCommandChain *sync_chain) {
	RK_TRACE (EDITOR);
	flushEdit ();
	QString command;
#warning TODO: move to model
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
