/***************************************************************************
                          rkcomponentproperties  -  description
                             -------------------
    begin                : Fri Nov 25 2005
    copyright            : (C) 2005, 2006, 2007, 2009 by Thomas Friedrichsmeier
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

#ifndef RKCOMPONETPROPERTIES_H
#define RKCOMPONETPROPERTIES_H

#include <qobject.h>

#include <limits.h>

/* we need these forward declarations for now, due to cylcic includes. TODO: fix this */
class RKComponentPropertyBase;
class RKComponentPropertyBool;
#include "rkcomponent.h"

/** Base class for all RKComponentProperties. The base class can handle only a string-property. See derived classes for special types of properties.

see \ref RKComponentProperties
*/
class RKComponentPropertyBase : public QObject, public RKComponentBase {
	Q_OBJECT
public:
/** constructor. Pass a valid QObject as parent so the property will be auto-destructed when no longer needed */
	RKComponentPropertyBase (QObject *parent, bool required);
/** destructor */
	virtual ~RKComponentPropertyBase ();
/** supplies the current value. Since more than one value may be supplied, modifier can be used to select a value. Default implementation only has  a single string, however. Reimplemented from RKComponentBase */
	QString value (const QString &modifier=QString::null);
/** set the value in string form.
@returns false if the value is illegal (in the base class, all strings are legal) */
	virtual bool setValue (const QString &string);
/** do not set the value, only check, whether it is legal */
	virtual bool isStringValid (const QString &) { return true; };
/** current setting valid? */
	bool isValid () { return is_valid; };
/** for RTTI. see RKComponentBase::RKComponentTypes */
	int type () { return PropertyBase; };
/** connect this property to a governor property (given as argument). If reconcile_requirements, the requirements of both properties are reconciled to the least common denominator. The dependent property will be notified on all changes made in the governing property, so it can update its value. 
Generally with few exceptions, you can only connect to properties that are either of the same class as this property, or of an extended class. Maybe in the future we will add some sophisticated converters allowing to connect vastly different types of properties in a meaningful way.
If you specify a modifier, only the sub-value indicated by the modifier will be retrieved from the governing property on governorValueChanged. In this case reconcile_requirements is ignored. */
	virtual void connectToGovernor (RKComponentPropertyBase *governor, const QString &modifier=QString::null, bool reconcile_requirements=true);

/** Some properties will be marked as internal, such as visibility properties, which are not meant to be set directly by the user. These will be ignored in RKComponent::fetchPropertyValuesRecursive() */
	void setInternal (bool internal) { is_internal = internal; };
	bool isInternal () const { return is_internal; };
signals:
/** property has changed its value. Any connected RKComponentPropertys/RKComponents should update their state
@param property A pointer to the changed property for easy reference */
	void valueChanged (RKComponentPropertyBase *property);
public slots:
/** the (Qt-)slot in which (by default) the (RKComponent-)property is notified, when a property it depends on has changed. Generally you should reimplement this function to add special handling for the properties you know about. */
	virtual void governorValueChanged (RKComponentPropertyBase *property);
protected:
	void warnModifierNotRecognized (const QString &modifier);
	bool is_valid;
	QString _value;
/** if we're only interested in a specific sub-information of the governor-property, we need to remember the corresponding modifier */
	QString governor_modifier;
private:
	bool is_internal;
};



///////////////////////////////////////////////// Bool ////////////////////////////////////////////////////////

