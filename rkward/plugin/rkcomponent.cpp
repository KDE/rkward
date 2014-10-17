/***************************************************************************
                          rkcomponent  -  description
                             -------------------
    begin                : Tue Dec 13 2005
    copyright            : (C) 2005, 2006, 2009, 2010, 2011, 2012, 2013, 2014 by Thomas Friedrichsmeier
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

#include "rkstandardcomponent.h"
#include "../misc/rkcommonfunctions.h"

#include "../debug.h"

//############### RKComponentBase #####################

RKComponentBase* RKComponentBase::lookupComponent (const QString &identifier, QString *remainder) {
	RK_TRACE (PLUGIN);
	RK_ASSERT (remainder);

	if (identifier.isEmpty ()) return this;
	RK_DEBUG (PLUGIN, DL_DEBUG, "looking up '%s'", identifier.toLatin1 ().data ());

	RKComponentBase *child = child_map.value (identifier.section (".", 0, 0));
	if (!child) {	// if we do not have such a child, return this (and set remainder)
		*remainder = identifier;
		return this;
	} else {	// else do recursive lookup
		return child->lookupComponent (identifier.section (".", 1), remainder);
	}
}

RKComponentPropertyBase* RKComponentBase::lookupProperty (const QString &identifier, QString *remainder, bool warn) {
	RK_TRACE (PLUGIN);
	QString _remainder;
	QString* p_remainder = remainder;
	if (!remainder) p_remainder = &_remainder;

	RKComponentBase* p = lookupComponent (identifier, p_remainder);
	if (p && p->isProperty ()) {
		if (!remainder && !p_remainder->isEmpty ()) {
			if (warn) RK_DEBUG (PLUGIN, DL_ERROR, "Modifier is not allowed, here, while looking up property: %s. (Remainder was %s)", qPrintable (identifier), qPrintable (*p_remainder));
			return 0;
		}
		return static_cast<RKComponentPropertyBase*> (p);
	}
	if (warn) RK_DEBUG (PLUGIN, DL_ERROR, "No such property: %s. Remainder was %s", qPrintable (identifier), qPrintable (*p_remainder));
	return 0;
}

void RKComponentBase::addChild (const QString &id, RKComponentBase *child) {
	RK_TRACE (PLUGIN);

	child_map.insertMulti (id, child);		// no overwriting even on duplicate ("#noid#") ids
}

void RKComponentBase::fetchPropertyValuesRecursive (PropertyValueMap *list, bool include_top_level, const QString &prefix, bool include_inactive_elements) const {
	RK_TRACE (PLUGIN);

	for (QHash<QString, RKComponentBase*>::const_iterator it = child_map.constBegin (); it != child_map.constEnd (); ++it) {
		if (it.key () == "#noid#") continue;
		if (it.value ()->isInternal ()) continue;

		if (it.value ()->isProperty ()) {
			if (include_top_level) {
				list->insert (prefix + it.key (), fetchStringValue (it.value ()));
			}
		} else {
			RK_ASSERT (it.value ()->isComponent ());
			if (static_cast<RKComponent *> (it.value ())->isInactive () && (!include_inactive_elements)) continue;
			it.value ()->fetchPropertyValuesRecursive (list, true, prefix + it.key () + '.');
		}
	}
}

void RKComponentBase::setPropertyValues (const PropertyValueMap *list, bool warn) {
	RK_TRACE (PLUGIN);
	// TODO: visibility enabledness and requiredness should be excluded, as those are not directly user settable. Perhaps even mark up all properties as user settable or not.

	for (PropertyValueMap::const_iterator it = list->constBegin (); it != list->constEnd (); ++it) {
		QString mod;
		RKComponentBase *prop = lookupComponent (it.key (), &mod);
		if (mod.isEmpty () && prop->isProperty ()) {		// found a property
			RKComponentPropertyBase* p = static_cast<RKComponentPropertyBase*>(prop);
			if (p->isInternal () && warn) {
				RK_DEBUG (PLUGIN, DL_WARNING, "Setting value for property %s, which is marked internal.", qPrintable (it.key ()));
			}
			p->setValue (it.value ());
		} else {
			if (warn) RK_DEBUG (PLUGIN, DL_WARNING, "Property %s not found while setting values. Remainder was %s.", qPrintable (it.key ()), qPrintable (mod));
		}
	}
}

//static
QString RKComponentBase::valueMapToString (const PropertyValueMap &map) {
	RK_TRACE (PLUGIN);

	QString out;
	for (PropertyValueMap::const_iterator it = map.constBegin (); it != map.constEnd (); ++it) {
		if (!out.isEmpty ()) out.append ("\n");
		out.append (RKCommonFunctions::escape (it.key () + "=" + it.value ()));
	}
	return out;
}

//static
bool RKComponentBase::stringListToValueMap (const QStringList &strings, PropertyValueMap *map) {
	RK_TRACE (PLUGIN);

	for (int i = 0; i < strings.size (); ++i) {
		QString line = RKCommonFunctions::unescape (strings[i]);
		int sep = line.indexOf ('=');
		if (sep < 0) return false;
		map->insert (line.left (sep), line.mid (sep+1));
	}
	return true;
}

QStringList RKComponentBase::matchAgainstState (const PropertyValueMap &state) {
	RK_TRACE (PLUGIN);
	QStringList probs;

	PropertyValueMap current;
	fetchPropertyValuesRecursive (&current, true);

	for (PropertyValueMap::const_iterator it = state.constBegin (); it != state.constEnd (); ++it) {
		QString current_value = current.value (it.key ());
		if (current_value != it.value ()) {
			// this is not necessarily a problem. The value may simply not be in the serialization, or in slightly different format
			QString dummy;
			RKComponentBase *prop = lookupComponent (it.key (), &dummy);
			if (dummy.isEmpty () && prop) {
				if ((prop->type () == PropertyDouble) && static_cast<RKComponentPropertyDouble*> (prop)->doubleValue () == it.value ().toDouble ()) {
					// COMPAT: In RKWard 0.5.1, the formatting of real numbers was different. Hence we compare the numeric values, instead
					continue;
				} else if ((prop->type () == PropertyBool) && (it.value () == prop->value ("labeled").toString ())) {
					// COMPAT: In RKWard 0.6.0, bool properties returned the labelled string, by default. Hence we also compare on the labelled value
					continue;
				} else if (fetchStringValue (prop) == it.value ()) {
					continue;
				} else {
					if (current_value.isEmpty ()) current_value = fetchStringValue (prop);	// TODO: Hm, what did I have in mind, here?
					probs.append (QString ("Tried to apply 'value %1' to property %2, but got '%3', instead").arg (it.value (), it.key (), current_value));
				}
			} else {
				probs.append (QString ("No such property %1 (remainder was %2)").arg (it.key (), dummy));
			}
		}
	}

	return probs;
}

QString RKComponentBase::fetchStringValue (RKComponentBase* prop, const QString &modifier) {
	// not tracing this simple helper
// TODO: we need a bit of special casing, here. Probably, instead, we should add new virutal functions serialize() and unserialize(QString()), which properties can re-implement, if needed.

	if (prop->type () == PropertyDouble) {
		if (modifier.isEmpty ()) return (prop->value ("formatted").toString ());
	} else if (prop->type () == PropertyStringList) {
		if (modifier.isEmpty ()) return (prop->value ("joined").toString ());
	} else if (prop->type () == PropertyRObjects) {
		return (prop->value (modifier).toStringList ().join ("\n"));
	}
	QVariant value = prop->value (modifier);
	if (value.type () == QVariant::StringList) {
		return value.toStringList ().join ("\n");
	}
	return (value.toString ());
}

QString RKComponentBase::fetchStringValue (const QString &identifier) {
	RK_TRACE (PLUGIN);

	QString mod;
	RKComponentBase *prop = lookupComponent (identifier, &mod);
	return fetchStringValue (prop, mod);
}

QVariant RKComponentBase::fetchValue (const QString &id, const int hint) {
	if (hint == StringValue) {
		return (fetchStringValue (id));
	} else if (hint == TraditionalValue) {
		QString val = fetchStringValue (id);
		// return "0" as numeric constant. Many plugins rely on this form PHP times.
		if (val == "0") return (QVariant (0.0));
		else return (QVariant (val));
	} else {
		QString mod;
		RKComponentBase *prop = lookupComponent (id, &mod);
		QVariant val = prop->value (mod);
		if (hint == BooleanValue) {
			bool ok;
			val = RKComponentPropertyBool::variantToBool (val, &ok);
			if (!ok) RK_DEBUG (PLUGIN, DL_WARNING, "Could not convert value of %s to boolean", qPrintable (id));
		} else {
			if (hint == StringlistValue) {
				if (val.type () != QVariant::StringList) RK_DEBUG (PLUGIN, DL_WARNING, "Value of %s is not a string list", qPrintable (id));
			} else if (hint == NumericValue) {
				if (!val.canConvert (QVariant::Double)) RK_DEBUG (PLUGIN, DL_WARNING, "Value of %s is not numeric", qPrintable (id));
			} else {
				RK_ASSERT (false);
			}
		}
		return (val);
	}
}

QVariant RKComponentBase::value (const QString &modifier) {
	RK_TRACE (PLUGIN);

	RK_DEBUG (PLUGIN, DL_WARNING, "Component type %d does not have a value. Remaining modifier is: '%s'", type (), modifier.toLatin1 ().data ());
	return QVariant ();
}

RKComponentBase::ComponentStatus RKComponentBase::recursiveStatus () {
	RK_TRACE (PLUGIN);

	bool processing = false;
	bool children_satisfied = true;
	// we always need to interate over all children, since we need to make sure to find any which are dead or processing.
	for (QHash<QString, RKComponentBase*>::const_iterator it = child_map.constBegin (); it != child_map.constEnd (); ++it) {
		ComponentStatus s = it.value ()->recursiveStatus ();
		if (s == Dead) return Dead;
		if (s == Processing) processing = true;
		else if (s != Satisfied) children_satisfied = false;
	}
	if (processing) return Processing;
	bool req = required;
	if (isComponent () && static_cast<RKComponent*>(this)->isInactive ()) req = false;
	if (!req) return Satisfied;
	if (children_satisfied && isValid ()) return Satisfied;
 	if (isComponent ()) RK_DEBUG (PLUGIN, DL_DEBUG, "component not satisfied: %s", qPrintable (static_cast<RKComponent*> (this)->getIdInParent ()));
	return Unsatisfied;
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
		updateEnablednessRecursive ((!parentComponent ()) || (parentComponent ()->isEnabled ()));
	} else if (property == requiredness_property) {
		required = requiredness_property->boolValue ();
		changed ();
	}
}

void RKComponent::updateEnablednessRecursive (bool parent_enabled) {
	RK_TRACE (PLUGIN);

	bool enabled = (enabledness_property->boolValue () && parent_enabled);
	bool changed = (enabled != isEnabled ());

	setEnabled (enabled);
	/* RKComponent hierarchy does not always correspond to QWidget hierarchy (although in _most_ cases, it does. For this reason,
	 * we need to update enabledness of all child components. */
	if (changed) {
		for (QHash<QString, RKComponentBase*>::const_iterator it = child_map.constBegin (); it != child_map.constEnd (); ++it) {
			if (it.value ()->isComponent()) {
				static_cast<RKComponent*> (it.value ())->updateEnablednessRecursive (enabled);
			}
		}
	}
}

