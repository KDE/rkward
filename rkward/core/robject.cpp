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

#include "../debug.h"

#define GET_META_COMMAND 1001

RObject::RObject (RContainerObject *parent, const QString &name) {
	RK_TRACE (OBJECTS);
	
	RObject::parent = parent;
	RObject::name = name;
	type = 0;
	state = 0;
	meta_map = 0;
}

RObject::~RObject () {
	RK_TRACE (OBJECTS);
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
	return "";
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

void RObject::setLabel (const QString &value) {
	RK_TRACE (OBJECTS);
	setMetaProperty ("label", value);
}

void RObject::setMetaProperty (const QString &id, const QString &value) {
	RK_TRACE (OBJECTS);
	if (value == "") {
		if (meta_map) {
			meta_map->remove (id);
			if (!meta_map->size ()) {
				delete meta_map;
				meta_map = 0;
				type -= (type & HasMetaObject);
			}
		}
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

	setMetaModified ();
	meta_map->insert (id, value);
}

QString RObject::makeChildName (const QString &short_child_name) {
	RK_TRACE (OBJECTS);
	return (getFullName () + "[[\"" + short_child_name + "\"]]");
}
	
void RObject::getMetaData (RCommandChain *chain) {
	RK_TRACE (OBJECTS);
	RCommand *command = new RCommand ("c (row.names (attr (" + getFullName () + ", \".rk.meta\")), as.vector (attr (" + getFullName () + ", \".rk.meta\")[[1]]), recursive=TRUE)", RCommand::App | RCommand::Sync | RCommand::GetStringVector, "", this, GET_META_COMMAND);
	RKGlobals::rInterface ()->issueCommand (command, chain);
}

void RObject::writeMetaData (RCommandChain *chain, bool force) {
	RK_TRACE (OBJECTS);
	if ((!force) && (!isMetaModified ())) return;
	
	if (!meta_map) {
		if (hasMetaObject ()) {
			RCommand *command = new RCommand ("attr (" + getFullName () + ", \".rk.meta\") <- NULL", RCommand::App | RCommand::Sync);
			RKGlobals::rInterface ()->issueCommand (command, chain);
		}
		type -= (type & HasMetaObject);
		return;
	}
	
	QString command_string = "attr (" + getFullName () + ", \".rk.meta\") <- data.frame (d=c (";
	QString row_names;
	int i=meta_map->size ();
	for (MetaMap::iterator it = meta_map->begin (); it != meta_map->end (); ++it) {
		row_names.append (rQuote (it.key ()));
		command_string.append (rQuote (it.data ()));
		if (--i) {
			row_names.append (", ");
			command_string.append (", ");
		}
	}
	command_string.append ("), row.names=c (" + row_names + "))");
	
	RCommand *command = new RCommand (command_string, RCommand::App | RCommand::Sync);
	RKGlobals::rInterface ()->issueCommand (command, chain);
	
	type |= HasMetaObject;
	type -= (type & MetaModified);
}

void RObject::rCommandDone (RCommand *command) {
	RK_TRACE (OBJECTS);
	if (command->getFlags () == GET_META_COMMAND) {
		RK_DO (qDebug ("command: %s", command->command ().latin1 ()), OBJECTS, DL_DEBUG);
		if (!meta_map) meta_map = new MetaMap;
		
		int len = command->stringVectorLength ();
		RK_ASSERT (!(len % 2));
		int cut = len/2;
		for (int i=0; i < cut; ++i) {
			RK_DO (qDebug ("meta: %s, value: %s", command->getStringVector ()[i], command->getStringVector ()[i+cut]), OBJECTS, DL_DEBUG);
			meta_map->insert (command->getStringVector ()[i], command->getStringVector ()[i+cut]);
		}
	}
}

void RObject::setDataModified () {
	RK_TRACE (OBJECTS);
	state |= DataModified;
	parent->setChildModified ();
}

void RObject::setMetaModified () {
	RK_TRACE (OBJECTS);
	state |= MetaModified;
	parent->setChildModified ();
}

void RObject::rename (const QString &new_short_name) {
	RK_TRACE (OBJECTS);
	parent->renameChild (this, new_short_name);
}

void RObject::remove () {
	RK_TRACE (OBJECTS);
	parent->removeChild (this);	
}

//static
QString RObject::typeToText (VarType var_type) {
	if (var_type == Unknown) {
		return "";
	} else if (var_type == Number) {
		return "Number";
	} else if (var_type == String) {
		return "String";
	} else if (var_type == Date) {
		return "Date";
	} else {
		return "Invalid";
	}
}

//static 
RObject::VarType RObject::textToType (const QString &text) {
	if (text == "") {
		return Unknown;
	} else if (text == "Number") {
		return Number;
	} else if (text == "String") {
		return String;
	} else if (text == "Date") {
		return Date;
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

