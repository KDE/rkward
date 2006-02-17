/***************************************************************************
                          rkcomponent  -  description
                             -------------------
    begin                : Tue Dec 13 2005
    copyright            : (C) 2005 by Thomas Friedrichsmeier
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

#include "rkcomponent.h"

#include "../debug.h"

//############### RKComponentBase #####################

RKComponentBase* RKComponentBase::lookupComponent (const QString &identifier, QString *modifier) {
	RK_TRACE (PLUGIN);

	if (identifier.isNull ()) return this;

	RKComponentBase *child = child_map.find (identifier.section ("::", 0, 0));
	if (!child) {	// if we do not have such a child, return 0 (RKComponentBase does not support modifiers)
		RK_DO (qDebug ("Failed component lookup"), PLUGIN, DL_WARNING);
		return 0;
	} else {	// else do recursive lookup
		return child->lookupComponent (identifier.section ("::", 1), modifier);
	}
}

void RKComponentBase::addChild (const QString &id, RKComponentBase *child) {
	RK_TRACE (PLUGIN);

	child_map.insert (id, child);
}




//############### RKComponent ########################

RKComponent::RKComponent (RKComponent *parent) : QWidget (parent) {
	RK_TRACE (PLUGIN);

	addChild ("enabled", enabledness_property = new RKComponentPropertyBool (this, false));
	connect (enabledness_property, SIGNAL (valueChanged (RKComponentPropertyBase *)), this, SLOT (propertyValueChanged (RKComponentPropertyBase *)));
	addChild ("visible", visibility_property = new RKComponentPropertyBool (this, false));
	connect (visibility_property, SIGNAL (valueChanged (RKComponentPropertyBase *)), this, SLOT (propertyValueChanged (RKComponentPropertyBase *)));
	addChild ("required", requiredness_property = new RKComponentPropertyBool (this, false));
	connect (requiredness_property, SIGNAL (valueChanged (RKComponentPropertyBase *)), this, SLOT (propertyValueChanged (RKComponentPropertyBase *)));

	_parent = parent;
}

RKComponent::~RKComponent () {
	RK_TRACE (PLUGIN);

	// properties are QObjects, and hence deleted automatically
}

/** generally the valueChanged () signal of all RKComponentPropertys directly owned by this component should be connected to this (Qt-)slot, so the component can update itself accordingly. Default implementation handles changes in visibilty, enabledness and requiredness properties. If you reimplement this, you will most likely still want to call the default implementation to handle these. */
void RKComponent::propertyValueChanged (RKComponentPropertyBase *property) {
	RK_TRACE (PLUGIN);

	if (property == visibility_property) {
		setShown (visibility_property->boolValue ());
	} else if (property == enabledness_property) {
		setEnabled (enabledness_property->boolValue ());
	} else if (property == requiredness_property) {
		checkSatisfied ();
	}
}

bool RKComponent::isSatisfied () {
	RK_TRACE (PLUGIN);
}

/** also notifies the parent, if applicable */
void RKComponent::setSatisfied (bool satisfied) {
	RK_TRACE (PLUGIN);
}

void RKComponent::setReady (bool ready) {
	RK_TRACE (PLUGIN);
}

void RKComponent::setVisible (bool visible) {
	visibilityProperty ()->setBoolValue (visible);
}

void RKComponent::setEnabled (bool enabled) {
	enablednessProperty ()->setBoolValue (enabled);
}

void RKComponent::setRequired (bool required) {
	requirednessProperty ()->setBoolValue (required);
}

#include "rkcomponent.moc"