bool RKComponent::isInactive () {
	if (!isEnabled ()) return true;
	if (parentWidget () && isHidden ()) return true;	// Note: Components embedded as button may be "hidden" without being inaccessible
	if (!visibility_property->boolValue ()) return true;	// Note for those, this is the appropriate check
	return false;
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

	emit (componentChanged (this));
}

RKStandardComponent *RKComponent::standardComponent (QString *id_adjust) const {
	RK_TRACE (PLUGIN);

	RKComponent *p = const_cast<RKComponent*> (this);
	while (p) {
		if (p->type () == RKComponent::ComponentStandard) return static_cast<RKStandardComponent*> (p);
		if (id_adjust) id_adjust->prepend (p->getIdInParent () + '.');
		p = p->parentComponent ();
	}
	RK_ASSERT (false);
	return 0;
}

RKStandardComponent* RKComponent::topmostStandardComponent () {
	RK_TRACE (PLUGIN);

	RKComponent *p = this;
	while (p->parentComponent ()) p = p->parentComponent ();
	if (p->type () == RKComponent::ComponentStandard) return static_cast<RKStandardComponent*> (p);
	// NOTE: currently, *only* standard components can be topmost
	RK_ASSERT (false);
	return 0;
}

XMLHelper* RKComponent::xmlHelper () const {
	RK_TRACE (PLUGIN);

	RKStandardComponent *sc = standardComponent ();
	return sc->getXmlHelper ();
}

void RKComponent::removeFromParent () {
	RK_TRACE (PLUGIN);

	if (!parentComponent ()) return;

	// unfortunately, several items might hvae the same key, and there seems to be no way to selectively remove the current item only.
	// however, this function should only ever be called in cases of emergency and to prevent crashes. So we make extra sure to remove the child,
	// even if we remove a little more than necessary along the way.
	QString key = getIdInParent ();
	while (parentComponent ()->child_map.remove (key)) {;}
	_parent = 0;
}

QString RKComponent::getIdInParent () const {
	RK_TRACE (PLUGIN);

	if (!parentComponent ()) return QString ();
	return (parentComponent ()->child_map.key (const_cast<RKComponent*> (this)));
}

#include "rkcomponent.moc"
