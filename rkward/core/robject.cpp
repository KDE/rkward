/*
robject - This file is part of RKWard (https://rkward.kde.org). Created: Thu Aug 19 2004
SPDX-FileCopyrightText: 2004-2019 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/


#include "robject.h"

#include <KLocalizedString>
#include <KMessageBox>

#include "../rbackend/rkrinterface.h"
#include "../rbackend/rkrbackendprotocol_shared.h"
#include "robjectlist.h"
#include "rcontainerobject.h"
#include "rkpseudoobjects.h"
#include "rkvariable.h"
#include "renvironmentobject.h"
#include "rfunctionobject.h"
#include "rkmodificationtracker.h"
#include "rkrownames.h"
#include "../settings/rksettingsmoduleobjectbrowser.h"

#include "../debug.h"

namespace RObjectPrivate {
	QVector<qint32> dim_null (1, 0);
}

/** Proxy to guard against the unlikely - but possible - case that an RObject is deleted while it still has RCommands outstanding.
 *  This is needed, because RObject is not QObject dereived. */
class RObjectLifeTimeGuard {
public:
	RObjectLifeTimeGuard(RObject *object) : command_count(0), object(object) {
		object->guard = this;
	};
	~RObjectLifeTimeGuard() {
		if (object) {
			object->guard = nullptr;
		}
	}
	void addCommandFinishedCallback(RCommand *command, std::function<void(RCommand*)> callback) {
		++command_count;
		QObject::connect(command->notifier(), &RCommandNotifier::commandFinished, [this, callback](RCommand* command) {
			if (object) {
				callback(command);
			}
			if (--command_count <= 0) delete this;
		});
	}
private:
	int command_count;
friend class RObject;
	RObject *object;
};

void RObject::whenCommandFinished(RCommand* command, std::function<void (RCommand *)> callback) {
	if (!guard) {
		guard = new RObjectLifeTimeGuard(this);
	}
	guard->addCommandFinishedCallback(command, callback);
}

// static
QHash<const RObject*, RObject::PseudoObjectType> RObject::pseudo_object_types;
QHash<const RObject*, RSlotsPseudoObject*> RObject::slots_objects;
QHash<const RObject*, REnvironmentObject*> RObject::namespace_objects;
QHash<const RObject*, RKRowNames*> RObject::rownames_objects;

RObject::RObject (RObject *parent, const QString &name) {
	RK_TRACE (OBJECTS);

	RObject::parent = parent;
	RObject::name = name;
	type = 0;
	meta_map = 0;
	contained_objects = 0;
	dimensions = RObjectPrivate::dim_null;	// safe initialization
	guard = nullptr;
}

RObject::~RObject () {
	RK_TRACE (OBJECTS);

	if (guard) {
		RK_DEBUG(OBJECTS, DL_INFO, "object deleted while still waiting for command results");
		guard->object = nullptr;
	}
	if (hasPseudoObject (SlotsObject)) delete slots_objects.take (this);
	if (hasPseudoObject (NamespaceObject)) delete namespace_objects.take (this);
	if (hasPseudoObject (RowNamesObject)) delete rownames_objects.take (this);
}

bool RObject::irregularShortName (const QString &name) {
	// no trace
	const int len = name.length();
	for (int i = 0; i < len; ++i) {
		const QChar c = name.at(i);
		// letters are allowed anywhere in the name
		// underscore is allowed, but not as the first character
		// dot, and digits are allowed, too unless they make the name start with a number (or with ...)
		if(c.isLetter()) continue;
		if(!i) {
			if(c.isDigit()) return true;
			if(c == '.') {
				if(len > 1 && name[1].isDigit()) return true;
				if(name == QStringLiteral("...")) return true;
				continue;
			}
		} else {
			if(c.isDigit()) continue;
			if(c == '.') continue;
			if(c == '_') continue;
		}
		return true;
	}
	return(false);
}

QString RObject::getFullName (int options) const {
	RK_TRACE (OBJECTS);
	return parent->makeChildName (RObject::name, type & Misplaced, options);
}

QString RObject::getLabel () const {
	RK_TRACE (OBJECTS);
	return getMetaProperty ("label");
}

RObject::ObjectList RObject::findObjects (const QStringList &path, bool partial, const QString &op) {
	RK_TRACE (OBJECTS);
	// not a container
	if (op == "@") {
		if (slotsPseudoObject ()) return (slotsPseudoObject ()->findObjects (path, partial, "$"));
	}
	return ObjectList();
}

