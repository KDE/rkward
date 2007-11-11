/***************************************************************************
                          robject  -  description
                             -------------------
    begin                : Thu Aug 19 2004
    copyright            : (C) 2004, 2006, 2007 by Thomas Friedrichsmeier
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
		Misplaced=8192,		/** < the object is not in the namespace where it would be expected */
		Numeric=1 << 14,
		Factor=2 << 14,
		Character=3 << 14,
		Logical=4 << 14,
		DataTypeMask=Numeric | Factor | Character | Logical,
		NeedDataUpdate=1 << 30,	/** < the object's data should be (re-) fetched from R */
		Pending=1 << 31		/** < the object is pending, i.e. it has been created in the object list, but we have not seen it in R, yet. This is used by data editors to create the illusion that a new object was added immediately, while in fact it takes some time to create it in the backend. */
	};

	enum RDataType {
		DataUnknown=0,
		DataNumeric=1,
		DataFactor=2,
		DataCharacter=3,
		DataLogical=4,

		MinKnownDataType = DataNumeric,
		MaxKnownDataType = DataLogical
	};

	#define ROBJECT_TYPE_INTERNAL_MASK (RObject::Container | RObject::Variable | RObject::Workspace | RObject::Environment | RObject::Function)
/** @returns false if an object of the given old type cannot represent an object of the given new type (e.g. (new_type & RObjectType::Variable), but (old_type & RObjectType::Container)). */
	static bool isMatchingType (int old_type, int new_type) { return ((old_type & ROBJECT_TYPE_INTERNAL_MASK) == (new_type & ROBJECT_TYPE_INTERNAL_MASK)); };
	
	QString getShortName () const { return name; };
	virtual QString getFullName () const;
	virtual QString getBaseName () const;
	QString getLabel () const;
	QString getMetaProperty (const QString &id) const;
	QString getDescription () const;
	
	virtual void setLabel (const QString &value, bool sync=true);
	virtual void setMetaProperty (const QString &id, const QString &value, bool sync=true);
	
	bool isContainer () const { return (type & (Container | Environment | Workspace)); };
	bool isDataFrame () const { return (type & DataFrame); };
	bool isVariable () const { return (type & Variable); };
	/** see RObjectType */
	bool isType (int type) const { return (RObject::type & type); };
	bool hasMetaObject () const { return (type & HasMetaObject); };
	/** see RObjectType::Pending */
	bool isPending () const { return type & Pending; };

/** trigger an update of this and all descendent objects */
	virtual void updateFromR (RCommandChain *chain);
/** fetch updated data from the backend, if there are any listeners. Default implementation does nothing except clearing the dirty flag */
	virtual void updateDataFromR (RCommandChain *chain);
/** mark the data of this object and all of its children as dirty (recursively). Dirty data will be updated *after* the new structure update (if the object is opened for editing) */
	void markDataDirty ();

	bool canEdit () const;
	bool canRead () const;
	bool canRename () const;
	bool canRemove () const;
	bool isInGlobalEnv () const;

	void rename (const QString &new_short_name);
	void remove (bool removed_in_workspace);

	unsigned int numClasses () const { return num_classes; };
	QString getClassName (int index) const { return classnames[index]; };
	QString makeClassString (const QString &sep) const;
/** @param class_name the name of the class to check for
@returns true, if the object has (among others) the given class, false otherwise */
	bool inherits (const QString &class_name) const;

/** get number of dimensions. For simplicity, In RKWard each object is considered to have at least one dimension (but that dimension may be 0 in length) */
	unsigned int numDimensions () const { return num_dimensions; };
/** get the length of the given dimension. The object is guaranteed to have at least 1 dimension, so calling getDimension (0) is always safe */
	int getDimension (int index) const { return dimensions[index]; };
/** short hand for getDimension (0). Meaningful for one-dimensional objects */
	int getLength () const { return dimensions[0]; };

/** A QList of RObjects. Internally the same as RObjectMap, but can be considered "public" */
	typedef QList<RObject*> ObjectList;
	typedef QMap<QString, RObject*> RObjectSearchMap;

/** A map of values to labels. This is used both in regular objects, in which it just represents a map of named values, if any. The more important use is in factors, where it represents the factor levels. Here, the key is always a string representation of a positive integer. */
	typedef QMap<QString, QString> ValueLabels;

/** write the MetaData to the backend. Commands will be issued in the given chain */
	virtual void writeMetaData (RCommandChain *chain);

