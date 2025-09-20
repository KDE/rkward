/*
robject - This file is part of RKWard (https://rkward.kde.org). Created: Thu Aug 19 2004
SPDX-FileCopyrightText: 2004-2019 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "robject.h"

#include <KLocalizedString>
#include <KMessageBox>

#include "../rbackend/rkrbackendprotocol_shared.h"
#include "../rbackend/rkrinterface.h"
#include "../settings/rksettingsmoduleobjectbrowser.h"
#include "rcontainerobject.h"
#include "renvironmentobject.h"
#include "rfunctionobject.h"
#include "rkmodificationtracker.h"
#include "rkpseudoobjects.h"
#include "rkrownames.h"
#include "rkvariable.h"
#include "robjectlist.h"

#include "../debug.h"

namespace RObjectPrivate {
QVector<qint32> dim_null(1, 0);
}

/** Proxy to guard against the unlikely - but possible - case that an RObject is deleted while it still has RCommands outstanding.
 *  This is needed, because RObject is not QObject dereived. */
class RObjectLifeTimeGuard {
  public:
	explicit RObjectLifeTimeGuard(RObject *object) : command_count(0), object(object) {
		object->guard = this;
	};
	~RObjectLifeTimeGuard() {
		if (object) {
			object->guard = nullptr;
		}
	}
	void addCommandFinishedCallback(RCommand *command, std::function<void(RCommand *)> callback) {
		++command_count;
		QObject::connect(command->notifier(), &RCommandNotifier::commandFinished, [this, callback](RCommand *command) { // clazy:exclude=connect-3arg-lambda
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

void RObject::whenCommandFinished(RCommand *command, std::function<void(RCommand *)> callback) {
	if (!guard) {
		guard = new RObjectLifeTimeGuard(this);
	}
	guard->addCommandFinishedCallback(command, callback);
}

// static
QHash<const RObject *, RObject::PseudoObjectType> RObject::pseudo_object_types;
QHash<const RObject *, RSlotsPseudoObject *> RObject::slots_objects;
QHash<const RObject *, REnvironmentObject *> RObject::namespace_objects;
QHash<const RObject *, RKRowNames *> RObject::rownames_objects;

RObject::RObject(RObject *parent, const QString &name) {
	RK_TRACE(OBJECTS);

	RObject::parent = parent;
	RObject::name = name;
	type = 0;
	meta_map = nullptr;
	contained_objects = 0;
	dimensions = RObjectPrivate::dim_null; // safe initialization
	guard = nullptr;
}

RObject::~RObject() {
	RK_TRACE(OBJECTS);

	if (guard) {
		RK_DEBUG(OBJECTS, DL_INFO, "object deleted while still waiting for command results");
		guard->object = nullptr;
	}
	if (hasPseudoObject(SlotsObject)) delete slots_objects.take(this);
	if (hasPseudoObject(NamespaceObject)) delete namespace_objects.take(this);
	if (hasPseudoObject(RowNamesObject)) delete rownames_objects.take(this);
}

bool RObject::irregularShortName(const QString &name) {
	// no trace
	const int len = name.length();
	for (int i = 0; i < len; ++i) {
		const QChar c = name.at(i);
		// letters are allowed anywhere in the name
		// underscore is allowed, but not as the first character
		// dot, and digits are allowed, too unless they make the name start with a number (or with ...)
		if (c.isLetter()) continue;
		if (!i) {
			if (c.isDigit()) return true;
			if (c == u'.') {
				if (len > 1 && name[1].isDigit()) return true;
				if (name == "..."_L1) return true;
				continue;
			}
		} else {
			if (c.isDigit()) continue;
			if (c == u'.') continue;
			if (c == u'_') continue;
		}
		return true;
	}
	return (false);
}

QString RObject::getFullName(int options) const {
	RK_TRACE(OBJECTS);
	return parent->makeChildName(RObject::name, options);
}

QString RObject::getLabel() const {
	RK_TRACE(OBJECTS);
	return getMetaProperty(u"label"_s);
}

RObject::ObjectList RObject::findObjects(const QStringList &path, bool partial, const QString &op) {
	RK_TRACE(OBJECTS);
	// not a container
	if (op == QLatin1String("@")) {
		if (slotsPseudoObject()) return (slotsPseudoObject()->findObjects(path, partial, QStringLiteral("$")));
	}
	return ObjectList();
}

QString RObject::getMetaProperty(const QString &id) const {
	RK_TRACE(OBJECTS);
	if (meta_map) return (meta_map->value(id));
	return QString();
}

QString RObject::getDescription() const {
	RK_TRACE(OBJECTS);
	if (meta_map) {
		QString label = meta_map->value(u"label"_s);
		if (!label.isEmpty()) return (getShortName() + u" ("_s + label + u')');
	}
	return getShortName();
}

QString RObject::getObjectDescription() const {
	RK_TRACE(OBJECTS);

#define ESCS replace(u'<', u"&lt;"_s)

	QString ret = u"<b>"_s + i18n("Full location:") + u" </b>"_s + getFullName().ESCS;
	QString lab = getLabel();
	if (!lab.isEmpty()) ret.append(u"<br><b>"_s + i18n("Label:") + u" </b>"_s + lab.ESCS);
	ret.append(u"<br><b>"_s + i18n("Type:") + u" </b>"_s);

	if (isType(Function)) {
		ret.append(i18n("Function"));
		ret.append(u"<br><b>"_s + i18n("Usage: ") + u" </b>"_s + getShortName().ESCS + u'(' + static_cast<const RFunctionObject *>(this)->printArgs().ESCS + u')');
	} else if (isType(DataFrame)) {
		ret.append(i18n("Data frame"));
	} else if (isType(Array)) {
		ret.append(i18n("Array"));
	} else if (isType(Matrix)) {
		ret.append(i18n("Matrix"));
	} else if (isType(List)) {
		ret.append(i18n("List"));
	} else if (isType(Variable)) {
		ret.append(i18nc("Noun; in brackets, data type of the var is stated", "Variable (%1)", typeToText(getDataType())));
	} else if (isType(Environment)) {
		ret.append(i18n("Environment"));
	}

	if (isType(Container | Variable)) {
		if (dimensions.size() == 1) {
			ret.append(u"<br><b>"_s + i18n("Length: ") + u" </b>"_s + QString::number(dimensions[0]));
		} else if (dimensions.size() > 1) {
			ret.append(u"<br><b>"_s + i18n("Dimensions: ") + u" </b>"_s);
			for (int i = 0; i < dimensions.size(); ++i) {
				if (i) ret.append(u", "_s);
				ret.append(QString::number(dimensions[i]));
			}
		}
	}

	if (!isType(Function)) ret.append(u"<br><b>"_s + i18n("Class(es):") + u" </b>"_s + makeClassString(u","_s).ESCS);

	return ret;
}

void RObject::setLabel(const QString &value, bool sync) {
	RK_TRACE(OBJECTS);
	setMetaProperty(u"label"_s, value, sync);
}

void RObject::setMetaProperty(const QString &id, const QString &value, bool sync) {
	RK_TRACE(OBJECTS);
	if (value.isEmpty()) {
		if (meta_map && meta_map->contains(id)) meta_map->remove(id);
		else return;
	} else {
		if (!meta_map) meta_map = new MetaMap;
		else if (meta_map->value(id) == value) return;

		meta_map->insert(id, value);
	}

	if (sync) writeMetaData(nullptr);
	RKModificationTracker::instance()->objectMetaChanged(this);
}

QString RObject::makeClassString(const QString &sep) const {
	RK_TRACE(OBJECTS);
	return (classnames.join(sep));
}

bool RObject::inherits(const QString &class_name) const {
	RK_TRACE(OBJECTS);

	return (classnames.contains(class_name));
}

QString RObject::makeChildName(const QString &short_child_name, int options) const {
	RK_TRACE(OBJECTS);
	if (options & DollarExpansion) {
		if (irregularShortName(short_child_name)) return (getFullName(options) + u'$' + rQuote(short_child_name));
		return (getFullName(options) + u'$' + short_child_name); // Do not return list$"member", unless necessary
	}
	return (getFullName(options) + u"[["_s + rQuote(short_child_name) + u"]]"_s);
}

void RObject::writeMetaData(RCommandChain *chain) {
	RK_TRACE(OBJECTS);

	if (!meta_map) return;

	QString map_string;
	if (meta_map->isEmpty()) {
		map_string.append(u"NULL"_s);

		delete meta_map; // now that it is synced, delete it
		meta_map = nullptr;
	} else {
		for (MetaMap::const_iterator it = meta_map->constBegin(); it != meta_map->constEnd(); ++it) {
			if (!map_string.isEmpty()) map_string.append(u", "_s);
			map_string.append(rQuote(it.key()) + u'=' + rQuote(it.value()));
		}
		map_string = u"c ("_s + map_string + u')';
	}

	RCommand *command = new RCommand(u".rk.set.meta ("_s + getFullName() + u", "_s + map_string + u')', RCommand::App | RCommand::Sync);
	command->setUpdatesObject(this);
	RInterface::issueCommand(command, chain);
}

void RObject::updateFromR(RCommandChain *chain) {
	RK_TRACE(OBJECTS);

	QString commandstring;
	if (parentObject() == RObjectList::getGlobalEnv()) {
		// We handle objects directly in .GlobalEnv differently. That's to avoid forcing promises, when addressing the object directly. In the long run,
		// .rk.get.structure should be reworked to simply not need the value-argument in any case.
		commandstring = u".rk.get.structure.global ("_s + rQuote(getShortName()) + u')';
	} else if (isType(Environment)) {
		REnvironmentObject *env = static_cast<REnvironmentObject *>(this);
		if (isType(PackageEnv) && RKSettingsModuleObjectBrowser::isPackageBlacklisted(env->packageName())) {
			KMessageBox::information(
			    nullptr,
			    i18n("The package '%1' (probably you just loaded it) is currently blacklisted for retrieving structure information. Practically this means, "
			         "the objects in this package will not appear in the object browser, and there will be no object name completion or function argument "
			         "hinting for objects in this package.\nPackages will typically be blacklisted, if they contain huge amount of data, that would take too "
			         "long to load. To unlist the package, visit Settings->Configure RKWard->Workspace.",
			         env->packageName()),
			    i18n("Package blacklisted"), u"packageblacklist"_s + env->packageName());
			return;
		}
		commandstring = u".rk.get.structure ("_s + getFullName(DefaultObjectNameOptions) + u", "_s + rQuote(getShortName());
		if (isType(GlobalEnv)) commandstring += u", envlevel=-1"_s; // in the .GlobalEnv recurse one more level
		if (isType(PackageEnv)) commandstring += u", namespacename="_s + rQuote(env->packageName());
		commandstring += u')';
	} else {
		// This is the less common branch, but we do call .rk.get.structure on sub-object, e.g. when fetching more levels in the Workspace Browser, or when
		// calling rk.sync(), explicitly
		commandstring = u".rk.get.structure ("_s + getFullName() + u", "_s + rQuote(getShortName()) + u')';
	}
	RCommand *command = new RCommand(commandstring, RCommand::App | RCommand::Sync | RCommand::GetStructuredData);
	whenCommandFinished(command, [this](RCommand *command) {
		if (command->failed()) {
			RK_DEBUG(OBJECTS, DL_INFO, "command failed while trying to update object '%s'. No longer present?", getShortName().toLatin1().data());
			// this may happen, if the object has been removed in the workspace in between
			RKModificationTracker::instance()->removeObject(this, nullptr, true);
			return;
		}
		if (parent && parent->isContainer()) {
			static_cast<RContainerObject *>(parent)->updateChildStructure(this, command); // this may result in a delete, so nothing after this!
		} else {
			updateStructure(command); // no (container) parent can happen for RObjectList and pseudo objects
		}
	});
	RInterface::issueCommand(command, chain);

	type |= Updating; // will be cleared, implicitly, when the new structure gets set
}

void RObject::fetchMoreIfNeeded(int levels) {
	RK_TRACE(OBJECTS);

	if (isType(Updating)) return;
	if (isType(Incomplete)) {
		updateFromR(nullptr);
		return;
	}
	RSlotsPseudoObject *spo = slotsPseudoObject();
	if (spo) spo->fetchMoreIfNeeded(levels);
	// Note: We do NOT do the same for any namespaceEnvironment, deliberately
	if (levels <= 0) return;
	if (!isContainer()) return;
	const RObjectMap children = static_cast<RContainerObject *>(this)->childmap;
	for (RObject *child : children) {
		child->fetchMoreIfNeeded(levels - 1);
	}
}

bool RObject::updateStructure(RData *new_data) {
	RK_TRACE(OBJECTS);
	if (new_data->getDataLength() == 0) { // can happen, if the object no longer exists
		return false;
	}

	RK_ASSERT(new_data->getDataLength() >= StorageSizeBasicInfo);
	RK_ASSERT(new_data->getDataType() == RData::StructureVector);

	if (!canAccommodateStructure(new_data)) return false;

	if (isPending()) type -= Pending;

	bool properties_change = false;

	RData::RDataStorage new_data_data = new_data->structureVector();
	properties_change = updateName(new_data_data.at(StoragePositionName));
	properties_change |= updateType(new_data_data.at(StoragePositionType));
	properties_change |= updateClasses(new_data_data.at(StoragePositionClass));
	properties_change |= updateMeta(new_data_data.at(StoragePositionMeta));
	properties_change |= updateDimensions(new_data_data.at(StoragePositionDims));
	properties_change |= updateSlots(new_data_data.at(StoragePositionSlots));

	if (properties_change) RKModificationTracker::instance()->objectMetaChanged(this);
	if (type & NeedDataUpdate) updateDataFromR(nullptr);

	if (type & Incomplete) {
		// If the (new!) type is "Incomplete", it means, the structure getter simply stopped at this point.
		// In case we already have child info, we should update it (TODO: perhaps only, if anything is listening for child objects?)
		if (numChildrenForObjectModel() && (!isType(Updating))) updateFromR(nullptr);
		return true;
	}

	return true;
}

// virtual
void RObject::updateDataFromR(RCommandChain *) {
	RK_TRACE(OBJECTS);

	type -= (type & NeedDataUpdate);
}

void RObject::markDataDirty() {
	RK_TRACE(OBJECTS);

	type |= NeedDataUpdate;
	if (isContainer()) {
		RContainerObject *this_container = static_cast<RContainerObject *>(this);
		RObjectMap children = this_container->childmap;
		for (int i = children.size() - 1; i >= 0; --i) {
			children[i]->markDataDirty();
		}
		if (this_container->hasPseudoObject(RowNamesObject)) this_container->rowNames()->markDataDirty();
	}
}

bool RObject::canAccommodateStructure(RData *new_data) {
	RK_TRACE(OBJECTS);
	RK_ASSERT(new_data->getDataLength() >= StorageSizeBasicInfo);
	RK_ASSERT(new_data->getDataType() == RData::StructureVector);

	RData::RDataStorage new_data_data = new_data->structureVector();
	if (!isValidName(new_data_data.at(StoragePositionName))) return false;
	if (!isValidType(new_data_data.at(StoragePositionType))) return false;
	return true;
}

bool RObject::isValidName(RData *new_data) {
	RK_TRACE(OBJECTS);
	RK_ASSERT(new_data->getDataLength() == 1);
	RK_ASSERT(new_data->getDataType() == RData::StringVector);

	QString new_name = new_data->stringVector().at(0);
	if (name != new_name) {
		RK_ASSERT(false);
		name = new_name;
		return false;
	}
	return true;
}

bool RObject::updateName(RData *new_data) {
	RK_TRACE(OBJECTS);
	RK_ASSERT(new_data->getDataLength() == 1);
	RK_ASSERT(new_data->getDataType() == RData::StringVector);

	bool changed = false;
	QString new_name = new_data->stringVector().at(0);
	if (name != new_name) {
		changed = true;
		RK_ASSERT(false);
		name = new_name;
	}
	return changed;
}

bool RObject::isValidType(RData *new_data) const {
	RK_TRACE(OBJECTS);
	RK_ASSERT(new_data->getDataLength() == 1);
	RK_ASSERT(new_data->getDataType() == RData::IntVector);

	int new_type = new_data->intVector().at(0);
	if (!isMatchingType(type, new_type)) return false;

	return true;
}

bool RObject::updateType(RData *new_data) {
	RK_TRACE(OBJECTS);
	RK_ASSERT(new_data->getDataLength() == 1);
	RK_ASSERT(new_data->getDataType() == RData::IntVector);

	bool changed = false;
	int new_type = new_data->intVector().at(0);
	if (type & PseudoObject) new_type |= PseudoObject;
	if (type & Pending) new_type |= Pending; // NOTE: why don't we just clear the pending flag, here? Well, we don't want to generate a change notification for this. TODO: rethink the logic, and maybe use an appropriate mask
	if (type & NeedDataUpdate) new_type |= NeedDataUpdate;
	if (type != new_type) {
		changed = true;
		type = new_type;
	}
	return changed;
}

bool RObject::updateClasses(RData *new_data) {
	RK_TRACE(OBJECTS);
	RK_ASSERT(new_data->getDataLength() >= 1); // or can there be classless objects in R?
	RK_ASSERT(new_data->getDataType() == RData::StringVector);

	bool change = false;

	QStringList new_classes = new_data->stringVector();
	if (new_classes != classnames) {
		change = true;
		classnames = new_classes;
	}

	return change;
}

bool RObject::updateMeta(RData *new_data) {
	RK_TRACE(OBJECTS);

	RK_ASSERT(new_data->getDataType() == RData::StringVector);

	QStringList data = new_data->stringVector();
	int len = data.size();
	bool change = false;
	if (len) {
		if (!meta_map) meta_map = new MetaMap;
		else meta_map->clear();

		RK_ASSERT(!(len % 2));
		int cut = len / 2;
		for (int i = 0; i < cut; ++i) {
			meta_map->insert(data.at(i), data.at(i + cut));
		}

		// TODO: only signal change, if there really was a change!
		change = true;
	} else { // no meta data received
		if (meta_map) {
			delete meta_map;
			meta_map = nullptr;
			change = true;
		}
	}
	return change;
}

bool RObject::updateDimensions(RData *new_data) {
	RK_TRACE(OBJECTS);
	RK_ASSERT(new_data->getDataLength() >= 1);
	RK_ASSERT(new_data->getDataType() == RData::IntVector);

	QVector<qint32> new_dimensions = new_data->intVector();
	if (new_dimensions != dimensions) {
		if (new_dimensions.isEmpty()) {
			if (dimensions != RObjectPrivate::dim_null) {
				dimensions = RObjectPrivate::dim_null;
				return (true);
			}
		} else {
			// TODO / HACK: Should be move to RKVariable, instead
			if (type & Variable) static_cast<RKVariable *>(this)->extendToLength(new_dimensions[0]);

			dimensions = new_dimensions;
			return (true);
		}
	}
	return (false);
}

bool RObject::updateSlots(RData *new_data) {
	RK_TRACE(OBJECTS);

	if (new_data->getDataLength()) {
		RK_ASSERT(new_data->getDataType() == RData::StructureVector);
		bool added = false;
		RSlotsPseudoObject *spo = slotsPseudoObject();
		if (!spo) {
			spo = new RSlotsPseudoObject(this);
			added = true;
			RKModificationTracker::instance()->lockUpdates(true);
		}
		bool ret = spo->updateStructure(new_data->structureVector().at(0));
		if (added) {
			RKModificationTracker::instance()->lockUpdates(false);
			setSpecialChildObject(spo, SlotsObject);
		}
		return ret;
	} else if (slotsPseudoObject()) {
		setSpecialChildObject(nullptr, SlotsObject);
	}
	return false;
}

int RObject::getObjectModelIndexOf(RObject *child) const {
	RK_TRACE(OBJECTS);

	int offset = 0;
	if (isContainer()) {
		int pos = static_cast<const RContainerObject *>(this)->childmap.indexOf(child);
		if (pos >= 0) return pos + offset;
		offset += static_cast<const RContainerObject *>(this)->childmap.size();
	}
	if (hasPseudoObject(SlotsObject)) {
		if (child == slotsPseudoObject()) return offset;
		offset += 1;
	}
	if (hasPseudoObject(NamespaceObject)) {
		if (child == namespaceEnvironment()) return offset;
		offset += 1;
	}
	if (isType(Workspace)) {
		if (child == static_cast<const RObjectList *>(this)->orphanNamespacesObject()) return offset;
	}
	return -1;
}

int RObject::numChildrenForObjectModel() const {
	RK_TRACE(OBJECTS);

	int ret = isContainer() ? static_cast<const RContainerObject *>(this)->numChildren() : 0;
	if (hasPseudoObject(SlotsObject)) ret += 1;
	if (hasPseudoObject(NamespaceObject)) ret += 1;
	if (isType(Workspace)) ret += 1; // for the RKOrphanNamespacesObject
	return ret;
}

RObject *RObject::findChildByObjectModelIndex(int index) const {
	if (isContainer()) {
		const RContainerObject *container = static_cast<const RContainerObject *>(this);
		if (index < container->numChildren()) return container->findChildByIndex(index);
		index -= container->numChildren();
	}
	if (hasPseudoObject(SlotsObject)) {
		if (index == 0) return slotsPseudoObject();
		--index;
	}
	if (hasPseudoObject(NamespaceObject)) {
		if (index == 0) return namespaceEnvironment();
		--index;
	}
	if (isType(Workspace)) {
		if (index == 0) return static_cast<const RObjectList *>(this)->orphanNamespacesObject();
	}
	return nullptr;
}

QList<RKEditor *> RObject::editors() const {
	return (RKModificationTracker::instance()->objectEditors(this));
}

void RObject::rename(const QString &new_short_name) {
	RK_TRACE(OBJECTS);
	RK_ASSERT(canRename());
	static_cast<RContainerObject *>(parent)->renameChild(this, new_short_name);
}

void RObject::setSpecialChildObject(RObject *special, PseudoObjectType special_type) {
	RK_TRACE(OBJECTS);

	RObject *old_special = nullptr;
	if (special_type == SlotsObject) old_special = slotsPseudoObject();
	else if (special_type == NamespaceObject) old_special = namespaceEnvironment();
	else if (special_type == RowNamesObject) old_special = rownames_objects.value(this);
	else RK_ASSERT(false);

	if (special == old_special) return;

	if (old_special) {
		RKModificationTracker::instance()->removeObject(old_special, nullptr, true);
		RK_ASSERT(!hasPseudoObject(special_type)); // should have been removed in the above statement via RObject::remove()
	}

	if (special) {
		if (special_type == SlotsObject) slots_objects.insert(this, static_cast<RSlotsPseudoObject *>(special));
		else if (special_type == NamespaceObject) namespace_objects.insert(this, static_cast<REnvironmentObject *>(special));
		else if (special_type == RowNamesObject) rownames_objects.insert(this, static_cast<RKRowNames *>(special));
		contained_objects |= special_type;

		if (special->isType(NonVisibleObject)) return;

		int index = getObjectModelIndexOf(special);
		// HACK: Newly added object must not be included in the index before beginAddObject (but must be included above for getObjectModelIncexOf() to work)
		contained_objects -= special_type;
		RKModificationTracker::instance()->beginAddObject(special, this, index);
		contained_objects |= special_type;
		RKModificationTracker::instance()->endAddObject(special, this, index);
	}
}

void RObject::remove(bool removed_in_workspace) {
	RK_TRACE(OBJECTS);
	RK_ASSERT(canRemove() || removed_in_workspace);

	if (isPseudoObject()) {
		RK_ASSERT(removed_in_workspace);
		PseudoObjectType type = getPseudoObjectType();
		if (parent->hasPseudoObject(type)) { // not always true for NamespaceObjects, which the RKOrphanNamespacesObject keeps as regular children!
			if (type == SlotsObject) slots_objects.remove(parent);
			else if (type == NamespaceObject) namespace_objects.remove(parent);
			else if (type == RowNamesObject) rownames_objects.remove(parent);
			parent->contained_objects -= type;
			delete this;
			return;
		}
	}

	static_cast<RContainerObject *>(parent)->removeChild(this, removed_in_workspace);
}

// static
QString RObject::typeToText(RDataType var_type) {
	// TODO: These are non-i18n, and not easily i18n-able due to being used, internally.
	// But they _are_ display strings, too.
	if (var_type == DataUnknown) {
		return QStringLiteral("Unknown");
	} else if (var_type == DataNumeric) {
		return QStringLiteral("Numeric");
	} else if (var_type == DataCharacter) {
		return QStringLiteral("String");
	} else if (var_type == DataFactor) {
		return QStringLiteral("Factor");
	} else if (var_type == DataLogical) {
		return QStringLiteral("Logical");
	} else {
		RK_ASSERT(false);
		return QStringLiteral("Invalid");
	}
}

// static
RObject::RDataType RObject::textToType(const QString &text) {
	if (text == QLatin1String("Unknown")) {
		return DataUnknown;
	} else if (text == QLatin1String("Numeric")) {
		return DataNumeric;
	} else if (text == QLatin1String("String")) {
		return DataCharacter;
	} else if (text == QLatin1String("Factor")) {
		return DataFactor;
	} else if (text == QLatin1String("Logical")) {
		return DataLogical;
	} else {
		RK_ASSERT(false);
		return DataUnknown;
	}
}

// static
QString RObject::rQuote(const QString &string) {
	return (RKRSharedFunctionality::quote(string));
}

// static
QStringList RObject::parseObjectPath(const QString &path) {
	RK_TRACE(OBJECTS);

	QStringList ret;
	QString fragment;

	int end = path.length();
	QChar quote_char;
	bool escaped = false;
	bool seek_bracket_end = false;
	for (int i = 0; i < end; ++i) {
		QChar c = path.at(i);
		if (quote_char.isNull()) {
			if (c == u'\'' || c == u'"' || c == u'`') {
				quote_char = c;
			} else {
				if (!seek_bracket_end) {
					if (c == u'$') {
						ret.append(fragment);
						ret.append(u"$"_s);
						fragment.clear();
					} else if (c == u'[') {
						ret.append(fragment);
						ret.append(u"$"_s);
						fragment.clear();
						if ((i + 1 < end) && (path.at(i + 1) == u'[')) ++i;
						seek_bracket_end = true;
					} else if (c == u':') {
						ret.append(fragment);
						if ((i + 1 < end) && (path.at(i + 1) == u':')) ++i;
						if ((i + 1 < end) && (path.at(i + 1) == u':')) {
							++i;
							ret.append(u":::"_s);
						} else ret.append(u"::"_s);
						fragment.clear();
					} else if (c == u'@') {
						ret.append(fragment);
						ret.append(u"@"_s);
						fragment.clear();
					} else {
						fragment.append(c);
					}
				} else {
					if (c == u']') {
						if ((i + 1 < end) && (path.at(i + 1) == u']')) ++i;
						seek_bracket_end = false;
						continue;
					} else {
						fragment.append(c);
					}
				}
			}
		} else { // inside a quote
			if (c == u'\\') escaped = !escaped;
			else {
				if (escaped) {
					if (c == u't') fragment.append(u'\t');
					else if (c == u'n') fragment.append(u'\n');
					else fragment.append(u'\\' + c);
				} else {
					if (c == quote_char) {
						quote_char = QChar();
					} else {
						fragment.append(c);
					}
				}
			}
		}
	}
	if (!fragment.isEmpty()) ret.append(fragment);
	RK_DEBUG(OBJECTS, DL_DEBUG, "parsed object path %s into %s", qPrintable(path), qPrintable(ret.join(u"-"_s)));
	return ret;
}

// virtual
void RObject::beginEdit() {
	RK_ASSERT(false);
}

// virtual
void RObject::endEdit() {
	RK_ASSERT(false);
}

bool RObject::canWrite() const {
	RK_TRACE(OBJECTS);

	// TODO: find out, if binding is locked:
	// if (isLocked ()) return false;
	return (isInGlobalEnv());
}

bool RObject::canRead() const {
	RK_TRACE(OBJECTS);

	return (this != RObjectList::getObjectList());
}

bool RObject::canRename() const {
	RK_TRACE(OBJECTS);

	if (isPseudoObject()) return false;
	if (parent && parent->isSlotsPseudoObject()) return false;
	// TODO: find out, if binding is locked:
	// if (isLocked ()) return false;
	return (isInGlobalEnv());
}

bool RObject::canRemove() const {
	RK_TRACE(OBJECTS);

	if (isPseudoObject()) return false;
	if (parent && parent->isSlotsPseudoObject()) return false;
	// TODO: find out, if binding is locked:
	// if (isLocked ()) return false;
	return (isInGlobalEnv());
}

REnvironmentObject *RObject::toplevelEnvironment() const {
	RK_TRACE(OBJECTS);

	// could be made recursive instead, but likely it's faster like this
	RObject *o = const_cast<RObject *>(this); // it's ok, all we need to do is find the toplevel parent
	while (o && (!o->isType(ToplevelEnv))) {
		o = o->parent;
	}
	if (!o) {
		RK_ASSERT(this == RObjectList::getObjectList());
		return RObjectList::getGlobalEnv();
	}
	return static_cast<REnvironmentObject *>(o);
}

RObject *RObject::globalEnvSymbol() const {
	RK_TRACE(OBJECTS);
	RObject *o = const_cast<RObject *>(this); // it's ok, all we need to do is find the toplevel parent
	if (o == RObjectList::getGlobalEnv()) return o;
	while (o->parent) {
		if (o->parent == RObjectList::getGlobalEnv()) return o;
		o = o->parent;
	}
	RK_ASSERT(false);
	return nullptr;
}

bool RObject::isInGlobalEnv() const {
	RK_TRACE(OBJECTS);

	RObject *o = toplevelEnvironment();
	if (o->isType(GlobalEnv)) {
		if (o != this) return true; // the GlobalEnv is not inside the GlobalEnv!
	}
	return false;
}
