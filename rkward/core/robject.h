/*
robject - This file is part of the RKWard project. Created: Thu Aug 19 2004
SPDX-FileCopyrightText: 2004-2019 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef ROBJECT_H
#define ROBJECT_H

#include <qobject.h>

#include <QHash>
#include <QVector>
#include <qmap.h>
#include <qstring.h>

class RSlotsPseudoObject;
class REnvironmentObject;
class RContainerObject;
class RKRowNames;
class RCommandChain;
class RKEditor;
class RData;
class RObjectLifeTimeGuard;
class RCommand;

using namespace Qt::Literals::StringLiterals;

/**
Base class for representations of objects in the R-workspace. RObject is never used directly (contains pure virtual functions).

@author Thomas Friedrichsmeier
*/

class RObject {
  public:
	RObject(RObject *parent, const QString &name);
	virtual ~RObject();

	/** types of objects, RKWard knows about */
	enum RObjectType {
		DataFrame = 1,
		Matrix = 1 << 1,
		Array = 1 << 2,
		List = 1 << 3,
		Container = 1 << 4,
		Variable = 1 << 5,
		Workspace = 1 << 6,
		Function = 1 << 7,
		Environment = 1 << 8,
		GlobalEnv = 1 << 9,
		ToplevelEnv = 1 << 10,
		PackageEnv = 1 << 11,
		//		Misplaced=1 << 12,		/** < the object is not in the namespace where it would be expected */
		S4Object = 1 << 13,
		Numeric = 1 << 14,
		Factor = 2 << 14,
		Character = 3 << 14,
		Logical = 4 << 14,
		DataTypeMask = Numeric | Factor | Character | Logical,
		PseudoObject = 1 << 25,     /** < The object is an internal representation, only, and does not exist in R. Currently, this is the case only for the slots-pseudo object */
		Updating = 1 << 26,         /** < The object is about to be updated from R */
		Incomplete = 1 << 27,       /** < The information on this object is not complete (typically, it's children have not been scanned, yet). */
		NonVisibleObject = 1 << 28, /** < the object is not listed in the object list. Currently, this is only the case for row.names()-objects */
		NeedDataUpdate = 1 << 29,   /** < the object's data should be (re-) fetched from R. The main purpose of this flag is to make sure the data is synced *after* the structure has been synced */
		Pending = 1 << 30           /** < the object is pending, i.e. it has been created in the object list, but we have not seen it in R, yet. This is used by data editors to create the illusion that a new object was added immediately, while in fact it takes some time to create it in the backend. */
	};

	enum RDataType {
		DataUnknown = 0,
		DataNumeric = 1,
		DataFactor = 2,
		DataCharacter = 3,
		DataLogical = 4,

		MinKnownDataType = DataNumeric,
		MaxKnownDataType = DataLogical
	};

	/** For passing data between RKStructureGetter and RObject. Be very careful about changing the values in this enum. It is for better readability / searchability of the code, only. */
	enum {
		StoragePositionName = 0,
		StoragePositionType = 1,
		StoragePositionClass = 2,
		StoragePositionMeta = 3,
		StoragePositionDims = 4,
		StoragePositionSlots = 5,
		StoragePositionChildren = 6,
		StoragePositionNamespace = 7,
		StoragePositionFunArgs = 6,
		StoragePositionFunValues = 7,
		StorageSizeBasicInfo = 6,
	};

	enum PseudoObjectType {
		InvalidPseudoObject = 0,
		SlotsObject = 1,
		NamespaceObject = 1 << 1,
		OrphanNamespacesObject = 1 << 2,
		RowNamesObject = 1 << 3
	};

#define ROBJECT_TYPE_INTERNAL_MASK (RObject::Container | RObject::Variable | RObject::Workspace | RObject::Environment | RObject::Function)
	/** @returns false if an object of the given old type cannot represent an object of the given new type (e.g. (new_type & RObjectType::Variable), but (old_type & RObjectType::Container)). */
	static bool isMatchingType(int old_type, int new_type) { return ((old_type & ROBJECT_TYPE_INTERNAL_MASK) == (new_type & ROBJECT_TYPE_INTERNAL_MASK)); };

