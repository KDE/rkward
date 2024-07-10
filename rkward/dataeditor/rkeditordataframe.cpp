/*
rkeditordataframe - This file is part of RKWard (https://rkward.kde.org). Created: Fri Aug 20 2004
SPDX-FileCopyrightText: 2004-2022 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "rkeditordataframe.h"

#include <kmessagebox.h>
#include <KLocalizedString>

#include "../rbackend/rkrinterface.h"
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

RKEditorDataFrame::RKEditorDataFrame (RContainerObject* object, QWidget *parent) : TwinTable (parent) {
	RK_TRACE (EDITOR);

	commonInit ();

	RK_ASSERT (!object->isPending ());
	RKEditor::object = object;
	RK_ASSERT (object->isDataFrame ());
	setGlobalContextProperty ("current_object", object->getFullName());

	RKVarEditDataFrameModel* model = new RKVarEditDataFrameModel (object, this);
	initTable (model, object);
	connect (model, &RKVarEditDataFrameModel::modelObjectDestroyed, this, &RKEditorDataFrame::detachModel);

	waitForLoad ();
}

RKEditorDataFrame::RKEditorDataFrame (const QString& new_object_name, QWidget* parent) : TwinTable (parent) {
	RK_TRACE (EDITOR);

	commonInit ();

	QString valid = RObjectList::getGlobalEnv ()->validizeName (new_object_name);
	if (valid != new_object_name) KMessageBox::error(this, i18n("The name you specified was already in use or not valid. Renamed to %1", valid), i18n("Invalid Name"));

	RKVarEditDataFrameModel* model = new RKVarEditDataFrameModel (valid, RObjectList::getGlobalEnv (), open_chain, 5, this);

	RKEditor::object = model->getObject ();
	RK_ASSERT (object->isDataFrame ());
	setGlobalContextProperty ("current_object", object->getFullName());

	initTable (model, object);
	connect (model, &RKVarEditDataFrameModel::modelObjectDestroyed, this, &RKEditorDataFrame::deleteLater);

	RInterface::closeChain (open_chain);
	open_chain = nullptr;
}

void RKEditorDataFrame::commonInit () {
	RK_TRACE (EDITOR);

	setPart (new RKDummyPart (this, this));
	getPart ()->insertChildClient (this);
	initializeActivationSignals ();

	open_chain = RInterface::startChain(nullptr);
}

RKEditorDataFrame::~RKEditorDataFrame () {
	RK_TRACE (EDITOR);
	if (open_chain) RInterface::closeChain(open_chain);
}

void RKEditorDataFrame::detachModel () {
	RK_TRACE (EDITOR);

	dataview->setRKModel(nullptr);
	metaview->setRKModel(nullptr);
	deleteLater();
}

void RKEditorDataFrame::flushChanges () {
	RK_TRACE (EDITOR);
	flushEdit ();
}

void RKEditorDataFrame::waitForLoad () {
	RK_TRACE (EDITOR);
	flushEdit ();

	enableEditing (false);

	RInterface::whenAllFinished(this, [this]() {
		RInterface::closeChain (open_chain);
		open_chain = nullptr;
		enableEditing (true);
	}, open_chain);
}

void RKEditorDataFrame::restoreObject (RObject *object) {
	RK_TRACE (EDITOR);

#ifdef __GNUC__
#	warning TODO: this interface should be moved to the model for good.
#endif
	datamodel->restoreObject(object, nullptr);
}

