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
#include "rfunctionobject.h"
#include "renvironmentobject.h"

#include "../rkglobals.h"
#include "rkmodificationtracker.h"

#include "../debug.h"

RContainerObject::RContainerObject (RContainerObject *parent, const QString &name) : RObject (parent, name) {
	RK_TRACE (OBJECTS);
	type = Container;
}

RContainerObject::~RContainerObject () {
	RK_TRACE (OBJECTS);
	
	// delete child objects. Note: the map itself is cleared/deleted automatically
	for (RObjectMap::iterator it = childmap.begin (); it != childmap.end (); ++it) {
		delete it.data ();
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
			RK_DO (qDebug (child->getFullName ().latin1 ()), OBJECTS, DL_ERROR);
			delete child;
			return 0;
		} else {
			if (RKGlobals::tracker ()->removeObject (child, 0, true)) {
				RData *child_name_data = new_data->getStructureVector ()[0];
				RK_ASSERT (child_name_data->getDataType () == RData::StringVector);
				RK_ASSERT (child_name_data->getDataLength () >= 1);
				QString child_name = child_name_data->getStringVector ()[0];
	
				return (createChildFromStructure (new_data, child_name));
			} else {
				return child;		// it was restored in it's old shape
			}
		}
	}
}

bool RContainerObject::updateStructure (RData *new_data) {
	RK_TRACE (OBJECTS);
	unsigned int data_length = new_data->getDataLength (); 
	RK_ASSERT (data_length >= 5);
	RK_ASSERT (new_data->getDataType () == RData::StructureVector);

	if (!RObject::updateStructure (new_data)) return false;

	if (data_length > 5) {
		RK_ASSERT (data_length == 6);

		RData *children_sub = new_data->getStructureVector ()[5];
		RK_ASSERT (children_sub->getDataType () == RData::StructureVector);
		updateChildren (children_sub);
	} else {
		RK_ASSERT (false);
	}

	return true;
}

RObject *RContainerObject::createChildFromStructure (RData *child_data, const QString &child_name) {
	RK_TRACE (OBJECTS);
	RK_ASSERT (child_data->getDataType () == RData::StructureVector);
	RK_ASSERT (childmap.find (child_name) == childmap.end ());
	RK_ASSERT (child_data->getDataLength () >= 2);		// need to see at least the type at this point

	RData *type_data = child_data->getStructureVector ()[1];
	RK_ASSERT (type_data->getDataType () == RData::IntVector);
	RK_ASSERT (type_data->getDataLength () == 1);

	int child_type = type_data->getIntVector ()[0];

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
		RK_DO (qDebug ("Can't represent object '%s', type %d", child_name.latin1 (), child_type), OBJECTS, DL_WARNING);
		return 0;
	}
	RK_ASSERT (child_object);
	RKGlobals::tracker ()->lockUpdates (true);	// object not yet added. prevent updates
	child_object = updateChildStructure (child_object, child_data, true);
	RKGlobals::tracker ()->lockUpdates (false);
	RK_ASSERT (child_object);

	if (child_object) childmap.insert (child_name, child_object);
	if (child_object) RKGlobals::tracker ()->addObject (child_object, 0);
	return child_object;
}

void RContainerObject::updateChildren (RData *new_children) {
	RK_TRACE (OBJECTS);
	RK_ASSERT (new_children->getDataType () == RData::StructureVector);
	unsigned int new_child_count = new_children->getDataLength ();

// first find out, which children are now available, copy the old ones, create the new ones
	RObjectMap new_childmap;
	for (unsigned int i = 0; i < new_child_count; ++i) {
		RData *child_data = new_children->getStructureVector ()[i];
		RK_ASSERT (child_data->getDataType () == RData::StructureVector);
		RK_ASSERT (child_data->getDataLength () >= 1);
		RData *child_name_data = child_data->getStructureVector ()[0];
		RK_ASSERT (child_name_data->getDataType () == RData::StringVector);
		RK_ASSERT (child_name_data->getDataLength () >= 1);
		QString child_name = child_name_data->getStringVector ()[0];

		RObject *child_object;
		RObjectMap::const_iterator it = childmap.find (child_name);
		if (it != childmap.end ()) {
			child_object = updateChildStructure (it.data (), child_data);
		} else {
			child_object = createChildFromStructure (child_data, child_name);
		}
		new_childmap.insert (child_name, child_object);
	}

// now find out, which old ones are missing
	QValueList<RObject*> removed_list;
	for (RObjectMap::const_iterator it = childmap.constBegin (); it != childmap.constEnd (); ++it) {
		QString child_string = it.key ();

		if (new_childmap.find (child_string) == new_childmap.end ()) {
			removed_list.append (it.data ());
		}
	}

// finally delete the missing old ones
	for (QValueList<RObject*>::iterator it = removed_list.begin (); it != removed_list.end (); ++it) {
		RK_DO (qDebug ("child no longer present: %s.", (*it)->getFullName ().latin1 ()), OBJECTS, DL_DEBUG);
		RKGlobals::tracker ()->removeObject ((*it), 0, true);
	}

	childmap = new_childmap;
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
	
	RCommand *command = new RCommand (renameChildCommand (object, new_name), RCommand::App | RCommand::Sync);
	RKGlobals::rInterface ()->issueCommand (command, 0);
	
	childmap.remove (it.key ());
	childmap.insert (new_name, object);
	
	object->name = new_name;
}

void RContainerObject::removeChild (RObject *object, bool removed_in_workspace) {
	RK_TRACE (OBJECTS);

	if (!removed_in_workspace) {
		RCommand *command = new RCommand (removeChildCommand (object), RCommand::App | RCommand::Sync | RCommand::ObjectListUpdate);
		RKGlobals::rInterface ()->issueCommand (command, 0);
	}

	RObjectMap::iterator it = childmap.find (object->getShortName ());
	if (it == childmap.end ()) {
		return;
	}
	RK_ASSERT (it.data () == object);

	childmap.remove (it);
	delete object;
}

QString RContainerObject::removeChildCommand (RObject *object) {
	RK_TRACE (OBJECTS);

	return (object->getFullName () + " <- NULL");
}

QString RContainerObject::renameChildCommand (RObject *object, const QString &new_name) {
	RK_TRACE (OBJECTS);

	return ("rk.rename.in.container (" + getFullName () + ", \"" + object->getShortName () + "\", \"" + new_name + "\")");
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
