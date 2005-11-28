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

\section TODO TODO
How should invalid values be handled? Currently we keep the bad value as the string value, but use a corrected default in
the specialized properties (e.g. RKComponentPropertyInt::intValue () always returns something valid). Does this really make sense?
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
	governorValueChanged (governor);
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

void RKComponentPropertyBool::setBoolValue (bool new_value) {
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

void RKComponentPropertyBool::governorValueChanged (RKComponentPropertyBase *property) {
	RK_TRACE (PLUGIN);

	if (governor_modifier.isEmpty ()) {
		if (property->type () == PropertyBool) {
			internalSetValue (static_cast<RKComponentPropertyBool *>(property)->boolValue ());
		} else if (property->type () == PropertyInt) {
			internalSetValue (static_cast<RKComponentPropertyInt *>(property)->intValue () != 0);
		} else {
			internalSetValue (property->value (QString::null));
		}
	} else {
		internalSetValue (property->value (governor_modifier));
	}
	emit (valueChanged (this));
}


///////////////////////////////////////////// Int //////////////////////////////////////////
RKComponentPropertyInt::RKComponentPropertyInt (QObject *parent, bool required, int default_value) : RKComponentPropertyBase (parent, required) {
	RK_TRACE (PLUGIN);

	validator = new QIntValidator (this);		// accepts all ints initially
	RKComponentPropertyInt::default_value = default_value;
	internalSetValue (default_value);
}

RKComponentPropertyInt::~RKComponentPropertyInt () {
	RK_TRACE (PLUGIN);
}

bool RKComponentPropertyInt::setIntValue (int new_value) {
	RK_TRACE (PLUGIN);

	internalSetValue (new_value);
	emit (valueChanged (this));
	return (isValid ());
}

bool RKComponentPropertyInt::setValue (const QString &string) {
	RK_TRACE (PLUGIN);

	internalSetValue (string);
	emit (valueChanged (this));
	return (isValid ());
}

void RKComponentPropertyInt::setMin (int lower) {
	RK_TRACE (PLUGIN);

	validator->setBottom (lower);
	if (default_value < lower) {
		RK_DO (qDebug ("default value in integer property is lower than lower boundary"), PLUGIN, DL_WARNING);
		default_value = lower;
	}
	if (current_value < lower) {
		setIntValue (lower);
	}
}

void RKComponentPropertyInt::setMax (int upper) {
	RK_TRACE (PLUGIN);

	validator->setTop (upper);
	if (default_value > upper) {
		RK_DO (qDebug ("default value in integer property is larger than upper boundary"), PLUGIN, DL_WARNING);
		default_value = upper;
	}
	if (current_value > upper) {
		setIntValue (upper);
	}
}

int RKComponentPropertyInt::minValue () {
	RK_TRACE (PLUGIN);
	RK_ASSERT (validator);

	return (validator->bottom ());
}

int RKComponentPropertyInt::maxValue () {
	RK_TRACE (PLUGIN);
	RK_ASSERT (validator);

	return (validator->top ());
}

int RKComponentPropertyInt::intValue () {
	RK_TRACE (PLUGIN);

	return current_value;
}

QString RKComponentPropertyInt::value (const QString &modifier) {
	RK_TRACE (PLUGIN);

	if (!modifier.isEmpty ()) {
		warnModifierNotRecognized (modifier);
		return QString::null;
	}
	return _value;
}

bool RKComponentPropertyInt::isStringValid (const QString &string) {
	RK_TRACE (PLUGIN);

	int dummy = 0;
	QString string_copy = string;
	return (validator->validate (string_copy, dummy) == QValidator::Acceptable);
}

void RKComponentPropertyInt::connectToGovernor (RKComponentPropertyBase *governor, const QString &modifier, bool reconcile_requirements) {
	RK_TRACE (PLUGIN);

	RK_ASSERT (governor);
	connect (governor, SIGNAL (valueChanged (RKComponentPropertyBase *)), this, SLOT (governorValueChanged (RKComponentPropertyBase *)));
	governor_modifier = modifier;

	// reconcile requirements if applicable
	if (reconcile_requirements && governor_modifier.isEmpty ()) {
		if (governor->type () == PropertyInt) {
			RKComponentPropertyInt *igov = static_cast<RKComponentPropertyInt *> (governor); 	// convenience pointer
			if (validator->bottom () > igov->minValue ()) {
				igov->setMin (validator->bottom ());
			}
			if (validator->top () < igov->maxValue ()) {
				igov->setMax (validator->top ());
			}
		} else if (governor->type () == PropertyDouble) {
			RKComponentPropertyDouble *dgov = static_cast<RKComponentPropertyDouble *> (governor); 	// convenience pointer
			if (validator->bottom () > (int) dgov->minValue ()) {
				dgov->setMin (validator->bottom ());
			}
			if (validator->top () < (int) dgov->maxValue ()) {
				dgov->setMax (validator->top ());
			}
		}
	}

	// fetch current value
	governorValueChanged (governor);
}

void RKComponentPropertyInt::governorValueChanged (RKComponentPropertyBase *property) {
	RK_TRACE (PLUGIN);

	if (governor_modifier.isEmpty ()) {
		if (property->type () == PropertyInt) {
			internalSetValue (static_cast<RKComponentPropertyInt *>(property)->intValue ());
		} else if (property->type () == PropertyDouble) {
			internalSetValue ((int) (static_cast<RKComponentPropertyDouble *>(property)->doubleValue ()));
		} else {
			internalSetValue (property->value (QString::null));
		}
	} else {
		internalSetValue (property->value (governor_modifier));
	}
	emit (valueChanged (this));
}

QIntValidator *RKComponentPropertyInt::getValidator () {
	RK_TRACE (PLUGIN);
	RK_ASSERT (validator);
	return validator;
}

void RKComponentPropertyInt::internalSetValue (int new_value) {
	current_value = new_value;
	_value = QString::number (current_value);
	is_valid = ((new_value >= validator->bottom ()) && (new_value <= validator->top ()));
	if (!is_valid) current_value = default_value;
}

void RKComponentPropertyInt::internalSetValue (QString new_value) {
	current_value = new_value.toInt (&is_valid);
	if (!is_valid) {
		_value = new_value;
		current_value = default_value;
		return;
	}
	internalSetValue (current_value);		// will check range and prettify _value
}

///////////////////////////////////////////// Double //////////////////////////////////////////

#include "rkcomponentproperties.moc"
