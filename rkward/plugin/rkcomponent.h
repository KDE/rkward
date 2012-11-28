/***************************************************************************
                          rkcomponent  -  description
                             -------------------
    begin                : Tue Dec 13 2005
    copyright            : (C) 2005, 2006, 2007, 2009, 2010, 2011, 2012 by Thomas Friedrichsmeier
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

#ifndef RKCOMPONENT_H
#define RKCOMPONENT_H

#include <QHash>
#include <qmap.h>
#include <qwidget.h>

class RKComponentPropertyBase;
class RKStandardComponent;

/** a very low level base for RKComponent and RKComponentProperty. */
class RKComponentBase {
public:
/** constructor */
	RKComponentBase () { required=true; is_internal=false; };
/** destructor */
	virtual ~RKComponentBase () {};
/** enum of types of properties. Used from RTTI. Don't change the values, as there are some range checks in the code (see isProperty ()) */
	enum RKComponentTypes {
		PropertyBase = 1,
		PropertyBool = 2,
		PropertyInt = 3,
		PropertyDouble = 4,
		PropertyRObjects = 5,
		PropertyCode = 6,
		PropertyLogic = 7,
		PropertyStringList = 8,
		PropertyUser = 1000,		/**< for user expansion */
		PropertyEnd = 1999,
		ComponentBase = 2001,
		Component = 2002,
		ComponentVarSelector = 2003,
		ComponentVarSlot = 2004,
		ComponentFormula = 2005,
		ComponentRadio = 2006,
		ComponentCheckBox = 2007,
		ComponentSpinBox = 2008,
		ComponentInput = 2009,
		ComponentBrowser = 2010,
		ComponentText = 2011,
		ComponentTab = 2012,
		ComponentDropDown = 2013,
		ComponentPreviewBox = 2014,
		ComponentSaveObject = 2015,
		ComponentFrame = 2016,
		ComponentOptionSet = 2017,
		ComponentMatrixInput = 2018,
		ComponentStandard = 2100,
		ComponentContextHandler = 2900,
		ComponentUser = 3000	/**< for user expansion */
	};
	enum UnserializeError {
		NoError,
		BadFormat,
		NotAllSettingsApplied,
		NoSuchComponent
	};
	enum ComponentStatus {
		Dead,		/**< one or more components are dead */
		Processing,		/**< one or more components are still processing. */
		Unsatisfied,		/**< the component is required, but it or one of its (required) children are not valid. */
		Satisfied		/**< the component is not required, or it, and all of its children are satisfied. */
	};
/** for RTTI. see RKComponentBase::RKComponentTypes */
	virtual int type () = 0;
/** tries to locate a component (or property) described by identifier as a child (of any generation) of this RKComponentBase. If found, a pointer to this is returned. Also, the modifier parameter is set to hold any remaining modifier contained in the identifier.
@param identifier The identifier string to look for (including a potential modifier suffix).
@param remainder If a non null pointer to QString is given, this will be set to the value of the remaining modifier
@returns a pointer to the RKComponentBase, if found, or the nearest parent that could be looked up */
	virtual RKComponentBase* lookupComponent (const QString &identifier, QString *remainder);
/** Locate the component.subcomponent.property.value described by identifier and return its value as a string. Especially useful as a callback in code templates! Recursively walks subcomponents/properties until the requested value is found. @See RKComponentBase::lookupComponent */
	QString fetchStringValue (const QString &identifier);
	static QString fetchStringValue (RKComponentBase* prop, const QString &modifier=QString ());
/** returns the "value" of this component or property. Properties generally return their value, components typically return the value of their "most important" property. Default implementation returns QString::null, and writes a debug message */
	virtual QVariant value (const QString &modifier=QString ());
	enum ValueTypeHint {
		TraditionalValue,
		BooleanValue,
		StringValue,
		StringlistValue,
		NumericValue
	};
	QVariant fetchValue (const QString &identifier, const int type_hint);
/** returns true, if this is a property */
	bool isProperty () { return (type () <= PropertyEnd); };
	bool isComponent () { return (type () >= ComponentBase); };
/** shorthand for recursiveStatus () == Satisfied */
	bool isSatisfied () { return (recursiveStatus () == Satisfied); };
/** returns state of the component. @see ComponentStatus */
	virtual ComponentStatus recursiveStatus ();
/** currently valid? default implementation always returns true. @see recursiveStatus()
  * reimplement this in subclasses, if components may become invalid.
  * 
  * @note: A component will be "satisfied" even when invalid, if is is not required. Also, a required component is implictily not satisfied, if any of its children are not statisfied.
  * In general, use isSatisfied() to query the status of components, not isValid(). */
	virtual bool isValid () { return true; };
/** set to required: will only be satisfied if it is valid (and all it's children). Else: always satisfied (but subclasses might override to always be dissatisfied on really bad values. By default RKComponentBase is required at construction */
	void setRequired (bool require) { required = require; };
/** simple convenience function to add a child to the map of children */
	virtual void addChild (const QString &id, RKComponentBase *child);

