/***************************************************************************
                          robject  -  description
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

#include <qregexp.h>

#include "robject.h"

#include "rcontainerobject.h"

#include "../rbackend/rinterface.h"
#include "../rkglobals.h"
#include "rkmodificationtracker.h"

#include "../debug.h"

RObject::RObject (RContainerObject *parent, const QString &name) {
	RK_TRACE (OBJECTS);
	
	RObject::parent = parent;
	RObject::name = name;
	type = 0;
	meta_map = 0;
	data = 0;
	num_classes = 0;
	classname = 0;
	dimension = new int [1];		// safe initialization
	dimension[0] = 0;
	num_dimensions = 0;
}

RObject::~RObject () {
	RK_TRACE (OBJECTS);

	if (data) discardEditData ();

	delete[] classname;

	delete dimension;
}

QString RObject::getShortName () {
	RK_TRACE (OBJECTS);
	return name;
}

QString RObject::getFullName () {
	RK_TRACE (OBJECTS);
	return parent->makeChildName (RObject::name);
}

QString RObject::getLabel () {
	RK_TRACE (OBJECTS);
	return getMetaProperty ("label");
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
	for (int i=0; i < num_classes; ++i) {
		ret.append (classname[i]);
		if (i < (num_classes - 1)) {
			ret.append (sep);
		}
	}
	return ret;
}

bool RObject::inherits (const QString &class_name) {
	RK_TRACE (OBJECTS);

	for (int i=0; i < num_classes; ++i) {
		if (classname[i] == class_name) {
			return true;
		}
	}
	return false;
}

QString RObject::makeChildName (const QString &short_child_name) {
	RK_TRACE (OBJECTS);
	return (getFullName () + "[[" + rQuote (short_child_name) + "]]");
}
	
void RObject::getMetaData (RCommandChain *chain, int flags) {
	RK_TRACE (OBJECTS);
	RCommand *command = new RCommand (".rk.get.meta (" + getFullName () + ")", RCommand::App | RCommand::Sync | RCommand::GetStringVector, QString::null, this, flags);
	RKGlobals::rInterface ()->issueCommand (command, chain);
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

void RObject::handleGetMetaCommand (RCommand *command) {
	RK_TRACE (OBJECTS);

	if (command->stringVectorLength ()) {
		if (!meta_map) meta_map = new MetaMap;
		meta_map->clear ();

		int len = command->stringVectorLength ();
		RK_ASSERT (!(len % 2));
		int cut = len/2;
		for (int i=0; i < cut; ++i) {
			meta_map->insert (command->getStringVector ()[i], command->getStringVector ()[i+cut]);
		}
		
		type |= HasMetaObject;
	} else {		// no meta data received
		delete meta_map;
		meta_map = 0;
		
		type -= (type & HasMetaObject);
	}

	// TODO: only signal change, if there really was a change!
	RKGlobals::tracker ()->objectMetaChanged (this);
}

bool RObject::handleUpdateClassCommand (RCommand *command) {
	RK_TRACE (OBJECTS);

	bool change = false;

	if (num_classes != command->stringVectorLength ()) {
		num_classes = command->stringVectorLength ();
		delete[] classname;
		classname = new QString [num_classes];
		change = true;
	}
	for (int cn=0; cn < num_classes; ++cn) {
		if (classname[cn] != command->getStringVector ()[cn]) change = true;
		classname[cn] = command->getStringVector ()[cn];
	}

	return change;
}

bool RObject::handleClassifyCommand (RCommand *command, bool *dims_changed) {
	RK_TRACE (OBJECTS);

	if (!command->intVectorLength ()) {
		RK_ASSERT (false);
		return false;
	}
	int new_type = command->getIntVector ()[0];

	if (!isMatchingType (type, new_type)){
		RK_DO (qDebug ("type-mismatch: name: %s, old_type: %d, new_type: %d", RObject::name.latin1 (), type, new_type), OBJECTS, DL_INFO);
		RObject::parent->typeMismatch (this, RObject::name);
		return false;	// will be deleted!
	}
	if (new_type != type) {
		*dims_changed = true;
		type = new_type;
	}

	// get dimensions
	if (num_dimensions != (command->intVectorLength () - 1)) {
		num_dimensions = command->intVectorLength () - 1;
		*dims_changed = true;
		delete dimension;
		if (num_dimensions < 1) {
			RK_ASSERT (false);
			num_dimensions = 1;
		}
		dimension = new int [num_dimensions];
	}
	for (int d=0; d < num_dimensions; ++d) {
		if (dimension[d] != command->getIntVector ()[d+1]) *dims_changed = true;
		dimension[d] = command->getIntVector ()[d+1];
	}

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
	RK_ASSERT (!data)

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