/** special type of RKComponentProperty, that is based on a bool setting */
class RKComponentPropertyBool : public RKComponentPropertyBase {
public:
/** @param value_true string value if true/on
@param value_false string value if false/off
@param default_state value to use, if invalid string value was set */
	RKComponentPropertyBool (QObject *parent, bool required, bool default_state=true, const QString &value_true="true", const QString &value_false="false");
/** destructor */
	~RKComponentPropertyBool ();
/** Set this property to the inverted, i.e. true if set to false and vice-versa. Used for the "not" sub-property. */
	void setInverted (bool invert) { inverted = invert; };
/** sets the bool value. Also takes care of notifying dependent components */
	void setBoolValue (bool new_value);
/** current value as bool */
	bool boolValue ();
/** reimplemented from RKComponentPropertyBase. Modifier "true" returns value if true. Modifier "false" returns value if false. Modifier QString::null returns current value. */
	QString value (const QString &modifier=QString::null);
/** reimplemented from RKComponentPropertyBase to convert to bool value according to current settings */
	bool setValue (const QString &string);
/** reimplemented from RKComponentPropertyBase to test whether conversion to bool value is possible according to current settings */
	bool isStringValid (const QString &string);
/** reimplemented from RKComponentPropertyBase to use special handling for bool and int properties (bools are copied directly, int handling: 0->false else true) */
	void governorValueChanged (RKComponentPropertyBase *property);
/** RTTI */
	int type () { return PropertyBool; };
/** reimplemented to return a new negated boolean property if the identifier is "not" */
	RKComponentBase* lookupComponent (const QString &identifier, QString *remainder);
private:
/** helper function. Sets the value without emitting change signal */
	void internalSetValue (bool new_value);
/** helper function. Sets the value without emitting change signal */
	void internalSetValue (const QString &new_value);
	bool default_value;
	bool current_value;
	bool inverted;
	QString value_true;
	QString value_false;
};

///////////////////////////////////////////////// Int ////////////////////////////////////////////////////////
#include <qvalidator.h>

class RKComponentPropertyInt : public RKComponentPropertyBase {
public:
/** constructor
@param default_value value to use, if invalid string value was set */
	RKComponentPropertyInt (QObject *parent, bool required, int default_value);
/** destructor */
	~RKComponentPropertyInt ();
/** sets the int value. Also takes care of notifying dependent components */
	bool setIntValue (int new_value);
/** set lower boundary. Default parameter will effectively remove the boundary. You should call this *before* connecting to any other properties, so limits can be reconciled */
	void setMin (int lower=INT_MIN);
/** set upper boundary. Default parameter will effectively remove the boundary. You should call this *before* connecting to any other properties, so limits can be reconciled */
	void setMax (int upper=INT_MAX);
/** return current min value */
	int minValue ();
/** return current max value */
	int maxValue ();
/** current value as int */
	int intValue ();
/** reimplemented from RKComponentPropertyBase. Return current value as a string. In the future, modifier might be used for format. */
	QString value (const QString &modifier=QString::null);
/** reimplemented from RKComponentPropertyBase to convert to int value according to current settings */
	bool setValue (const QString &string);
/** reimplemented from RKComponentPropertyBase to test whether conversion to int value is possible according to current settings (is a number, and within limits min and max) */
	bool isStringValid (const QString &string);
/** reimplemented from RKComponentPropertyBase to actually reconcile requirements with other numeric slots */
	void connectToGovernor (RKComponentPropertyBase *governor, const QString &modifier=QString::null, bool reconcile_requirements=true);
/** reimplemented from RKComponentPropertyBase to use special handling for int and double properties (ints are copied directly, doubles are rounded) */
	void governorValueChanged (RKComponentPropertyBase *property);
/** RTTI */
	int type () { return PropertyInt; };
/** returns a validator for use in lineedits or similar widgets. */
	QIntValidator *getValidator ();
private:
/** helper function. Sets the value without emitting change signal */
	void internalSetValue (int new_value);
/** helper function. Sets the value without emitting change signal */
	void internalSetValue (const QString &new_value);
	int default_value;
	int current_value;
/** we could do without the validator, and create the logic on our own. Using Qt's validator, however, a) saves some typing b) allows to provide a validator object in use in lineedits, etc. (see getValidator ()) */
	QIntValidator *validator;
};

///////////////////////////////////////////////// Double ////////////////////////////////////////////////////////
#include <float.h>