QString RObject::getMetaProperty (const QString &id) const {
	RK_TRACE (OBJECTS);
	if (meta_map) return (meta_map->value (id));
	return QString ();
}

QString RObject::getDescription () const {
	RK_TRACE (OBJECTS);
	if (meta_map) {
		QString label = meta_map->value ("label");
		if (!label.isEmpty ()) return (getShortName () + " (" + label + ')');
	}
	return getShortName ();
}

QString RObject::getObjectDescription () const {
	RK_TRACE (OBJECTS);

#define ESCS replace ('<', "&lt;")

	QString ret;
	ret.append ("<i>" + getShortName ().ESCS + "</i>");
	ret.append ("<br><b>" + i18n ("Full location:") + " </b>" + getFullName ().ESCS);
	QString lab = getLabel ();
	if (!lab.isEmpty ()) ret.append ("<br><b>" + i18n ("Label:") + " </b>" + lab.ESCS);
	ret.append ("<br><b>" + i18n ("Type:") + " </b>");

	if (isType (Function)) {
		ret.append (i18n ("Function"));
		ret.append ("<br><b>" + i18n ("Usage: ") + " </b>" + getShortName ().ESCS + '(' + static_cast<const RFunctionObject *> (this)->printArgs ().ESCS + ')');
	} else if (isType (DataFrame)) {
		ret.append (i18n ("Data frame"));
	} else if (isType (Array)) {
		ret.append (i18n ("Array"));
	} else if (isType (Matrix)) {
		ret.append (i18n ("Matrix"));
	} else if (isType (List)) {
		ret.append (i18n ("List"));
	} else if (isType (Variable)) {
		ret.append (i18n ("Variable"));
		ret.append ("<br><b>" + i18n ("Data Type:") + " </b>" + typeToText (getDataType ()));
	} else if (isType (Environment)) {
		ret.append (i18n ("Environment"));
	}

	if (isType (Container | Variable)) {
		if (dimensions.size () == 1) {
			ret.append ("<br><b>" + i18n ("Length: ") + " </b>"  + QString::number (dimensions[0]));
		} else if (dimensions.size () > 1) {
			ret.append ("<br><b>" + i18n ("Dimensions: ") + " </b>");
			for (int i=0; i < dimensions.size (); ++i) {
				if (i) ret.append (", ");
				ret.append (QString::number (dimensions[i]));
			}
		}
	}

	ret.append ("<br><b>" + i18n ("Class(es):") + " </b>" + makeClassString (",").ESCS);

	return ret;
}

void RObject::setLabel (const QString &value, bool sync) {
	RK_TRACE (OBJECTS);
	setMetaProperty ("label", value, sync);
}

void RObject::setMetaProperty (const QString &id, const QString &value, bool sync) {
	RK_TRACE (OBJECTS);
	if (value.isEmpty ()) {
		if (meta_map && meta_map->contains (id)) meta_map->remove (id);
		else return;
	} else {
		if (!meta_map) meta_map = new MetaMap;
		else if (meta_map->value (id) == value) return;

		meta_map->insert (id, value);
	}

	if (sync) writeMetaData (0);
	RKModificationTracker::instance()->objectMetaChanged (this);
}

QString RObject::makeClassString (const QString &sep) const {
	RK_TRACE (OBJECTS);
	return (classnames.join (sep));
}

bool RObject::inherits (const QString &class_name) const {
	RK_TRACE (OBJECTS);

	return (classnames.contains (class_name));
}

QString RObject::makeChildName (const QString &short_child_name, bool, int options) const {
	RK_TRACE (OBJECTS);
	if (options & DollarExpansion) {
		if (irregularShortName (short_child_name)) return (getFullName (options) + '$' + rQuote (short_child_name));
		return (getFullName (options) + '$' + short_child_name);  // Do not return list$"member", unless necessary
	}
	return (getFullName (options) + "[[" + rQuote (short_child_name) + "]]");
}

