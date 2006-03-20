/***************************************************************************
                          rkcomponentproperties  -  description
                             -------------------
    begin                : Fri Nov 25 2005
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
- How should invalid values be handled? Currently we keep the bad value as the string value, but use a corrected default in
the specialized properties (e.g. RKComponentPropertyInt::intValue () always returns something valid). Does this really make sense?

- Maybe Int and Double properties could be joined to a numeric property?

- Add something like RKComponentPropertySelect for a property that accepts one or more of a set of predefined strings (like e.g. for a radio-box)
	- Maybe then, and in conjunction with sub-properties, the bool-property can be abstracted away (it would just be a select property, and an internal int-property could be used for bool purposes)?

- Carefully check whether all API-elements are really needed once the implementation is complete

- All these TODOs should be delayed until there is at least a rudimentary implementation of components to play with
*/

#include "rkcomponentproperties.h"

#include "../debug.h"

///////////////////////////////////////////// Base //////////////////////////////////////////

RKComponentPropertyBase::RKComponentPropertyBase (QObject *parent, bool required) : QObject (parent) {
	RK_TRACE (PLUGIN);
	RKComponentPropertyBase::required = required;
	is_valid = true;
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
	// no need to reconcile any requirements, as the RKComponentPropertyBase does not have any requirements

	// fetch current value
	governorValueChanged (governor);
}

void RKComponentPropertyBase::governorValueChanged (RKComponentPropertyBase *property) {
	RK_TRACE (PLUGIN);

	setValue (property->value (governor_modifier));
}

void RKComponentPropertyBase::warnModifierNotRecognized (const QString &modifier) {
	RK_TRACE (PLUGIN);
	RK_DO (qDebug ("Modifier '%s' not recongnized.", modifier.latin1 ()), PLUGIN, DL_ERROR);
}

///////////////////////////////////////////// Bool //////////////////////////////////////////

RKComponentPropertyBool::RKComponentPropertyBool (QObject *parent, bool required, bool default_state, const QString &value_true, const QString &value_false) : RKComponentPropertyBase (parent, required) {
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
	if (modifier == "not") {
		if (current_value) return value_false;
		else return value_true;
	}
	if (modifier == "numeric") {
		if (current_value) return "1";
		else return "0";
	}

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
		RK_DO (qDebug ("default value in integer property is lower than lower boundary"), PLUGIN, DL_DEBUG);	// actually this is ok. In this case the default is simply "invalid"
	}
	if (current_value < lower) {
		setIntValue (lower);
	}
}

