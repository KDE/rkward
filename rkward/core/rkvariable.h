/***************************************************************************
                          rkvariable  -  description
                             -------------------
    begin                : Thu Aug 12 2004
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
#ifndef RKVARIABLE_H
#define RKVARIABLE_H

#include <qstring.h>

#include "robject.h"

class RContainerObject;

/** Abstract representation of a variable, along with helper functions to get different name/labels/descriptions etc.
The implementation will look a lot different when done, but you can go ahead and use functions like getLabel (), getShortName (),
getFullName (), getTypeString () etc. where appropriate.

@author Thomas Friedrichsmeier
*/
class RKVariable : public RObject {
public:
    RKVariable (RContainerObject *parent, const QString &name);

    ~RKVariable ();
	
	QString getVarTypeString ();
	RObject::VarType getVarType () { return var_type; };
	void setVarType (RObject::VarType, bool sync=true);

	int getLength () { return length; };
	QString getTable ();

	void updateFromR ();
protected:
friend class RContainerObject;
	int length;
	RObject::VarType var_type;
	
	void rCommandDone (RCommand *command);
};

typedef RKVariable* RKVarPtr;

#endif