void RObject::writeMetaData (RCommandChain *chain) {
	RK_TRACE (OBJECTS);

	if (!meta_map) return;

	QString map_string;
	if (meta_map->isEmpty ()) {
		map_string.append ("NULL");

		delete meta_map;	// now that it is synced, delete it
		meta_map = 0;
	} else {
		for (MetaMap::const_iterator it = meta_map->constBegin (); it != meta_map->constEnd (); ++it) {
			if (!map_string.isEmpty ()) map_string.append (", ");
			map_string.append (rQuote (it.key ()) + '=' + rQuote (it.value ()));
		}
		map_string = "c (" + map_string + ')';
	}

	RCommand *command = new RCommand (".rk.set.meta (" + getFullName () + ", " + map_string + ')', RCommand::App | RCommand::Sync);
	command->setUpdatesObject(this);
	RInterface::issueCommand (command, chain);
}

void RObject::updateFromR (RCommandChain *chain) {
	RK_TRACE (OBJECTS);

	QString commandstring;
	if (parentObject () == RObjectList::getGlobalEnv ()) {
// We handle objects directly in .GlobalEnv differently. That's to avoid forcing promises, when addressing the object directly. In the long run, .rk.get.structure should be reworked to simply not need the value-argument in any case.
		commandstring = ".rk.get.structure.global (" + rQuote (getShortName ()) + ')';
	} else if (isType(Environment)) {
		REnvironmentObject *env = static_cast<REnvironmentObject*>(this);
		if (isType(PackageEnv) && RKSettingsModuleObjectBrowser::isPackageBlacklisted(env->packageName())) {
			KMessageBox::information (0, i18n ("The package '%1' (probably you just loaded it) is currently blacklisted for retrieving structure information. Practically this means, the objects in this package will not appear in the object browser, and there will be no object name completion or function argument hinting for objects in this package.\nPackages will typically be blacklisted, if they contain huge amount of data, that would take too long to load. To unlist the package, visit Settings->Configure RKWard->Workspace.", env->packageName()), i18n("Package blacklisted"), "packageblacklist" + env->packageName());
			return;
		}
		commandstring = ".rk.get.structure (" + getFullName(DefaultObjectNameOptions) + ", " + rQuote(getShortName());
		if (isType(GlobalEnv)) commandstring += ", envlevel=-1";  // in the .GlobalEnv recurse one more level
		if (isType(PackageEnv)) commandstring += ", namespacename=" + rQuote(env->packageName());
		commandstring += ')';
	} else {
// This is the less common branch, but we do call .rk.get.structure on sub-object, e.g. when fetching more levels in the Workspace Browser, or when calling rk.sync(), explicitly
		commandstring = ".rk.get.structure (" + getFullName () + ", " + rQuote (getShortName ()) + ')';
	}
	RCommand *command = new RCommand(commandstring, RCommand::App | RCommand::Sync | RCommand::GetStructuredData);
	whenCommandFinished(command, [this](RCommand* command) {
		if (command->failed ()) {
			RK_DEBUG (OBJECTS, DL_INFO, "command failed while trying to update object '%s'. No longer present?", getShortName ().toLatin1 ().data ());
			// this may happen, if the object has been removed in the workspace in between
			RKModificationTracker::instance()->removeObject (this, 0, true);
			return;
		}
		if (parent && parent->isContainer()) {
			static_cast<RContainerObject*>(parent)->updateChildStructure(this, command);		// this may result in a delete, so nothing after this!
		} else {
			updateStructure(command);		// no (container) parent can happen for RObjectList and pseudo objects
		}
	});
	RInterface::issueCommand(command, chain);

	type |= Updating;	// will be cleared, implicitly, when the new structure gets set
}

void RObject::fetchMoreIfNeeded (int levels) {
	RK_TRACE (OBJECTS);

	if (isType (Updating)) return;
	if (isType (Incomplete)) {
		updateFromR (0);
		return;
	}
	RSlotsPseudoObject *spo = slotsPseudoObject ();
	if (spo) spo->fetchMoreIfNeeded (levels);
	// Note: We do NOT do the same for any namespaceEnvironment, deliberately
	if (levels <= 0) return;
	if (!isContainer ()) return;
	const RObjectMap children = static_cast<RContainerObject*> (this)->childmap;
	foreach (RObject* child, children) {
		child->fetchMoreIfNeeded (levels - 1);
	}
}

