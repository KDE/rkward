/***************************************************************************
                          robject  -  description
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
#ifndef ROBJECT_H
#define ROBJECT_H

#include <qobject.h>

#include <qstring.h>
#include <qmap.h>

#include "../rbackend/rcommandreceiver.h"

class RContainerObject;

/**
Base class for representations of objects in the R-workspace

@author Thomas Friedrichsmeier
*/

// TODO: this dependency on QObject is annoying! It's needed only to get RCommand results. RCommands should not use signals/slots after all!
class RObject : public RCommandReceiver {
public:
	RObject(RContainerObject *parent, const QString &name);

	virtual ~RObject();

	enum RObjectType { DataFrame=1, Matrix=2, Array=4, List=8, Container=16, Variable=32, Workspace=64 };
	
	QString getShortName ();
	virtual QString getFullName ();
	virtual QString getLabel () = 0;
	virtual QString getDescription () = 0;
	
	bool isContainer () { return (type & Container); };
	bool isVariable () { return (type & Variable); };
	
	typedef QMap<QString, RObject*> RObjectMap;
	
	virtual void updateFromR () = 0;
	
	RContainerObject *getContainer () { return (parent); };
	
protected:
	RContainerObject *parent;
	QString name;
	int type;
};

#endif
