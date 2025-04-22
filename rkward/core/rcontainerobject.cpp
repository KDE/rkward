/*
rcontainerobject - This file is part of RKWard (https://rkward.kde.org). Created: Thu Aug 19 2004
SPDX-FileCopyrightText: 2004-2019 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "rcontainerobject.h"

#include <QRegularExpression>

#include "../rbackend/rkrinterface.h"
#include "robjectlist.h"
#include "rkpseudoobjects.h"
#include "rkvariable.h"
#include "rfunctionobject.h"
#include "renvironmentobject.h"
#include "rkrownames.h"
#include "rkmodificationtracker.h"

#include "../debug.h"

RContainerObject::RContainerObject (RObject *parent, const QString &name) : RObject (parent, name) {
	RK_TRACE (OBJECTS);
	type = Container;
}

RContainerObject::~RContainerObject () {
	RK_TRACE (OBJECTS);
	
	// delete child objects. Note: the map itself is cleared/deleted automatically
	for (int i = childmap.size () - 1; i >= 0; --i) {
		delete childmap[i];
	}
}

RObject *RContainerObject::updateChildStructure (RObject *child, RData *new_data, bool just_created) {
	RK_TRACE (OBJECTS);
	RK_ASSERT (child);
	RK_ASSERT (new_data);

	if (child->updateStructure (new_data)) {
		return child;
	} else {
		if (just_created) {
			RK_ASSERT (false);
			RK_DEBUG (OBJECTS, DL_ERROR, "%s cannot be represented", child->getFullName ().toLatin1 ().data ());
			delete child;
			return nullptr;
		} else {
			int child_index = childmap.indexOf (child);
			RK_ASSERT (child_index >= 0);
			if (RKModificationTracker::instance()->removeObject(child, nullptr, true)) {
				RData *child_name_data = new_data->structureVector ().at (StoragePositionName);
				RK_ASSERT (child_name_data->getDataType () == RData::StringVector);
				RK_ASSERT (child_name_data->getDataLength () >= 1);
				QString child_name = child_name_data->stringVector ().at (0);

				return (createChildFromStructure (new_data, child_name, child_index));
			} else {
				return child;		// it was restored in it's old shape
			}
		}
	}
}

bool RContainerObject::updateStructure (RData *new_data) {
	RK_TRACE (OBJECTS);
	unsigned int data_length = new_data->getDataLength (); 
	RK_ASSERT (data_length >= StorageSizeBasicInfo);
	RK_ASSERT (new_data->getDataType () == RData::StructureVector);

	if (!RObject::updateStructure (new_data)) return false;

	if (data_length > StorageSizeBasicInfo) {
		RK_ASSERT (data_length == (StorageSizeBasicInfo + 1));

		RData *children_sub = new_data->structureVector ().at (StoragePositionChildren);
		RK_ASSERT (children_sub->getDataType () == RData::StructureVector);
		updateChildren (children_sub);
		updateRowNamesObject ();
	} else {
		RK_ASSERT (false);
	}

	return true;
}

RObject *RContainerObject::createChildFromStructure (RData *child_data, const QString &child_name, int position) {
	RK_TRACE (OBJECTS);
	RK_ASSERT (child_data->getDataType () == RData::StructureVector);
	RK_ASSERT (child_data->getDataLength () >= (StoragePositionType + 1));		// need to see at least the type at this point

	RData *type_data = child_data->structureVector ().at (StoragePositionType);
	RK_ASSERT (type_data->getDataType () == RData::IntVector);
	RK_ASSERT (type_data->getDataLength () == 1);

	int child_type = type_data->intVector ().at (0);

	RObject *child_object;
	if (child_type & RObject::Environment) {
		child_object = new REnvironmentObject (this, child_name);
	} else if (child_type & RObject::Container) {
		child_object = new RContainerObject (this, child_name);
	} else if (child_type & RObject::Function) {
		child_object = new RFunctionObject (this, child_name);
	} else if (child_type & RObject::Variable) {
		child_object = new RKVariable (this, child_name);
	} else {
		RK_DEBUG (OBJECTS, DL_WARNING, "Can't represent object '%s', type %d", child_name.toLatin1 ().data (), child_type);
		return nullptr;
	}
	RK_ASSERT (child_object);
	RKModificationTracker::instance()->lockUpdates (true);	// object not yet added. prevent updates
	child_object = updateChildStructure (child_object, child_data, true);
	RKModificationTracker::instance()->lockUpdates (false);

	if (!child_object) {
		RK_ASSERT (false);
		return nullptr;
	}
	RKModificationTracker::instance()->beginAddObject (child_object, this, position);
	childmap.insert (position, child_object);
	RKModificationTracker::instance()->endAddObject (child_object, this, position);
	return child_object;
}

void RContainerObject::updateChildren (RData *new_children) {
	RK_TRACE (OBJECTS);

	RK_ASSERT (new_children->getDataType () == RData::StructureVector);
	unsigned int new_child_count = new_children->getDataLength ();

	// first find out, which children are now available, copy the old ones, create the new ones
	RObjectMap new_childmap, old_childmap;
	old_childmap = childmap;
	RData::RDataStorage nc_data = new_children->structureVector ();
	for (unsigned int i = 0; i < new_child_count; ++i) {
		RData *child_data = nc_data.at (i);
		RK_ASSERT (child_data->getDataType () == RData::StructureVector);
		RK_ASSERT (child_data->getDataLength () >= (StoragePositionName + 1));
		RData *child_name_data = child_data->structureVector ().at (StoragePositionName);
		RK_ASSERT (child_name_data->getDataType () == RData::StringVector);
		RK_ASSERT (child_name_data->getDataLength () >= 1);
		QString child_name = child_name_data->stringVector ().at (0);

		RObject *child_object = nullptr;
		for (int j = 0; j < old_childmap.size (); ++j) {
			RObject *obj = old_childmap[j];
			if (obj && (obj->getShortName () == child_name)) {
				child_object = obj;
				old_childmap[j] = nullptr;	// in case of duplicate names, avoid finding the same child over and over again
				break;
			}
		}
		if (child_object) {
			child_object = updateChildStructure (child_object, child_data);
		} else {
			child_object = createChildFromStructure (child_data, child_name, i);
		}
		new_childmap.insert (i, child_object);
	}

	// now find out, which old ones are missing or changed position
	for (int i = 0; i < childmap.size (); ++i) {	// do *not* cache the childmap.size ()! We may change it in the loop.
		RObject* old_child = childmap[i];

		int new_pos = new_childmap.indexOf (old_child);
		if (new_pos < 0) {
			if (old_child->isPending ()) {
				new_childmap.insert (i, old_child);
			} else {
				RK_DEBUG (OBJECTS, DL_DEBUG, "child no longer present: %s.", old_child->getFullName ().toLatin1 ().data ());
				if (RKModificationTracker::instance()->removeObject(old_child, nullptr, true)) --i;
				else (new_childmap.insert (i, old_child));
			}
		} else {
			if (i != new_pos) {
				// this call is rather expensive, all in all, but fortunately called very rarely
				moveChild (old_child, i, new_pos);
			}
		}
	}

	RK_DO (RK_ASSERT (childmap == new_childmap), OBJECTS, DL_DEBUG);	// this is an expensive assert, hence wrapping it inside RK_DO
}

void RContainerObject::moveChild (RObject* child, int from_index, int to_index) {
	RK_TRACE (OBJECTS);

	RK_ASSERT (from_index != to_index);

	RK_DEBUG (OBJECTS, DL_DEBUG, "Child position changed from %d to %d, %s", from_index, to_index, child->getFullName ().toLatin1 ().data ());

	RK_ASSERT (childmap[from_index] == child);
	RK_ASSERT (from_index < childmap.size ());
	RK_ASSERT (to_index < childmap.size ());
	RKModificationTracker::instance()->moveObject (this, child, from_index, to_index);
}

int RContainerObject::numChildren () const {
	RK_TRACE (OBJECTS);
	return childmap.size ();
}

RObject *RContainerObject::findChildByName (const QString &name) const {
	RK_TRACE (OBJECTS);

	for (int i = childmap.size () - 1; i >= 0; --i) {
		RObject* obj = childmap[i];
		if (obj->getShortName () == name) return (obj);
	}
	return nullptr;
}

RObject *RContainerObject::findChildByIndex (int position) const {
	// don't trace this
	if ((position >= 0) && (position < childmap.size ())) {
		return childmap[position];
	}
	RK_ASSERT (false);
	return nullptr;
}

RKRowNames* RContainerObject::rowNames () {
	RK_TRACE (OBJECTS);

	if (!hasPseudoObject (RowNamesObject)) {
 		setSpecialChildObject (new RKRowNames (this), RowNamesObject);
		updateRowNamesObject ();
	}
	return rownames_objects.value (this);
}

void RContainerObject::updateRowNamesObject () {
	RK_TRACE (OBJECTS);

	RKRowNames *rownames_object = nullptr;
	if (hasPseudoObject (RowNamesObject)) rownames_object = rownames_objects.value (this);
	if (!rownames_object) return;

	int childlen = 0;
	if (!childmap.isEmpty ()) childlen = childmap[0]->getLength ();
	rownames_object->extendToLength (childlen);	// in case it is being edited
	rownames_object->dimensions[0] = childlen;

	if (rownames_object->isType (NeedDataUpdate) && (!isPending ())) {
		rownames_object->updateDataFromR(nullptr);
	}
}

RObject::ObjectList RContainerObject::findObjects (const QStringList &path, bool partial, const QString &op) {
	RK_TRACE (OBJECTS);

	fetchMoreIfNeeded ();

	if (op != QLatin1String("$")) return RObject::findObjects (path, partial, op);

	if (path.length () > 1) {
		RObject* found = findChildByName (path.value (0));
		if (found) return found->findObjects (path.mid (2), partial, path.value (1));
	} else {
		RObject::ObjectList ret;
		if (!partial) {
			RObject* found = findChildByName (path.value (0));
			if (found) ret.append (found);
		} else {
			QString partial_name = path.value (0);
			if (partial_name.isEmpty ()) {
				ret = childmap;
			} else {
				for (int i = 0; i < childmap.size (); ++i) {
					RObject* child = childmap[i];
					if (child->getShortName ().startsWith (partial_name)) ret.append (child);
				}
			}
		}
		return ret;
	}
	return RObject::ObjectList();
}

RObject *RContainerObject::createPendingChild (const QString &name, int position, bool container, bool data_frame) {
	RK_TRACE (OBJECTS);
	RK_ASSERT (isType (GlobalEnv) || isInGlobalEnv ());

#ifdef __GNUC__
#	warning TODO validize name
#endif
	RObject *ret;
	if (container) {
		ret = new RContainerObject (this, name);
		if (data_frame) {
			ret->type |= DataFrame | List | Array | Matrix;
		}
	} else {
		ret = new RKVariable (this, name);
	}
	ret->type |= Pending;

	if ((position < 0) || (position > childmap.size ())) position = childmap.size ();

	RKModificationTracker::instance()->beginAddObject (ret, this, position);
	childmap.insert (position, ret);
	RKModificationTracker::instance()->endAddObject (ret, this, position);

	return ret;
}

void RContainerObject::renameChild (RObject *object, const QString &new_name) {
	RK_TRACE (OBJECTS);

	RK_ASSERT (findChildByName (object->getShortName ()) == object);
	if (isType (Environment) && (!isType (GlobalEnv))) {
		RK_ASSERT (false);
		return;
	}

	RCommand *command = new RCommand (renameChildCommand (object, new_name), RCommand::App | RCommand::Sync);
	command->setUpdatesObject(object);  // NOTE will be mapped to toplevel object, anyway
	RInterface::issueCommand(command, nullptr);

	object->name = new_name;
}

void RContainerObject::removeChildNoDelete (RObject *object) {
	RK_TRACE (OBJECTS);

	if (!childmap.removeOne (object)) RK_ASSERT (false);
}

void RContainerObject::insertChild (RObject* child, int position) {
	RK_TRACE (OBJECTS);

	RK_ASSERT (child->parentObject () == this);
	if ((position < 0) || (position > childmap.size ())) position = childmap.size ();
	childmap.insert (position, child);
}

void RContainerObject::removeChild (RObject *object, bool removed_in_workspace) {
	RK_TRACE (OBJECTS);

	if (!removed_in_workspace) {
		if (isType (Environment) && (!isType (GlobalEnv))) {
			RK_ASSERT (false);
			return;
		} else if (isType (Workspace)) {
			RK_ASSERT (false);
			return;
		}

		RCommand *command = new RCommand (removeChildCommand (object), RCommand::App | RCommand::Sync | RCommand::ObjectListUpdate);
		RInterface::issueCommand(command, nullptr);
	}

	removeChildNoDelete (object);
	delete object;
}

QString RContainerObject::removeChildCommand (RObject *object) const {
	RK_TRACE (OBJECTS);

	return (object->getFullName () + " <- NULL");
}

QString RContainerObject::renameChildCommand (RObject *object, const QString &new_name) const {
	RK_TRACE (OBJECTS);

	return ("rk.rename.in.container (" + getFullName () + ", \"" + object->getShortName () + "\", \"" + new_name + "\")");
}

QString RContainerObject::validizeName (const QString &child_name, bool unique) const {
	RK_TRACE (OBJECTS);
	RK_ASSERT (isType (GlobalEnv) || isInGlobalEnv ());
	static const QRegularExpression validizename1(QStringLiteral("[^a-zA-Z0-9_]"));
	static const QRegularExpression validizename2(QStringLiteral("^\\.*[0-9]+"));

	QString ret = child_name;
	if (ret.isEmpty ()) ret = QLatin1String("var");
	else {
		ret = ret.replace(validizename1, QLatin1String("."));
		ret = ret.replace(validizename2, QLatin1String("."));
	}
	if (!unique) return ret;

// NOTE: this is potentially a quadratic time algorithm with respect to number of children.
// Its only called on user actions, though, and hopefully users will not keep all objects named "varX".
	int i=0;
	QString postfix;
	while (findChildByName (ret + postfix)) {
		postfix.setNum (++i);
	}
	return (ret + postfix);
}