bool RObject::updateStructure (RData *new_data) {
	RK_TRACE (OBJECTS);
	if (new_data->getDataLength () == 0) { // can happen, if the object no longer exists
		return false;
	}

	RK_ASSERT (new_data->getDataLength () >= StorageSizeBasicInfo);
	RK_ASSERT (new_data->getDataType () == RData::StructureVector);

	if (!canAccommodateStructure (new_data)) return false;

	if (isPending ()) type -= Pending;

	bool properties_change = false;

	RData::RDataStorage new_data_data = new_data->structureVector ();
	properties_change = updateName (new_data_data.at (StoragePositionName));
	properties_change = updateType (new_data_data.at (StoragePositionType));
	properties_change = updateClasses (new_data_data.at (StoragePositionClass));
	properties_change = updateMeta (new_data_data.at (StoragePositionMeta));
	properties_change = updateDimensions (new_data_data.at (StoragePositionDims));
	properties_change = updateSlots (new_data_data.at (StoragePositionSlots));

	if (properties_change) RKModificationTracker::instance()->objectMetaChanged (this);
	if (type & NeedDataUpdate) updateDataFromR (0);

	if (type & Incomplete) {
		// If the (new!) type is "Incomplete", it means, the structure getter simply stopped at this point.
		// In case we already have child info, we should update it (TODO: perhaps only, if anything is listening for child objects?)
		if (numChildrenForObjectModel () && (!isType (Updating))) updateFromR (0);
		return true;
	}

	return true;
}

//virtual
void RObject::updateDataFromR (RCommandChain *) {
	RK_TRACE (OBJECTS);

	type -= (type & NeedDataUpdate);
}

void RObject::markDataDirty () {
	RK_TRACE (OBJECTS);

	type |= NeedDataUpdate;
	if (isContainer ()) {
	    RContainerObject* this_container = static_cast<RContainerObject*> (this);
		RObjectMap children = this_container->childmap;
		for (int i = children.size () - 1; i >= 0; --i) {
			children[i]->markDataDirty ();
		}
		if (this_container->hasPseudoObject (RowNamesObject)) this_container->rowNames ()->markDataDirty ();
	}
}

bool RObject::canAccommodateStructure (RData *new_data) {
	RK_TRACE (OBJECTS);
	RK_ASSERT (new_data->getDataLength () >= StorageSizeBasicInfo);
	RK_ASSERT (new_data->getDataType () == RData::StructureVector);

	RData::RDataStorage new_data_data = new_data->structureVector ();
	if (!isValidName (new_data_data.at (StoragePositionName))) return false;
	if (!isValidType (new_data_data.at (StoragePositionType))) return false;
	return true;
}

bool RObject::isValidName (RData *new_data) {
	RK_TRACE (OBJECTS);
	RK_ASSERT (new_data->getDataLength () == 1);
	RK_ASSERT (new_data->getDataType () == RData::StringVector);

	QString new_name = new_data->stringVector ().at (0);
	if (name != new_name) {
		RK_ASSERT (false);
		name = new_name;
		return false;
	}
	return true;
}

bool RObject::updateName (RData *new_data) {
	RK_TRACE (OBJECTS);
	RK_ASSERT (new_data->getDataLength () == 1);
	RK_ASSERT (new_data->getDataType () == RData::StringVector);

	bool changed = false;
	QString new_name = new_data->stringVector ().at (0);
	if (name != new_name) {
		changed = true;
		RK_ASSERT (false);
		name = new_name;
	}
	return changed;
}

bool RObject::isValidType (RData *new_data) const {
	RK_TRACE (OBJECTS);
	RK_ASSERT (new_data->getDataLength () == 1);
	RK_ASSERT (new_data->getDataType () == RData::IntVector);

	int new_type = new_data->intVector ().at (0);
	if (!isMatchingType (type, new_type)) return false;

	return true;
}

bool RObject::updateType (RData *new_data) {
	RK_TRACE (OBJECTS);
	RK_ASSERT (new_data->getDataLength () == 1);
	RK_ASSERT (new_data->getDataType () == RData::IntVector);

	bool changed = false;
	int new_type = new_data->intVector ().at (0);
	if (type & PseudoObject) new_type |= PseudoObject;
	if (type & Misplaced) new_type |= Misplaced;
	if (type & Pending) new_type |= Pending;	// NOTE: why don't we just clear the pending flag, here? Well, we don't want to generate a change notification for this. TODO: rethink the logic, and maybe use an appropriate mask
	if (type & NeedDataUpdate) new_type |= NeedDataUpdate;
	if (type != new_type) {
		changed = true;
		type = new_type;
	}
	return changed;
}

