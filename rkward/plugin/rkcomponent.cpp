/***************************************************************************
                          rkcomponent  -  description
                             -------------------
    begin                : Tue Dec 13 2005
    copyright            : (C) 2005, 2006 by Thomas Friedrichsmeier
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

	if (identifier.isEmpty ()) return this;
	RK_DO (qDebug ("looking up '%s'", identifier.latin1 ()), PLUGIN, DL_DEBUG);

	RKComponentBase *child = child_map.find (identifier.section ("::", 0, 0));
	if (!child) {	// if we do not have such a child, return 0 unless this is a property
		if (isProperty ()) {
			if (modifier) {
				*modifier = identifier.section ("::", 1);
			}
			return this;
		}
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

QString RKComponentBase::fetchStringValue (const QString &identifier) {
	RK_TRACE (PLUGIN);

	QString mod;
	RKComponentBase *prop = lookupComponent (identifier, &mod);

	if (prop && (prop->isProperty ())) {
		return (static_cast<RKComponentPropertyBase *> (prop)->value (mod));
	} else {
		RK_DO (qDebug ("Failed lookup or not a property: '%s'", identifier.latin1 ()), PLUGIN, DL_WARNING);
		return QString::null;
	}
}

bool RKComponentBase::isSatisfied () {
	RK_TRACE (PLUGIN);
	if (!required) return true;
	if (isValid ()) return true;
	return false;		// never happens in RKComponentBase, but might in subclasses
}

//############### RKComponent ########################

RKComponent::RKComponent (RKComponent *parent_component, QWidget *parent_widget) : QWidget (parent_widget) {
	RK_TRACE (PLUGIN);

	addChild ("enabled", enabledness_property = new RKComponentPropertyBool (this, false));
	connect (enabledness_property, SIGNAL (valueChanged (RKComponentPropertyBase *)), this, SLOT (propertyValueChanged (RKComponentPropertyBase *)));
	addChild ("visible", visibility_property = new RKComponentPropertyBool (this, false));
	connect (visibility_property, SIGNAL (valueChanged (RKComponentPropertyBase *)), this, SLOT (propertyValueChanged (RKComponentPropertyBase *)));
	addChild ("required", requiredness_property = new RKComponentPropertyBool (this, false));
	connect (requiredness_property, SIGNAL (valueChanged (RKComponentPropertyBase *)), this, SLOT (propertyValueChanged (RKComponentPropertyBase *)));

	_parent = parent_component;
}

RKComponent::~RKComponent () {
	RK_TRACE (PLUGIN);

	// properties are QObjects, and hence deleted automatically
}

void RKComponent::propertyValueChanged (RKComponentPropertyBase *property) {
	RK_TRACE (PLUGIN);

	if (property == visibility_property) {
		setShown (visibility_property->boolValue ());
	} else if (property == enabledness_property) {
		setEnabled (enabledness_property->boolValue ());
	} else if (property == requiredness_property) {
		changed ();
	}
}

bool RKComponent::isValid () {
	RK_TRACE (PLUGIN);

	for (QDictIterator<RKComponentBase> it (child_map); it.current (); ++it) {
		if (!(it.current ()->isSatisfied ())) return false;
	}
	return true;
}

/** also notifies the parent, if applicable */
void RKComponent::setSatisfied (bool satisfied) {
	RK_TRACE (PLUGIN);

	// TODO
}

void RKComponent::setReady (bool ready) {
	RK_TRACE (PLUGIN);

	// TODO
}

void RKComponent::setVisible (bool visible) {
	RK_TRACE (PLUGIN);

	visibilityProperty ()->setBoolValue (visible);
}

void RKComponent::setEnabled (bool enabled) {
	RK_TRACE (PLUGIN);

	enablednessProperty ()->setBoolValue (enabled);
}

void RKComponent::setRequired (bool required) {
	RK_TRACE (PLUGIN);

	requirednessProperty ()->setBoolValue (required);
}

void RKComponent::changed () {
	RK_TRACE (PLUGIN);

	if (parentComponent ()) {
		parentComponent ()->changed ();
	}
}

#include "rkcomponent.moc"
