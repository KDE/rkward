/***************************************************************************
                          rkcomponent  -  description
                             -------------------
    begin                : Tue Dec 13 2005
    copyright            : (C) 2005, 2006, 2009, 2010, 2011, 2012 by Thomas Friedrichsmeier
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
		if (it.key () == "#noid#") continue;
		if (it.value ()->isInternal ()) continue;

		if (it.value ()->isProperty ()) {
			if (include_top_level) {
				list->insert (prefix + it.key (), static_cast<RKComponentPropertyBase*> (it.value ())->value ());
			}
		} else {
			RK_ASSERT (it.value ()->isComponent ());
			if (static_cast<RKComponent *> (it.value ())->isInactive ()) continue;
			it.value ()->fetchPropertyValuesRecursive (list, true, prefix + it.key () + '.');
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

RKComponent::UnserializeError RKComponentBase::unserializeState (const QStringList &state) {
	RK_TRACE (PLUGIN);

	QMap<QString, QString> props;

	for (int i = 0; i < state.count (); ++i) {
		QString line = state[i];
		int sep = line.indexOf ('=');
		if (sep < 0) return BadFormat;
		props.insert (RKCommonFunctions::unescape (line.left (sep)), RKCommonFunctions::unescape (line.mid (sep+1)));
	}

	setPropertyValues (&props);

	// verify
	UnserializeError error = NoError;
	for (QMap<QString, QString>::const_iterator it = props.constBegin (); it != props.constEnd (); ++it) {
		if (fetchStringValue (it.key ()) != it.value ()) {
			// COMPAT: In RKWard 0.5.1, the formatting of real numbers was different. Hence we compare the numeric values, instead
			QString dummy;
			RKComponentBase *prop = lookupComponent (it.key (), &dummy);
			if (dummy.isEmpty () && prop && prop->type () == PropertyDouble) {
				if (static_cast<RKComponentPropertyDouble*> (prop)->doubleValue () == it.value ().toDouble ()) {
					continue;
				}
			}

			RK_DO(qDebug ("Tried to apply value %s to property %s, but got %s", qPrintable (it.value ()), qPrintable (it.key ()), qPrintable (fetchStringValue (it.key ()))), PLUGIN, DL_WARNING);
			error = NotAllSettingsApplied;
		}
	}

	return error;
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
 	if (isComponent ()) RK_DO (qDebug ("component not satisfied: %s", qPrintable (static_cast<RKComponent*> (this)->getIdInParent ())), PLUGIN, DL_DEBUG);
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

RKStandardComponent *RKComponent::standardComponent (QString *id_adjust) {
	RK_TRACE (PLUGIN);

	RKComponent *p = this;
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