bool RObject::updateClasses (RData *new_data) {
	RK_TRACE (OBJECTS);
	RK_ASSERT (new_data->getDataLength () >= 1);		// or can there be classless objects in R?
	RK_ASSERT (new_data->getDataType () == RData::StringVector);

	bool change = false;

	QStringList new_classes = new_data->stringVector ();
	if (new_classes != classnames) {
		change = true;
		classnames = new_classes;
	}

	return change;
}

bool RObject::updateMeta (RData *new_data) {
	RK_TRACE (OBJECTS);

	RK_ASSERT (new_data->getDataType () == RData::StringVector);

	QStringList data= new_data->stringVector ();
	int len = data.size ();
	bool change = false;
	if (len) {
		if (!meta_map) meta_map = new MetaMap;
		else meta_map->clear ();

		RK_ASSERT (!(len % 2));
		int cut = len/2;
		for (int i=0; i < cut; ++i) {
			meta_map->insert (data.at (i), data.at (i+cut));
		}

		// TODO: only signal change, if there really was a change!
		change = true;
	} else {		// no meta data received
		if (meta_map) {
			delete meta_map;
			meta_map = 0;
			change = true;
		}
	}
	return change;
}

bool RObject::updateDimensions (RData *new_data) {
	RK_TRACE (OBJECTS);
	RK_ASSERT (new_data->getDataLength () >= 1);
	RK_ASSERT (new_data->getDataType () == RData::IntVector);

	QVector<qint32> new_dimensions = new_data->intVector ();
	if (new_dimensions != dimensions) {
		if (new_dimensions.isEmpty ()) {
			if (dimensions != RObjectPrivate::dim_null) {
				dimensions = RObjectPrivate::dim_null;
				return (true);
			}
		} else {
#ifdef __GNUC__
#	warning TODO: ugly hack. Should be moved to RKVariable, somehow.
#endif
			if (type & Variable) static_cast<RKVariable*> (this)->extendToLength (new_dimensions[0]);

			dimensions = new_dimensions;
			return (true);
		}
	}
	return (false);
}

bool RObject::updateSlots (RData *new_data) {
	RK_TRACE (OBJECTS);

	if (new_data->getDataLength ()) {
		RK_ASSERT (new_data->getDataType () == RData::StructureVector);
		bool added = false;
		RSlotsPseudoObject *spo = slotsPseudoObject ();
		if (!spo) {
			spo = new RSlotsPseudoObject (this);
			added = true;
			RKModificationTracker::instance()->lockUpdates (true);
		}
		bool ret = spo->updateStructure (new_data->structureVector ().at (0));
		if (added) {
			RKModificationTracker::instance()->lockUpdates (false);
			setSpecialChildObject (spo, SlotsObject);
		}
		return ret;
	} else if (slotsPseudoObject ()) {
		setSpecialChildObject (0, SlotsObject);
	}
	return false;
}

int RObject::getObjectModelIndexOf (RObject *child) const {
	RK_TRACE (OBJECTS);

	int offset = 0;
	if (isContainer ()) {
		int pos = static_cast<const RContainerObject*> (this)->childmap.indexOf (child);
		if (pos >= 0) return pos + offset;
		offset += static_cast<const RContainerObject*> (this)->childmap.size ();
	}
	if (hasPseudoObject (SlotsObject)) {
		if (child == slotsPseudoObject ()) return offset;
		offset += 1;
	}
	if (hasPseudoObject (NamespaceObject)) {
		if (child == namespaceEnvironment ()) return offset;
		offset += 1;
	}
	if (isType (Workspace)) {
		if (child == static_cast<const RObjectList*> (this)->orphanNamespacesObject ()) return offset;
		offset += 1;
	}
	return -1;
}

int RObject::numChildrenForObjectModel () const {
	RK_TRACE (OBJECTS);

	int ret = isContainer () ? static_cast<const RContainerObject*>(this)->numChildren () : 0;
	if (hasPseudoObject (SlotsObject)) ret += 1;
	if (hasPseudoObject (NamespaceObject)) ret += 1;
	if (isType (Workspace)) ret += 1;	// for the RKOrphanNamespacesObject
	return ret;
}