class RKComponentPropertyDouble :public RKComponentPropertyBase {
public:
/** constructor
@param default_value value to use, if invalid string value was set */
	RKComponentPropertyDouble (QObject *parent, bool required, double default_value);
/** destructor */
	~RKComponentPropertyDouble ();
/** sets the int value. Also takes care of notifying dependent components */
	bool setDoubleValue (double new_value);
/** reimplemented from RKComponentPropertyBase to convert to int value according to current settings */
	bool setValue (const QString &string);
/** set lower boundary. Default parameter will effectively remove the boundary. You should call this *before* connecting to any other properties, so limits can be reconciled */
	void setMin (double lower=FLT_MIN);
/** set upper boundary. Default parameter will effectively remove the boundary. You should call this *before* connecting to any other properties, so limits can be reconciled */
	void setMax (double upper=FLT_MAX);
/** set text precision (default = 6) */
	void setPrecision (int digits) { precision = digits; };
/** return current min value */
	double minValue ();
/** return current max value */
	double maxValue ();
/** current value as double */
	double doubleValue ();
/** reimplemented from RKComponentPropertyBase. Return current value as a string. In the future, modifier might be used for format. */
	QString value (const QString &modifier=QString::null);
/** reimplemented from RKComponentPropertyBase to test whether conversion to int value is possible according to current settings (is a number, and within limits min and max) */
	bool isStringValid (const QString &string);
/** reimplemented from RKComponentPropertyBase to actually reconcile requirements with other numeric slots */
	void connectToGovernor (RKComponentPropertyBase *governor, const QString &modifier=QString::null, bool reconcile_requirements=true);
/** reimplemented from RKComponentPropertyBase to use special handling for int and double properties (ints and doubles are copied directly) */
	void governorValueChanged (RKComponentPropertyBase *property);
/** RTTI */
	int type () { return PropertyDouble; };
/** returns a validator for use in lineedits or similar widgets. */
	QDoubleValidator *getValidator ();
private:
/** helper function. Sets the value without emitting change signal */
	void internalSetValue (double new_value);
/** helper function. Sets the value without emitting change signal */
	void internalSetValue (const QString &new_value);
	double default_value;
	double current_value;
/** we could do without the validator, and create the logic on our own. Using Qt's validator, however, a) saves some typing b) allows to provide a validator object in use in lineedits, etc. (see getValidator ()) */
	QDoubleValidator *validator;
	int precision;
};

///////////////////////////////////////////////// RObjects ////////////////////////////////////////////////////////
class RKObjectListView;

#include <qstringlist.h>
#include <qlist.h>

#include "../core/robject.h"
#include "../core/rkmodificationtracker.h"

