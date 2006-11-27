/***************************************************************************
                          rcontainerobject  -  description
                             -------------------
    begin                : Thu Aug 19 2004
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
#ifndef RCONTAINEROBJECT_H
#define RCONTAINEROBJECT_H

#include "robject.h"

class RCommand;
class RKEditor;

/**
Internal representation of objects in the R-workspace that contain other objects

@author Thomas Friedrichsmeier
*/

class RContainerObject : public RObject {
public:
	RContainerObject (RContainerObject *parent, const QString &name);

	~RContainerObject ();

	void writeChildMetaData (RCommandChain *chain);

	/** update the given child with the given data. Since the child may be mismatching, and may need to be recreated, returns a pointer to the child (old or new) */
	RObject *updateChildStructure (RObject *child, RData *new_data, bool just_created=false);
	RObject *createChildFromStructure (RData *child_data, const QString &child_name);

	/** reimplemented from RObject to also update children */
	bool updateStructure (RData *new_data);

	int numChildren ();
	RObject **children ();

	/** like findObject (), but does not recurse, i.e. only direct children */
	RObject *findChild (const QString &name);
	bool isParentOf (RObject *object, bool recursive=false);
	
	/** creates a new child. Right now only RKVariables (false, false), or data.frames (true, true), or unspecified containers (true, false) can be created.
	API will likely change. The child is NOT created in the workspace. That's your resonsibility. All this function returns is a new RObject* of the given
	type and with the name (if neccessary) changed to a legal value. TODO: checking for and changing illegal names is not yet implemented */
	virtual RObject *createNewChild (const QString &name, RKEditor *creator=0, bool container=false, bool data_frame=false);

	/** returns true, if there are no children in this container. Note: of course the object list may not be up to date! */
	bool isEmpty () { return childmap.isEmpty (); };

	/** given child_name, constructs a name which is as close as possible to the orginial but valid (i.e. not alreay in use, not contaning illegal characters */
	virtual QString validizeName (const QString &child_name, bool unique=true);

	/** reimplemented from RObject to actually search for the object */
	virtual RObject *findObject (const QString &name, bool is_canonified=false);

	/** reimplemented from RObject to actually search for matching objects */
	void findObjectsMatching (const QString &partial_name, RObjectMap *current_list, bool name_is_canonified=false);
protected:
	void updateChildren (RData *new_children);
	RObjectMap childmap;
	// why do I need this to make it compile?!
	friend class RObjectList;
	friend class RObject;
	virtual void renameChild (RObject *object, const QString &new_name);
	virtual void removeChild (RObject *object, bool removed_in_workspace);
	virtual QString removeChildCommand (RObject *object);
	virtual QString renameChildCommand (RObject *object, const QString &new_name);
};

#endif