RObject *RObject::findChildByObjectModelIndex (int index) const {
	if (isContainer ()) {
		const RContainerObject *container = static_cast<const RContainerObject*>(this);
		if (index < container->numChildren ()) return container->findChildByIndex (index);
		index -= container->numChildren ();
	}
	if (hasPseudoObject (SlotsObject)) {
		if (index == 0) return slotsPseudoObject ();
		--index;
	}
	if (hasPseudoObject (NamespaceObject)) {
		if (index == 0) return namespaceEnvironment ();
		--index;
	}
	if (isType (Workspace)) {
		if (index == 0) return static_cast<const RObjectList *> (this)->orphanNamespacesObject ();
		--index;
	}
	return 0;
}

QList <RKEditor*> RObject::editors () const {
	return (RKModificationTracker::instance()->objectEditors (this));
}

void RObject::rename (const QString &new_short_name) {
	RK_TRACE (OBJECTS);
	RK_ASSERT (canRename ());
	static_cast<RContainerObject*> (parent)->renameChild (this, new_short_name);
}

void RObject::setSpecialChildObject (RObject* special, PseudoObjectType special_type) {
	RK_TRACE (OBJECTS);

	RObject *old_special = 0;
	if (special_type == SlotsObject) old_special = slotsPseudoObject ();
	else if (special_type == NamespaceObject) old_special = namespaceEnvironment ();
	else if (special_type == RowNamesObject) old_special = rownames_objects.value (this);
	else RK_ASSERT (false);

	if (special == old_special) return;

	if (old_special) {
		RKModificationTracker::instance()->removeObject (old_special, 0, true);
		RK_ASSERT (!hasPseudoObject (special_type));	// should have been removed in the above statement via RObject::remove()
	}

	if (special) {
		if (special_type == SlotsObject) slots_objects.insert (this, static_cast<RSlotsPseudoObject*> (special));
		else if (special_type == NamespaceObject) namespace_objects.insert (this, static_cast<REnvironmentObject*> (special));
		else if (special_type == RowNamesObject) rownames_objects.insert (this, static_cast<RKRowNames*> (special));
		contained_objects |= special_type;

		if (special->isType (NonVisibleObject)) return;

		int index = getObjectModelIndexOf (special);
		// HACK: Newly added object must not be included in the index before beginAddObject (but must be included above for getObjectModelIncexOf() to work)
		contained_objects -= special_type;
		RKModificationTracker::instance()->beginAddObject (special, this, index);
		contained_objects |= special_type;
		RKModificationTracker::instance()->endAddObject (special, this, index);
	}
}

void RObject::remove (bool removed_in_workspace) {
	RK_TRACE (OBJECTS);
	RK_ASSERT (canRemove () || removed_in_workspace);

	if (isPseudoObject ()) {
		RK_ASSERT (removed_in_workspace);
		PseudoObjectType type = getPseudoObjectType ();
		if (parent->hasPseudoObject (type)) {	// not always true for NamespaceObjects, which the RKOrphanNamespacesObject keeps as regular children!
			if (type == SlotsObject) slots_objects.remove (parent);
			else if (type == NamespaceObject) namespace_objects.remove (parent);
			else if (type == RowNamesObject) rownames_objects.remove (parent);
			parent->contained_objects -= type;
			delete this;
			return;
		}
	}

	static_cast<RContainerObject*> (parent)->removeChild (this, removed_in_workspace);
}

//static
QString RObject::typeToText (RDataType var_type) {
	// TODO: These are non-i18n, and not easily i18n-able due to being used, internally.
	// But they _are_ display strings, too.
	if (var_type == DataUnknown) {
		return "Unknown";
	} else if (var_type == DataNumeric) {
		return "Numeric";
	} else if (var_type == DataCharacter) {
		return "String";
	} else if (var_type == DataFactor) {
		return "Factor";
	} else if (var_type == DataLogical) {
		return "Logical";
	} else {
		RK_ASSERT (false);
		return "Invalid";
	}
}

//static 
RObject::RDataType RObject::textToType (const QString &text) {
	if (text == "Unknown") {
		return DataUnknown;
	} else if (text == "Numeric") {
		return DataNumeric;
	} else if (text == "String") {
		return DataCharacter;
	} else if (text == "Factor") {
		return DataFactor;
	} else if (text == "Logical") {
		return DataLogical;
	} else {
		RK_ASSERT (false);
		return DataUnknown;
	}
}

//static
QString RObject::rQuote (const QString &string) {
	return (RKRSharedFunctionality::quote (string));
}

