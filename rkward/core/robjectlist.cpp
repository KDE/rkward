/*
robjectlist - This file is part of RKWard (https://rkward.kde.org). Created: Wed Aug 18 2004
SPDX-FileCopyrightText: 2004-2022 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "robjectlist.h"

#define UPDATE_DELAY_INTERVAL 500

#include <qtimer.h>
#include <qstringlist.h>

#include <KLocalizedString>

#include "renvironmentobject.h"
#include "../rbackend/rkrinterface.h"
#include "rkmodificationtracker.h"
#include "../misc/rkprogresscontrol.h"
#include "../settings/rksettingsmoduler.h"
#include "rkpseudoobjects.h"

#include "../debug.h"

// static
RObjectList *RObjectList::object_list = nullptr;

RObjectList::RObjectList() : RContainerObject(nullptr, QString()) {
	RK_TRACE (OBJECTS);
	RK_ASSERT(!object_list);
	object_list = this;

	update_timer = new QTimer (this);
	update_timer->setSingleShot (true);
	connect(update_timer, &QTimer::timeout, this, [this]() { updateFromR(nullptr); });
	update_chain = nullptr;

	type = RObject::Workspace;
	name = "search()";

	globalenv = new REnvironmentObject(nullptr, QStringLiteral(".GlobalEnv"));
	globalenv->updateFromR(nullptr);

   // TODO: Do we really need tracker notification at this stage?
	RKOrphanNamespacesObject *obj = new RKOrphanNamespacesObject (this);
	RKModificationTracker::instance()->beginAddObject (obj, this, 0);      // first child after GlobalEnv
	orphan_namespaces = obj;
	RKModificationTracker::instance()->endAddObject (obj, this, 0);
}

RObjectList::~RObjectList () {
	RK_TRACE (OBJECTS);
	delete orphan_namespaces;
	delete globalenv;
}

// static
void RObjectList::init() {
	RK_TRACE(OBJECTS);
	if (!object_list) {
		 new RObjectList();
	} else {
		auto *globalenv = object_list->globalenv; // easier typing
		for (int i = globalenv->numChildren() - 1; i >= 0; --i) {
			auto obj = globalenv->findChildByIndex(i);
			RK_ASSERT(obj->editors().isEmpty());
			RKModificationTracker::instance()->removeObject(obj, nullptr, true);
		}
		object_list->updateEnvironments(QStringList() << ".GlobalEnv", false);
		object_list->updateNamespaces(QStringList());
	}
}

QString RObjectList::getObjectDescription () const {
	return i18n ("This section contains environments that are not part of <i>.GlobalEnv</i> / your \"workspace\". Most importantly, this includes loaded packages, but also objects added to R's <i>search()<i>-path using <i>attach()</i>.");
}

QStringList RObjectList::detachPackages (const QStringList &packages, RCommandChain *chain, RKInlineProgressControl* control) {
	RK_TRACE (OBJECTS);

	QStringList remove;
	QStringList reject;
	for (int i = 0; i < packages.size(); ++i) {
		QString shortname = packages[i];
		shortname.remove ("package:");
		if (RKSettingsModuleRPackages::essentialPackages ().contains (shortname)) {
			reject.append (i18n ("Did not unload package %1. It is required in RKWard. If you really want to do this, do so on the R Console.", shortname));
		} else if (!findChildByName (packages[i])) {
			RK_ASSERT (false);
			reject.append (i18n ("Package %1 appears not to have been loaded", shortname));
		} else {
			remove.append (packages[i]);
		}
	}
	for (int i = 0; i < remove.size (); ++i) {
		RCommand *command = new RCommand ("detach (" + rQuote (remove[i]) + ')', RCommand::App | RCommand::ObjectListUpdate);

		if (control) control->addRCommand (command);
		RInterface::issueCommand (command, chain);
	}

	return reject;
}

void RObjectList::updateFromR (RCommandChain *chain) {
	RK_TRACE (OBJECTS);

	if (update_chain) {
		// gee, looks like another update is still on the way. lets schedule one for later:
		update_timer->start (UPDATE_DELAY_INTERVAL);
		RK_DEBUG (OBJECTS, DL_DEBUG, "another object-list update is already running. Rescheduling a further update for later");
		return;
	}

	Q_EMIT updateStarted();
	update_chain = RInterface::startChain (chain);

	RCommand *command = new RCommand("list (search (), loadedNamespaces ())", RCommand::App | RCommand::Sync | RCommand::GetStructuredData);
	whenCommandFinished(command, [this](RCommand* command) {
		RK_ASSERT (command->getDataType () == RData::StructureVector);
		const RData::RDataStorage & data = command->structureVector ();
		RK_ASSERT (data.size () == 2);

		QStringList new_environments = data[0]->stringVector ();
		RK_ASSERT (new_environments.size () >= 2);

		updateEnvironments (new_environments, true);
		updateNamespaces (data[1]->stringVector ());

		makeUpdateCompleteCallback();
	});
	RInterface::issueCommand (command, update_chain);
}

void RObjectList::updateFromR (RCommandChain *chain, const QStringList &current_searchpath, const QStringList &current_namespaces) {
	RK_TRACE (OBJECTS);

// TODO: can this happen? when?
	if (update_chain) {
		// gee, looks like another update is still on the way. lets schedule one for later:
		update_timer->start (UPDATE_DELAY_INTERVAL);
		RK_DEBUG (OBJECTS, DL_DEBUG, "another object-list update is already running. Rescheduling a further update for later");
		return;
	}

	Q_EMIT updateStarted();
	update_chain = RInterface::startChain (chain);

	updateEnvironments (current_searchpath, false);
	updateNamespaces (current_namespaces);

	makeUpdateCompleteCallback();
}

void RObjectList::makeUpdateCompleteCallback() {
	RK_TRACE(OBJECTS);
	RCommand* command = new RCommand(QString(), RCommand::App | RCommand::Sync | RCommand::EmptyCommand);
	whenCommandFinished(command, [this](RCommand*) {
		RK_ASSERT (update_chain);
		RInterface::closeChain (update_chain);
		update_chain = nullptr;

		RK_DEBUG (OBJECTS, DL_DEBUG, "object list update complete");
		Q_EMIT updateComplete();
	});
	RInterface::issueCommand(command, update_chain);
}

void RObjectList::updateEnvironments (const QStringList &_env_names, bool force_globalenv_update) {
	RK_TRACE (OBJECTS);

	RObjectMap newchildmap;
	QStringList env_names = _env_names;
	if (!env_names.isEmpty ()) {
		QString dummy = env_names.takeFirst ();
		RK_ASSERT (dummy == ".GlobalEnv");
		if (force_globalenv_update) {
			// for now, we only update the .GlobalEnv. All others we assume to be static
			getGlobalEnv ()->updateFromR (update_chain);
		}
	} else {
		RK_ASSERT (!env_names.isEmpty ());
	}

	// find which items are new, and copy the old ones
	for (int i = 0; i < env_names.count (); ++i) {
		QString name = env_names[i];

		RObject *obj = findChildByName (name);
		if (obj && (i > 0) && (env_names.lastIndexOf (name, i-1) > -1)) {		// duplicate environment names can happen (e.g. if a data.frame is attached multiple times)
			obj = nullptr;	// only copy the old item once
		}
		if (!obj) {
			obj = createTopLevelEnvironment (name);
			RKModificationTracker::instance()->beginAddObject (obj, this, i);
			childmap.insert (i, obj);
			RKModificationTracker::instance()->endAddObject (obj, this, i);
		}
		newchildmap.insert (i, obj);
	}

	// check which envs have been removed or changed position
	for (int i = 0; i < childmap.size (); ++i) {	// do *not* cache the childmap.size ()! We may change it in the loop.
		RObject *obj = childmap[i];
		int new_pos = newchildmap.indexOf (obj);
		
		if (new_pos < 0) {	// environment is gone
			RK_DEBUG (OBJECTS, DL_INFO, "removing toplevel environment %s from list", obj->getShortName ().toLatin1 ().data ());
			if (RKModificationTracker::instance()->removeObject(obj, nullptr, true)) --i;
			else (newchildmap.insert (i, obj));
		} else if (new_pos != i) {
			// this call is rather expensive, all in all, but fortunately called very rarely
			moveChild (obj, i, new_pos);
		}
	}

	RK_DO (RK_ASSERT (childmap == newchildmap), OBJECTS, DL_DEBUG);	// this is an expensive assert, hence wrapping it inside RK_DO
}

void RObjectList::updateNamespaces (const QStringList &namespace_names) {
	RK_TRACE (OBJECTS);

	QStringList orphan_namespace_names;
	for (int i = 0; i < namespace_names.size (); ++i) {
		if (!findPackage (namespace_names[i])) orphan_namespace_names.append (namespace_names[i]);
	}
	orphan_namespaces->updateNamespacesFromR (update_chain, orphan_namespace_names);
}

REnvironmentObject *RObjectList::createTopLevelEnvironment (const QString &name) {
	RK_TRACE (OBJECTS);

	REnvironmentObject *envobj = new REnvironmentObject (this, name);
	envobj->updateFromR (update_chain);
	return envobj;
}

RObject::ObjectList RObjectList::findObjects (const QStringList &path, bool partial, const QString &op) {
	RK_TRACE (OBJECTS);
	RK_ASSERT (op == "$");

	RObject::ObjectList ret;
	if (path.value (1) == "::") {
		RObject *environment = findPackage (path[0]);
		if (environment) return (environment->findObjects (path.mid (2), partial, "$"));
		return ret;
	} else if (path.value (1) == ":::") {
		RObject *environment = findPackage (path[0]);
		if (environment) environment = static_cast<REnvironmentObject*> (environment)->namespaceEnvironment ();
		if (!environment) environment = orphan_namespaces->findOrphanNamespace (path[0]);
		if (environment) return (environment->findObjects (path.mid (2), partial, "$"));
		return ret;
	} else if (path.value (0) == ".GlobalEnv") {
		if (path.length () > 1) return getGlobalEnv ()->findObjects (path.mid (2), partial, "$");
		// else we'll find base::.GlobalEnv, below
	}

	// no namespace given. Search all environments for matches, .GlobalEnv, first
	ret = getGlobalEnv ()->findObjects (path, partial, "$");
	for (int i = 0; i < childmap.size (); ++i) {
		if (!(partial || ret.isEmpty ())) return ret;

		ret.append (childmap[i]->findObjects (path, partial, "$"));
	}
	return ret;
}

QStringList RObject::getFullNames (const RObject::ObjectList &matches, int options) {
	RK_TRACE (OBJECTS);

	QStringList ret;
	QSet<QString> unique_names;
	for (int i = 0; i < matches.count (); ++i) {
		if (options & IncludeEnvirIfMasked) {
			// - If the name is *not* masked (yet), return the plain name.
			// - If the name *is* masked, return the full qualitfied name.
			// NOTE: This assumes objects are given in search order!
			QString base_name = matches[i]->getFullName (options);
			if (unique_names.contains (base_name)) {
				base_name = matches[i]->getFullName (options | IncludeEnvirIfNotGlobalEnv | IncludeEnvirForGlobalEnv);
			}
			RK_ASSERT (!unique_names.contains (base_name));
			unique_names.insert (base_name);
			ret.append (base_name);
		} else {
			ret.append (matches[i]->getFullName (options));
		}
	}
	return ret;
}

REnvironmentObject* RObjectList::findPackage (const QString &namespacename) const {
	RK_TRACE (OBJECTS);

	for (int i = childmap.size () - 1; i >= 0; --i) {
		RObject* child = childmap[i];
		if (!child->isType (PackageEnv)) continue;	// Skip Autoloads
		REnvironmentObject* env = static_cast<REnvironmentObject *> (child);
		if (env->packageName () == namespacename) {
			return env;
		}
	}
	return nullptr;
}

bool RObjectList::updateStructure (RData *) {
	RK_TRACE (OBJECTS);

	RK_ASSERT (false);

	return true;
}

QString RObjectList::renameChildCommand (RObject *object, const QString &new_name) const {
	RK_TRACE (OBJECTS);

	return (makeChildName (new_name, false, IncludeEnvirIfNotGlobalEnv) + " <- " + object->getFullName () + '\n' + removeChildCommand (object));
}

QString RObjectList::removeChildCommand (RObject *object) const {
	RK_TRACE (OBJECTS);

	return ("remove (" + object->getFullName () + ')');
}