/** special type of RKComponentProperty, that prepresents one or more RObject (s) */
class RKComponentPropertyRObjects : public RKComponentPropertyBase, public RObjectListener {
	Q_OBJECT
public:
/** constructor */
	RKComponentPropertyRObjects (QObject *parent, bool required);
/** destructor */
	~RKComponentPropertyRObjects ();
/** how many objects can this property hold? Use default values (0) to remove constraints
@param min_num_objects Minimum number of objects for this property to be valid
@param min_num_objects_if_any Some properties may be valid, if they hold either no objects at all, or at least a certain number of objects
@param max_num_objects Maximum number of objects for this property to be valid */
	void setListLength (int min_num_objects=0, int min_num_objects_if_any=0, int max_num_objects=0);
/** add an object value */
	bool addObjectValue (RObject *object);
/** Set property to only accept certain classes. If you provide an empty list, all classes will be accepted*/
	void setClassFilter (const QStringList &classes);
/** Set property to only accept certain object types. If you provide an empty list, all types will be accepted */
	void setTypeFilter (const QStringList &types);
/** Set property to only accept objects of certain dimensions. If you provide default parameters (0), all objects will be accepted
@param dimensionality Number of dimensions the object must have. 0 will accept objects of all dimensions
@param min_length Minimum length of first dimension. 0 will accept objects of all lengths
@param max_length Maximum length of first dimension. 0 (or INT_MAX) will accept objects of all lengths */
	void setDimensionFilter (int dimensionality=0, int min_length=0, int max_length=0);
/** Directly set an RObject. Warning: This sets the list to contain only exactly this one item. Generally you do not want to use this, unless your list is in single mode. Use addObjectValue () instead, if the property can hold more than one object
@returns false if the object does not qualify as a valid selection according to current settings (class/type/dimensions), true otherwise */
	bool setObjectValue (RObject *object);
/** set all the objects in the given new list (if valid), and only those. Emit a signal if there was any change */
	void setObjectList (const RObject::ObjectList &newlist);
/** Check whether an object is valid for this property.
@returns false if the object does not qualify as a valid selection according to current settings (class/type/dimensions), true otherwise */
	bool isObjectValid (RObject *object);
/** Get current object. If the property can hold several objects, only the first is returned. See objectList ().
@returns 0 if no valid object is selected */
	RObject *objectValue ();
/** Get current list of objects. Do not modify this list! It is the very same list, the property uses internally!
@returns an empty list if no valid object is selected */
	RObject::ObjectList objectList ();
/** set separator (used to concatenate object names/labels, etc. if more than one object is selected) */
	void setSeparator (const QString &sep) { separator = sep; emit (valueChanged (this)); };
/** reimplemented from RKComponentPropertyBase. Modifier "label" returns label. Modifier "shortname" returns short name. Modifier QString::null returns full name. If no object is set, returns an empty string */
	QString value (const QString &modifier=QString::null);
/** reimplemented from RKComponentPropertyBase to convert to RObject with current constraints
@returns false if no such object(s) could be found or the object(s) are invalid */
	bool setValue (const QString &value);
/** reimplemented from RKComponentPropertyBase to test whether conversion to RObject is possible with current constraints */
	bool isStringValid (const QString &value);
/** RTTI */
	int type () { return PropertyRObjects; };
/** reimplemented from RKComponentPropertyBase to actually reconcile requirements with other object properties */
	void connectToGovernor (RKComponentPropertyBase *governor, const QString &modifier=QString::null, bool reconcile_requirements=true);
/** reimplemented from RKComponentPropertyBase to use special handling for object properties */
	void governorValueChanged (RKComponentPropertyBase *property);
/** @returns true, if the property holds the maximum number of items (or more) */
	bool atMaxLength ();
	void removeObjectValue (RObject* object) { objectRemoved (object); };
protected:
/** remove an object value. reimplemented from RObjectListener::objectRemoved (). This is so we get notified if the object currently selected is removed TODO: is this effectively a duplication of setFromList? */
	void objectRemoved (RObject *removed);
/** reimplemented from RObjectListener::objectMetaChanged (). This is so we get notified if the object currently selected is changed */
	void objectMetaChanged (RObject *changed);
private:
/** check all objects currently in the list for validity. Remove invalid objects. Determine validity state depending on how many (valid) objects remain in the list. If the list was changed during validation, and silent!=false a valueChanged () signal is emitted */
	void validizeAll (bool silent=false);
/** simple helper function: Check whether the number of objects currently selected (and only that!), and set the valid state accordingly */
	void checkListLengthValid ();
	RObject::ObjectList object_list;
	int dims;
	int min_length;
	int max_length;
	int min_num_objects;
	int min_num_objects_if_any;
	int max_num_objects;
	QStringList classes;
/** TODO: use a list of enums instead for internal purposes! */
	QStringList types;
	QString separator;
};

///////////////////////////////////////////////// Code ////////////////////////////////////////////////////////

/** special type of RKComponentProperty used to contain R code. All stand-alone RKComponents have this. The great thing about this, is that code can be made available to embedding RKComponents by just fetching the component.code.preprocess (or .calculate, .printout) value */
class RKComponentPropertyCode : public RKComponentPropertyBase {
	Q_OBJECT
public:
/** constructor */
	RKComponentPropertyCode (QObject *parent, bool required);
/** destructor */
	~RKComponentPropertyCode ();
/** the preprocess code */
	QString preprocess () { return preprocess_code; };
/** the calculate code */
	QString calculate () { return calculate_code; };
/** the printout code */		// TODO, maybe we can abstract this away. A component should _either_ do calculation _or_ printout, hence it could all be calculate () only, as well.
	QString printout () { return printout_code; };
/** the preview code */
	QString preview () { return preview_code; };

