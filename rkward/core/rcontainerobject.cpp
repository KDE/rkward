/***************************************************************************
                          rcontainerobject  -  description
                             -------------------
    begin                : Thu Aug 19 2004
    copyright            : (C) 2004 by Thomas Friedrichsmeier
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

RContainerObject::RContainerObject (RContainerObject *parent, const QString &name) : RObject (parent, name) {
	RK_TRACE (OBJECTS);
	classname = 0;
	dimension = 0;
	num_classes = num_dimensions = 0;
	num_children_updating = 0;
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
	RCommand *command = new RCommand (".rk.classify (" + getFullName () + ")", RCommand::App | RCommand::Sync | RCommand::GetIntVector, "", this, CLASSIFY_COMMAND);
	RKGlobals::rInterface ()->issueCommand (command, RKGlobals::rObjectList()->getUpdateCommandChain ());
}

void RContainerObject::rCommandDone (RCommand *command) {
	RK_TRACE (OBJECTS);
	RObject::rCommandDone (command);

	bool properties_changed = false;
	if (command->getFlags () == CLASSIFY_COMMAND) {
		if (!command->intVectorLength ()) {
			RK_ASSERT (false);
			return;
		} else {
			int new_type = command->getIntVector ()[0];

			// check whether this  is still a container object
			if ((RObject::type) && (new_type != RObject::type)) {
				if (!(new_type & RObject::Container)) {
					RK_DO (qDebug ("type-mismatch: name: %s, old_type: %d, new_type: %d", RObject::name.latin1 (), type, new_type), OBJECTS, DL_INFO);
					RObject::parent->typeMismatch (this, RObject::name);
					return;	// will be deleted!
				}
			}
			if (new_type != RObject::type) {
				properties_changed = true;
				RObject::type = new_type;
			}

			// get dimensions
			if (num_dimensions != (command->intVectorLength () - 1)) {
				num_dimensions = command->intVectorLength () - 1;
				properties_changed = true;
				delete dimension;
				dimension = new int [num_dimensions];
			}
			for (int d=0; d < num_dimensions; ++d) {
				if (dimension[d] != command->getIntVector ()[d+1]) properties_changed = true;
				dimension[d] = command->getIntVector ()[d+1];
			}

			// classifiy command was successful. now get further information.
			if (hasMetaObject ()) getMetaData (RKGlobals::rObjectList()->getUpdateCommandChain ());

			RCommand *command = new RCommand ("class (" + getFullName () + ")", RCommand::App | RCommand::Sync | RCommand::GetStringVector, "", this, UPDATE_CLASS_COMMAND);
			RKGlobals::rInterface ()->issueCommand (command, RKGlobals::rObjectList()->getUpdateCommandChain ());

			command = new RCommand ("names (" + getFullName () + ")", RCommand::App | RCommand::Sync | RCommand::GetStringVector, "", this, UPDATE_CHILD_LIST_COMMAND);
			RKGlobals::rInterface ()->issueCommand (command, RKGlobals::rObjectList()->getUpdateCommandChain ());
		}
		if (properties_changed) RKGlobals::tracker ()->objectMetaChanged (this);
		
	} else if (command->getFlags () == UPDATE_CHILD_LIST_COMMAND) {
		// first check, whether all known children still exist:
		checkRemovedChildren (command->getStringVector (), command->stringVectorLength ());
		
		// next, update the existing and/or new children
		num_children_updating = command->stringVectorLength ();
		// empty object?
		if (!num_children_updating) {
			parent->childUpdateComplete ();
		}
		for (int i = 0; i < command->stringVectorLength (); ++i) {
			QString cname = command->getStringVector ()[i];
			if (childmap.find (cname) != childmap.end ()) {
				RK_DO (qDebug ("updating existing child: %s", cname.latin1 ()), APP, DL_DEBUG);
				childmap[cname]->updateFromR ();
			} else {
				RK_DO (qDebug ("creating new child: %s", cname.latin1 ()), APP, DL_DEBUG);
				RKGlobals::rObjectList()->createFromR (this, cname);
			}
		}
		
	} else if (command->getFlags () == UPDATE_CLASS_COMMAND) {
		if (num_classes != command->stringVectorLength ()) {
			num_classes = command->stringVectorLength ();
			delete classname;
			classname = new QString [num_classes];
			properties_changed = true;
		}
		for (int cn=0; cn < num_classes; ++cn) {
			if (classname[cn] != command->getStringVector ()[cn]) properties_changed = true;
			classname[cn] = command->getStringVector ()[cn];
		}
		if (properties_changed) RKGlobals::tracker ()->objectMetaChanged (this);
	}
}

void RContainerObject::typeMismatch (RObject *child, QString childname) {
	RK_TRACE (OBJECTS);
	delete child;
	childmap.remove (childname);
	
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

QString RContainerObject::makeClassString (const QString &sep) {
	RK_TRACE (OBJECTS);
	QString ret;
	for (int i=0; i < num_classes; ++i) {
		ret.append (classname[i]);
		if (i < (num_classes - 1)) {
			ret.append (sep);
		}
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

void RContainerObject::checkRemovedChildren (char **current_children, int current_child_count) {
	RK_TRACE (OBJECTS);
	QValueList<RObject*> removed_list;
	
// is there a more efficient algorythm for doing this?
	for (RObjectMap::iterator it = childmap.begin (); it != childmap.end (); ++it) {
		QString child_string = it.key ();
		bool found = false;
		for (int i=0; i < current_child_count; ++i) {
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
	if (ret == "") ret = "var";
	int i=-1;
	QString postfix = "";
	while (childmap.contains (ret + postfix)) {
		postfix.setNum (++i);
	}
	return (ret +postfix);
}

