/***************************************************************************
                          renvironmentobject  -  description
                             -------------------
    begin                : Wed Sep 27 2006
    copyright            : (C) 2006, 2009, 2011 by Thomas Friedrichsmeier
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
This class roughly corresponds to an environment in R. It keeps a list of all objects in that environment.

@author Thomas Friedrichsmeier
*/
class REnvironmentObject : public RContainerObject {
public:
	REnvironmentObject (RContainerObject *parent, const QString &name);
	~REnvironmentObject ();

	void updateFromR (RCommandChain *chain);
/** like updateFromR, but only update new / removed symbols from R. Theoretically this could be defined in RContainerObject, but the only use case is for environments. */
	void updateFromR (RCommandChain *chain, const QStringList &current_symbols);

	QString getFullName () const;
	QString makeChildName (const QString &short_child_name, bool misplaced=false) const;
	QString makeChildBaseName (const QString &short_child_name) const;
/** reimplemented from RContainerObject: If this is an environment var, call RContainerObject::writeMetaData (). Else, do nothing. An environment has no meta data. */
	void writeMetaData (RCommandChain *chain);
	QString packageName () const;
protected:
	bool updateStructure (RData *new_data);
/// reimplemented from RContainerObject to call "remove (objectname)" instead of "objectname <- NULL"
	QString removeChildCommand (RObject *object) const;
/// reimplemented from RContainerObject to call "remove (objectname)" instead of "objectname <- NULL"
	QString renameChildCommand (RObject *object, const QString &new_name) const;
friend class RObject;
	void updateNamespace (RData *new_data);
};
 
#endif
