/***************************************************************************
                          rfunctionobject  -  description
                             -------------------
    begin                : Wed Apr 26 2006
    copyright            : (C) 2006-2019 by Thomas Friedrichsmeier
    email                : thomas.friedrichsmeier@kdemail.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef RFUNCTION_H
#define RFUNCTION_H

#include "robject.h"

class RCommand;

/**
Internal representation of function objects in the R workspace

@author Thomas Friedrichsmeier
*/

class RFunctionObject : public RObject {
public:
	RFunctionObject (RObject *parent, const QString &name);
	~RFunctionObject ();

/** reimplemented from RObject to handle function arguments */
	bool updateStructure (RData *new_data) override;
	QString printArgs () const;
	QStringList argumentNames () const { return argnames; }
	QStringList argumentDefaults () const { return argvalues; }
protected:
	QStringList argnames;
	QStringList argvalues;
	bool updateArguments (RData *new_data);
};

#endif
