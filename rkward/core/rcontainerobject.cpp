/***************************************************************************
                          rcontainerobject  -  description
                             -------------------
    begin                : Thu Aug 19 2004
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
#include "rcontainerobject.h"

#include <qregexp.h>

#include "../rbackend/rinterface.h"
#include "robjectlist.h"
#include "rkvariable.h"

#include "../rkglobals.h"
#include "rkmodificationtracker.h"

#include "../debug.h"

#define CLASSIFY_COMMAND 1
#define UPDATE_CLASS_COMMAND 2
#define UPDATE_CHILD_LIST_COMMAND 3
#define GET_META_COMMAND 4

RContainerObject::RContainerObject (RContainerObject *parent, const QString &name) : RObject (parent, name) {
	RK_TRACE (OBJECTS);
	num_children_updating = 0;
	type = Container;
}

RContainerObject::~RContainerObject () {
	RK_TRACE (OBJECTS);
	
	// delete child objects. Note: the map itself is cleared/deleted automatically
	for (RObjectMap::iterator it = childmap.begin (); it != childmap.end (); ++it) {
		delete it.data ();
	}
}

void RContainerObject::updateFromR () {
	RK_TRACE (OBJECTS);

// TODO: move classification / type mismatch-checking to RObject
	RCommand *command = new RCommand (".rk.classify (" + getFullName () + ")", RCommand::App | RCommand::Sync | RCommand::GetIntVector, QString::null, this, CLASSIFY_COMMAND);
	RKGlobals::rInterface ()->issueCommand (command, RKGlobals::rObjectList()->getUpdateCommandChain ());
}

//virtual
QString RContainerObject::listChildrenCommand () {
	RK_TRACE (OBJECTS);

	return ("names (" + getFullName () + ")");
}

void RContainerObject::rCommandDone (RCommand *command) {
	RK_TRACE (OBJECTS);

	bool properties_changed = false;
	if (command->getFlags () == GET_META_COMMAND) {
		handleGetMetaCommand (command);
	} else if (command->getFlags () == CLASSIFY_COMMAND) {
		if (!handleClassifyCommand (command, &properties_changed)) {
			return; // will be deleted!
		}

		// classifiy command was successful. now get further information.
		if (hasMetaObject ()) getMetaData (RKGlobals::rObjectList()->getUpdateCommandChain (), GET_META_COMMAND);

		RCommand *command = new RCommand ("class (" + getFullName () + ")", RCommand::App | RCommand::Sync | RCommand::GetStringVector, QString::null, this, UPDATE_CLASS_COMMAND);
		RKGlobals::rInterface ()->issueCommand (command, RKGlobals::rObjectList()->getUpdateCommandChain ());

		command = new RCommand (listChildrenCommand (), RCommand::App | RCommand::Sync | RCommand::GetStringVector, QString::null, this, UPDATE_CHILD_LIST_COMMAND);
		RKGlobals::rInterface ()->issueCommand (command, RKGlobals::rObjectList()->getUpdateCommandChain ());

		if (properties_changed) RKGlobals::tracker ()->objectMetaChanged (this);

	} else if (command->getFlags () == UPDATE_CHILD_LIST_COMMAND) {
		// first check, whether all known children still exist:
		checkRemovedChildren (command->getStringVector (), command->getDataLength ());
		
		// next, update the existing and/or new children
		num_children_updating = command->getDataLength ();
		// empty object?
		if (!num_children_updating) {
			parent->childUpdateComplete ();
		}
		for (unsigned int i = 0; i < command->getDataLength (); ++i) {
			QString cname = command->getStringVector ()[i]; 	// for easier typing
			if (childmap.find (cname) != childmap.end ()) {
				RK_DO (qDebug ("updating existing child: %s", cname.latin1 ()), APP, DL_DEBUG);
				childmap[cname]->updateFromR ();
			} else {
				RK_DO (qDebug ("creating new child: %s", cname.latin1 ()), APP, DL_DEBUG);
				RKGlobals::rObjectList()->createFromR (this, cname);
			}
		}
		
	} else if (command->getFlags () == UPDATE_CLASS_COMMAND) {
		if (handleUpdateClassCommand (command)) properties_changed = true;
		if (properties_changed) RKGlobals::tracker ()->objectMetaChanged (this);
	}
}

void RContainerObject::typeMismatch (RObject *child, QString childname) {
	RK_TRACE (OBJECTS);
	/* I no longer know, why I added the uncommented lines below. From what I can tell today, tracker->removeObject () will call removeChild ()
	and the object will be deleted there. Will need to valgrind sonner or later to find out, if those lines did serve a purpose, after all. */
	/* delete child;
	childmap.remove (childname); */
	
	RKGlobals::tracker ()->removeObject (child, 0, true);
	RKGlobals::rObjectList()->createFromR (this, childname);
}

void RContainerObject::childUpdateComplete () {
	RK_TRACE (OBJECTS);
	RK_ASSERT (num_children_updating);
	if ((--num_children_updating) <= 0) parent->childUpdateComplete ();
}

void RContainerObject::addChild (RObject *child, QString childname) {
	RK_TRACE (OBJECTS);
	RK_ASSERT (child);

	childmap.insert (childname, child);
	child->updateFromR ();
}

