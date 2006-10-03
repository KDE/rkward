/***************************************************************************
                          renvironmentobject  -  description
                             -------------------
    begin                : Wed Sep 27 2006
    copyright            : (C) 2006 by Thomas Friedrichsmeier
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
#ifndef RENVIRONMENTOBJECT_H
#define RENVIRONMENTOBJECT_H

#include "rcontainerobject.h"

class RCommand;
class RCommandChain;

/**
This class roughly corresponds to an enviroment in R. It keeps a list of all objects in that environment.

@author Thomas Friedrichsmeier
*/
class REnvironmentObject : public RContainerObject {
public:
	REnvironmentObject (RContainerObject *parent, const QString &name);
	~REnvironmentObject ();

	void updateFromR ();

	QString getFullName ();
	QString makeChildName (const QString &short_child_name);
/** reimplemented from RContainerObject: If this is an environment var, call RContainerObject::writeMetaData (). Else, do nothing. An environment has no meta data. */
	void writeMetaData (RCommandChain *chain);

	bool isGlobalEnv () { return (type & GlobalEnv); };
/** 
# search ()
or rather
# loadedNamespaces?! No, namespaces are evil! maybe rename to RKNamespaceObject? No. Let's deal with envirs for now.
# ls (envir=as.environment ("package:base"))
name is base::something

How to deal with those names?
You can't assign like this:
envir::object <- something
Probably it's best not to support assignments outside the .GlobalEnv at all.
It's important to find out, when objects are masked, however. If they are, reading should return a full qualified name. Writing should return a simple name, to allow creation of a masking object in .GlobalEnv

What should be the algorithm?
1) Maybe we should always return the full qualified name.
2) When editing an object in any other env, *first* (suggest to?) make a copy to the .GlobalEnv, and edit that copy
3) Essentially objects in any other environment remain read-only

4) check whether objects are masked, and warn when viewing / editing (only needed for objects outside the global env)

RContainerObject::findObjectsMatching (...) for code completion popups
RContainerObject::canonifyName
*/
protected:
	friend class RObjectList;
	friend class RContainerObject;
	bool updateStructure (RData *new_data);
/** reimplemented from RContainerObject to raise an assert if this is not the isGlobalEnv (). Otherwise calls "remove (objectname)" instead of objectname <- NULL" */
	void renameChild (RObject *object, const QString &new_name);
/** reimplemented from RContainerObject to raise an assert if this is not the isGlobalEnv (). Otherwise calls "remove (objectname)" instead of objectname <- NULL" */
	void removeChild (RObject *object, bool removed_in_workspace);
/// reimplemented from RContainerObject to call "remove (objectname)" instead of "objectname <- NULL"
	QString removeChildCommand (RObject *object);
/// reimplemented from RContainerObject to call "remove (objectname)" instead of "objectname <- NULL"
	QString renameChildCommand (RObject *object, const QString &new_name);
	QString namespace_name;
};
 
#endif
