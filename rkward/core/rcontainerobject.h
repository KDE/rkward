/***************************************************************************
                          rcontainerobject  -  description
                             -------------------
    begin                : Thu Aug 19 2004
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
    RContainerObject(RContainerObject *parent, const QString &name);

    ~RContainerObject();

	int numClasses () { return num_classes; };
	QString getClassName (int index) { return classname[index]; };
	QString makeClassString (const QString &sep);
	
	void writeChildMetaData (RCommandChain *chain);
	
	void updateFromR ();

	virtual void childUpdateComplete ();
	
	int numChildren ();
	RObject **children ();
	
	RObject *findChild (const QString &name);
	bool isParentOf (RObject *object, bool recursive=false);
	
	/** creates a new child. Right now only RKVariables (false, false), or data.frames (true, true), or unspecified containers (true, false) can be created.
	API will likely change. The child is NOT created in the workspace. That's your resonsibility. All this function returns is a new RObject* of the given
	type and with the name (if neccessary) changed to a legal value. TODO: checking for and changing illegal names is not yet implemented */
	RObject *createNewChild (const QString &name, RKEditor *creator=0, bool container=false, bool data_frame=false);
	
	int numDimensions () { return num_dimensions; };
	int getDimension (int index) { return dimension[index]; };

	/** returns true, if there are no children in this container. Note: of course the object list may not be up to date! */
	bool isEmpty () { return childmap.isEmpty (); };
private:
	friend class RObject;
	void typeMismatch (RObject *child, QString childname);
protected:
	RObjectMap childmap;
	// why do I need this to make it compile?!
	friend class RObjectList;
	void addChild (RObject *child, QString childname);
	virtual void renameChild (RObject *object, const QString &new_name);
	virtual void removeChild (RObject *object);
/** given the current list of children (as returned by the "names"-command or similar in derived classes) find out, which children have been removed,
and takes the appropriate measures */
	void checkRemovedChildren (char **current_children, int current_child_count);

	int num_classes;
	QString *classname;
	int num_dimensions;
	int *dimension;
	int num_children_updating;

	void rCommandDone (RCommand *command);
};

#endif
