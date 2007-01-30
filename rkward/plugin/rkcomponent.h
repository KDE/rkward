/***************************************************************************
                          rkcomponent  -  description
                             -------------------
    begin                : Tue Dec 13 2005
    copyright            : (C) 2005, 2006, 2007 by Thomas Friedrichsmeier
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

#include <qdict.h>
#include <qmap.h>
#include <qwidget.h>

class RKComponentPropertyBase;

/** a very low level base for RKComponent and RKComponentProperty. */
class RKComponentBase {
public:
/** constructor */
	RKComponentBase () { required=true; };
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
		ComponentStandard = 2100,
		ComponentUser = 3000	/**< for user expansion */
	};
/** for RTTI. see RKComponentBase::RKComponentTypes */
	virtual int type () { return ComponentBase; };
/** tries to locate a component (or property) described by identifier as a child (of any generation) of this RKComponentBase. If found, a pointer to this is returned. Also, the modifier parameter is set to hold any remaining modifier contained in the identifier.
@param identifier The identifier string to look for (including a potential modifier suffix).
@param remainder If a non null pointer to QString is given, this will be set to the value of the remaining modifier
@returns a pointer to the RKComponentBase, if found, or the nearest parent that could be looked up */
	virtual RKComponentBase* lookupComponent (const QString &identifier, QString *remainder);
/** Locate the component.subcomponent.property.value described by identifier and return its value as a string. Especially useful as a callback in code templates! Recursively walks subcomponents/properties until the requested value is found. @See RKComponentBase::lookupComponent */
	QString fetchStringValue (const QString &identifier);
/** returns the "value" of this component or property as a string. Properties generally return their value, components typically return the value of their "most important" property. Default implementation returns QString::null, and writes a debug message */
	virtual QString value (const QString &modifier=QString::null);
/** returns true, if this is a property */
	bool isProperty () { return (type () <= PropertyEnd); };
/** returns satisfaction state. see setRequired () */
	virtual bool isSatisfied ();
/** currently valid (i.e. satisfied, even if required)? default implementation always returns true */
	virtual bool isValid () { return true; };
/** set to required: will only be satisfied if it is valid. Else: always satisfied (but subclasses might override to always be dissatisfied on really bad values. By default RKComponentBase is required at construction */
	void setRequired (bool require) { required = require; };
/** simple convenience function to add a child to the map of children */
	void addChild (const QString &id, RKComponentBase *child);
protected:
	QDict<RKComponentBase> child_map;
	bool required;
/** recursively fetch the current values of all properties present as direct or indirect children of this component. Used to transfer values e.g. when switching interfaces (or to store settings per plugin in the future). Values are placed in the dictionary provided (be sure to create one first!).
@param list the list to store the object values in
@param include_top_level include direct properties of the component in the list (or only properties of children)
@param prefix used during recursion to provide full ids for the added objects */
	void fetchPropertyValuesRecursive (QMap<QString, QString> *list, bool include_top_level=false, const QString &prefix=QString::null);
/** counterpart to fetchPropertyValuesRecursive (). Tries to apply all values from the list to properties of the given names. If some keys can not be found, or do not resolve to properties, the are ignored.
@param list a list of id->value such as generated by fetchPropertyValuesRecursive () */
	void setPropertyValues (QMap<QString, QString> *list);
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
/** reimplemented to only return true, if all children are satisfied */
	bool isValid ();
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

/** convenience call to set visibilty property (and hence visibility of this component). Can't inline due to inclusion problems. */
	void setVisible (bool visible);
/** convenience call to set visibilty property (and hence visibility of this component) */
	void setEnabledness (bool enabled);
/** convenience call to set visibilty property (and hence visibility of this component) */
	void setRequired (bool required);

/** The parent of this component. Should be notified, whenever isSatisfied () or isReady ()-state changed. */
	RKComponent *parentComponent () { return _parent; };

/** Is the component "ready"? I.e. it is up to date according to current settings. Does not imply it is also satisfied. Default implementation always returns true. */
	virtual bool isReady () { return true; };
protected slots:
/** if a child component self-destructs, it should remove itself from its parent *before* destructing. Don't use in a regular destructor. Call only if the child dies unexpectedly */
	void removeFromParent ();
protected:
	RKComponentPropertyBool *visibility_property;
	RKComponentPropertyBool *enabledness_property;
	RKComponentPropertyBool *requiredness_property;
	RKComponent *_parent;
/** usually happens during construction, so you don't need to call this - unless you're RKStandardComponent, and discard the children at some point of time */
	void createDefaultProperties ();
};

#endif
