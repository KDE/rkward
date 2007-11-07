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
#ifndef RKEDITORDATAFRAME_H
#define RKEDITORDATAFRAME_H

#include "rkeditor.h"
#include "twintable.h"
#include "../rbackend/rcommandreceiver.h"

class TwinTable;
class RCommand;
class RKVariable;

/**
An RKEditor for data.frames.

@author Thomas Friedrichsmeier
*/
class RKEditorDataFrame : public TwinTable, public RCommandReceiver {
	Q_OBJECT
public:
/** constructor.
@param object an existing R object
@param parent parent widget */
	RKEditorDataFrame (RObject* object, QWidget *parent);
/** This constructor creates a new (empty) data.frame with the given name and then opens it for editing.
@param new_object_name name of the new data.frame
@param parent parent widget */
	RKEditorDataFrame (const QString& new_object_name, QWidget *parent);
/** destructor */
	~RKEditorDataFrame ();

	void flushChanges ();
	
	//void objectDeleted (RObject *object);
	//void objectMetaModified (RObject *object);

/** Tells the editor to (unconditionally!) remove the object from its list. */
	void removeObject (RObject *object);
/** Tells the editor to restore the given object in the R-workspace from its copy of the data */
	void restoreObject (RObject *object);
/** Tells the editor to (unconditionally!) rename the object (the object already carries the new name, so the editor can read the new name from the object). */
	void renameObject (RObject *object);
/** Tell the editor to (unconditionally) update its representation of the object meta data */
	void updateObjectMeta (RObject *object);
/** Tell the editor to (unconditionally) update its representation of the object data (in the range given in the ChangeSet) */
	void updateObjectData (RObject *object, RObject::ChangeSet *changes);
public slots:
	void columnDeletionRequested (int col);
private:
/// syncs the whole table.
	void pushTable (RCommandChain *sync_chain);
	void commonInit ();
	RCommandChain *open_chain;
	void enableEditing (bool on);
	void updateMetaValue (RObject *obj, int row, int col, bool sync=true);

	void modifyObjectMeta (RObject *object, int column);
protected:
	void openObject (RObject *object);
	void rCommandDone (RCommand *command);
};

#endif
