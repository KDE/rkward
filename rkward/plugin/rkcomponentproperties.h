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

#ifndef RKCOMPONETPROPERTIES_H
#define RKCOMPONETPROPERTIES_H

#include <qobject.h>

/** Base class for all RKComponentProperties. The base class can handle only a string-property. See derived classes for special types of properties.

see \ref RKComponentProperties
*/
class RKComponentPropertyBase : public QObject {
	Q_OBJECT
public:
/** constructor. Pass a valid QObject as parent so the property will be auto-destructed when no longer needed */
	RKComponentPropertyBase (QObject *parent);
/** destructor */
	virtual ~RKComponentPropertyBase ();
/** enum of types of properties. Used from RTTI */
	enum RKComponentPropertyTypes {
		PropertyBase = 1,
		PropertyBool = 2,
		PropertyInt = 3,
		PropertyDouble = 4,
		PropertyRObject = 5,
		PropertyRObjectList = 6,
		PropertyCode = 7,
		PropertyUser = 1000		/**< for user expansion */
	};
/** supplies the current value. Since more than one value may be supplied, modifier can be used to select a value. Default implementation only has  a single string, however. */
	virtual QString value (const QString &modifier=QString::null);
/** set the value in string form.
@returns false if the value is illegal (in the base class, all strings are legal) */
	virtual bool setValue (const QString &string);
/** do not set the value, only check, whether it is legal */
	virtual bool isValid (const QString &string);
/** current setting valid? */
	virtual bool isValid () { return true; };
/** set to required: will only be satisfied if it holds a valid value. Else: satisfied if valid *or empty* */
	void setRequired (bool require)  { required = require; };
/** see setRequired () */
	virtual bool isSatisfied () { return true; };
/** for RTTI. see RKComponentPropertyTypes */
	virtual int type () { return PropertyBase; };
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
	bool required;
/** if we're only interested in a specific sub-information of the governor-property, we need to remember the corresponding modifier */
	QString governor_modifier;
};

/** special type of RKComponentProperty, that is based on a bool setting */
class RKComponentPropertyBool : public RKComponentPropertyBase {
public:
/** param value_true string value if true/on
param value_false string value if false/off
param default value to use, if invalid string value was set */
	RKComponentPropertyBool (const QString &value_true, const QString &value_false, bool default_state);
/** sets the bool value. Also takes care of notifying dependent components */
	void setValue (bool value);
/** current value as bool */
	bool boolValue ();
/** reimplemented from RKComponentPropertyBase. Modifier "true" returns value if true. Modifier "false" returns value if false. Modifier QString::null returns current value. */
	QString value (const QString &modifier=QString::null);
/** reimplemented from RKComponentPropertyBase to convert to bool value according to current settings */
	bool setValue (const QString &value);
/** reimplemented from RKComponentPropertyBase to test whether conversion to bool value is possible according to current settings */
	bool isValid (const QString &value);
};

class RKComponentPropertyInt;		// min, max
class RKComponentPropertyDouble;		// min, max

class RObject;

/** special type of RKComponentProperty, that prepresents an RObject
//TODO: this property should auto-connect to RKModificationTracker, to be safe when the object gets deleted/changed */
class RKComponentPropertyRObject : public RKComponentPropertyBase {
public:
	void setClassFilter (const QString &classes);
	void setTypeFilter (const QString &types);
	void setDimensionFilter (int dimensionality, int min_length=-1, int max_length=-1);
	bool setValue (RObject *object);
	bool isValid (RObject *object);
	RObject *objectValue ();
/** reimplemented from RKComponentPropertyBase. Modifier "label" returns label. Modifier "shortname" returns short name. Modifier QString::null returns full name. */
	QString value (const QString &modifier=QString::null);
/** reimplemented from RKComponentPropertyBase to convert to RObject, if possible with current constraints */
	bool setValue (const QString &value);
/** reimplemented from RKComponentPropertyBase to test whether conversion to RObject, is possible with current constraints */
	bool isValid (const QString &value);
};

#include <qvaluelist.h>

/** extension of RKComponentPropertyRObject, allowing to hold several RObjects at once. */
class RKComponentPropertyRObjectList : public RKComponentPropertyRObject {
public:
	void setListLength (int min_length, int min_length_if_any=-1, int max_length=-1);
	bool addValue (RObject *object);
	void removeValue (RObject *object);
	bool isValid (RObject *object);
/** reimplemented from RKComponentPropertyBase to return the first RObject in the list */
	RObject *objectValue ();
	QValueList<RObject *> objectList ();
/** reimplemented from RKComponentPropertyBase. Modifier "label" returns label. Modifier "shortname" returns short name. Modifier QString::null returns full name. */
	QString value (const QString &modifier=QString::null);
/** reimplemented from RKComponentPropertyBase to convert to list of RObject, if possible with current constraints */
	bool setValue (const QString &value);
/** reimplemented from RKComponentPropertyBase to test whether conversion to list of RObject, is possible with current constraints */
	bool isValid (const QString &value);
};

/** special type of RKComponentProperty used to contain R code. All stand-alone RKComponents have this. The great thing about this, is that code can be made available to embedding RKComponents by just fetching the component.code.preprocess (or .calculate, .printout, .cleanup) value */
class RKComponentPropertyCode : public RKComponentPropertyBase {
public:
/** the preprocess code */
	QString preprocess ();
/** the calculate code */
	QString calculate ();
/** the printout code */		// TODO, maybe we can abstract this away. A component should _either_ do calculation _or_ printout, hence it could all be calculate () only, as well.
	QString printout ();
/** the cleanup code */
	QString cleanup ();
};


#endif

