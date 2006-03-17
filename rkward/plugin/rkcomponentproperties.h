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

#ifndef RKCOMPONETPROPERTIES_H
#define RKCOMPONETPROPERTIES_H

#include <qobject.h>

/* we need these foward declarations for now, due to cylcic includes. TODO: fix this */
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
/** supplies the current value. Since more than one value may be supplied, modifier can be used to select a value. Default implementation only has  a single string, however. */
	virtual QString value (const QString &modifier=QString::null);
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
};



///////////////////////////////////////////////// Bool ////////////////////////////////////////////////////////

/** special type of RKComponentProperty, that is based on a bool setting */
class RKComponentPropertyBool : public RKComponentPropertyBase {
public:
/** @param value_true string value if true/on
@param value_false string value if false/off
@param default_state value to use, if invalid string value was set */
	RKComponentPropertyBool (QObject *parent, bool required, const QString &value_true="TRUE", const QString &value_false="FALSE", bool default_state=true);
/** destructor */
	~RKComponentPropertyBool ();
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
private:
/** helper function. Sets the value without emitting change signal */
	void internalSetValue (bool new_value);
/** helper function. Sets the value without emitting change signal */
	void internalSetValue (QString new_value);
	bool default_value;
	bool current_value;
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
	void internalSetValue (QString new_value);
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
	void internalSetValue (QString new_value);
	double default_value;
	double current_value;
/** we could do without the validator, and create the logic on our own. Using Qt's validator, however, a) saves some typing b) allows to provide a validator object in use in lineedits, etc. (see getValidator ()) */
	QDoubleValidator *validator;
	int precision;
};

///////////////////////////////////////////////// RObjects ////////////////////////////////////////////////////////
class RObject;
class RKObjectListView;

#include <qstringlist.h>
#include <qvaluelist.h>

/** for easier typing. A list of RObjects. Should probably be defined somewhere in core-dir instead. */
typedef QValueList<RObject *> ObjectList;

/** special type of RKComponentProperty, that prepresents one or more RObject (s) */
class RKComponentPropertyRObjects : public RKComponentPropertyBase {
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
@param min_length Minimum length of first dimension. 0 will accept objects of all lenghts
@param max_length Maximum length of first dimension. 0 will accept objects of all lengths */
	void setDimensionFilter (int dimensionality=0, int min_length=0, int max_length=0);
/** Directly set an RObject. Warning: This sets the list to contain only exactly this one item. Generally you do not want to use this, unless your list is in single mode. Use addObjectValue () instead, if the property can hold more than one object
@returns false if the object does not qualify as a valid selection according to current settings (class/type/dimensions), true otherwise */
	bool setObjectValue (RObject *object);
/** set all the objects in the given new list (if valid), and only those. Emit a signal if there was any change */
	void setObjectList (const ObjectList &newlist);
/** Check whether an object is valid for this property.
@returns false if the object does not qualify as a valid selection according to current settings (class/type/dimensions), true otherwise */
	bool isObjectValid (RObject *object);
/** Get current object. If the property can hold several objects, only the first is returned. See objectList ().
@returns 0 if no valid object is selected */
	RObject *objectValue ();
/** Get current list of objects. Do not modify this list! It is the very same list, the property uses internally!
@returns an empty list if no valid object is selected */
	ObjectList objectList ();
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
/** fill this property with all objects shown in the given RKObjectListView. Emit a signal, if there was a change. Only valid objects are added!
@param selected_only if true, only the currently selected objects are filled into this property */
	void setFromListView (RKObjectListView *list_view, bool selected_only=false);
/** @returns true, if the property holds the maximum number of items (or more) */
	bool atMaxLength ();
public slots:
/** remove an object value. to be connected to RKModificationTracker::objectRemoved (). This is so we get notified if the object currently selected is removed TODO: is this effectively a duplication of setFromList? */
	void removeObjectValue (RObject *object);
/** to be connected to RKModificationTracker::objectPropertiesChanged (). This is so we get notified if the object currently selected is changed */
	void objectPropertiesChanged (RObject *object);
private:
/** check all objects currently in the list for validity. Remove invalid objects. Determine validity state depending on how many (valid) objects remain in the list. If the list was changed during validation, and silent!=false a valueChanged () signal is emitted */
	void validizeAll (bool silent=false);
/** simple helper function: Check whether the number of objects currently selected (and only that!), and set the valid state accordingly */
	void checkListLengthValid ();
	ObjectList object_list;
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

/** special type of RKComponentProperty used to contain R code. All stand-alone RKComponents have this. The great thing about this, is that code can be made available to embedding RKComponents by just fetching the component.code.preprocess (or .calculate, .printout, .cleanup) value */
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
/** the cleanup code */
	QString cleanup () { return cleanup_code; };

	QString value () { return (preprocess () + calculate () + printout () + cleanup ()); };

	void setPreprocess (const QString &code) { preprocess_code = code; };
	void setCalculate (const QString &code) { calculate_code = code; };
	void setPrintout (const QString &code) { printout_code = code; };
	void setCleanup (const QString &code) { cleanup_code = code; };

/** Sets all code to null strings and satisfied to false */
	void reset ();

	bool isValid () { return (have_preprocess && have_calculate && have_printout && have_cleanup); };

/** RTTI */
	int type () { return PropertyCode; };
private:
	QString preprocess_code;
	QString calculate_code;
	QString printout_code;
	QString cleanup_code;

	bool have_preprocess;
	bool have_calculate;
	bool have_printout;
	bool have_cleanup;
};


#endif