	typedef QMap<QString, QString> PropertyValueMap;
	static QString valueMapToString (const PropertyValueMap &map);
	static bool stringListToValueMap (const QStringList &strings, PropertyValueMap *map);
/** serialize the state of this component / property and all its children. Note: Only the non-internal property-values are serialzed, not the components / properties themselves. @see fetchPropertyValuesRecursive() */
	void serializeState (PropertyValueMap *map) const { fetchPropertyValuesRecursive (map, true); };
/** set values from a map as created with serializeState(). @see serializeState (), @see setPropertyValues ().
@returns status code */
	void applyState (const PropertyValueMap &state) { setPropertyValues (&state, true); };
	QStringList matchAgainstState (const PropertyValueMap &state);

/** Some properties/components will be marked as internal, such as visibility properties, which are not meant to be set directly by the user. These will be ignored in RKComponent::fetchPropertyValuesRecursive() */
	void setInternal (bool internal) { is_internal = internal; };
	bool isInternal () const { return is_internal; };
protected:
friend class RKOptionSet;
	QHash<QString, RKComponentBase*> child_map;
	bool required;
/** recursively fetch the current values of all properties present as direct or indirect children of this component. Used to transfer values e.g. when switching interfaces (or to store settings per plugin in the future). Values are placed in the dictionary provided (be sure to create one first!). Internal properties are ignored (@see RKComponentPropertyBase::isInternal ());
@param list the list to store the object values in
@param include_top_level include direct properties of the component in the list (or only properties of children)
@param prefix used during recursion to provide full ids for the added objects */
	virtual void fetchPropertyValuesRecursive (PropertyValueMap *list, bool include_top_level=false, const QString &prefix=QString ()) const;
	friend class RKComponentBuilder;
/** counterpart to fetchPropertyValuesRecursive (). Tries to apply all values from the list to properties of the given names. If some keys can not be found, or do not resolve to properties, they are ignored.
@param list a list of id->value such as generated by fetchPropertyValuesRecursive () */
	void setPropertyValues (const PropertyValueMap *list, bool warn = false);
private:
	bool is_internal;
};

#include "rkcomponentproperties.h"

