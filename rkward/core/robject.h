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
class RCommandChain;

/**
Base class for representations of objects in the R-workspace

@author Thomas Friedrichsmeier
*/

class RObject : public RCommandReceiver {
public:
	RObject(RContainerObject *parent, const QString &name);

	virtual ~RObject();

	enum RObjectType { DataFrame=1, Matrix=2, Array=4, List=8, Container=16, Variable=32, Workspace=64, HasMetaObject=128 };
	
	QString getShortName ();
	virtual QString getFullName ();
	virtual QString getLabel ();
	virtual QString getMetaProperty (const QString &id);
	virtual QString getDescription ();
	
	virtual void setLabel (const QString &value);
	virtual void setMetaProperty (const QString &id, const QString &value);
	
	bool isContainer () { return (type & Container); };
	bool isDataFrame () { return (type & DataFrame); };
	bool isVariable () { return (type & Variable); };
	bool hasMetaObject () { return (type & HasMetaObject); };
	
	bool isOpened () { return (state & OpenedInRKWard); };
	bool isMetaModified () { return (state & MetaModified); };
	bool isDataModified () { return (state & DataModified); };
	virtual void setDataModified ();
	virtual void setMetaModified ();
	bool hasModifiedChildren () { return (state & ChildrenModified); };
	bool needsSyncToR () { return (state & (MetaModified | DataModified | ChildrenModified)); };
	
	typedef QMap<QString, RObject*> RObjectMap;
	
	virtual void updateFromR () = 0;
	virtual void writeMetaData (RCommandChain *chain, bool force=false);
	
	RContainerObject *getContainer () { return (parent); };
	enum RObjectState { OpenedInRKWard=1, MetaModified=2, DataModified=4, ChildrenModified=8 };
	
	virtual int numChildren () { return 0; };
	virtual RObject **children () { return 0; };
protected:
	friend class RContainerObject;
	RContainerObject *parent;
	QString name;
	int type;
	int state;
	
	virtual void getMetaData (RCommandChain *chain);
	virtual QString makeChildName (const QString &short_child_name);
	
	typedef QMap<QString, QString> MetaMap;
	MetaMap *meta_map;
	
	void rCommandDone (RCommand *command);
};

#endif
