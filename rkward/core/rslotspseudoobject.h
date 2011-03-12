/***************************************************************************
                          rslotspseudoobject  -  description
                             -------------------
    begin                : Fri Mar 11 2011
    copyright            : (C) 2011 by Thomas Friedrichsmeier
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
#ifndef RSLOTSPSEUDOOBJECT_H
#define RSLOTSPSEUDOOBJECT_H

#include "rcontainerobject.h"

class RCommand;
class RCommandChain;

/**
This class represents a "pseudo" list of the S4 slots of the corresponding parent object. In R, no such object exists.

@author Thomas Friedrichsmeier
*/
class RSlotsPseudoObject : public RContainerObject {
public:
	RSlotsPseudoObject (RObject *parent, const QString &name);
	~RSlotsPseudoObject ();

	QString getFullName () const;
	QString makeChildName (const QString &short_child_name, bool misplaced=false) const;
};
 
#endif