/** abstract base class of all RKComponents, including component widgets */
class RKComponent : public QWidget, public RKComponentBase {
	Q_OBJECT
public:
/** constructor.
@param parent_component The parent RKComponent. If 0, this RKComponent will be a top-level component
@param parent_widget The parent QWidget. This may be the same as the parent_component or a different specific widget.. If 0, this RKComponent will be represented by a top-level widget */
	RKComponent (RKComponent *parent_component, QWidget *parent_widget);
/** destructor */
	virtual ~RKComponent ();
	int type () { return Component; };
/** change notification mechanism. Call this, if something in the component changed that could result in a change in code/values/satisfaction state. Default implementation propagates the change upwards to parent components, if any, but does not do anything further. Reimplement, for instance, to regenerate code */
	virtual void changed ();
/** The component as a wizardish (multi-page) interface. Default implementation returns false */
	virtual bool isWizardish ();
/** If the component isWizardish (), returns true, if it has a next/previous page
@param next if true, returns true, if there is a next page (i.e. not at last page). If false, returns true if there is a previous page (i.e. not at first page). False otherwise. Default implementation returns false at all times. */
	virtual bool havePage (bool next);
/** go to page
@param next if true, go to next (shown) page, if false go to previous (shown) page. Default implementation does nothing */
	virtual void movePage (bool next);
/** returns true, if the current page is satisfied (see isWizardish ()). Default implementation returns isSatisfied () */
	virtual bool currentPageSatisfied () { return (isSatisfied ()); };
/** add a Page to the component. Don't worry, you'll only have to implement this is a meaningful way, if your component isWizardish (). Default implementation simply returns a new RKComponent (and raises an assert). */
	virtual RKComponent *addPage ();
/** For wizardish guis: this gets called to register a component on the current page during construction. The component does not get reparented. It will have to be satisfied in order to move to the next page in the wizard. See isWizardish () see addPage (). Default implementation does nothing. */
	virtual void addComponentToCurrentPage (RKComponent *component);
/** @returns true if the component is inactive, i.e. disabled, or hidden in the GUI */
	bool isInactive ();
public slots:
/** This handles changes in the default properties (enabledness, visibility, requiredness). You will use similar slots in derived classes to handle
specialized properties */
	void propertyValueChanged (RKComponentPropertyBase *property);
/** If you add an outside property to a component, connect it to this slot, so the component will update itself. used in RKComponentBuilder::parseLogic () */
	void outsideValueChanged (RKComponentPropertyBase *) { changed (); }
public:
/** standard property controlling visibility */
	RKComponentPropertyBool *visibilityProperty () { return visibility_property; };
/** standard property controlling enabledness */
	RKComponentPropertyBool *enablednessProperty () { return enabledness_property; };
/** standard property controlling requiredness */
	RKComponentPropertyBool *requirednessProperty ()  { return requiredness_property; };

/** convenience call to set requiredness property (and hence requiredness of this component) */
	void setRequired (bool required);

/** The parent of this component. Should be notified, whenever isSatisfied () or recursiveStatus () changed. */
	RKComponent *parentComponent () const { return _parent; };
/** The standard component containing this component (if any). If @param id_adjust is given, it will be set to a relative path to the standard component. */
	RKStandardComponent *standardComponent (QString *id_adjust=0);
/** Like standardcomponent, but will return the topmost component in case of embedding. */
	RKStandardComponent *topmostStandardComponent ();

/** Find the id of this component. NOTE: this is slow. Better to store the id in the first place, if needed */
	QString getIdInParent () const;
protected slots:
/** if a child component self-destructs, it should remove itself from its parent *before* destructing. Don't use in a regular destructor. Call only if the child dies unexpectedly */
	void removeFromParent ();
signals:
/** emitted from changed() */
	void componentChanged (RKComponent* component);
protected:
	RKComponentPropertyBool *visibility_property;
	RKComponentPropertyBool *enabledness_property;
	RKComponentPropertyBool *requiredness_property;
	RKComponent *_parent;
/** usually happens during construction, so you don't need to call this - unless you're RKStandardComponent, and discard the children at some point of time */
	void createDefaultProperties ();
/** This function is needed internally, to set the Qt enabledness of this widget, and all child component widgets. Note that the enabledness as stored in the enabledness_property is not necessarily the same as the enabledness in the GUI (and is not affected by this call). In general, a component is enabled in the GUI, if and only if a) it's enabledness_property is set to true, b) its parent widget is enabled in Qt, and c) it's parent component is also enabled. */
	void updateEnablednessRecursive (bool parent_component_enabled);
};

#endif