	QString getShortName() const { return name; };
	enum ObjectNameOptions {
		DollarExpansion = 1,            /**< Return list members as list$member, instead of list[["member"]]  */
		IncludeEnvirIfNotGlobalEnv = 2, /**< Include package name for objects on the search path  */
		IncludeEnvirForGlobalEnv = 4,   /**< Include ".GlobalEnv" for objects inside globalenv  */
		IncludeEnvirIfMasked = 8,       /**< Include package name for objects that are masked (only applicable for object lists, i.e. getFullNames()) */
		ExplicitSlotsExpansion = 16,    /**< Return slots as slot(object, member), intead of object\@member */
		NoIncludeEnvir = 0,             /**< Label for missing include-envirs */
		DefaultObjectNameOptions = IncludeEnvirIfNotGlobalEnv
	};
	virtual QString getFullName(int name_options = DefaultObjectNameOptions) const;
	QString getLabel() const;
	QString getMetaProperty(const QString &id) const;
	QString getDescription() const;

	void setLabel(const QString &value, bool sync = true);
	void setMetaProperty(const QString &id, const QString &value, bool sync = true);

	bool isContainer() const { return (type & (Container | Environment | Workspace)); };
	bool isDataFrame() const { return (type & DataFrame); };
	bool isVariable() const { return (type & Variable); };
	/** see RObjectType */
	bool isType(int type) const { return (RObject::type & type); };
	bool isPseudoObject() const { return isType(PseudoObject); };
	PseudoObjectType getPseudoObjectType() const { return pseudo_object_types.value(this, InvalidPseudoObject); };
	bool isSlotsPseudoObject() const { return (isPseudoObject() && (getPseudoObjectType() == SlotsObject)); };
	bool isPackageNamespace() const { return (isPseudoObject() && (getPseudoObjectType() == NamespaceObject)); };
	bool hasPseudoObject(const PseudoObjectType type) const { return (contained_objects & type); };
	bool hasMetaObject() const { return (meta_map); };
	/** see RObjectType::Pending */
	bool isPending() const { return type & Pending; };

	/** trigger an update of this and all descendent objects */
	virtual void updateFromR(RCommandChain *chain);
	/** fetch updated data from the backend, if there are any listeners. Default implementation does nothing except clearing the dirty flag */
	virtual void updateDataFromR(RCommandChain *chain);
	/** mark the data of this object and all of its children as dirty (recursively). Dirty data will be updated *after* the new structure update (if the object is opened for editing) */
	void markDataDirty();

	/** Returns the editor of this object, if any, or 0 */
	QList<RKEditor *> editors() const;
	bool canWrite() const;
	bool canRead() const;
	bool canRename() const;
	bool canRemove() const;
	/** returns true, if this object is inside the .GlobalEnv. The .GlobalEnv is not considered to be inside itself. */
	bool isInGlobalEnv() const;
	/** returns the toplevel environment that this object is in. May the same as the object. */
	REnvironmentObject *toplevelEnvironment() const;
	/** returns the R symbol this object belongs to. E.g. for mylist$sublist$vector returns mylist. May be the same as the object. Behavior should be assumed undefined for object outside of globalenv() */
	RObject *globalEnvSymbol() const;

	void rename(const QString &new_short_name);
	void remove(bool removed_in_workspace);

	const QStringList &classNames() const { return classnames; };
	QString makeClassString(const QString &sep) const;
	/** @param class_name the name of the class to check for
	@returns true, if the object has (among others) the given class, false otherwise */
	bool inherits(const QString &class_name) const;

	/** get vector of dimensions. For simplicity, In RKWard each object is considered to have at least one dimension (but that dimension may be 0 in length) */
	const QVector<qint32> &getDimensions() const { return dimensions; };
	/** short hand for getDimension (0). Meaningful for one-dimensional objects */
	int getLength() const { return dimensions[0]; };

	/** return the index of the given child, or -1 if there is no such child */
	int getObjectModelIndexOf(RObject *child) const;
	int numChildrenForObjectModel() const;
	RObject *findChildByObjectModelIndex(int) const;

	/** A QList of RObjects. Internally the same as RObjectMap, but can be considered "public" */
	typedef QList<RObject *> ObjectList;

	/** A map of values to labels. This is used both in regular objects, in which it just represents a map of named values, if any. The more important use is in factors, where it represents the factor levels. Here, the key is always a string representation of a positive integer. */
	typedef QMap<QString, QString> ValueLabels;

