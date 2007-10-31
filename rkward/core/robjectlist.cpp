/***************************************************************************
                          robjectlist  -  description
                             -------------------
    begin                : Wed Aug 18 2004
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

#include "../rkglobals.h"

#include "../debug.h"

// static
RObjectList *RObjectList::object_list = 0;

RObjectList::RObjectList () : RContainerObject (0, QString::null) {
	RK_TRACE (OBJECTS);
	object_list = this;

	update_timer = new QTimer (this);
	
	connect (update_timer, SIGNAL (timeout ()), this, SLOT (timeout ()));
	
	//update_timer->start (AUTO_UPDATE_INTERVAL, true);
	
	type = RObject::Workspace;
	
	update_chain = 0;
	RKGlobals::tracker ()->addObject (createTopLevelEnvironment (".GlobalEnv"), this, 0);
}

RObjectList::~RObjectList () {
	RK_TRACE (OBJECTS);
}

void RObjectList::updateFromR (RCommandChain *chain) {
	RK_TRACE (OBJECTS);
	if (update_chain) {
		// gee, looks like another update is still on the way. lets schedule one for later:
		update_timer->start (UPDATE_DELAY_INTERVAL, true);
		RK_DO (qDebug ("another object-list update is already running. Rescheduling a further update for later"), OBJECTS, DL_DEBUG);
		return;
	}

	emit (updateStarted ());
	update_chain = RKGlobals::rInterface ()->startChain (chain);

	RCommand *command = new RCommand ("search ()", RCommand::App | RCommand::Sync | RCommand::GetStringVector, QString::null, this, ROBJECTLIST_UDPATE_ENVIRONMENTS_COMMAND);
	RKGlobals::rInterface ()->issueCommand (command, update_chain);
}

void RObjectList::rCommandDone (RCommand *command) {
	RK_TRACE (OBJECTS);

	if (command->getFlags () == ROBJECTLIST_UDPATE_ENVIRONMENTS_COMMAND) {
		unsigned int num_new_environments = command->getDataLength ();
		RK_ASSERT (command->getDataType () == RData::StringVector);
		RK_ASSERT (num_new_environments >= 2);
		QString *new_environments = command->getStringVector ();

		updateEnvironments (new_environments, num_new_environments);

		RKGlobals::rInterface ()->issueCommand (QString (), RCommand::App | RCommand::Sync | RCommand::EmptyCommand, QString (), this, ROBJECTLIST_UDPATE_COMPLETE_COMMAND, update_chain);
	} else if (command->getFlags () == ROBJECTLIST_UDPATE_COMPLETE_COMMAND) {
		RK_ASSERT (update_chain);
		RKGlobals::rInterface ()->closeChain (update_chain);
		update_chain = 0;
	
		RK_DO (qDebug ("object list update complete"), OBJECTS, DL_DEBUG);
		emit (updateComplete ());
	} else {
		RK_ASSERT (false);
	}
}

void RObjectList::updateEnvironments (QString *env_names, unsigned int env_count) {
	RK_TRACE (OBJECTS);

	RObjectMap newchildmap;

	// find which items are new, and copy the old ones
	for (unsigned int i = 0; i < env_count; ++i) {
		QString name = env_names[i];

		RObject* obj = findChildByName (name);
		if (obj) {
			// for now, we only update the .GlobalEnv. All others we assume to be static
			if (obj->isType (GlobalEnv)) {
				obj->updateFromR (update_chain);
			}
		} else {
			obj = createTopLevelEnvironment (name);
			RKGlobals::tracker ()->addObject (obj, this, i);
		}
		newchildmap.insert (i, obj);
	}

	// check which envs have been removed or changed position
	for (int i = childmap.size () - 1; i >= 0; --i) {
		RObject *obj = childmap[i];
		int new_pos = newchildmap.indexOf (obj);
		
		if (new_pos < 0) {	// environment is gone
			RK_DO (qDebug ("removing toplevel environment %s from list", obj->getShortName ().toLatin1 ().data ()), OBJECTS, DL_INFO);
			if (RKGlobals::tracker ()->removeObject (obj, 0, true)) --i;
			else (newchildmap.insert (i, obj));
		} else if (new_pos != i) {
			// this call is rather expensive, all in all, but fortunately called very rarely
			moveChild (obj, i, new_pos);
		}
	}

	RK_DO (RK_ASSERT (childmap == newchildmap), OBJECTS, DL_DEBUG);	// this is an expensive assert, hence wrapping it inside RK_DO
}

REnvironmentObject *RObjectList::createTopLevelEnvironment (const QString &name) {
	RK_TRACE (OBJECTS);

	REnvironmentObject *envobj = new REnvironmentObject (this, name);
	envobj->updateFromR (update_chain);
	return envobj;
}

RObject *RObjectList::findObject (const QString &name, bool is_canonified) const {
	RK_TRACE (OBJECTS);

	QString canonified = name;
	if (!is_canonified) canonified = canonifyName (name);

	// TODO: there could be objects with "::" in their names!
	if (canonified.contains ("::")) {
		QString env = canonified.section ("::", 0, 0);
		QString remainder = canonified.section ("::", 1);

		RObject *found = findChildByNamespace (env);
		if (!found) return 0;

		return (found->findObject (remainder, true));
	}

	// no "::"-qualification given, do normal search in all environments, return first match
	for (int i = 0; i < childmap.size (); ++i) {
		RObject *found = childmap[i]->findObject (canonified, true);
		if (found) return found;
	}
	return 0;
}

void RObjectList::findObjectsMatching (const QString &partial_name, RObjectSearchMap *current_list, bool name_is_canonified) const {
	RK_TRACE (OBJECTS);
	RK_ASSERT (current_list);

	QString canonified = partial_name;
	if (!name_is_canonified) canonified = canonifyName (partial_name);

	// TODO: there could be objects with "::" in their names!
	if (canonified.contains ("::")) {
		QString env = canonified.section ("::", 0, 0);
		QString remainder = canonified.section ("::", 1);

		RObject *found = findChildByNamespace (env);
		if (!found) return;
		
		found->findObjectsMatching (remainder, current_list, true);
		return;
	}

	// no namespace given. Search all environments for matches
	for (int i = 0; i < childmap.size (); ++i) {
		childmap[i]->findObjectsMatching (canonified, current_list, true);
	}
}

REnvironmentObject* RObjectList::findChildByNamespace (const QString &namespacename) const {
	RK_TRACE (OBJECTS);

	for (int i = childmap.size () - 1; i >= 0; --i) {
		RObject* child = childmap[i];
		RK_ASSERT (child->isType (Environment));
		REnvironmentObject* env = static_cast<REnvironmentObject *> (child);
		if (env->namespaceName () == namespacename) {
			return env;
		}
	}
	return 0;
}

RObject *RObjectList::createNewChild (const QString &name, int position, RKEditor *creator, bool container, bool data_frame) {
	RK_TRACE (OBJECTS);

	return (getGlobalEnv ()->createNewChild (name, position, creator, container, data_frame));
}

QString RObjectList::validizeName (const QString &child_name, bool unique) const {
	RK_TRACE (OBJECTS);

	return (getGlobalEnv ()->validizeName (child_name, unique));
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

	RK_ASSERT (list->numChildren ());
	REnvironmentObject *envobj = static_cast<REnvironmentObject*> (list->childmap[0]);
	RK_ASSERT (envobj);
	RK_ASSERT (envobj->isType (RObject::GlobalEnv));

	return envobj;
}

#include "robjectlist.moc"
