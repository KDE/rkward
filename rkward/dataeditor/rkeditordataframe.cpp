/***************************************************************************
                          rkeditordataframe  -  description
                             -------------------
    begin                : Fri Aug 20 2004
    copyright            : (C) 2004, 2006, 2010 by Thomas Friedrichsmeier
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
#include "rkvareditmodel.h"
#include "../core/robject.h"
#include "../core/robjectlist.h"
#include "../core/renvironmentobject.h"
#include "../core/rcontainerobject.h"
#include "../misc/rkdummypart.h"
#include "../windows/rkworkplace.h"

#include "../debug.h"

#define LOAD_COMPLETE_COMMAND 1
// warning! numbers above GET_DATA_OFFSET are used to determine, which row, the data should go to!
#define GET_DATA_OFFSET 10

RKEditorDataFrame::RKEditorDataFrame (RContainerObject* object, QWidget *parent) : TwinTable (parent) {
	RK_TRACE (EDITOR);

	commonInit ();

	RK_ASSERT (!object->isPending ());
	RKEditor::object = object;
	RK_ASSERT (object->isDataFrame ());

	RKVarEditDataFrameModel* model = new RKVarEditDataFrameModel (object, this);
	initTable (model, object);
	connect (model, SIGNAL (modelObjectDestroyed()), this, SLOT (deleteLater()));

	waitForLoad ();
}

RKEditorDataFrame::RKEditorDataFrame (const QString& new_object_name, QWidget* parent) : TwinTable (parent) {
	RK_TRACE (EDITOR);

	commonInit ();

	QString valid = RObjectList::getGlobalEnv ()->validizeName (new_object_name);
	if (valid != new_object_name) KMessageBox::sorry (this, i18n ("The name you specified was already in use or not valid. Renamed to %1", valid), i18n ("Invalid Name"));

	RKVarEditDataFrameModel* model = new RKVarEditDataFrameModel (valid, RObjectList::getGlobalEnv (), open_chain, 5, this);

	RKEditor::object = model->getObject ();;
	RK_ASSERT (object->isDataFrame ());

	initTable (model, object);
	connect (model, SIGNAL (modelObjectDestroyed()), this, SLOT (deleteLater()));

	RKGlobals::rInterface ()->closeChain (open_chain);
}

void RKEditorDataFrame::commonInit () {
	RK_TRACE (EDITOR);

	setPart (new RKDummyPart (this, this));
	getPart ()->insertChildClient (this);
	initializeActivationSignals ();

	open_chain = RKGlobals::rInterface ()->startChain (0);
}

RKEditorDataFrame::~RKEditorDataFrame () {
	RK_TRACE (EDITOR);
}

void RKEditorDataFrame::flushChanges () {
	RK_TRACE (EDITOR);
	flushEdit ();
}

void RKEditorDataFrame::waitForLoad () {
	RK_TRACE (EDITOR);
	flushEdit ();

	enableEditing (false);

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

void RKEditorDataFrame::restoreObject (RObject *object) {
	RK_TRACE (EDITOR);

#warning TODO: this interface should be moved to the model for good.
	datamodel->restoreObject (object, 0);
}