int RContainerObject::numChildren () {
	RK_TRACE (OBJECTS);
	return childmap.size ();
}

RObject **RContainerObject::children () {
	RK_TRACE (OBJECTS);
	RObject **ret = new RObject *[childmap.size ()];

	int i = 0;
	for (RObjectMap::iterator it = childmap.begin (); it != childmap.end (); ++it) {
		ret[i++] = it.data ();
	}
	return ret;
}

void RContainerObject::writeChildMetaData (RCommandChain *chain) {
	RK_TRACE (OBJECTS);
	for (RObjectMap::iterator it = childmap.begin (); it != childmap.end (); ++it) {
		it.data ()->writeMetaData (chain);
	}
}

RObject *RContainerObject::findChild (const QString &name) {
	RK_TRACE (OBJECTS);
	RObjectMap::iterator it = childmap.find (name);
	RK_ASSERT (it != childmap.end ());
	return (it.data ());
}

RObject *RContainerObject::findObject (const QString &name, bool is_canonified) {
	RK_TRACE (OBJECTS);

	QString canonified = name;
	if (!is_canonified) {
		// yeah, ok, this could be made more efficient relatively easily ...
		canonified = canonified.replace ("[\"", "$").replace ('[', "").replace ("\"]", "").replace (']', "");
	}

	// TODO: there could be objects with "$" in their names!
	QString current_level = canonified.section (QChar ('$'), 0, 0);
	QString remainder = canonified.section (QChar ('$'), 1);

	RObjectMap::iterator it = childmap.find (current_level);
	if (it == childmap.end ()) return 0;

	RObject *found = it.data ();
	if (remainder.isEmpty ()) return found;

	return (found->findObject (remainder, true));
}

RObject *RContainerObject::createNewChild (const QString &name, RKEditor *creator, bool container, bool data_frame) {
	RK_TRACE (OBJECTS);
	RK_ASSERT (childmap.find (name) == childmap.end ());

	RObject *ret;
	if (container) {
		ret = new RContainerObject (this, name);
		ret->type = Container;
		if (data_frame) {
			ret->type |= DataFrame | List | Array | Matrix;
		}
	} else {
		ret = new RKVariable (this, name);
	}
	
	childmap.insert (name, ret);

	RKGlobals::tracker ()->addObject (ret, creator);
	
	return ret;
}

void RContainerObject::renameChild (RObject *object, const QString &new_name) {
	RK_TRACE (OBJECTS);

	RObjectMap::iterator it = childmap.find (object->getShortName ());
	RK_ASSERT (it.data () == object);
	
	RCommand *command = new RCommand ("rk.rename.in.container (" + getFullName () + ", \"" + object->getShortName () + "\", \"" + new_name + "\")", RCommand::App | RCommand::Sync);
	RKGlobals::rInterface ()->issueCommand (command, 0);
	
	childmap.remove (it.key ());
	childmap.insert (new_name, object);
	
	object->name = new_name;
}

void RContainerObject::removeChild (RObject *object, bool removed_in_workspace) {
	RK_TRACE (OBJECTS);

	RObjectMap::iterator it = childmap.find (object->getShortName ());
	RK_ASSERT (it.data () == object);
	
	if (!removed_in_workspace) {
		RCommand *command = new RCommand (object->getFullName () + " <- NULL", RCommand::App | RCommand::Sync);
		RKGlobals::rInterface ()->issueCommand (command, 0);
	}

	childmap.remove (it);
	delete object;
}

bool RContainerObject::isParentOf (RObject *object, bool recursive) {
	RK_TRACE (OBJECTS);

	for (RObjectMap::iterator it = childmap.begin (); it != childmap.end (); ++it) {
		RObject *child = it.data ();
		if (child == object) {
			return true;
		} else if (recursive && child->isContainer ()) {
			if (static_cast<RContainerObject *>(child)->isParentOf (object, true)) {
				return true;
			}
		}
	}
	
	return false;
}

void RContainerObject::checkRemovedChildren (QString *current_children, int child_count) {
	RK_TRACE (OBJECTS);
	QValueList<RObject*> removed_list;

// is there a more efficient algorithm for doing this?
	for (RObjectMap::iterator it = childmap.begin (); it != childmap.end (); ++it) {
		QString child_string = it.key ();
		bool found = false;
		for (int i = 0; i < child_count; ++i) {
			if (child_string == current_children[i]) {
				found = true;
				break;
			}
		}
		if (!found) {
			removed_list.append (it.data ());
		}
	}

	for (QValueList<RObject*>::iterator it = removed_list.begin (); it != removed_list.end (); ++it) {
		RK_DO (qDebug ("child no longer present: %s.", (*it)->getFullName ().latin1 ()), OBJECTS, DL_DEBUG);
		RKGlobals::tracker ()->removeObject ((*it), 0, true);
	}
}

QString RContainerObject::validizeName (const QString &child_name) {
	RK_TRACE (OBJECTS);
	QString ret = child_name;
	ret = ret.replace (QRegExp ("[^a-zA-Z0-9]"), ".");
	ret = ret.replace (QRegExp ("^\\.*[0-9]+"), ".");
	if (ret.isEmpty ()) ret = "var";
	int i=-1;
	QString postfix;
	while (childmap.contains (ret + postfix)) {
		postfix.setNum (++i);
	}
	return (ret +postfix);
}

