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

#include "../rbackend/rinterface.h"
#include "robjectlist.h"

#include "../rkglobals.h"

#include "../debug.h"

#define UPDATE_DIM_COMMAND 1
#define UPDATE_CLASS_COMMAND 2
#define UPDATE_CHILD_LIST_COMMAND 3
#define UPDATE_TYPE_COMMAND 4

RContainerObject::RContainerObject (RContainerObject *parent, const QString &name) : RObject (parent, name) {
	container_type = 0;
	classname = 0;
	dimension = 0;
	num_classes = num_dimensions = 0;
}

RContainerObject::~RContainerObject () {
}

QString RContainerObject::getLabel () {
	return ("Container-Object: " + RObject::name);
}

QString RContainerObject::getDescription () {
	return getLabel ();
}

void RContainerObject::updateFromR () {
	RCommand *command = new RCommand ("class (" + getFullName () + ")", RCommand::App | RCommand::Sync | RCommand::GetStringVector, "", this, SLOT (gotRResult (RCommand *)), UPDATE_CLASS_COMMAND);
	RKGlobals::rInterface ()->issueCommand (command, 0);

	command = new RCommand ("c (is.data.frame (" + getFullName () + "), is.matrix (" + getFullName () + "), is.array (" + getFullName () + "), is.list (" + getFullName () + "))", RCommand::App | RCommand::Sync | RCommand::GetIntVector, "", this, SLOT (gotRResult (RCommand *)), UPDATE_TYPE_COMMAND);
	RKGlobals::rInterface ()->issueCommand (command, 0);
	
	command = new RCommand ("dim (" + getFullName () + ")", RCommand::App | RCommand::Sync | RCommand::GetIntVector, "", this, SLOT (gotRResult (RCommand *)), UPDATE_DIM_COMMAND);
	RKGlobals::rInterface ()->issueCommand (command, 0);

	command = new RCommand ("names (" + getFullName () + ")", RCommand::App | RCommand::Sync | RCommand::GetStringVector, "", this, SLOT (gotRResult (RCommand *)), UPDATE_CHILD_LIST_COMMAND);
	RKGlobals::rInterface ()->issueCommand (command, 0);
}

void RContainerObject::gotRResult (RCommand *command) {
	bool changed = false;

	if (command->getFlags () == UPDATE_CHILD_LIST_COMMAND) {
		for (int i = 0; i < command->stringVectorLength (); ++i) {
			QString cname = command->getStringVector ()[i];
			if (childmap.find (cname) != childmap.end ()) {
				childmap[cname]->updateFromR ();
			} else {
				RKGlobals::rObjectList()->createFromR (this, cname);
				changed = true;
			}
		}
	} else if (command->getFlags () == UPDATE_CLASS_COMMAND) {

		if (num_classes != command->stringVectorLength ()) {
			num_classes = command->stringVectorLength ();
			delete classname;
			classname = new QString [num_classes];
			changed = true;
		}
		for (int cn=0; cn < num_classes; ++cn) {
			if (classname[cn] != command->getStringVector ()[cn]) changed = true;
			classname[cn] = command->getStringVector ()[cn];
		}
	
	} else if (command->getFlags () == UPDATE_TYPE_COMMAND) {

		int new_type = 0;
		if (command->intVectorLength () != 4) {
			RK_ASSERT (false);
		} else {
			if (command->getIntVector ()[0]) {
				new_type = RObject::DataFrame | RObject::Matrix | RObject::Array | RObject::List | RObject::Container;
			} else if (command->getIntVector ()[1]) {
				new_type = RObject::Matrix | RObject::Array | RObject::List | RObject::Container;
			} else if (command->getIntVector ()[2]) {
				new_type = RObject::Array | RObject::List | RObject::Container;
			} else if (command->getIntVector ()[3]) {
				new_type = RObject::List | RObject::Container;
			}
		}
		if ((container_type) && (new_type != container_type)) {
			changed = true;
			container_type = new_type;
			if (!(container_type & RObject::Container)) {
				RObject::parent->typeMismatch (this, RObject::name);
				return;	// will be deleted!
			}
		}

	} else if (command->getFlags () == UPDATE_DIM_COMMAND) {

		if (num_dimensions != command->intVectorLength ()) {
			num_dimensions = command->intVectorLength ();
			changed = true;
			delete dimension;
			dimension = new int [num_dimensions];
		}

		for (int d=0; d < num_dimensions; ++d) {
			if (dimension[d] != command->getIntVector ()[d]) changed=true;
			dimension[d] = command->getIntVector ()[d];
		}

		/// TODO: remove debug code from here!
		RK_DO (qDebug ("object name: %s", RObject::name.latin1 ()), APP, DL_DEBUG);
		RK_DO (qDebug ("%s", "class names:"), APP, DL_DEBUG);
		for (int a = 0; a < num_classes; ++a) {
			RK_DO (qDebug ("%d: %s", a, classname[a].latin1 ()), APP, DL_DEBUG);
		}
		RK_DO (qDebug ("%s", "dimensions:"), APP, DL_DEBUG);
		for (int a = 0; a < num_dimensions; ++a) {
			RK_DO (qDebug ("%d: %d", a, dimension[a]), APP, DL_DEBUG);
		}
		RK_DO (qDebug ("%s %d", "type:", type), APP, DL_DEBUG);
	}
	
	// TODO: sig
}

void RContainerObject::typeMismatch (RObject *child, QString childname) {
	delete child;
	childmap.remove (childname);
	
	RKGlobals::rObjectList()->createFromR (this, childname);
}

void RContainerObject::addChild (RObject *child, QString childname) {
	RK_DO (qDebug ("child added: %s", childname.latin1 ()), APP, DL_DEBUG);

	childmap.insert (childname, child);
	child->updateFromR ();
}
