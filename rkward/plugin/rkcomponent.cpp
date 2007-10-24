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

RKComponentBase* RKComponentBase::lookupComponent (const QString &identifier, QString *remainder) {
	RK_TRACE (PLUGIN);
	RK_ASSERT (remainder);

	if (identifier.isEmpty ()) return this;
	RK_DO (qDebug ("looking up '%s'", identifier.toLatin1 ().data ()), PLUGIN, DL_DEBUG);

	RKComponentBase *child = child_map.find (identifier.section (".", 0, 0));
	if (!child) {	// if we do not have such a child, return this (and set remainder)
		*remainder = identifier;
		return this;
	} else {	// else do recursive lookup
		return child->lookupComponent (identifier.section (".", 1), remainder);
	}
}

void RKComponentBase::addChild (const QString &id, RKComponentBase *child) {
	RK_TRACE (PLUGIN);

	child_map.insert (id, child);		// no overwriting even on duplicate ("#noid#") ids, als this is really a QDict, not a QMap
}

void RKComponentBase::fetchPropertyValuesRecursive (QMap<QString, QString> *list, bool include_top_level, const QString &prefix) {
	RK_TRACE (PLUGIN);

	for (Q3DictIterator<RKComponentBase> it (child_map); it.current (); ++it) {
		if (it.currentKey () != "#noid#") {
			if (it.current ()->isProperty ()) {
				if (include_top_level) {
					list->insert (prefix + it.currentKey (), it.current ()->value ());
				}
			} else {
				it.current ()->fetchPropertyValuesRecursive (list, true, prefix + it.currentKey () + '.');
			}
		}
	}
}

void RKComponentBase::setPropertyValues (QMap<QString, QString> *list) {
	RK_TRACE (PLUGIN);
	// TODO: visibility enabledness and requiredness should be excluded, as those are not directly user settable. Perhaps even mark up all properties as user settable or not.

	for (QMap<QString, QString>::const_iterator it = list->constBegin (); it != list->constEnd (); ++it) {
		QString mod;
		RKComponentBase *prop = lookupComponent (it.key (), &mod);
		if (mod.isEmpty () && prop->isProperty ()) {		// found a property
			static_cast<RKComponentPropertyBase*>(prop)->setValue (it.data ());
		}
	}
}

QString RKComponentBase::fetchStringValue (const QString &identifier) {
	RK_TRACE (PLUGIN);

	QString mod;
	RKComponentBase *prop = lookupComponent (identifier, &mod);

	return prop->value (mod);
}

QString RKComponentBase::value (const QString &modifier) {
	RK_TRACE (PLUGIN);

	RK_DO (qDebug ("Component type %d does not have a value. Remaining modifier is: '%s'", type (), modifier.toLatin1 ().data ()), PLUGIN, DL_WARNING);
	return QString ();
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

	createDefaultProperties ();

	_parent = parent_component;
	// even if this is component has (parent_widget == 0), the component should be added as a QObject child of the parent.
	if (_parent && (!parent_widget)) _parent->insertChild (this);
}

void RKComponent::createDefaultProperties () {
	RK_TRACE (PLUGIN);

	addChild ("enabled", enabledness_property = new RKComponentPropertyBool (this, false));
	enabledness_property->setBoolValue (true);
	connect (enabledness_property, SIGNAL (valueChanged (RKComponentPropertyBase *)), this, SLOT (propertyValueChanged (RKComponentPropertyBase *)));
	addChild ("visible", visibility_property = new RKComponentPropertyBool (this, false));
	visibility_property->setBoolValue (true);
	connect (visibility_property, SIGNAL (valueChanged (RKComponentPropertyBase *)), this, SLOT (propertyValueChanged (RKComponentPropertyBase *)));
	addChild ("required", requiredness_property = new RKComponentPropertyBool (this, false));
	requiredness_property->setBoolValue (true);
	connect (requiredness_property, SIGNAL (valueChanged (RKComponentPropertyBase *)), this, SLOT (propertyValueChanged (RKComponentPropertyBase *)));
}

RKComponent::~RKComponent () {
	RK_TRACE (PLUGIN);

	// properties are QObjects, and hence deleted automatically
}

void RKComponent::propertyValueChanged (RKComponentPropertyBase *property) {
	RK_TRACE (PLUGIN);

	// slightly more elaborat than necessary on first thought, to prevent loops
	if (property == visibility_property) {
		if (visibility_property->boolValue ()) {
			if (!isShown ()) show ();
		} else {
			if (isShown ()) hide ();
		}
	} else if (property == enabledness_property) {
		updateEnablednessRecursive ();
	} else if (property == requiredness_property) {
		required = requiredness_property->boolValue ();
		changed ();
	}
}

void RKComponent::updateEnablednessRecursive () {
	RK_TRACE (PLUGIN);

	bool enabled;
	if (enabledness_property->boolValue ()) {
		enabled = ((!parentComponent ()) || (parentComponent ()->isEnabled ()));
	} else {
		enabled = false;
	}

	setEnabled (enabled);	/* We do this, even if the state *seems* to be unchanged. This is needed, as isEnabled () also returns false, if the parent QWidget is not enabled. However, the parent QWidget may not always be the parent component. */
	if (enabled != isEnabled ()) {
		for (Q3DictIterator<RKComponentBase> it (child_map); it.current (); ++it) {
			if (it.current ()->isComponent()) {
				static_cast<RKComponent*> (it.current ())->updateEnablednessRecursive ();
			}
		}
	}
}

bool RKComponent::isValid () {
	RK_TRACE (PLUGIN);

	for (Q3DictIterator<RKComponentBase> it (child_map); it.current (); ++it) {
		if (!(it.current ()->isSatisfied ())) return false;
	}
	return true;
}

bool RKComponent::isWizardish () {
	RK_TRACE (PLUGIN);

	return false;
}

bool RKComponent::havePage (bool) {
	RK_TRACE (PLUGIN);
	RK_ASSERT (false);		// should not be called as isWizardish returns false

	return false;
}

void RKComponent::movePage (bool) {
	RK_TRACE (PLUGIN);
	RK_ASSERT (false);		// should not be called as isWizardish returns false
}

RKComponent *RKComponent::addPage () {
	RK_TRACE (PLUGIN);
	RK_ASSERT (false);		// should not be called as isWizardish returns false

	return (new RKComponent (this, this));
}

void RKComponent::addComponentToCurrentPage (RKComponent *) {
	RK_TRACE (PLUGIN);
	RK_ASSERT (false);		// should not be called as isWizardish returns false
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

void RKComponent::removeFromParent () {
	RK_TRACE (PLUGIN);

	if (!parentComponent ()) return;
	for (Q3DictIterator<RKComponentBase> it (parentComponent ()->child_map); it.current (); ++it) {
		if (it.current () == this) {
			QString key = it.currentKey ();
	// unfortunately, several items might hvae the same key, and there seems to be no way to selectively remove the current item only.
	// however, this function should only ever be called in cases of emergency and to prevent crashes. So we make extra sure to remove the child,
	// even if we remove a little more than necessary along the way.
			while (parentComponent ()->child_map.remove (key));
			return;
		}
	}

	RK_ASSERT (false);
}

#include "rkcomponent.moc"
