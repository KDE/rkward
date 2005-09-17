/***************************************************************************
                          rkeditor  -  description
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
#ifndef RKEDITOR_H
#define RKEDITOR_H

#include <qwidget.h>

#include "../core/robject.h"

#include <kparts/part.h>
#include <kmdichildview.h>

class RCommandChain;
class RKDrag;

/**
Use a a base class for all widgets that can be used to display and edit RObjects of whatever type.

@author Thomas Friedrichsmeier
*/
class RKEditor : public KMdiChildView {
Q_OBJECT
protected:
    RKEditor (QWidget *parent);

    virtual ~RKEditor ();
public:
/// flushes all pending edit operations and syncs the data to R. Implement in the child classes
	virtual void flushChanges () = 0;
/// returns the object that is being edited in this editor
	RObject *getObject () { return object; };
	
/// editing functions:
	virtual void clearSelected () = 0;
	virtual RKDrag *makeDrag () = 0;
	virtual void paste (QByteArray content) = 0;
	enum PasteMode {PasteEverywhere, PasteToTable, PasteToSelection};
	virtual void setPasteMode (PasteMode mode) = 0;

/** Tells the editor to (unconditionally!) remove the object from its list. */
	virtual void removeObject (RObject *object) = 0;
/** Tells the editor to restore the given object in the R-workspace from its copy of the data */
	virtual void restoreObject (RObject *object) = 0;
/** Tells the editor to (unconditionally!) rename the object (the object already carries the new name, so the editor can read the new name from the object). */
	virtual void renameObject (RObject *object) = 0;
/** Tell the editor to (unconditionally) add the given object to its view */
	virtual void addObject (RObject *object) = 0;
/** Tell the editor to (unconditionally) update its representation of the object meta data */
	virtual void updateObjectMeta (RObject *object) = 0;
/** Tell the editor to (unconditionally) update its representation of the object data (in the range given in the ChangeSet) */
	virtual void updateObjectData (RObject *object, RObject::ChangeSet *changes) = 0;

	KParts::Part *getPart () { return part; };
protected:
friend class RKEditorManager;
/// opens the given object. Implement in the child-classes
	virtual void openObject (RObject *object, bool initialize_to_empty=false) = 0;

	RObject *object;

	KParts::Part *part;
};

#endif
