/*
renvironmentobject - This file is part of RKWard (https://rkward.kde.org). Created: Wed Sep 27 2006
SPDX-FileCopyrightText: 2006-2019 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "renvironmentobject.h"

#include <KLocalizedString>
#include <KMessageBox>

#include "../rbackend/rkrinterface.h"
#include "../rkward.h"
#include "rkmodificationtracker.h"
#include "rkpseudoobjects.h"
#include "robjectlist.h"

#include "../debug.h"

REnvironmentObject::REnvironmentObject(RContainerObject *parent, const QString &name) : RContainerObject(parent, name) {
	RK_TRACE(OBJECTS);

	type = Environment;
	if (parent == RObjectList::getObjectList()) {
		type |= ToplevelEnv;
		if (name.contains(u':')) {
			type |= PackageEnv;
		}
	} else if (parent == nullptr) {
		RK_ASSERT(name == u".GlobalEnv"_s);
		type |= ToplevelEnv | GlobalEnv;
	}
}

REnvironmentObject::~REnvironmentObject() {
	RK_TRACE(OBJECTS);
}

QString REnvironmentObject::getObjectDescription() const {
	if (isType(GlobalEnv)) {
		return i18n("This section contains data in your \"workspace\". This is data that you created or imported, in contrast to data contained in a loaded R package. Technically, this corresponds to the <i>.GlobalEnv</i> environment.");
	}
	return RContainerObject::getObjectDescription();
}

QString REnvironmentObject::packageName() const {
	RK_ASSERT(isType(PackageEnv));
	if (!isType(PackageEnv)) RK_DEBUG(OBJECTS, DL_WARNING, "%s", qPrintable(name));
	return name.section(u':', 1);
}

QString REnvironmentObject::getFullName(int options) const {
	RK_TRACE(OBJECTS);

	if (type & GlobalEnv) return name; // .GlobalEnv
	if ((type & ToplevelEnv) && (options & IncludeEnvirIfNotGlobalEnv)) return (u"as.environment ("_s + rQuote(name) + u')');
	return parent->makeChildName(name, options);
}

QString REnvironmentObject::makeChildName(const QString &short_child_name, int options) const {
	RK_TRACE(OBJECTS);

	QString safe_name;
	bool irregular = false;
	if (irregularShortName(short_child_name)) {
		irregular = true;
		safe_name = rQuote(short_child_name);
	} else safe_name = short_child_name;

	if (type & GlobalEnv) { // don't print as ".GlobalEnv$something" unless asked to, or childname needs fixing
		if (irregular || (options & IncludeEnvirForGlobalEnv)) return (getShortName() + u'$' + safe_name);
		return (safe_name);
	}
	if (type & ToplevelEnv) {
		if (!(options & IncludeEnvirIfNotGlobalEnv)) return (short_child_name);
		if (type & PackageEnv) return (packageName() + u"::"_s + safe_name);
		return (getFullName(options) + u'$' + safe_name);
	}
	return (getFullName(options) + u'$' + safe_name);
}

void REnvironmentObject::writeMetaData(RCommandChain *chain) {
	RK_TRACE(OBJECTS);

	if (type & ToplevelEnv) return;
	RContainerObject::writeMetaData(chain);
}

void REnvironmentObject::updateFromR(RCommandChain *chain, const QStringList &added_symbols, const QStringList &removed_symbols) {
	RK_TRACE(OBJECTS);

	for (int i = removed_symbols.size() - 1; i >= 0; --i) {
		RObject *removed_object = findChildByName(removed_symbols[i]);
		if (!removed_object) RK_DEBUG(OBJECTS, DL_ERROR, "Removed object %s was not registered", qPrintable(removed_symbols[i]));
		if (removed_object) RKModificationTracker::instance()->removeObject(removed_object, nullptr, true);
	}

	for (int i = added_symbols.size() - 1; i >= 0; --i) {
		RObject *child = findChildByName(added_symbols[i]);
		if (!child) child = createPendingChild(added_symbols[i], i, false, false);
		if (child->isPending()) {
			child->type -= RObject::Pending; // HACK: Child is not actually pending: We've just seen it!
			child->updateFromR(chain);
		}
	}
	if (this == RObjectList::getGlobalEnv() && numChildren() >= 10000) {
		static bool excess_object_warned = false; // show this at most once per session
		if (!excess_object_warned) {
			KMessageBox::information(RKWardMainWindow::getMain(), i18n("Your workspace contains more than 10.000 top level objects. RKWard is not optimized for this situation, and you may experience lag between R commands. Should this situation constitute a regular use case for your work, please let us know at rkward@kde.org, so we can consider possible solutions."), i18n("Many objects in workspace"), QStringLiteral("excess objects"));
			excess_object_warned = true;
		}
	}
}

bool REnvironmentObject::updateStructure(const RData *new_data) {
	RK_TRACE(OBJECTS);
	RK_ASSERT(new_data->getDataType() == RData::StructureVector);
	RK_ASSERT(new_data->getDataLength() >= StorageSizeBasicInfo);

	if (!(type & ToplevelEnv)) {
		if (!RContainerObject::updateStructure(new_data)) return false;
	}

	const auto &new_data_data = new_data->structureVector();
	if (new_data->getDataLength() > StorageSizeBasicInfo) {
		const RData *children_sub = new_data_data.at(StoragePositionChildren);
		RK_ASSERT(children_sub->getDataType() == RData::StructureVector);
		updateChildren(children_sub);

		// a namespace to go with that?
		if (new_data->getDataLength() > (StorageSizeBasicInfo + 1)) {
			RK_ASSERT(new_data->getDataLength() == (StorageSizeBasicInfo + 2));
			updateNamespace(new_data_data.at(StoragePositionNamespace));
		} else updateNamespace(nullptr);
	} else {
		RK_ASSERT(false);
	}
	return true;
}

void REnvironmentObject::updateNamespace(const RData *new_data) {
	RK_TRACE(OBJECTS);

	if (!new_data) {
		setSpecialChildObject(nullptr, NamespaceObject);
		return;
	}

	RK_ASSERT(new_data->getDataType() == RData::StructureVector);
	bool added = false;
	REnvironmentObject *namespace_envir = namespaceEnvironment();
	if (!namespace_envir) {
		namespace_envir = new RKNamespaceObject(this);
		added = true;
		RKModificationTracker::instance()->lockUpdates(true);
	}
	namespace_envir->updateStructure(new_data->structureVector().at(0));
	if (added) {
		RKModificationTracker::instance()->lockUpdates(false);
		setSpecialChildObject(namespace_envir, NamespaceObject);
	}
}

QString REnvironmentObject::renameChildCommand(RObject *object, const QString &new_name) const {
	RK_TRACE(OBJECTS);

	return (makeChildName(new_name, IncludeEnvirIfNotGlobalEnv) + u" <- "_s + object->getFullName() + u'\n' + removeChildCommand(object));
}

QString REnvironmentObject::removeChildCommand(RObject *object) const {
	RK_TRACE(OBJECTS);

	return (u"remove ("_s + object->getFullName() + u')');
}
