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

	enum RObjectType { DataFrame=1, Matrix=2, Array=4, List=8, Container=16, Variable=32, Workspace=64, Function=128, HasMetaObject=256 };
	enum VarType { Unknown=0, Number=1, String=2, Date=3, Invalid=4 };
	
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
	
	void rename (const QString &new_short_name);
	void remove ();
	
	typedef QMap<QString, RObject*> RObjectMap;
	
	virtual void updateFromR () = 0;
	virtual void writeMetaData (RCommandChain *chain);
	
	RContainerObject *getContainer () { return (parent); };
	
	virtual int numChildren () { return 0; };
	virtual RObject **children () { return 0; };
	
	static QString typeToText (VarType var_type);
	static VarType textToType (const QString &text);
	
	static QString rQuote (const QString &string);
	
	/// For now, the ChangeSet only handles RKVariables!
	struct ChangeSet {
		int from_index;
		int to_index;
	};
protected:
// why do I need those to compile? I thought they were derived classes!
	friend class RContainerObject;
	friend class RObjectList;
	RContainerObject *parent;
	QString name;
	int type;
	
	virtual void getMetaData (RCommandChain *chain);
	virtual QString makeChildName (const QString &short_child_name);
	
	typedef QMap<QString, QString> MetaMap;
	MetaMap *meta_map;
	
	void rCommandDone (RCommand *command);
};

#endif