/** Returns the parent / container of this object. All objects have a parent except for the RObjectList (which returns 0) */
	RContainerObject *getContainer () const { return (parent); };

	RDataType getDataType () const { return (typeToDataType (type)); };
	int getType () const { return type; };
	static RDataType typeToDataType (int ftype) { return ((RDataType) ((ftype & DataTypeMask) >> 14)); };
	void setDataType (RDataType new_type) {
		int n_type = type - (type & DataTypeMask);
		type = n_type + (new_type << 14);
	};
/** returns a textual representation of the given RDataType */
	static QString typeToText (RDataType);
/** converts the given text to a VarType. Returns Invalid on failure */
	static RDataType textToType (const QString &text);
/** Returns the given string in quotes, taking care of escaping quotation marks inside the string. */
	static QString rQuote (const QString &string);
/** Returns a pretty description of the object, and its most important properties. TODO should this be virtual or not? I suppose, it's a close call. For now, we do all work here with casts */
	QString getObjectDescription () const;
/** Returns a canonified name given a non-canoified name. Warning! This is not (necessarily) suitable for submission to
R, only for internal lookup. For submission to R, always use RObject::getFullName (), as it will apply more complicated (and correct) rules depending on object type */
	static QString canonifyName (const QString &from);
/** Function for code completion: given the partial name, find all objects matching this partial name
@param partial_name The partial name to look up
@param current_list A pointer to a valid (but probably initially empty) RObjectMap. Matches will be added to this list
@param name_is_canonified internal parameter. Set to true, if the name to match is already canonfied (else it will be canonified internally) */
	virtual void findObjectsMatching (const QString &partial_name, RObjectSearchMap *current_list, bool name_is_canonified=false) const;

/// For now, the ChangeSet only handles RKVariables!
	struct ChangeSet {
		int from_index;
		int to_index;
	};

/** try to find the object as a child object of this object. Default implementation always returns 0, as this is not a container
@param name of the object (relative to this object)
@param is_canonified the object name may usually have to be canonified. Since this function may be called recursively, canonification may already have occurred on a higher level. In this case the argument is set to true to avoid some duplicate work. When calling from outside always leave the default false.
@returns a pointer to the object (if found) or 0 if not found */
	virtual RObject *findObject (const QString &name, bool is_canonified=false) const;
protected:
// why do I need those to compile? I thought they were derived classes!
	friend class RContainerObject;
	friend class RObjectList;
	friend class REnvironmentObject;
/** A map of objects accessible by index. Used in RContainerObject. Defined here for technical reasons. */
	typedef QList<RObject*> RObjectMap;

	RContainerObject *parent;
	QString name;
	int type;
	int *dimensions;
	unsigned int num_dimensions;
	QString *classnames;
	unsigned int num_classes;

/** generates a (full) name for a child of this object with the given name. */
	virtual QString makeChildName (const QString &short_child_name, bool misplaced=false) const;
	virtual QString makeChildBaseName (const QString &short_child_name) const;

/** Update object to reflect the structure passed in the new_data argument. If the data is mismatching (i.e. can not be accommodated by this type of object) false is returned (calls canAccommodateStructure () internally). In this case you should delete the object, and create a new one.
@returns true if the changes could be done, false if this  */
	virtual bool updateStructure (RData *new_data);

	typedef QMap<QString, QString> MetaMap;
	MetaMap *meta_map;

	virtual bool canAccommodateStructure (RData *new_data);
	bool isValidName (RData *new_data);
	bool isValidType (RData *new_data) const;

/** handles updating the object name from the given data (common functionality between RContainerObject and RKVariable. This should really never return true, as the name should never change. Hence also raises an assert. Is still useful for it's side effect of detaching and deleting the data from the RData structure after checking it.
@param new_data The data. Make sure it really is the classes field of an .rk.get.structure-command to update classes *before* calling this function! WARNING: the new_data object may get changed during this call. Call canAccommodateStructure () before calling this function!
@returns whether this caused any changes */
	bool updateName (RData *new_data);
/** update type information from the given data.
@param new_data The command. Make sure it really is the classification field of an .rk.get.structure-command to update classes *before* calling this function! WARNING: the new_data object may get changed during this call. Call canAccommodateStructure () before calling this function!
@returns whether this caused any changes */
	virtual bool updateType (RData *new_data);
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

friend class RKModificationTracker;
/** Notify the object that some model needs its data. The object should take care of fetching the data from the backend, unless it already has the data. The default implementation does nothing (raises an assert). */
	virtual void beginEdit ();
/** Notify the object that a model no longer needs its data. If there have been as many endEdit() as beginEdit() calls, the object should discard its data storage. The default implementation does nothing (raises an assert). */
	virtual void endEdit ();

	void rCommandDone (RCommand *command);
};

typedef RObject* RObjectPtr;

#endif
