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

/**
Internal representation of objects in the R-workspace that contain other objects

@author Thomas Friedrichsmeier
*/

class RContainerObject : public RObject {
public:
    RContainerObject(RContainerObject *parent, const QString &name);

    ~RContainerObject();
	
	QString getLabel ();
	QString getDescription ();

	int numClasses () { return num_classes; };
	QString getClassName (int index) { return classname[index]; };
	QString makeClassString (const QString &sep);
	
	void updateFromR ();

	virtual void childUpdateComplete ();
	
	int numChildren ();
	RObject **children ();
private:
	friend class RObject;
	void typeMismatch (RObject *child, QString childname);
protected:
	RObjectMap childmap;
	// why do I need this to make it compile?!
	friend class RObjectList;
	void addChild (RObject *child, QString childname);
	
	int num_classes;
	QString *classname;
	int num_dimensions;
	int *dimension;
	int num_children_updating;

	void rCommandDone (RCommand *command);
};

#endif
