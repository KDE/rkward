/***************************************************************************
                          robject  -  description
                             -------------------
    begin                : Thu Aug 19 2004
    copyright            : (C) 2004, 2006 by Thomas Friedrichsmeier
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
class RData;

#define ROBJECT_UDPATE_STRUCTURE_COMMAND 1

/**
Base class for representations of objects in the R-workspace. RObject is never used directly (contains pure virtual functions).

@author Thomas Friedrichsmeier
*/

class RObject : public RCommandReceiver {
public:
	RObject (RContainerObject *parent, const QString &name);
	virtual ~RObject ();

/** types of objects, RKWard knows about */
	enum RObjectType {
		DataFrame=1,
		Matrix=2,
		Array=4,
		List=8,
		Container=16,
		Variable=32,
		Workspace=64,
		Function=128,
		Environment=256,
		GlobalEnv=512,
		ToplevelEnv=1024,
		PackageEnv=2048,
		HasMetaObject=4096,
		Misplaced=8192		/** < the object is not in the namespace where it would be expected */
	};

	#define ROBJECT_TYPE_INTERNAL_MASK (RObject::Container | RObject::Variable | RObject::Workspace | RObject::Environment | RObject::Function)
/** @returns false if an object of the given old type cannot represent an object of the given new type (e.g. (new_type & RObjectType::Variable), but (old_type & RObjectType::Container)). */
	static bool isMatchingType (int old_type, int new_type) { return ((old_type & ROBJECT_TYPE_INTERNAL_MASK) == (new_type & ROBJECT_TYPE_INTERNAL_MASK)); };
/** types of variables, RKWard knows about. See \ref RKVariable */
	enum VarType {
		Unknown=0,
		Number=1,
		Factor=2,
		String=3,
		Invalid=4
	};
	
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
	bool isType (int type) { return (RObject::type & type); };
	bool hasMetaObject () { return (type & HasMetaObject); };

/** trigger an update of this and all descendent objects */
	virtual void updateFromR ();

	bool canEdit ();
	bool canRead ();
	bool canRename ();
	bool canRemove ();
	bool isInGlobalEnv ();

	void rename (const QString &new_short_name);
	void remove (bool removed_in_workspace);

	unsigned int numClasses () { return num_classes; };
	QString getClassName (int index) { return classnames[index]; };
	QString makeClassString (const QString &sep);
/** @param class_name the name of the class to check for
@returns true, if the object has (among others) the given class, false otherwise */
	bool inherits (const QString &class_name);

/** get number of dimensions. For simplicity, In RKWard each object is considered to have at least one dimension (but that dimension may be 0 in length) */
	unsigned int numDimensions () { return num_dimensions; };
/** get the length of the given dimension. The object is guaranteed to have at least 1 dimension, so calling getDimension (0) is always safe */
	int getDimension (int index) { return dimensions[index]; };
/** short hand for getDimension (0). Meaningful for one-dimensional objects */
	int getLength () { return dimensions[0]; };

/** A map of objects accessible by their short name. Used in RContainerObject. Defined here for technical reasons. */
	typedef QMap<QString, RObject*> RObjectMap;

/** A map of values to labels. This is used both in regular objects, in which it just represents a map of named values, if any. The more important use is in factors, where it represents the factor levels. Here, the key is always a string representation of a positive integer. */
	typedef QMap<QString, QString> ValueLabels;

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

/** try to find the object as a child object of this object. Default implementation always returns 0, as this is not a container
@param name of the object (relative to this object)
@param is_canonified the object name may usually have to be canonified. Since this function may be called recursively, canonification may already have occured on a higher level. In this case the argument is set to true to avoid some duplicate work. When calling from outside always leave the default false.
@returns a pointer to the object (if found) or 0 if not found */
	virtual RObject *findObject (const QString &name, bool is_canonified=false);
protected:
// why do I need those to compile? I thought they were derived classes!
	friend class RContainerObject;
	friend class RObjectList;
	friend class REnvironmentObject;
	RContainerObject *parent;
	QString name;
	int type;
	int *dimensions;
	unsigned int num_dimensions;
	QString *classnames;
	unsigned int num_classes;

/** generates a (full) name for a child of this object with the given name. */
	virtual QString makeChildName (const QString &short_child_name, bool misplaced=false);

/** Update object to reflect the structure passed in the new_data argument. If the data is mismatching (i.e. can not be accommodated by this type of object) false is returned (calls canAccommodateStructure () internally). In this case you should delete the object, and create a new one.
@returns true if the changes could be done, false if this  */
	virtual bool updateStructure (RData *new_data);

	typedef QMap<QString, QString> MetaMap;
	MetaMap *meta_map;

	virtual bool canAccommodateStructure (RData *new_data);
	bool isValidName (RData *new_data);
	bool isValidType (RData *new_data);

/** handles updating the object name from the given data (common functionality between RContainerObject and RKVariable. This should really never return true, as the name should never change. Hence also raises an assert. Is still useful for it's side effect of detaching and deleting the data from the RData structure after checking it.
@param new_data The data. Make sure it really is the classes field of an .rk.get.structure-command to update classes *before* calling this function! WARNING: the new_data object may get changed during this call. Call canAccommodateStructure () before calling this function!
@returns whether this caused any changes */
	bool updateName (RData *new_data);
/** update type information from the given data.
@param new_data The command. Make sure it really is the classification field of an .rk.get.structure-command to update classes *before* calling this function! WARNING: the new_data object may get changed during this call. Call canAccommodateStructure () before calling this function!
@returns whether this caused any changes */
	bool updateType (RData *new_data);
/** handles updating class names from the given data (common functionality between RContainerObject and RKVariable
@param new_data The data. Make sure it really is the classes field of an .rk.get.structure-command to update classes *before* calling this function! WARNING: the new_data object may get changed during this call. Call canAccommodateStructure () before calling this function!
@returns whether this caused any changes */
	bool updateClasses (RData *new_data);
/** handles updating the meta data from the given data (common functionality between RContainerObject and RKVariable. WARNING: the new_data object may get changed during this call. Call canAccommodateStructure () before calling this function!
@param new_data The data. Make sure it really is the meta field of an .rk.get.structure-command to update classes *before* calling this function!
@returns whether this caused any changes */
	bool updateMeta (RData *new_data);
/** update dimension information from the given data.
@param new_data The command. Make sure it really is the dims field of an .rk.get.structure-command to update classes *before* calling this function! WARNING: the new_data object may get changed during this call. Call canAccommodateStructure () before calling this function!
@returns whether this caused any changes */
	bool updateDimensions (RData *new_data);

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

	void rCommandDone (RCommand *command);
};

typedef RObject* RObjectPtr;

#endif
