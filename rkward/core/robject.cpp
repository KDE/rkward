/***************************************************************************
                          robject  -  description
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

#include <qregexp.h>
#include <klocale.h>

#include "robject.h"

#include "rcontainerobject.h"

#include "../rbackend/rinterface.h"
#include "../rkglobals.h"
#include "robjectlist.h"
#include "rcontainerobject.h"
#include "rkvariable.h"
#include "renvironmentobject.h"
#include "rfunctionobject.h"
#include "rkmodificationtracker.h"

#include "../debug.h"

RObject::RObject (RContainerObject *parent, const QString &name) {
	RK_TRACE (OBJECTS);
	
	RObject::parent = parent;
	RObject::name = name;
	type = 0;
	meta_map = 0;
	data = 0;
	classnames = 0;
	num_classes = 0;
	dimensions = new int[1];	// safe initialization
	dimensions[0] = 0;
	num_dimensions = 0;
}

RObject::~RObject () {
	RK_TRACE (OBJECTS);

	if (data) discardEditData ();

	delete [] dimensions;
	delete [] classnames;
}

QString RObject::getShortName () {
	RK_TRACE (OBJECTS);
	return name;
}

QString RObject::getFullName () {
	RK_TRACE (OBJECTS);
	return parent->makeChildName (RObject::name, type & Misplaced);
}

QString RObject::getLabel () {
	RK_TRACE (OBJECTS);
	return getMetaProperty ("label");
}

RObject *RObject::findObject (const QString &, bool) {
	RK_TRACE (OBJECTS);
	return 0;
}

QString RObject::getMetaProperty (const QString &id) {
	RK_TRACE (OBJECTS);
	if (meta_map) {
		RObject::MetaMap::iterator it;
		if ((it = meta_map->find (id)) != meta_map->end ()) {
			return (it.data ());
		}
	}
	return QString::null;
}

QString RObject::getDescription () {
	RK_TRACE (OBJECTS);
	if (meta_map) {
		RObject::MetaMap::iterator it;
		if ((it = meta_map->find ("label")) != meta_map->end ()) {
			return (getShortName () + " (" + it.data () + ")");
		}
	}
	return getShortName ();;
}

QString RObject::getObjectDescription () {
	RK_TRACE (OBJECTS);

#define ESCS replace ('<', "&lt;")

	QString ret;
	ret.append ("<i>" + getShortName ().ESCS + "</i>");
	ret.append ("<br><b>" + i18n ("Full location:") + " </b>" + getFullName ().ESCS);
	QString lab = getLabel ();
	if (!lab.isEmpty ()) ret.append ("<br><b>" + i18n ("Label:") + " </b>" + lab.ESCS);
	ret.append ("<br><b>" + i18n ("Type:") + " </b>");

	if (isType (Function)) {
		ret.append (i18n ("Function"));
		ret.append ("<br><b>" + i18n ("Usage: ") + " </b>" + getShortName ().ESCS + "(" + static_cast<RFunctionObject *> (this)->printArgs ().ESCS + ")");
	} else if (isType (DataFrame)) {
		ret.append (i18n ("Data frame"));
	} else if (isType (Array)) {
		ret.append (i18n ("Array"));
	} else if (isType (Matrix)) {
		ret.append (i18n ("Matrix"));
	} else if (isType (List)) {
		ret.append (i18n ("List"));
	} else if (isType (Variable)) {
		ret.append (i18n ("Variable"));
		ret.append ("<br><b>" + i18n ("Data Type:") + " </b>" + static_cast<RKVariable *> (this)->getVarTypeString ());
	} else if (isType (Environment)) {
		ret.append (i18n ("Environment"));
	}

	if (isType (Container | Variable)) {
		if (num_dimensions == 1) {
			ret.append ("<br><b>" + i18n ("Length: ") + QString::number (dimensions[0]));
		} else if (num_dimensions > 1) {
			ret.append ("<br><b>" + i18n ("Dimensions: "));
			for (unsigned int i=0; i < num_dimensions; ++i) {
				if (i) ret.append (", ");
				ret.append (QString::number (dimensions[i]));
			}
		}
	}

	ret.append ("<br><b>" + i18n ("Class(es):") + " </b>" + makeClassString (",").ESCS);

	return ret;
}

void RObject::setLabel (const QString &value, bool sync) {
	RK_TRACE (OBJECTS);
	setMetaProperty ("label", value, sync);
}

void RObject::setMetaProperty (const QString &id, const QString &value, bool sync) {
	RK_TRACE (OBJECTS);
	if (value.isEmpty ()) {
		if (meta_map) {
			meta_map->remove (id);
			if (!meta_map->size ()) {
				delete meta_map;
				meta_map = 0;
				type -= (type & HasMetaObject);
			}
		}
		RKGlobals::tracker ()->objectMetaChanged (this);
		return;
	}
	
	if (meta_map) {
		RObject::MetaMap::iterator it;
		if ((it = meta_map->find (id)) != meta_map->end ()) {
			if (it.data () == value) return;
		}
	} else {
		meta_map = new MetaMap;
	}

	meta_map->insert (id, value);
	if (sync) writeMetaData (0);
	RKGlobals::tracker ()->objectMetaChanged (this);
}

QString RObject::makeClassString (const QString &sep) {
	RK_TRACE (OBJECTS);
	QString ret;
	bool first = true;
	for (unsigned int i=0; i < numClasses (); ++i) {
		if (first) first = false;
		else ret.append (sep);
		ret.append (getClassName (i));
	}
	return ret;
}

bool RObject::inherits (const QString &class_name) {
	RK_TRACE (OBJECTS);

	for (unsigned int i=0; i < numClasses (); ++i) {
		if (getClassName (i) == class_name) {
			return true;
		}
	}
	return false;
}

QString RObject::makeChildName (const QString &short_child_name, bool) {
	RK_TRACE (OBJECTS);
	return (getFullName () + "[[" + rQuote (short_child_name) + "]]");
}

void RObject::writeMetaData (RCommandChain *chain) {
	RK_TRACE (OBJECTS);
	
	if (!meta_map) {
		if (hasMetaObject ()) {
			RCommand *command = new RCommand ("attr (" + getFullName () + ", \".rk.meta\") <- NULL", RCommand::App | RCommand::Sync);
			RKGlobals::rInterface ()->issueCommand (command, chain);
		}
		type -= (type & HasMetaObject);
		return;
	}
	
	QString command_string = ".rk.set.meta (" + getFullName () + ", c (";
	QString data_string = "), c (";
	int i=meta_map->size ();
	for (MetaMap::iterator it = meta_map->begin (); it != meta_map->end (); ++it) {
		data_string.append (rQuote (it.data ()));
		command_string.append (rQuote (it.key ()));
		if (--i) {
			data_string.append (", ");
			command_string.append (", ");
		}
	}
	command_string.append (data_string + "))");
	
	RCommand *command = new RCommand (command_string, RCommand::App | RCommand::Sync);
	RKGlobals::rInterface ()->issueCommand (command, chain);
	
	type |= HasMetaObject;
}

void RObject::updateFromR () {
	RK_TRACE (OBJECTS);

	RCommand *command = new RCommand (".rk.get.structure (" + getFullName () + ", " + rQuote (getShortName ()) + ")", RCommand::App | RCommand::Sync | RCommand::GetStructuredData, QString::null, this, ROBJECT_UDPATE_STRUCTURE_COMMAND);
	RKGlobals::rInterface ()->issueCommand (command, RObjectList::getObjectList ()->getUpdateCommandChain ());
}

void RObject::rCommandDone (RCommand *command) {
	RK_TRACE (OBJECTS);

	if (command->getFlags () == ROBJECT_UDPATE_STRUCTURE_COMMAND) {
		if (command->failed ()) {
			RK_DO (qDebug ("command failed while trying to update object '%s'. No longer present?", getShortName ().latin1 ()), OBJECTS, DL_INFO);
			// this may happen, if the object has been removed in the workspace in between
			RKGlobals::tracker ()->removeObject (this, 0, true);
			return;
		}
		if (parent) parent->updateChildStructure (this, command);		// this may result in a delete, so nothing after this!
		else updateStructure (command);		// if we have no parent, likely we're the RObjectList 
		return;
	} else {
		RK_ASSERT (false);
	}
}

bool RObject::updateStructure (RData *new_data) {
	RK_TRACE (OBJECTS);
	if (new_data->getDataLength () == 0) { // can happen, if the object no longer exists
		return false;
	}

	RK_ASSERT (new_data->getDataLength () >= 5);
	RK_ASSERT (new_data->getDataType () == RData::StructureVector);

	if (!canAccommodateStructure (new_data)) return false;

	bool properties_change = false;

	properties_change = updateName (new_data->getStructureVector ()[0]);
	properties_change = updateType (new_data->getStructureVector ()[1]);
	properties_change = updateClasses (new_data->getStructureVector ()[2]);
	properties_change = updateMeta (new_data->getStructureVector ()[3]);
	properties_change = updateDimensions (new_data->getStructureVector ()[4]);

	if (properties_change) RKGlobals::tracker ()->objectMetaChanged (this);

	return true;
}

bool RObject::canAccommodateStructure (RData *new_data) {
	RK_TRACE (OBJECTS);
	RK_ASSERT (new_data->getDataLength () >= 5);
	RK_ASSERT (new_data->getDataType () == RData::StructureVector);

	if (!isValidName (new_data->getStructureVector ()[0])) return false;
	if (!isValidType (new_data->getStructureVector ()[1])) return false;
	return true;
}

bool RObject::isValidName (RData *new_data) {
	RK_TRACE (OBJECTS);
	RK_ASSERT (new_data->getDataLength () == 1);
	RK_ASSERT (new_data->getDataType () == RData::StringVector);

	if (name != new_data->getStringVector ()[0]) {
		RK_ASSERT (false);
		name = new_data->getStringVector ()[0];
		return false;
	}
	return true;
}

bool RObject::updateName (RData *new_data) {
	RK_TRACE (OBJECTS);
	RK_ASSERT (new_data->getDataLength () == 1);
	RK_ASSERT (new_data->getDataType () == RData::StringVector);

	bool changed = false;
	if (name != new_data->getStringVector ()[0]) {
		changed = true;
		RK_ASSERT (false);
		name = new_data->getStringVector ()[0];
	}
	return changed;
}

bool RObject::isValidType (RData *new_data) {
	RK_TRACE (OBJECTS);
	RK_ASSERT (new_data->getDataLength () == 1);
	RK_ASSERT (new_data->getDataType () == RData::IntVector);

	int new_type = new_data->getIntVector ()[0];
	if (!isMatchingType (type, new_type)) return false;

	return true;
}

bool RObject::updateType (RData *new_data) {
	RK_TRACE (OBJECTS);
	RK_ASSERT (new_data->getDataLength () == 1);
	RK_ASSERT (new_data->getDataType () == RData::IntVector);

	bool changed = false;
	int new_type = new_data->getIntVector ()[0];
	if (type & Misplaced) new_type |= Misplaced;
	if (type != new_type) {
		changed = true;
		type = new_type;
	}
	return changed;
}

bool RObject::updateClasses (RData *new_data) {
	RK_TRACE (OBJECTS);
	RK_ASSERT (new_data->getDataLength () >= 1);		// or can there be classless objects in R?
	RK_ASSERT (new_data->getDataType () == RData::StringVector);

	bool change = false;

	unsigned int new_len = new_data->getDataLength ();
	QString *new_classes = new_data->getStringVector ();
	new_data->detachData ();

	if (numClasses () != new_len) {
		change = true;
	} else {
		for (unsigned int cn=0; cn < numClasses (); ++cn) {
			if (classnames[cn] != new_classes[cn]) {
				change = true;
				break;
			}
		}
	}

	num_classes = new_len;
	delete [] classnames;
	classnames = new_classes;

	return change;
}

bool RObject::updateMeta (RData *new_data) {
	RK_TRACE (OBJECTS);

	RK_ASSERT (new_data->getDataType () == RData::StringVector);

	unsigned int len = new_data->getDataLength ();
	if (len == 1) len = 0;		// if it's a single element, it's just a dummy
	bool change = false;
	if (len) {
		if (!meta_map) meta_map = new MetaMap;
		meta_map->clear ();

		RK_ASSERT (!(len % 2));
		unsigned int cut = len/2;
		for (unsigned int i=0; i < cut; ++i) {
			meta_map->insert (new_data->getStringVector ()[i], new_data->getStringVector ()[i+cut]);
		}

		type |= HasMetaObject;
		// TODO: only signal change, if there really was a change!
		change = true;
	} else {		// no meta data received
		if (meta_map) {
			delete meta_map;
			meta_map = 0;
			change = true;
		}

		type -= (type & HasMetaObject);
	}
	return change;
}

bool RObject::updateDimensions (RData *new_data) {
	RK_TRACE (OBJECTS);
	RK_ASSERT (new_data->getDataLength () >= 1);
	RK_ASSERT (new_data->getDataType () == RData::IntVector);

	bool changed = false;

	unsigned int new_len = new_data->getDataLength ();
	int *new_dimensions = new_data->getIntVector ();
	new_data->detachData ();

	if (num_dimensions != new_len) {
		changed = true;
	} else {
		for (unsigned int d=0; d < num_dimensions; ++d) {
			if (dimensions[d] != new_dimensions[d]) {
				changed = true;
				break;
			}
		}
	}
	delete [] dimensions;
	num_dimensions = new_len;
	dimensions = new_dimensions;

	return true;
}

void RObject::rename (const QString &new_short_name) {
	RK_TRACE (OBJECTS);
	parent->renameChild (this, new_short_name);
}

void RObject::remove (bool removed_in_workspace) {
	RK_TRACE (OBJECTS);
	parent->removeChild (this, removed_in_workspace);
}

//static
QString RObject::typeToText (VarType var_type) {
	if (var_type == Unknown) {
		return "Unknown";
	} else if (var_type == Number) {
		return "Number";
	} else if (var_type == String) {
		return "String";
	} else if (var_type == Factor) {
		return "Factor";
	} else {
		return "Invalid";
	}
}

//static 
RObject::VarType RObject::textToType (const QString &text) {
	if (text == "Unknown") {
		return Unknown;
	} else if (text == "Number") {
		return Number;
	} else if (text == "String") {
		return String;
	} else if (text == "Factor") {
		return Factor;
	} else {
		return Invalid;
	}
}

//static
QString RObject::rQuote (const QString &string) {
	// TODO: this is not entirely correct, yet (let alone efficient)!
	QString copy = string;
	return ("\"" + copy.replace (QRegExp ("\""), "\\\"") + "\"");
}

RKEditor *RObject::objectOpened () {
	RK_TRACE (OBJECTS);

	if (!data) return 0;
	return data->editor;
}

void RObject::setObjectOpened (RKEditor *editor, bool opened) {
	RK_TRACE (OBJECTS);

	// TODO: only for now! Currently only a single editor may operate on an object
	if (opened) {
		RK_ASSERT (!data);
	} else {
		RK_ASSERT (data);
	}

	if (opened) {
		if (!data) {
			allocateEditData ();
			initializeEditData (false);
		}
		data->editor = editor;
	} else {
		discardEditData ();
	}
}

void RObject::setCreatedInEditor (RKEditor *editor) {
	RK_TRACE (OBJECTS);

	// TODO: only for now! Currently only a single editor may operate on an object
	RK_ASSERT (!data);

	if (!data) {
		allocateEditData ();
		initializeEditData (true);
	}
	data->editor = editor;
}

// virtual
void RObject::allocateEditData () {
	RK_TRACE (OBJECTS);

	// this assert should stay even when more than one editor is allowed per object. After all, the edit-data should only ever be allocated once!
	RK_ASSERT (!data);
	
	data = new EditData;
}

// virtual
void RObject::initializeEditData (bool) {
	RK_TRACE (OBJECTS);
}

// virtual
void RObject::discardEditData () {
	RK_TRACE (OBJECTS);

	RK_ASSERT (data);
	
	delete data;
	data = 0;
}

bool RObject::canEdit () {
	RK_TRACE (OBJECTS);

	// TODO: find out, if binding is locked:
	// if (isLocked ()) return false;
	return (isInGlobalEnv ());
}

bool RObject::canRead () {
	RK_TRACE (OBJECTS);

	return (this != RObjectList::getObjectList ());
}

bool RObject::canRename () {
	RK_TRACE (OBJECTS);

	// TODO: find out, if binding is locked:
	// if (isLocked ()) return false;
	return (isInGlobalEnv ());
}

bool RObject::canRemove () {
	RK_TRACE (OBJECTS);

	// TODO: find out, if binding is locked:
	// if (isLocked ()) return false;
	return (isInGlobalEnv ());
}

bool RObject::isInGlobalEnv () {
	RK_TRACE (OBJECTS);

// could be made recursive instead, but likely it's faster like this
	RObject *o = this;
	while (o && (!o->isType (ToplevelEnv))) {
		o = o->parent;
	}

	if (!o) {
		RK_ASSERT (this == RObjectList::getObjectList ());
		return false;
	}
	if (o->isType (GlobalEnv)) {
		if (o != this) return true;	// the GlobalEnv is not inside the GlobalEnv!
	}
	return false;
}
