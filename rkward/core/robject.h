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
class RKEditor;

/**
Base class for representations of objects in the R-workspace. RObject is never used directly (contains pure virtual functions).

@author Thomas Friedrichsmeier
*/

class RObject : public RCommandReceiver {
public:
	RObject(RContainerObject *parent, const QString &name);

	virtual ~RObject();

	enum RObjectType { DataFrame=1, Matrix=2, Array=4, List=8, Container=16, Variable=32, Workspace=64, Function=128, HasMetaObject=256 };
	enum VarType { Unknown=0, Number=1, Factor=2, String=3, Invalid=4 };
	
	QString getShortName ();
	virtual QString getFullName ();
	virtual QString getLabel ();
	virtual QString getMetaProperty (const QString &id);
	virtual QString getDescription ();
	
	virtual void setLabel (const QString &value, bool sync=true);
	virtual void setMetaProperty (const QString &id, const QString &value, bool sync=true);
	
	bool isContainer () { return (type & Container); };
	bool isDataFrame () { return (type & DataFrame); };
	bool isVariable () { return (type & Variable); };
	bool hasMetaObject () { return (type & HasMetaObject); };
	
	void rename (const QString &new_short_name);
	void remove (bool removed_in_workspace);

/** A map of objects accessible by their short name. Used in RContainerObject. Defined here for technical reasons. */
	typedef QMap<QString, RObject*> RObjectMap;

/** Get the data for this object from the backend. Implemented in derived classes. */
	virtual void updateFromR () = 0;
/** write the MetaData to the backend. Commands will be issued in the given chain */
	virtual void writeMetaData (RCommandChain *chain);

/** Returns the parent / container of this object. All objects have a parent except for the RObjectList (which returns 0) */
	RContainerObject *getContainer () { return (parent); };

/** number of child objects. Always 0, reimplemented in RContainerObject */
	virtual int numChildren () { return 0; };
/** array of child objects. Always 0, reimplemented in RContainerObject */
	virtual RObject **children () { return 0; };

/** returns a textual representation of the given VarType */
	static QString typeToText (VarType var_type);
/** converts the given text to a VarType. Returns Invalid on failure */
	static VarType textToType (const QString &text);
/** Returns the given string in quotes, taking care of escaping quotation marks inside the string. */
	static QString rQuote (const QString &string);

/** If the object is being edited, returns that editor (in the future probably a list of editors). Else returns 0 */
	RKEditor *objectOpened ();
/** Tells the object it has been opened (opened=true) or closed (opened=false) by the given editor. If the object is opened by the first editor, it will
automatically take care of fetching its data. When closed by all editors, takes care of de-allocating that memory. */
	void setObjectOpened (RKEditor *editor, bool opened);
/** similar to setObjectOpened, but tells the object it has been created in the given editor. Does not try to fetch data from the backend. */
	void setCreatedInEditor (RKEditor *editor);

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

/** fetches the meta data from the backend */
	virtual void getMetaData (RCommandChain *chain);
/** generates a (full) name for a child of this object with the given name. */
	virtual QString makeChildName (const QString &short_child_name);
	
	typedef QMap<QString, QString> MetaMap;
	MetaMap *meta_map;
	
	void rCommandDone (RCommand *command);
	
/** an instance of this struct is created, when the object is opened for editing. For one thing, it keeps track of which editor(s) are working on the object.
In subclasses like RKVariable, the struct is extended to additionally hold the data of the object, etc. */
	struct EditData {
		RKEditor *editor;
	};
/** see EditData. 0 if the object is not being edited. */
	EditData *data;
/** see EditData. Allocates the data member. To be reimplemented in classes that need more information in the EditData struct */
	virtual void allocateEditData ();
/** companion to allocateEditData (). Fetches all necessary information (data) for this object if to_empty==false. Else initializes the data to empty (NA). Default implementation does nothing. Reimplemented in derived classes. */
	virtual void initializeEditData (bool to_empty=false);
/** see above */
	virtual void discardEditData ();
};

#endif
