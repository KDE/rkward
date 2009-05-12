/***************************************************************************
                          rkcomponent  -  description
                             -------------------
    begin                : Tue Dec 13 2005
    copyright            : (C) 2005, 2006, 2009 by Thomas Friedrichsmeier
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

#include "../misc/rkcommonfunctions.h"

#include "../debug.h"

//############### RKComponentBase #####################

RKComponentBase* RKComponentBase::lookupComponent (const QString &identifier, QString *remainder) {
	RK_TRACE (PLUGIN);
	RK_ASSERT (remainder);

	if (identifier.isEmpty ()) return this;
	RK_DO (qDebug ("looking up '%s'", identifier.toLatin1 ().data ()), PLUGIN, DL_DEBUG);

	RKComponentBase *child = child_map.value (identifier.section (".", 0, 0));
	if (!child) {	// if we do not have such a child, return this (and set remainder)
		*remainder = identifier;
		return this;
	} else {	// else do recursive lookup
		return child->lookupComponent (identifier.section (".", 1), remainder);
	}
}

void RKComponentBase::addChild (const QString &id, RKComponentBase *child) {
	RK_TRACE (PLUGIN);

	child_map.insertMulti (id, child);		// no overwriting even on duplicate ("#noid#") ids
}

void RKComponentBase::fetchPropertyValuesRecursive (QMap<QString, QString> *list, bool include_top_level, const QString &prefix) const {
	RK_TRACE (PLUGIN);

	for (QHash<QString, RKComponentBase*>::const_iterator it = child_map.constBegin (); it != child_map.constEnd (); ++it) {
		if (it.key () != "#noid#") {
			if (it.value ()->isProperty ()) {
				if (include_top_level) {
					RKComponentPropertyBase *p = static_cast<RKComponentPropertyBase*> (it.value ());
					if (!p->isInternal ()) { 
						list->insert (prefix + it.key (), it.value ()->value ());
					}
				}
			} else {
				it.value ()->fetchPropertyValuesRecursive (list, true, prefix + it.key () + '.');
			}
		}
	}
}

void RKComponentBase::setPropertyValues (QMap<QString, QString> *list, bool warn_internal) {
	RK_TRACE (PLUGIN);
	// TODO: visibility enabledness and requiredness should be excluded, as those are not directly user settable. Perhaps even mark up all properties as user settable or not.

	for (QMap<QString, QString>::const_iterator it = list->constBegin (); it != list->constEnd (); ++it) {
		QString mod;
		RKComponentBase *prop = lookupComponent (it.key (), &mod);
		if (mod.isEmpty () && prop->isProperty ()) {		// found a property
			RKComponentPropertyBase* p = static_cast<RKComponentPropertyBase*>(prop);
			RK_ASSERT (!(p->isInternal () && warn_internal));
			p->setValue (it.value ());
		}
	}
}

QString RKComponentBase::serializeState () const {
	RK_TRACE (PLUGIN);

	QMap<QString, QString> props;
	fetchPropertyValuesRecursive (&props, true);

	QString out;
	for (QMap<QString, QString>::const_iterator it = props.constBegin (); it != props.constEnd (); ++it) {
		if (!out.isEmpty ()) out.append ("\n");
		out.append (RKCommonFunctions::escape (it.key () + "=" + it.value ()));
	}

	return out;
}

bool RKComponentBase::unserializeState (const QString &state) {
	RK_TRACE (PLUGIN);

	QMap<QString, QString> props;

	QStringList lines = state.split ('\n');
	for (int i = 0; i < lines.count (); ++i) {
		QString line = lines[i];
		int sep = line.indexOf ('=');
		if (sep < 0) return false;		// TODO: message
		props.insert (RKCommonFunctions::unescape (line.left (sep)), RKCommonFunctions::unescape (line.mid (sep+1)));
	}

	setPropertyValues (&props);

	return true;
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
	if (_parent && (!parent_widget)) setParent (_parent);
}

void RKComponent::createDefaultProperties () {
	RK_TRACE (PLUGIN);

	addChild ("enabled", enabledness_property = new RKComponentPropertyBool (this, false));
	enabledness_property->setBoolValue (true);
	enabledness_property->setInternal (true);
	connect (enabledness_property, SIGNAL (valueChanged (RKComponentPropertyBase *)), this, SLOT (propertyValueChanged (RKComponentPropertyBase *)));
	addChild ("visible", visibility_property = new RKComponentPropertyBool (this, false));
	visibility_property->setBoolValue (true);
	visibility_property->setInternal (true);
	connect (visibility_property, SIGNAL (valueChanged (RKComponentPropertyBase *)), this, SLOT (propertyValueChanged (RKComponentPropertyBase *)));
	addChild ("required", requiredness_property = new RKComponentPropertyBool (this, false));
	requiredness_property->setBoolValue (true);
	requiredness_property->setInternal (true);
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
			if (isHidden ()) show ();
		} else {
			if (!isHidden ()) hide ();
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
		for (QHash<QString, RKComponentBase*>::const_iterator it = child_map.constBegin (); it != child_map.constEnd (); ++it) {
			if (it.value ()->isComponent()) {
				static_cast<RKComponent*> (it.value ())->updateEnablednessRecursive ();
			}
		}
	}
}

bool RKComponent::isValid () {
	RK_TRACE (PLUGIN);

	for (QHash<QString, RKComponentBase*>::const_iterator it = child_map.constBegin (); it != child_map.constEnd (); ++it) {
		if (!(it.value ()->isSatisfied ())) return false;
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

	for (QHash<QString, RKComponentBase*>::const_iterator it = parentComponent ()->child_map.constBegin (); it != parentComponent ()->child_map.constEnd (); ++it) {
		if (it.value () == this) {
			QString key = it.key ();
	// unfortunately, several items might hvae the same key, and there seems to be no way to selectively remove the current item only.
	// however, this function should only ever be called in cases of emergency and to prevent crashes. So we make extra sure to remove the child,
	// even if we remove a little more than necessary along the way.
			while (parentComponent ()->child_map.remove (key)) {;}
			return;
		}
	}

	RK_ASSERT (false);
}

#include "rkcomponent.moc"