	/** write the MetaData to the backend. Commands will be issued in the given chain */
	virtual void writeMetaData(RCommandChain *chain);

	/** Returns the parent of this object. All objects have a parent except for the RObjectList (which returns 0) */
	RObject *parentObject() const { return (parent); };

	RDataType getDataType() const { return (typeToDataType(type)); };
	int getType() const { return type; };
	static RDataType typeToDataType(int ftype) { return ((RDataType)((ftype & DataTypeMask) >> 14)); };
	void setDataType(RDataType new_type) {
		int n_type = type - (type & DataTypeMask);
		type = n_type + (new_type << 14);
	};
	/** returns a textual representation of the given RDataType */
	static QString typeToText(RDataType);
	/** converts the given text to a VarType. Returns Invalid on failure */
	static RDataType textToType(const QString &text);
	/** Returns the given string in quotes, taking care of escaping quotation marks inside the string. */
	static QString rQuote(const QString &string);
	/** Returns a pretty description of the object, and its most important properties. */
	virtual QString getObjectDescription() const;
	/** Parses an object path (such as package::name[["a"]]$b@slot) into its components, returning them as a list (in this case 'package', '::' 'name', '$', 'a', '$', 'b', '@', 'slot'). */
	static QStringList parseObjectPath(const QString &path);
	/** Tests whether the given name is "irregular", i.e. contains spaces, quotes, operators, or the like. @see RContainerObject::validizeName () */
	static bool irregularShortName(const QString &name);
	/** try to find the object as a child object of this object.
	@param name of the object (relative to this object)
	@returns a pointer to the object (if found) or 0 if not found */
	RObject *findObject(const QString &name) { return findObjects(parseObjectPath(name), false, QStringLiteral("$")).value(0); };
	/** Function for code completion: given the partial name, find all objects matching this partial name
	@param partial_name The partial name to look up */
	RObject::ObjectList findObjectsMatching(const QString &partial_name) { return findObjects(parseObjectPath(partial_name), true, QStringLiteral("$")); };
	/** Get full-qualified object names for a list of objects as returned by findObjectsMatching */
	static QStringList getFullNames(const RObject::ObjectList &objects, int options);

	/** Fetch more levels of object representation (if needed). Note: Data is fetched asynchronously.
	@param levels levels to recurse (0 = only direct children). */
	void fetchMoreIfNeeded(int levels = 1);

	/** Representation of changes to an edited object (currently for vector data, only) */
	struct ChangeSet {
		ChangeSet(int from = -1, int to = -1, bool reset = false) : from_index(from), to_index(to), full_reset(reset){};
		int from_index;  /**< first changed index */
		int to_index;    /**< last changed index */
		bool full_reset; /**< Model should do a full reset (e.g. dimensions may have changed) */
	};

	/** generates a (full) name for a child of this object with the given name. */
	virtual QString makeChildName(const QString &short_child_name, int object_name_options = DefaultObjectNameOptions) const;

  protected:
	// why do I need those to compile? I thought they were derived classes!
	friend class RContainerObject;
	friend class RObjectList;
	friend class REnvironmentObject;
	/** A map of objects accessible by index. Used in RContainerObject. Defined here for technical reasons. */
	typedef QList<RObject *> RObjectMap;

	RObject *parent;
	QString name;
	/** or-ed combination of RObjectType flags for this object */
	int type;
	QVector<qint32> dimensions;
	QStringList classnames;
	/** or-ed combination of PseudoObjectType flags of pseudo objects available in this object */
	qint8 contained_objects;
	RSlotsPseudoObject *slotsPseudoObject() const { return (hasPseudoObject(SlotsObject) ? slots_objects.value(this) : nullptr); };
	/** returns the namespace environment for this object. Always returns 0 for objects which are not a package environment! */
	REnvironmentObject *namespaceEnvironment() const { return (hasPseudoObject(NamespaceObject) ? namespace_objects.value(this) : nullptr); };
	void setSpecialChildObject(RObject *special, PseudoObjectType special_type);

	/** Worker function for findObject() and findObjectsMatching().
	 *  @If partial true: Look for partial matches (objects starting with the given pattern), false: look for exact matches, only. */
	virtual ObjectList findObjects(const QStringList &path, bool partial, const QString &op);

