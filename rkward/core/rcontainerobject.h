/***************************************************************************
                          rcontainerobject  -  description
                             -------------------
    begin                : Thu Aug 19 2004
    copyright            : (C) 2004-2013 by Thomas Friedrichsmeier
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
#ifndef RCONTAINEROBJECT_H
#define RCONTAINEROBJECT_H

#include "robject.h"

class RCommand;
class RKEditor;
class RKRowNames;

/**
Internal representation of objects in the R-workspace that contain other objects

@author Thomas Friedrichsmeier
*/

class RContainerObject : public RObject {
public:
	RContainerObject (RObject *parent, const QString &name);

	~RContainerObject ();

	/** update the given child with the given data. Since the child may be mismatching, and may need to be recreated, returns a pointer to the child (old or new) */
	RObject *updateChildStructure (RObject *child, RData *new_data, bool just_created=false);
	RObject *createChildFromStructure (RData *child_data, const QString &child_name, int position);

	/** reimplemented from RObject to also update children */
	bool updateStructure (RData *new_data);

	int numChildren () const;
	/** returns true, if there are no children in this container. Note: of course the object list may not be up to date! */
	bool isEmpty () const { return childmap.isEmpty (); };

	/** like findObject (), but does not recurse, i.e. only direct children */
	RObject *findChildByName (const QString &name) const;
	/** fetches the child at the given position. This is very fast. */
	RObject *findChildByIndex (int position) const;
	
	/** creates a new child. Right now only RKVariables (false, false), or data.frames (true, true), or unspecified containers (true, false) can be created.
	API will likely change. The child is NOT created in the workspace. That's your resonsibility. All this function returns is a new RObject* of the given
	type and with the name (if necessary) changed to a legal value. TODO: checking for and changing illegal names is not yet implemented */
	RObject *createPendingChild (const QString &name, int position=-1, bool container=false, bool data_frame=false);

	/** given child_name, constructs a name which is as close as possible to the orginial but valid (i.e. not already in use, not contaning illegal characters */
	QString validizeName (const QString &child_name, bool unique=true) const;

	void moveChild (RObject* child, int from_index, int to_index);

	/** reimplemented from RObject to do nothing at all, including not raising an assert. This is because container objects do not have any edit data, themselves, but may be opened for editing, e.g. as a data.frame */
	void beginEdit () {};
	/** see beginEdit() */
	void endEdit () {};
	/** return an RKVariable representing the row-names object for this container */
	RKRowNames* rowNames ();
private:
	/** usually, we do not update the structure of the row.names() from R, as it is always the same. However, we may need to adjust the length, and this hack does that. */
	void updateRowNamesObject ();
protected:
	/** reimplemented from RObject to actually search for matching objects among the children */
	RObject *findObjects (const QStringList &path, RObjectSearchMap *matches, const QString &op);

	void updateChildren (RData *new_children);
	RObjectMap childmap;
	// why do I need this to make it compile?!
	friend class RObjectList;
	friend class RObject;
	void renameChild (RObject *object, const QString &new_name);
	void removeChild (RObject *object, bool removed_in_workspace);
	virtual QString removeChildCommand (RObject *object) const;
	virtual QString renameChildCommand (RObject *object, const QString &new_name) const;
friend class RKModificationTracker;
	void insertChild (RObject* child, int position);
	void removeChildNoDelete (RObject* child);
};

#endif