void RKComponentPropertyInt::setMax (int upper) {
	RK_TRACE (PLUGIN);

	validator->setTop (upper);
	if (default_value > upper) {
		RK_DO (qDebug ("default value in integer property is larger than upper boundary"), PLUGIN, DL_DEBUG);	// see above
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

RKComponentPropertyDouble::RKComponentPropertyDouble (QObject *parent, bool required, double default_value) : RKComponentPropertyBase (parent, required) {
	RK_TRACE (PLUGIN);

	validator = new QDoubleValidator (this);		// accepts all ints initially
	RKComponentPropertyDouble::default_value = default_value;
	precision = 6;
	internalSetValue (default_value);
}

RKComponentPropertyDouble::~RKComponentPropertyDouble () {
	RK_TRACE (PLUGIN);
}

bool RKComponentPropertyDouble::setDoubleValue (double new_value) {
	RK_TRACE (PLUGIN);

	internalSetValue (new_value);
	emit (valueChanged (this));
	return (isValid ());
}

bool RKComponentPropertyDouble::setValue (const QString &string) {
	RK_TRACE (PLUGIN);

	internalSetValue (string);
	emit (valueChanged (this));
	return (isValid ());
}

void RKComponentPropertyDouble::setMin (double lower) {
	RK_TRACE (PLUGIN);

	validator->setBottom (lower);
	if (default_value < lower) {
		RK_DO (qDebug ("default value in double property is lower than lower boundary"), PLUGIN, DL_DEBUG);	// actually this is ok. In this case the default is simply "invalid"
	}
	if (current_value < lower) {
		setDoubleValue (lower);
	}
}

void RKComponentPropertyDouble::setMax (double upper) {
	RK_TRACE (PLUGIN);

	validator->setTop (upper);
	if (default_value > upper) {
		RK_DO (qDebug ("default value in double property is larger than upper boundary"), PLUGIN, DL_DEBUG);	// see above
	}
	if (current_value > upper) {
		setDoubleValue (upper);
	}
}

double RKComponentPropertyDouble::minValue () {
	RK_TRACE (PLUGIN);
	RK_ASSERT (validator);

	return (validator->bottom ());
}

double RKComponentPropertyDouble::maxValue () {
	RK_TRACE (PLUGIN);
	RK_ASSERT (validator);

	return (validator->top ());
}

double RKComponentPropertyDouble::doubleValue () {
	RK_TRACE (PLUGIN);

	return current_value;
}

QString RKComponentPropertyDouble::value (const QString &modifier) {
	RK_TRACE (PLUGIN);

	if (!modifier.isEmpty ()) {
		warnModifierNotRecognized (modifier);
		return QString::null;
	}
	return _value;
}

bool RKComponentPropertyDouble::isStringValid (const QString &string) {
	RK_TRACE (PLUGIN);

	int dummy = 0;
	QString string_copy = string;
	return (validator->validate (string_copy, dummy) == QValidator::Acceptable);
}

void RKComponentPropertyDouble::connectToGovernor (RKComponentPropertyBase *governor, const QString &modifier, bool reconcile_requirements) {
	RK_TRACE (PLUGIN);

	RK_ASSERT (governor);
	connect (governor, SIGNAL (valueChanged (RKComponentPropertyBase *)), this, SLOT (governorValueChanged (RKComponentPropertyBase *)));
	governor_modifier = modifier;

	// reconcile requirements if applicable
	if (reconcile_requirements && governor_modifier.isEmpty ()) {
		if (governor->type () == PropertyInt) {
			RKComponentPropertyInt *igov = static_cast<RKComponentPropertyInt *> (governor); 	// convenience pointer
			if (validator->bottom () > igov->minValue ()) {
				igov->setMin ((int) validator->bottom ());			// no (real) need to worry about integer overflow, as we only do this if the integers bottom limit is lower. Bad things could happen, if the bottom limit of this double property is extremely large (> INT_MAX), but this should rarely happen.
			}
			if (validator->top () < igov->maxValue ()) {
				igov->setMax ((int) validator->top ());			// see above comment
			}
		} else if (governor->type () == PropertyDouble) {
			RKComponentPropertyDouble *dgov = static_cast<RKComponentPropertyDouble *> (governor); 	// convenience pointer
			if (validator->bottom () > dgov->minValue ()) {
				dgov->setMin (validator->bottom ());
			}
			if (validator->top () < dgov->maxValue ()) {
				dgov->setMax (validator->top ());
			}
		}
	}

	// fetch current value
	governorValueChanged (governor);
}

void RKComponentPropertyDouble::governorValueChanged (RKComponentPropertyBase *property) {
	RK_TRACE (PLUGIN);

	if (governor_modifier.isEmpty ()) {
		if (property->type () == PropertyInt) {
			internalSetValue ((double) (static_cast<RKComponentPropertyInt *>(property)->intValue ()));
		} else if (property->type () == PropertyDouble) {
			internalSetValue (static_cast<RKComponentPropertyDouble *>(property)->doubleValue ());
		} else {
			internalSetValue (property->value (QString::null));
		}
	} else {
		internalSetValue (property->value (governor_modifier));
	}
	emit (valueChanged (this));
}

QDoubleValidator *RKComponentPropertyDouble::getValidator () {
	RK_TRACE (PLUGIN);
	RK_ASSERT (validator);
	return validator;
}

void RKComponentPropertyDouble::internalSetValue (double new_value) {
	RK_TRACE (PLUGIN);

	current_value = new_value;
	_value = QString::number (current_value, 'g', precision);
	is_valid = ((new_value >= validator->bottom ()) && (new_value <= validator->top ()));
	if (!is_valid) current_value = default_value;
}

void RKComponentPropertyDouble::internalSetValue (QString new_value) {
	RK_TRACE (PLUGIN);

	current_value = new_value.toDouble (&is_valid);
	if (!is_valid) {
		_value = new_value;
		current_value = default_value;
		return;
	}
	internalSetValue (current_value);		// will check range and prettify _value
}

///////////////////////////////////////////////// RObjects ////////////////////////////////////////////////////////

#include "../rkglobals.h"
#include "../core/robjectlist.h"
#include "../core/rkvariable.h"
#include "../core/rcontainerobject.h"
#include "../core/rkmodificationtracker.h"
#include "../misc/rkobjectlistview.h"

RKComponentPropertyRObjects::RKComponentPropertyRObjects (QObject *parent, bool required) : RKComponentPropertyBase (parent, required) {
	RK_TRACE (PLUGIN);

// no initial requirements
	dims = min_length = max_length = min_num_objects = min_num_objects_if_any = max_num_objects = -1;
	separator = ";";

// get notifications about changed/removed objects
	connect (RKGlobals::tracker (), SIGNAL (objectRemoved (RObject *)), this, SLOT (removeObjectValue (RObject *)));
	connect (RKGlobals::tracker (), SIGNAL (objectPropertiesChanged (RObject *)), this, SLOT (objectPropertiesChanged (RObject *)));
}

RKComponentPropertyRObjects::~RKComponentPropertyRObjects () {
	RK_TRACE (PLUGIN);
}

void RKComponentPropertyRObjects::setListLength (int min_num_objects, int min_num_objects_if_any, int max_num_objects) {
	RK_TRACE (PLUGIN);

	RKComponentPropertyRObjects::min_num_objects = min_num_objects;
	RKComponentPropertyRObjects::min_num_objects_if_any = min_num_objects_if_any;
	RKComponentPropertyRObjects::max_num_objects = max_num_objects;

	validizeAll ();
}

bool RKComponentPropertyRObjects::addObjectValue (RObject *object) {
	RK_TRACE (PLUGIN);

	if (isObjectValid (object)) {
		if (!object_list.contains (object)) {
			object_list.append (object);
			checkListLengthValid ();
			emit (valueChanged (this));
		}
		return isValid ();
	}
	return false;
}

void RKComponentPropertyRObjects::removeObjectValue (RObject *object) {
	RK_TRACE (PLUGIN);

	ObjectList::Iterator it = object_list.find (object);
	if (it != object_list.end ()) {
		object_list.erase (it);
		checkListLengthValid ();
		emit (valueChanged (this));
	}
}

void RKComponentPropertyRObjects::setClassFilter (const QStringList &classes) {
	RK_TRACE (PLUGIN);

	RKComponentPropertyRObjects::classes = classes;
	validizeAll ();
}

void RKComponentPropertyRObjects::setTypeFilter (const QStringList &types) {
	RK_TRACE (PLUGIN);

	RKComponentPropertyRObjects::types = types;
	validizeAll ();
}

void RKComponentPropertyRObjects::setDimensionFilter (int dimensionality, int min_length, int max_length) {
	RK_TRACE (PLUGIN);

	dims = dimensionality;
	RKComponentPropertyRObjects::min_length = min_length;
	RKComponentPropertyRObjects::max_length = max_length;
	validizeAll ();
}

bool RKComponentPropertyRObjects::setObjectValue (RObject *object) {
	RK_TRACE (PLUGIN);

	object_list.clear ();
	return (addObjectValue (object));
}

void RKComponentPropertyRObjects::setObjectList (const ObjectList &newlist) {
	RK_TRACE (PLUGIN);

	bool changes = false;

	// remove items from the old list that are not in the new list
	ObjectList::Iterator it = object_list.begin ();
	while (it != object_list.end ()) {
		if (newlist.contains (*it)) {
			++it;
		} else {
			it = object_list.erase (it);		// it now points to the next object in the list
			changes = true;
		}
	}

	// now add items from the new list that are not in the old list
	ObjectList::const_iterator cit = newlist.begin ();
	while (cit != newlist.end ()) {
		if (!object_list.contains (*cit)) {
			if (isObjectValid (*cit));
			object_list.append (*cit);
			changes = true;
		}
		++cit;
	}

	// emit a signal if there have been any changes
	if (changes) {
		checkListLengthValid ();
		emit (valueChanged (this));
	}
}

bool RKComponentPropertyRObjects::isObjectValid (RObject *object) {
	RK_TRACE (PLUGIN);

	// first check dimensionality
	if (dims > 0) {
		if (object->numDimensions () != dims) return false;
	}
	int olength = object->getLength ();
	if ((min_length > 0) && (olength < min_length)) return false;
	if ((max_length >= 0) && (olength > max_length)) return false;

	// next, check classes
	if (!classes.isEmpty ()) {
		bool ok = false;
		QStringList::const_iterator it = classes.begin ();
		while ((!ok) && (it != classes.end ())) {
			if (object->inherits (*it)) {
				ok = true;
			}
			++it;
		}
		if (!ok) return false;
	}

	// finally, check type
	if (!types.isEmpty ()) {
		// TODO: this is not entirely correct, yet
		if (object->isVariable ()) {
			if (!types.contains (static_cast<RKVariable *> (object)->getVarTypeString ().lower ())) {
				return false;
			}
		}
	}

	return true;
}

RObject *RKComponentPropertyRObjects::objectValue () {
	RK_TRACE (PLUGIN);

	return (object_list.first ());
}

QValueList<RObject *> RKComponentPropertyRObjects::objectList () {
	RK_TRACE (PLUGIN);

	return (object_list);
}

QString RKComponentPropertyRObjects::value (const QString &modifier) {
	RK_TRACE (PLUGIN);

	QStringList ret;
	if (modifier.isEmpty ()) {
		for (ObjectList::const_iterator it = object_list.begin (); it != object_list.end ();++it) {
			ret.append ((*it)->getFullName ());
		}
	} else if (modifier == "shortname") {
		for (ObjectList::const_iterator it = object_list.begin (); it != object_list.end ();++it) {
			ret.append ((*it)->getShortName ());
		}
	} else if (modifier == "label") {
		for (ObjectList::const_iterator it = object_list.begin (); it != object_list.end ();++it) {
			ret.append ((*it)->getLabel ());
		}
	} else {
		warnModifierNotRecognized (modifier);
	}
	return ret.join (separator);
}

bool RKComponentPropertyRObjects::setValue (const QString &value) {
	RK_TRACE (PLUGIN);

	object_list.clear ();

	bool ok = true;
	QStringList slist = QStringList::split (separator, value);

	for (QStringList::const_iterator it = slist.begin (); it != slist.end (); ++it) {
		RObject *obj = RKGlobals::rObjectList ()->findObject (value);
		if (obj && isObjectValid (obj)) {
			object_list.append (obj);
		} else {
			ok = false;
		}
	}

	checkListLengthValid ();
	return (isValid () && ok);
}

bool RKComponentPropertyRObjects::isStringValid (const QString &value) {
	RK_TRACE (PLUGIN);

	QStringList slist = QStringList::split (separator, value);

	for (QStringList::const_iterator it = slist.begin (); it != slist.end (); ++it) {
		RObject *obj = RKGlobals::rObjectList ()->findObject (value);
		if (!(obj && isObjectValid (obj))) {
			return false;
		}
	}

	return true;
}

void RKComponentPropertyRObjects::connectToGovernor (RKComponentPropertyBase *governor, const QString &modifier, bool reconcile_requirements) {
	RK_TRACE (PLUGIN);

	RK_ASSERT (governor);
	connect (governor, SIGNAL (valueChanged (RKComponentPropertyBase *)), this, SLOT (governorValueChanged (RKComponentPropertyBase *)));
	governor_modifier = modifier;

	// reconcile requirements if applicable
	if (reconcile_requirements && governor_modifier.isEmpty ()) {
		if (governor->type () == PropertyRObjects) {
			RKComponentPropertyRObjects *ogov = static_cast<RKComponentPropertyRObjects *> (governor); 	// convenience pointer

			// reconcile dimensionality filter
			if (dims > 0) {
				if (ogov->dims <= 0) {
					ogov->dims = dims;
				} else if (ogov->dims != dims) {
					RK_DO (qDebug ("Could not reconcile dimensionality in RObject properties"), PLUGIN, DL_WARNING);
				}
			}
			if (ogov->min_length < min_length) {
				ogov->min_length = min_length;
			}
			if (max_length > 0) {
				if (ogov->max_length > max_length) {
					ogov->max_length = max_length;
				}
			}

			// reconcile number of objects filter
			if (ogov->min_num_objects < min_num_objects) {
				ogov->min_num_objects = min_num_objects;
			}
			if (ogov->min_num_objects_if_any < min_num_objects_if_any) {
				ogov->min_num_objects_if_any = min_num_objects_if_any;
			}
			if (max_num_objects && (ogov->max_num_objects > max_num_objects)) {
				ogov->max_num_objects = max_num_objects;
			}

			// reconcile class filter
			if (!classes.isEmpty ()) {
				if (ogov->classes.isEmpty ()) {
					ogov->classes= classes;
				} else {
					QStringList::Iterator it = ogov->classes.begin ();
					while (it != ogov->classes.end ()) {
						if (classes.contains (*it)) {
							++it;
						} else {
							ogov->classes.erase (it);		// automatically advances to the next item
						}
					}
					if (ogov->classes.isEmpty ()) {
						RK_DO (qDebug ("Incompatible class filters for RObject properties"), PLUGIN, DL_WARNING);
						ogov->classes = classes;
					}
				}
			}

			// reconcile type filter
			if (!types.isEmpty ()) {
				if (ogov->types.isEmpty ()) {
					ogov->types = types;
				} else {
					QStringList::Iterator it = ogov->types.begin ();
					while (it != ogov->types.end ()) {
						if (types.contains (*it)) {
							++it;
						} else {
							ogov->types.erase (it);		// automatically advances to the next item
						}
					}
					if (ogov->types.isEmpty ()) {
						RK_DO (qDebug ("Incompatible type filters for RObject properties"), PLUGIN, DL_WARNING);
						ogov->types = types;
					}
				}
			}

			// make governor recheck its values
			ogov->validizeAll ();
		}
	}

	// fetch current value
	governorValueChanged (governor);
}

void RKComponentPropertyRObjects::governorValueChanged (RKComponentPropertyBase *property) {
	RK_TRACE (PLUGIN);

	if ((property->type () == PropertyRObjects) && governor_modifier.isEmpty ()) {
		setObjectList (static_cast <RKComponentPropertyRObjects *> (property)->objectList ());
	} else {
		setValue (property->value (governor_modifier));
	}
}

void RKComponentPropertyRObjects::setFromListView (RKObjectListView *list_view, bool selected_only) {
	RK_TRACE (PLUGIN);

	// first find out the list of objects from the listview
	ObjectList newlist;
	QListViewItem *current;
	current = list_view->firstChild ();
	while (current->itemBelow ()) {
		current = current->itemBelow ();
		if ((!selected_only) || current->isSelected ()) {
			RObject *obj = list_view->findItemObject (current);
			RK_ASSERT (obj);
			newlist.append (static_cast<RKVariable*> (obj));
		}
	}

	// invalid objects will be ignored in setObjectList
	setObjectList (newlist);
}

void RKComponentPropertyRObjects::objectPropertiesChanged (RObject *object) {
	RK_TRACE (PLUGIN);

	// if object list contains this object, check whether it is still valid. Otherwise remove it, revalidize and signal change.
	ObjectList::Iterator it = object_list.find (object);
	if (it != object_list.end ()) {
		if (!isObjectValid (object)) {
			object_list.erase (it);
			checkListLengthValid ();
			emit (valueChanged (this));
		}
	}
}

void RKComponentPropertyRObjects::validizeAll (bool silent) {
	RK_TRACE (PLUGIN);

	bool changes = false;

	ObjectList::Iterator it = object_list.begin ();
	while (it != object_list.end ()) {
		if (isObjectValid (*it)) {
			++it;
		} else {
			it = object_list.erase (it);		// it now points to the next object in the list
			changes = true;
		}
	}

	checkListLengthValid ();		// we should do this even if there are no changes in the list. There might have still been changes in the filter!
	if (changes) {
		if (!silent) emit (valueChanged (this));
	}
}

void RKComponentPropertyRObjects::checkListLengthValid () {
	RK_TRACE (PLUGIN);

	is_valid = true;	// innocent until proven guilty
	if (min_num_objects || max_num_objects || min_num_objects_if_any) {
		int len = object_list.count ();
		if (len < min_num_objects) is_valid = false;
		if (len && (len < min_num_objects_if_any)) is_valid = false;
		if (max_num_objects && (len > max_num_objects)) is_valid = false;
	}
}

bool RKComponentPropertyRObjects::atMaxLength () {
	RK_TRACE (PLUGIN);

	int len = object_list.count ();
	if (max_num_objects && (len >= max_num_objects)) return true;
	return false;
}



/////////////////////////////////////////// Code ////////////////////////////////////////////////

RKComponentPropertyCode::RKComponentPropertyCode (QObject *parent, bool required) : RKComponentPropertyBase (parent, required) {
	RK_TRACE (PLUGIN);

	preprocess_code = calculate_code = printout_code = cleanup_code = QString::null;
}

RKComponentPropertyCode::~RKComponentPropertyCode () {
	RK_TRACE (PLUGIN);
}

QString RKComponentPropertyCode::value (const QString &modifier) {
	RK_TRACE (PLUGIN);

	if (modifier == "preprocess") return preprocess ();
	if (modifier == "calculate") return calculate ();
	if (modifier == "printout") return printout ();
	if (modifier == "cleanup") return cleanup ();
	if (!modifier.isEmpty ()) warnModifierNotRecognized (modifier);

	return (preprocess () + calculate () + printout () + cleanup ());
}

/////////////////////////////////////////// Convert ////////////////////////////////////////////////

RKComponentPropertyConvert::RKComponentPropertyConvert (RKComponent *parent) : RKComponentPropertyBool (parent, false) {
	RK_TRACE (PLUGIN);

	_mode = Equals;
	require_true = false;
	c_parent = parent;
	// get notified of own changes
	connect (this, SIGNAL (valueChanged (RKComponentPropertyBase *)), this, SLOT (selfChanged (RKComponentPropertyBase *)));
}

RKComponentPropertyConvert::~RKComponentPropertyConvert () {
	RK_TRACE (PLUGIN);
}

void RKComponentPropertyConvert::setMode (ConvertMode mode) {
	RK_TRACE (PLUGIN);

	_mode = mode;
	sourcePropertyChanged (0);
}

void RKComponentPropertyConvert::setSources (const QString &source_string) {
	RK_TRACE (PLUGIN);

	sources.clear ();
	QStringList source_ids = QStringList::split (";", source_string);
	for (QStringList::const_iterator it = source_ids.constBegin (); it != source_ids.constEnd (); ++it) {
		Source s;
		RKComponentBase *prop = c_parent->lookupComponent (*it, &(s.modifier));
		if (prop && prop->isProperty ()) {
			s.property = static_cast<RKComponentPropertyBase *>(prop);
			sources.append (s);
			connect (s.property, SIGNAL (valueChanged (RKComponentPropertyBase *)), this, SLOT (sourcePropertyChanged (RKComponentPropertyBase *)));
		} else {
			RK_DO (qDebug ("Not found or not a property: %s", (*it).latin1 ()), PLUGIN, DL_WARNING);
		}
	}

	sourcePropertyChanged (0);
}

void RKComponentPropertyConvert::setStandard (const QString &standard) {
	RK_TRACE (PLUGIN);

	RKComponentPropertyConvert::standard = standard;
	sourcePropertyChanged (0);
}

void RKComponentPropertyConvert::setRange (double min, double max) {
	RK_TRACE (PLUGIN);

	RKComponentPropertyConvert::min = min;
	RKComponentPropertyConvert::max = max;
	sourcePropertyChanged (0);
}

void RKComponentPropertyConvert::selfChanged (RKComponentPropertyBase *) {
	RK_TRACE (PLUGIN);

	c_parent->changed ();
}

void RKComponentPropertyConvert::sourcePropertyChanged (RKComponentPropertyBase *) {
	RK_TRACE (PLUGIN);

	for (QValueList<Source>::const_iterator it = sources.constBegin (); it != sources.constEnd (); ++it) {
		Source source = *it;		// easier typing
		switch (_mode) {
			case Equals: {
				if (source.property->value (source.modifier) != standard) {
					setBoolValue (false);
					return;
				}
				break;
			} case NotEquals: {
				if (source.property->value (source.modifier) == standard) {
					setBoolValue (false);
					return;
				}
				break;
			} case Range: {
				double val;
				if (source.property->type () == PropertyInt) {
					val = (double) static_cast<RKComponentPropertyInt *>(source.property)->intValue ();
				} else if (source.property->type () == PropertyDouble) {
					val = (double) static_cast<RKComponentPropertyDouble *>(source.property)->doubleValue ();
				} else {
					val = min;
					RK_DO (qDebug ("Non-numeric property in convert sources, cannot check range"), PLUGIN, DL_WARNING);
				}

				if ((min > val) || (max < val)) {
					setBoolValue (false);
					return;
				}
				break;
			} case And: {
				if ((source.property->type () == PropertyBool) || (source.property->type () == PropertyLogic)) {
					if (!(static_cast<RKComponentPropertyBool *>(source.property)->boolValue ())) {
						setBoolValue (false);
						return;
					}
				} else {
					RK_DO (qDebug ("Non-boolean property in convert sources, cannot check AND"), PLUGIN, DL_WARNING);
				}
				break;
			} case Or: {
				if ((source.property->type () == PropertyBool) || (source.property->type () == PropertyLogic)) {
					if (static_cast<RKComponentPropertyBool *>(source.property)->boolValue ()) {
						setBoolValue (true);
						return;
					}
				} else {
					RK_DO (qDebug ("Non-boolean property in convert sources, cannot check OR"), PLUGIN, DL_WARNING);
				}
				break;
			}
		}
	}

	// if we did not return above, this is the default value:
	switch (_mode) {
		case Equals:
		case NotEquals:
		case Range:
		case And: { setBoolValue (true); break; }
		case Or: { setBoolValue (false); break; }
	}
}

void RKComponentPropertyConvert::setRequireTrue (bool require_true) {
	RK_TRACE (PLUGIN);

	RKComponentPropertyConvert::require_true = require_true;
	required = require_true;
}

bool RKComponentPropertyConvert::isValid () {
	RK_TRACE (PLUGIN);

	if (require_true) {
		return (boolValue ());
	}

	return is_valid;
}

#include "rkcomponentproperties.moc"