//static 
QStringList RObject::parseObjectPath (const QString &path) {
	RK_TRACE (OBJECTS);

	QStringList ret;
	QString fragment;

	int end = path.length ();
	QChar quote_char;
	bool escaped = false;
	bool seek_bracket_end = false;
	for (int i = 0; i < end; ++i) {
		QChar c = path.at (i);
		if (quote_char.isNull ()) {
			if (c == '\'' || c == '\"' || c == '`') {
				quote_char = c;
			} else {
				if (!seek_bracket_end) {
					if (c == '$') {
						ret.append (fragment);
						ret.append ("$");
						fragment.clear ();
					} else if (c == '[') {
						ret.append (fragment);
						ret.append ("$");
						fragment.clear ();
						if ((i+1 < end) && (path.at (i+1) == '[')) ++i;
						seek_bracket_end = true;
					} else if (c == ':') {
						ret.append (fragment);
						if ((i+1 < end) && (path.at (i+1) == ':')) ++i;
						if ((i+1 < end) && (path.at (i+1) == ':')) {
							++i;
							ret.append (":::");
						} else ret.append ("::");
						fragment.clear ();
					} else if (c == '@') {
						ret.append (fragment);
						ret.append ("@");
						fragment.clear ();
					} else {
						fragment.append (c);
					}
				} else {
					if (c == ']') {
						if ((i+1 < end) && (path.at (i+1) == ']')) ++i;
						seek_bracket_end = false;
						continue;
					} else {
						fragment.append (c);
					}
				}
			}
		} else {	// inside a quote
			if (c == '\\') escaped = !escaped;
			else {
				if (escaped) {
					if (c == 't') fragment.append ('\t');
					else if (c == 'n') fragment.append ('\n');
					else fragment.append ('\\' + c);
				} else {
					if (c == quote_char) {
						quote_char = QChar ();
					} else {
						fragment.append (c);
					}
				}
			}
		}
	}
	if (!fragment.isEmpty ()) ret.append (fragment);
	RK_DEBUG (OBJECTS, DL_DEBUG, "parsed object path %s into %s", qPrintable (path), qPrintable (ret.join ("-")));
	return ret;
}

//virtual
void RObject::beginEdit () {
	RK_ASSERT (false);
}

//virtual
void RObject::endEdit () {
	RK_ASSERT (false);
}

bool RObject::canWrite () const {
	RK_TRACE (OBJECTS);

	// TODO: find out, if binding is locked:
	// if (isLocked ()) return false;
	return (isInGlobalEnv ());
}

bool RObject::canRead () const {
	RK_TRACE (OBJECTS);

	return (this != RObjectList::getObjectList ());
}
 
bool RObject::canRename () const {
	RK_TRACE (OBJECTS);

	if (isPseudoObject ()) return false;
	if (parent && parent->isSlotsPseudoObject ()) return false;
	// TODO: find out, if binding is locked:
	// if (isLocked ()) return false;
	return (isInGlobalEnv ());
}

bool RObject::canRemove () const {
	RK_TRACE (OBJECTS);

	if (isPseudoObject ()) return false;
	if (parent && parent->isSlotsPseudoObject ()) return false;
	// TODO: find out, if binding is locked:
	// if (isLocked ()) return false;
	return (isInGlobalEnv ());
}

REnvironmentObject* RObject::toplevelEnvironment () const {
	RK_TRACE (OBJECTS);

// could be made recursive instead, but likely it's faster like this
	RObject *o = const_cast<RObject*> (this);	// it's ok, all we need to do is find the toplevel parent
	while (o && (!o->isType (ToplevelEnv))) {
		o = o->parent;
	}
	if (!o) {
		RK_ASSERT (this == RObjectList::getObjectList ());
		return RObjectList::getGlobalEnv ();
	}
	return static_cast<REnvironmentObject*> (o);
}

RObject *RObject::globalEnvSymbol() const {
	RK_TRACE (OBJECTS);
	RObject *o = const_cast<RObject*>(this);	// it's ok, all we need to do is find the toplevel parent
	while (o->parent) {
		if (o->parent == RObjectList::getGlobalEnv()) return o;
		o = o->parent;
	}
	RK_ASSERT(false);
	return nullptr;
}

bool RObject::isInGlobalEnv () const {
	RK_TRACE (OBJECTS);

	RObject* o = toplevelEnvironment ();
	if (o->isType (GlobalEnv)) {
		if (o != this) return true;	// the GlobalEnv is not inside the GlobalEnv!
	}
	return false;
}
