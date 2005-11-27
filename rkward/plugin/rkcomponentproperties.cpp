/***************************************************************************
                          rkcomponentproperties  -  description
                             -------------------
    begin                : Fri Nov 25 2005
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

/**
\page RKComponentProperties Components and Properties
\brief Provides an overview of RKComponentProperty classes and how they are used in rkward (plugin) components

This page provides a rough overview, about how the RKComponentProperty classes work (see \ref RKComponentPropertyBase to find out which classes are derived from that).

\section RKComponentPropertiesIntro What are "component properties" anyway?

A detailed elsewhere, RKWard is extensible using so called "components", in some cases also called plugins. These run-time created components usually provide a GUI for the user to select some variables, make some settings, and then generate some R-code to carry out the requested action.

For several reasons it is desirable in components to have some separation between the actual settings/properties, and the GUI used to manipulate those settings. For instance, as the most important use case when embedding components into each other, some settings may already be preset.

As an example, suppose you want to create a component to provide two simple related pieces of information on a single variable, (let's say mean and median to have a somewhat useless but simple example). You could easily do this by just creating a component that does those two actions. However, if both parts of it (mean and median) already exist, why not just reuse those? So instead we create a component that does not do anything more than simply embed the two above mentioned ones. However this poses a few practical challenges. For one thing, now you only want one varslot to select variables, while previously both components provided their own varslot. So we'll have to hide one of the varslots. Further, when the setting in that varslot changes, the other embedded component (the one with the varslot now hidded) should automatically be updated accordingly.

The solution is to provide a "property" for the variable selected. This property can be connected to another "property". So in this case, we'd connect the property "variable selected for median" to the property "variable selected for mean". After that the two properties will automatically keep in sync and this without the components even knowing what's going on internally.

\section RKComponentPropertyTypes Types of Properties

\section RKComponentPropertyModifiers Modifiers: Properties of properties

\section RKComponentPropertyConversions Future extensions: Conversions and logic

\section RKComponentPropertyInternals Internal workings

\section RKComponentPropertiesAndComponents Interaction with RKComponents

*/

#include "rkcomponentproperties.h"

#include "../debug.h"

///////////////////////////////////////////// Base //////////////////////////////////////////

RKComponentPropertyBase::RKComponentPropertyBase (QObject *parent, bool required) : QObject (parent) {
	RK_TRACE (PLUGIN);
	RKComponentPropertyBase::required = required;
}

RKComponentPropertyBase::~RKComponentPropertyBase () {
	RK_TRACE (PLUGIN);
}

QString RKComponentPropertyBase::value (const QString &modifier) {
	RK_TRACE (PLUGIN);
	if (!modifier.isEmpty ()) {
		warnModifierNotRecognized (modifier);
		return QString::null;
	}
	return _value;
}

bool RKComponentPropertyBase::setValue (const QString &string) {
	RK_TRACE (PLUGIN);

	_value = string;
	emit (valueChanged (this));
	return true;
}

void RKComponentPropertyBase::connectToGovernor (RKComponentPropertyBase *governor, const QString &modifier, bool) {
	RK_TRACE (PLUGIN);

	RK_ASSERT (governor);
	connect (governor, SIGNAL (valueChanged (RKComponentPropertyBase *)), this, SLOT (governorValueChanged (RKComponentPropertyBase *)));
	governor_modifier = modifier;
	// no need to reconcile any requirements, and the RKComponentPropertyBase does not have any requirements

	// fetch current value
	setValue (governor->value (modifier));
}

void RKComponentPropertyBase::governorValueChanged (RKComponentPropertyBase *property) {
	RK_TRACE (PLUGIN);

	setValue (property->value (governor_modifier));
}

bool RKComponentPropertyBase::isSatisfied () {
	RK_TRACE (PLUGIN);
	if (!required) return true;
	if (isValid ()) return true;
	return false;		// never happens in RKComponentPropertyBase, but might in subclasses
}

void RKComponentPropertyBase::warnModifierNotRecognized (const QString &modifier) {
	RK_TRACE (PLUGIN);
	RK_DO (qDebug ("Modifier '%s' not recongnized.", modifier.latin1 ()), PLUGIN, DL_ERROR);
}

///////////////////////////////////////////// Bool //////////////////////////////////////////

RKComponentPropertyBool::RKComponentPropertyBool (QObject *parent, bool required, const QString &value_true, const QString &value_false, bool default_state) : RKComponentPropertyBase (parent, required) {
	RK_TRACE (PLUGIN);
	RKComponentPropertyBool::value_true = value_true;
	RKComponentPropertyBool::value_false = value_false;
	default_value = default_state;
	internalSetValue (default_state);
}

RKComponentPropertyBool::~RKComponentPropertyBool () {
	RK_TRACE (PLUGIN);
}

void RKComponentPropertyBool::internalSetValue (bool new_value) {
	RK_TRACE (PLUGIN);

	current_value = new_value;
	if (new_value) {
		_value = value_true;
	} else {
		_value = value_false;
	}
	is_valid = true;
}

void RKComponentPropertyBool::internalSetValue (QString new_value) {
	RK_TRACE (PLUGIN);

	_value = new_value;

// try to convert to bool
	is_valid = true;
	if (new_value == value_true) {
		current_value = true;
	} else if (new_value == value_false) {
		current_value = false;
	} else {
		is_valid = false;
	}
}

void RKComponentPropertyBool::setValue (bool new_value) {
	RK_TRACE (PLUGIN);

	internalSetValue (new_value);
	emit (valueChanged (this));
}

bool RKComponentPropertyBool::boolValue () {
	RK_TRACE (PLUGIN);

	return current_value;
}

QString RKComponentPropertyBool::value (const QString &modifier) {
	RK_TRACE (PLUGIN);

	if (modifier.isEmpty ()) return _value;
	if (modifier == "true") return value_true;
	if (modifier == "false") return value_false;

	warnModifierNotRecognized (modifier);
	return QString::null;
}

bool RKComponentPropertyBool::setValue (const QString &string) {
	RK_TRACE (PLUGIN);

	internalSetValue (string);
	emit (valueChanged (this));
	return isValid ();
}

bool RKComponentPropertyBool::isStringValid (const QString &string) {
	RK_TRACE (PLUGIN);

	if ((string == value_true) || (string == value_false)) {
		return true;
	}
	return false;
}

#include "rkcomponentproperties.moc"
