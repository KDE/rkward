/*
rcontainerobject - This file is part of the RKWard project. Created: Thu Aug 19 2004
SPDX-FileCopyrightText: 2004-2019 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
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
	bool updateStructure (RData *new_data) override;

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

	/** given child_name, constructs a name which is as close as possible to the orginial but valid (i.e. not already in use, not containing illegal characters */
	QString validizeName (const QString &child_name, bool unique=true) const;

	void moveChild (RObject* child, int from_index, int to_index);

	/** reimplemented from RObject to do nothing at all, including not raising an assert. This is because container objects do not have any edit data, themselves, but may be opened for editing, e.g. as a data.frame */
	void beginEdit () override {};
	/** see beginEdit() */
	void endEdit () override {};
	/** return an RKVariable representing the row-names object for this container */
	RKRowNames* rowNames ();
private:
	/** usually, we do not update the structure of the row.names() from R, as it is always the same. However, we may need to adjust the length, and this hack does that. */
	void updateRowNamesObject ();
protected:
	/** reimplemented from RObject to actually search for matching objects among the children */
	RObject::ObjectList findObjects (const QStringList &path, bool partial, const QString &op) override;

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
