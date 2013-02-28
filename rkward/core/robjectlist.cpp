/***************************************************************************
                          robjectlist  -  description
                             -------------------
    begin                : Wed Aug 18 2004
    copyright            : (C) 2004-2013 by Thomas Friedrichsmeier
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
#include "robjectlist.h"

#define UPDATE_DELAY_INTERVAL 500

#define ROBJECTLIST_UDPATE_ENVIRONMENTS_COMMAND 1
#define ROBJECTLIST_UDPATE_COMPLETE_COMMAND 2

#include <qtimer.h>
#include <qstringlist.h>

#include <klocale.h>

#include "renvironmentobject.h"
#include "../rbackend/rinterface.h"
#include "rkmodificationtracker.h"
#include "../misc/rkprogresscontrol.h"
#include "../settings/rksettingsmoduler.h"
#include "rkpseudoobjects.h"

#include "../rkglobals.h"

#include "../debug.h"

// static
RObjectList *RObjectList::object_list = 0;

RObjectList::RObjectList () : RContainerObject (0, QString::null) {
	RK_TRACE (OBJECTS);
	object_list = this;

	update_timer = new QTimer (this);
	update_timer->setSingleShot (true);
	connect (update_timer, SIGNAL (timeout ()), this, SLOT (timeout ()));
	
	//update_timer->start (AUTO_UPDATE_INTERVAL, true);
	
	type = RObject::Workspace;
	
	update_chain = 0;

	RObject *globalenv = createTopLevelEnvironment (".GlobalEnv");
	RKGlobals::tracker ()->beginAddObject (globalenv, this, 0);	// 1 == first child after GlobalEnv
	childmap.insert (0, globalenv);
	RKGlobals::tracker ()->endAddObject (globalenv, this, 0);

	RKOrphanNamespacesObject *obj = new RKOrphanNamespacesObject (this);
	RKGlobals::tracker ()->beginAddObject (obj, this, 1);	// 1 == first child after GlobalEnv
	orphan_namespaces = obj;
	RKGlobals::tracker ()->endAddObject (obj, this, 1);
}

RObjectList::~RObjectList () {
	RK_TRACE (OBJECTS);
	delete orphan_namespaces;
}

QStringList RObjectList::detachPackages (const QStringList &packages, RCommandChain *chain, RKProgressControl* control) {
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
		RCommand *command = new RCommand ("detach (" + rQuote (remove[i]) + ")", RCommand::App | RCommand::ObjectListUpdate);

		if (control) control->addRCommand (command);
		RKGlobals::rInterface()->issueCommand (command, chain);
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

	emit (updateStarted ());
	update_chain = RKGlobals::rInterface ()->startChain (chain);

	RCommand *command = new RCommand ("list (search (), loadedNamespaces ())", RCommand::App | RCommand::Sync | RCommand::GetStructuredData, QString::null, this, ROBJECTLIST_UDPATE_ENVIRONMENTS_COMMAND);
	RKGlobals::rInterface ()->issueCommand (command, update_chain);
}

void RObjectList::updateFromR (RCommandChain *chain, const QStringList &current_searchpath) {
	RK_TRACE (OBJECTS);

// TODO: can this happen? when?
	if (update_chain) {
		// gee, looks like another update is still on the way. lets schedule one for later:
		update_timer->start (UPDATE_DELAY_INTERVAL);
		RK_DEBUG (OBJECTS, DL_DEBUG, "another object-list update is already running. Rescheduling a further update for later");
		return;
	}

	emit (updateStarted ());
	update_chain = RKGlobals::rInterface ()->startChain (chain);

	updateEnvironments (current_searchpath, false);
#warning TODO
#warning TODO
#warning TODO
#warning TODO
#warning TODO
#warning TODO
#warning TODO
	RKGlobals::rInterface ()->issueCommand (QString (), RCommand::App | RCommand::Sync | RCommand::EmptyCommand, QString (), this, ROBJECTLIST_UDPATE_COMPLETE_COMMAND, update_chain);
}

void RObjectList::rCommandDone (RCommand *command) {
	RK_TRACE (OBJECTS);

	if (command->getFlags () == ROBJECTLIST_UDPATE_ENVIRONMENTS_COMMAND) {
		RK_ASSERT (command->getDataType () == RData::StructureVector);
		const RData::RDataStorage & data = command->structureVector ();
		RK_ASSERT (data.size () == 2);
		
		QStringList new_environments = data[0]->stringVector ();
		RK_ASSERT (new_environments.size () >= 2);

		updateEnvironments (new_environments, true);
		updateNamespaces (data[1]->stringVector ());

		RKGlobals::rInterface ()->issueCommand (QString (), RCommand::App | RCommand::Sync | RCommand::EmptyCommand, QString (), this, ROBJECTLIST_UDPATE_COMPLETE_COMMAND, update_chain);
	} else if (command->getFlags () == ROBJECTLIST_UDPATE_COMPLETE_COMMAND) {
		RK_ASSERT (update_chain);
		RKGlobals::rInterface ()->closeChain (update_chain);
		update_chain = 0;
	
		RK_DEBUG (OBJECTS, DL_DEBUG, "object list update complete");
		emit (updateComplete ());
	} else {
		RK_ASSERT (false);
	}
}

void RObjectList::updateEnvironments (const QStringList &env_names, bool force_globalenv_update) {
	RK_TRACE (OBJECTS);

	RObjectMap newchildmap;

	// find which items are new, and copy the old ones
	for (int i = 0; i < env_names.count (); ++i) {
		QString name = env_names[i];

		RObject* obj = findChildByName (name);
		if (obj && (i > 0) && (env_names.lastIndexOf (name, i-1) > -1)) {		// duplicate environment names can happen (e.g. if a data.frame is attached multiple times)
			obj = 0;	// only copy the old item once
		}
		if (obj) {
			// for now, we only update the .GlobalEnv. All others we assume to be static
			if (obj->isType (GlobalEnv) && force_globalenv_update) {
				obj->updateFromR (update_chain);
			}
		} else {
			obj = createTopLevelEnvironment (name);
			RKGlobals::tracker ()->beginAddObject (obj, this, i);
			childmap.insert (i, obj);
			RKGlobals::tracker ()->endAddObject (obj, this, i);
		}
		newchildmap.insert (i, obj);
	}

	// check which envs have been removed or changed position
	for (int i = 0; i < childmap.size (); ++i) {	// do *not* cache the childmap.size ()! We may change it in the loop.
		RObject *obj = childmap[i];
		int new_pos = newchildmap.indexOf (obj);
		
		if (new_pos < 0) {	// environment is gone
			RK_DEBUG (OBJECTS, DL_INFO, "removing toplevel environment %s from list", obj->getShortName ().toLatin1 ().data ());
			if (RKGlobals::tracker ()->removeObject (obj, 0, true)) --i;
			else (newchildmap.insert (i, obj));
		} else if (new_pos != i) {
			// this call is rather expensive, all in all, but fortunately called very rarely
			moveChild (obj, i, new_pos);
		}
	}

	RK_DO (RK_ASSERT (childmap == newchildmap), OBJECTS, DL_DEBUG);	// this is an expensive assert, hence wrapping it inside RK_DO
}

void RObjectList::updateNamespaces (const QStringList namespace_names) {
	RK_TRACE (OBJECTS);

	QStringList orphan_namespace_names;
	for (int i = 0; i < namespace_names.size (); ++i) {
		if (!findPackage (namespace_names[i])) orphan_namespace_names.append (namespace_names[i]);
	}
	orphan_namespaces->updateFromR (update_chain, orphan_namespace_names);
}

REnvironmentObject *RObjectList::createTopLevelEnvironment (const QString &name) {
	RK_TRACE (OBJECTS);

	REnvironmentObject *envobj = new REnvironmentObject (this, name);
	envobj->updateFromR (update_chain);
	return envobj;
}

RObject *RObjectList::findObjects (const QStringList &path, RObjectSearchMap *matches, const QString &op) {
	RK_TRACE (OBJECTS);
	RK_ASSERT (op == "$");

	if (path.value (1) == "::") {
		RObject *environment = findPackage (path[0]);
		if (!environment) return 0;
		
		return environment->findObjects (path.mid (2), matches, "$");
	} else if (path.value (1) == ":::") {
		RObject *environment = findPackage (path[0]);
		if (environment) environment = static_cast<REnvironmentObject*> (environment)->namespaceEnvironment ();
		if (!environment) environment = orphan_namespaces->findOrphanNamespace (path[0]);
		if (!environment) return 0;

		return environment->findObjects (path.mid (2), matches, "$");
	} else if (path.value (0) == ".GlobalEnv") {
		if (path.length () > 1) return getGlobalEnv ()->findObjects (path.mid (2), matches, "$");
		else if (matches) matches->insert (path.value (0), getGlobalEnv ());	// no return, here: At least one more match will be found in base
		else return getGlobalEnv ();
	}

	// no namespace given. Search all environments for matches
	for (int i = 0; i < childmap.size (); ++i) {
		RObject *found = childmap[i]->findObjects (path, matches, "$");
		if (found && !matches) return found;
	}
	return 0;
}

REnvironmentObject* RObjectList::findPackage (const QString &namespacename) const {
	RK_TRACE (OBJECTS);

	for (int i = childmap.size () - 1; i >= 1; --i) {	// NOTE: childmap[0] is the .GlobalEnv
		RObject* child = childmap[i];
		if (!child->isType (PackageEnv)) continue;	// Skip Autoloads
		REnvironmentObject* env = static_cast<REnvironmentObject *> (child);
		if (env->packageName () == namespacename) {
			return env;
		}
	}
	return 0;
}

bool RObjectList::updateStructure (RData *) {
	RK_TRACE (OBJECTS);

	RK_ASSERT (false);

	return true;
}

void RObjectList::timeout () {
	RK_TRACE (OBJECTS);

	updateFromR (0);
}

QString RObjectList::renameChildCommand (RObject *object, const QString &new_name) const {
	RK_TRACE (OBJECTS);

	return (makeChildName (new_name, false) + " <- " + object->getFullName () + '\n' + removeChildCommand (object));
}

QString RObjectList::removeChildCommand (RObject *object) const {
	RK_TRACE (OBJECTS);

	return ("remove (" + object->getFullName () + ')');
}

//static
REnvironmentObject *RObjectList::getGlobalEnv () {
	RK_TRACE (OBJECTS);

	RObjectList *list = getObjectList ();
	RK_ASSERT (list);

	RK_ASSERT (!list->isEmpty ());
	REnvironmentObject *envobj = static_cast<REnvironmentObject*> (list->childmap[0]);
	RK_ASSERT (envobj);
	RK_ASSERT (envobj->isType (RObject::GlobalEnv));

	return envobj;
}

#include "robjectlist.moc"