	QString value (const QString &modifier=QString::null);

/** set the preprocess code.
@param code The code to set. If this is QString::null, the property is seen to lack preprocess code and hence is not valid (see isValid ()). In contrast, empty strings are seen as valid */
	void setPreprocess (const QString &code) { preprocess_code = code; emit (valueChanged (this)); };
/** see setPreprocess () */
	void setCalculate (const QString &code) { calculate_code = code; emit (valueChanged (this)); };
/** see setPreprocess () */
	void setPrintout (const QString &code) { printout_code = code; emit (valueChanged (this)); };
/** see setPreview () */
	void setPreview (const QString &code) { preview_code = code; emit (valueChanged (this)); };

	bool isValid () { return (!(preprocess_code.isNull () || calculate_code.isNull () || printout_code.isNull ())); };

/** RTTI */
	int type () { return PropertyCode; };
private:
	QString preprocess_code;
	QString calculate_code;
	QString printout_code;
	QString preview_code;
};

class RKComponent;	// what the ... why do I need this?
/** A bool component that can do some logical transformations. This property works slightly different than the others, and is not intended to be connected the usual way. Also by default it is not required, since it does not really hold a validity state (unless in require mode).

@author Thomas Friedrichsmeier */
class RKComponentPropertyConvert : public RKComponentPropertyBool {
	Q_OBJECT
public:
/** constructor. Note that this property *requires* an RKComponent as parent (the one at the top of all the source properties) */
	RKComponentPropertyConvert (RKComponent *parent);
	~RKComponentPropertyConvert ();

/** Mode of operation. see setMode () */
	enum ConvertMode {
		Equals = 0,		/** < Check, whether some property has exactly the given string value (see setSources (), setStandard ()) */
		NotEquals = 1,		/** < Opposite of ConvertMode::Equals */
		Range = 2,		/** < Check, whether some *numeric* property is in the given range (see setSources (), setRange ()) */
		And = 3,			/** < Check, whether several *boolean* properties are all true at once (see setSources ()) */
		Or = 4				/** < Check, whether one of several *boolean* properties is true (see setSources ()) */
	};

/** set the mode. Usually you will only call this once, after construction, and usually followed by setSources () - if applicable - setStandard () or setRange (). */
	void setMode (ConvertMode mode);
/** set the sources, i.e. the properties to check against. To specify multiple sources, separate their IDs with ";" */
	void setSources (const QString &source_string);
/** set the standard for comparison. Only meaningful in Equals mode */
	void setStandard (const QString &standard);
/** set the range for comparison. Only meaningful in Range mode */
	void setRange (double min, double max);
/** set this property to required. Also it will only be valid if it is currently true. So it will block it's parent component in false state. */
	void setRequireTrue (bool require_true);

/** reimplemented for setRequireTrue ()*/
	bool isValid ();

/** string represenation of the options in ConvertMode. For use in XMLHelper::getMultiChoiceAttribute */
	static QString convertModeOptionString () { return ("equals;notequals;range;and;or"); };
public slots:
/** unfortuntely, as the parent component likely does not know about us, we have to notify it manually of any changes. That's done from this slot */
	void selfChanged (RKComponentPropertyBase *);
/** a source property changed. Check the state */
	void sourcePropertyChanged (RKComponentPropertyBase *);
private:
	ConvertMode _mode;
	double min, max;
	QString standard;
	bool require_true;
	RKComponent *c_parent;		// actually the same as parent (), but without the hassle of conversion
	struct Source {
		RKComponentPropertyBase *property;
		QString modifier;
	};
	QList<Source> sources;
};

#endif

