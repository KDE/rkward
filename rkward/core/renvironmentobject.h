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
	QString makeChildName (const QString &short_child_name, bool misplaced=false);
/** reimplemented from RContainerObject: If this is an environment var, call RContainerObject::writeMetaData (). Else, do nothing. An environment has no meta data. */
	void writeMetaData (RCommandChain *chain);
/** reimplemented from RContainerObject: If this is the global env, add a watch for the new child */
	RObject *createNewChild (const QString &name, RKEditor *creator=0, bool container=false, bool data_frame=false);
protected:
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
