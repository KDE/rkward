/***************************************************************************
                          robjectlist  -  description
                             -------------------
    begin                : Wed Aug 18 2004
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
#include "robjectlist.h"

#define AUTO_UPDATE_INTERVAL 10000
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
	toplevel_environments = new REnvironmentObject*[1];
	num_toplevel_environments = 1;
	toplevel_environments[0] = createTopLevelEnvironment (".GlobalEnv");
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

	QValueList<REnvironmentObject *> removelist;

	// check which envs are removed
	// we could as well iterate over the childmap, but this is easier
	for (unsigned int i = 0; i < num_toplevel_environments; ++i) {
		bool found = false;
		for (unsigned int j = 0; j < env_count; ++j) {
			if (toplevel_environments[i]->getShortName () == env_names[j]) {
				found = true;
				break;
			}
		}
		if (!found) removelist.append (toplevel_environments[i]);
	}

	// remove the environments which are gone
	for (QValueList<REnvironmentObject *>::const_iterator it = removelist.constBegin (); it != removelist.constEnd (); ++it) {
		RK_DO (qDebug ("removing toplevel environment %s from list", (*it)->getShortName ().latin1 ()), OBJECTS, DL_INFO);
		RKGlobals::tracker ()->removeObject (*it, 0, true);
	}

	// find which items are new
	for (unsigned int i = 0; i < env_count; ++i) {
		QString name = env_names[i];
		if (childmap.find (name) == childmap.end ()) {
			createTopLevelEnvironment (name);
		} else {
			RObject *obj = childmap[name];
			// for now, we only update the .GlobalEnv. All others we assume to be static
			if (obj->isType (GlobalEnv)) {
				obj->updateFromR (update_chain);
			}
		}
	}

	// set the new list of environments in the correct order
	delete [] toplevel_environments;
	toplevel_environments = new REnvironmentObject*[env_count];
	num_toplevel_environments = env_count;
	for (unsigned int i = 0; i < env_count; ++i) {
		RObject *obj = childmap[env_names[i]];
		RK_ASSERT (obj);
		RK_ASSERT (obj->isType (Environment));

		toplevel_environments[i] = static_cast<REnvironmentObject *> (obj); 
	}
}

REnvironmentObject *RObjectList::createTopLevelEnvironment (const QString &name) {
	RK_TRACE (OBJECTS);
	RK_ASSERT (childmap.find (name) == childmap.end ());
	REnvironmentObject *envobj = new REnvironmentObject (this, name);

	childmap.insert (name, envobj);
	RKGlobals::tracker ()->addObject (envobj, 0);
	envobj->updateFromR (update_chain);

	return envobj;
}

RObject *RObjectList::findObject (const QString &name, bool is_canonified) {
	RK_TRACE (OBJECTS);

	QString canonified = name;
	if (!is_canonified) canonified = canonifyName (name);

	// TODO: there could be objects with "::" in their names!
	if (canonified.contains ("::")) {
		QString env = canonified.section ("::", 0, 0);
		QString remainder = canonified.section ("::", 1);

		RObject *found = 0;
		for (unsigned int i = 0; i < num_toplevel_environments; ++i) {
			if (toplevel_environments[i]->namespaceName () == env) {
				found = toplevel_environments[i];
				break;
			}
		}
		if (!found) return 0;

		return (found->findObject (remainder, true));
	}

	// no "::"-qualification given, do normal search in all environments, return first match
	for (unsigned int i = 0; i < num_toplevel_environments; ++i) {
		RObject *found = toplevel_environments[i]->findObject (canonified, true);
		if (found) return found;
	}
	return 0;
}

void RObjectList::findObjectsMatching (const QString &partial_name, RObjectMap *current_list, bool name_is_canonified) {
	RK_TRACE (OBJECTS);
	RK_ASSERT (current_list);

	QString canonified = partial_name;
	if (!name_is_canonified) canonified = canonifyName (partial_name);

	// TODO: there could be objects with "::" in their names!
	if (canonified.contains ("::")) {
		QString env = canonified.section ("::", 0, 0);
		QString remainder = canonified.section ("::", 1);

		RObject *found = 0;
		for (unsigned int i = 0; i < num_toplevel_environments; ++i) {
			if (toplevel_environments[i]->namespaceName () == env) {
				found = toplevel_environments[i];
				break;
			}
		}
		if (!found) return;
		
		found->findObjectsMatching (remainder, current_list, true);
		return;
	}

	// no "::"-qualification given, do normal search in all environments.
	for (unsigned int i = 0; i < num_toplevel_environments; ++i) {
		toplevel_environments[i]->findObjectsMatching (canonified, current_list, true);
	}
}

RObject *RObjectList::createNewChild (const QString &name, RKEditor *creator, bool container, bool data_frame) {
	RK_TRACE (OBJECTS);

	return (getGlobalEnv ()->createNewChild (name, creator, container, data_frame));
}

QString RObjectList::validizeName (const QString &child_name) {
	RK_TRACE (OBJECTS);

	return (getGlobalEnv ()->validizeName (child_name));
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

QString RObjectList::renameChildCommand (RObject *object, const QString &new_name) {
	RK_TRACE (OBJECTS);

	return (makeChildName (new_name, false) + " <- " + object->getFullName () + "\n" + removeChildCommand (object));
}

QString RObjectList::removeChildCommand (RObject *object) {
	RK_TRACE (OBJECTS);

	return ("remove (" + object->getFullName () + ")");
}

void RObjectList::removeChild (RObject *object, bool removed_in_workspace) {
	RK_TRACE (OBJECTS);

	if (removed_in_workspace) {
		// remove from list of toplevel environments
		REnvironmentObject **new_toplevel_envs = new REnvironmentObject*[num_toplevel_environments];
		unsigned int num_new_toplevel_envs = 0;
		for (unsigned int i=0; i < num_toplevel_environments; ++i) {
			if (toplevel_environments[i] != object) new_toplevel_envs[num_new_toplevel_envs++] = toplevel_environments[i];
		}
		RK_ASSERT ((num_toplevel_environments - 1) == num_new_toplevel_envs);
		delete [] toplevel_environments;
		toplevel_environments = new_toplevel_envs;
		num_toplevel_environments = num_new_toplevel_envs;

		RContainerObject::removeChild (object, removed_in_workspace);
	} else {
		RK_ASSERT (false);
	}
}

//static
REnvironmentObject *RObjectList::getGlobalEnv () {
	RK_TRACE (OBJECTS);

	RObjectList *list = getObjectList ();
	RK_ASSERT (list);

	REnvironmentObject *envobj = list->toplevel_environments[0];
	RK_ASSERT (envobj);
	RK_ASSERT (envobj->isType (RObject::GlobalEnv));

	return envobj;
}

#include "robjectlist.moc"