	/** Update object to reflect the structure passed in the new_data argument. If the data is mismatching (i.e. can not be accommodated by this type of object) false is returned (calls canAccommodateStructure () internally). In this case you should delete the object, and create a new one.
	@returns true if the changes could be done, false if this  */
	virtual bool updateStructure(RData *new_data);

	typedef QMap<QString, QString> MetaMap;
	MetaMap *meta_map;

	virtual bool canAccommodateStructure(RData *new_data);
	bool isValidName(RData *new_data);
	bool isValidType(RData *new_data) const;

	/** handles updating the object name from the given data (common functionality between RContainerObject and RKVariable. This should really never return true, as the name should never change. Hence also raises an assert. Is still useful for it's side effect of detaching and deleting the data from the RData structure after checking it.
	@param new_data The data. Make sure it really is the classes field of an .rk.get.structure-command to update classes *before* calling this function! WARNING: the new_data object may get changed during this call. Call canAccommodateStructure () before calling this function!
	@returns whether this caused any changes */
	bool updateName(RData *new_data);
	/** update type information from the given data.
	@param new_data The command. Make sure it really is the classification field of an .rk.get.structure-command to update classes *before* calling this function! WARNING: the new_data object may get changed during this call. Call canAccommodateStructure () before calling this function!
	@returns whether this caused any changes */
	virtual bool updateType(RData *new_data);
	/** handles updating class names from the given data (common functionality between RContainerObject and RKVariable
	@param new_data The data. Make sure it really is the classes field of an .rk.get.structure-command to update classes *before* calling this function! WARNING: the new_data object may get changed during this call. Call canAccommodateStructure () before calling this function!
	@returns whether this caused any changes */
	bool updateClasses(RData *new_data);
	/** handles updating the meta data from the given data (common functionality between RContainerObject and RKVariable. WARNING: the new_data object may get changed during this call. Call canAccommodateStructure () before calling this function!
	@param new_data The data. Make sure it really is the meta field of an .rk.get.structure-command to update classes *before* calling this function!
	@returns whether this caused any changes */
	bool updateMeta(RData *new_data);
	/** update dimension information from the given data.
	@param new_data The command. Make sure it really is the dims field of an .rk.get.structure-command to update classes *before* calling this function! WARNING: the new_data object may get changed during this call. Call canAccommodateStructure () before calling this function!
	@returns whether this caused any changes */
	bool updateDimensions(RData *new_data);
	/** update information on slots of this object (if it is an S4 object)
	@param new_data The command. Make sure it really is the slots field of an .rk.get.structure-command to update classes *before* calling this function! WARNING: the new_data object may get changed during this call. Call canAccommodateStructure () before calling this function!
	@returns whether this caused any changes */
	bool updateSlots(RData *new_data);

	friend class RKModificationTracker;
	/** Notify the object that some model needs its data. The object should take care of fetching the data from the backend, unless it already has the data. The default implementation does nothing (raises an assert). */
	virtual void beginEdit();
	/** Notify the object that a model no longer needs its data. If there have been as many endEdit() as beginEdit() calls, the object should discard its data storage. The default implementation does nothing (raises an assert). */
	virtual void endEdit();

	/** wrapper around RCommand::whenFinishe(), needed because RObject is not a Qbject subclass */
	void whenCommandFinished(RCommand *command, std::function<void(RCommand *)> callback);

	/* Storage hashes for special objects which are held by some but not all objects, and thus should not have a pointer
	 * in the class declaration. Some apply only to specific RObject types, but moving storage to the relevant classes, would make it more
	 * difficult to maintain the generic bits. */
	static QHash<const RObject *, RSlotsPseudoObject *> slots_objects;
	static QHash<const RObject *, REnvironmentObject *> namespace_objects;
	static QHash<const RObject *, RKRowNames *> rownames_objects;
	friend class RObjectLifeTimeGuard;
	RObjectLifeTimeGuard *guard;

	friend class RSlotsPseudoObject;
	friend class RKPackageNamespaceObject;
	friend class RKOrphanNamespacesObject;
	friend class RKRowNames;
	static QHash<const RObject *, PseudoObjectType> pseudo_object_types;
};

#endif
