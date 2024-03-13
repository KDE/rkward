/*
rkeditordataframe - This file is part of the RKWard project. Created: Fri Aug 20 2004
SPDX-FileCopyrightText: 2004-2022 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef RKEDITORDATAFRAME_H
#define RKEDITORDATAFRAME_H

#include "rkeditor.h"
#include "twintable.h"

class TwinTable;
class RCommand;
class RKVariable;

/**
An RKEditor for data.frames.

@author Thomas Friedrichsmeier
*/
class RKEditorDataFrame : public TwinTable {
	Q_OBJECT
public:
/** constructor.
@param object an existing R object
@param parent parent widget */
	RKEditorDataFrame (RContainerObject* object, QWidget *parent);
/** This constructor creates a new (empty) data.frame with the given name and then opens it for editing.
@param new_object_name name of the new data.frame
@param parent parent widget */
	RKEditorDataFrame (const QString& new_object_name, QWidget *parent);
/** destructor */
	~RKEditorDataFrame ();

	void flushChanges () override;

/** Tells the editor to restore the given object in the R-workspace from its copy of the data */
	void restoreObject (RObject *object) override;
private Q_SLOTS:
	void detachModel ();
private:
/// syncs the whole table.
	void pushTable (RCommandChain *sync_chain);
	void commonInit ();
	RCommandChain *open_chain;
	void waitForLoad ();
};

#endif
