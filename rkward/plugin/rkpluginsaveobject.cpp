/***************************************************************************
                          rkpluginsaveobject  -  description
                             -------------------
    begin                : Tue Jan 30 2007
    copyright            : (C) 2007, 2010, 2012 by Thomas Friedrichsmeier
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

#include "rkpluginsaveobject.h"

#include <QVBoxLayout>
#include <QGroupBox>

#include <klocale.h>

#include "../misc/xmlhelper.h"
#include "../misc/rksaveobjectchooser.h"
#include "../rkglobals.h"
#include "../debug.h"

RKPluginSaveObject::RKPluginSaveObject (const QDomElement &element, RKComponent *parent_component, QWidget *parent_widget) : RKComponent (parent_component, parent_widget) {
	RK_TRACE (PLUGIN);

	// read settings
	XMLHelper *xml = XMLHelper::getStaticHelper ();

	bool checkable = xml->getBoolAttribute (element, "checkable", false, DL_INFO);
	bool checked = xml->getBoolAttribute (element, "checked", false, DL_INFO);
	bool required = xml->getBoolAttribute (element, "required", true, DL_INFO);
	QString label = xml->getStringAttribute (element, "label", i18n ("Save to:"), DL_INFO);
	QString initial = xml->getStringAttribute (element, "initial", i18n ("my.data"), DL_INFO);

	// create and add properties
	addChild ("selection", selection = new RKComponentPropertyBase (this, required));
	connect (selection, SIGNAL (valueChanged (RKComponentPropertyBase *)), this, SLOT (externalChange ()));
	selection->setInternal (true);	// the two separate properties "parent" and "objectname" are used for (re-)storing.
	addChild ("parent", parent = new RKComponentPropertyRObjects (this, false));
	connect (parent, SIGNAL (valueChanged (RKComponentPropertyBase *)), this, SLOT (externalChange ()));
	addChild ("objectname", objectname = new RKComponentPropertyBase (this, false));
	connect (objectname, SIGNAL (valueChanged (RKComponentPropertyBase *)), this, SLOT (externalChange ()));
	addChild ("active", active = new RKComponentPropertyBool (this, false, false, "1", "0"));
	connect (active, SIGNAL (valueChanged (RKComponentPropertyBase *)), this, SLOT (externalChange ()));
	if (!checkable) active->setInternal (true);

	// create GUI
	groupbox = new QGroupBox (label, this);
	groupbox->setCheckable (checkable);
	if (checkable) groupbox->setChecked (checked);
	connect (groupbox, SIGNAL (toggled(bool)), this, SLOT (internalChange ()));

	selector = new RKSaveObjectChooser (groupbox, initial);
	connect (selector, SIGNAL (changed (bool)), SLOT (internalChange ()));

	QVBoxLayout *vbox = new QVBoxLayout (this);
	vbox->setContentsMargins (0, 0, 0, 0);

	QVBoxLayout *vbox_b = new QVBoxLayout (groupbox);
	vbox_b->setContentsMargins (0, 0, 0, 0);
	vbox_b->addWidget (selector);

	vbox->addWidget (groupbox);

	// initialize
	setRequired (required);
	updating = false;
	internalChange ();
}

RKPluginSaveObject::~RKPluginSaveObject () {
	RK_TRACE (PLUGIN);
}

void RKPluginSaveObject::update () {
	RK_TRACE (PLUGIN);

	if (isSatisfied ()) selector->setBackgroundColor (QColor (255, 255, 255));
	else selector->setBackgroundColor (QColor (255, 0, 0));
	changed ();
}

void RKPluginSaveObject::externalChange () {
	RK_TRACE (PLUGIN);

	if (updating) return;

	// NOTE: the selection-property is read-only!
	selector->setBaseName (fetchStringValue (objectname));
	selector->setRootObject (parent->objectValue ());
	if (groupbox->isCheckable ()) {
		groupbox->setChecked (active->boolValue ());
	}

	// call internalChange, now, in case one or more setings could not be applied
	internalChange ();
}

void RKPluginSaveObject::internalChange () {
	RK_TRACE (PLUGIN);

	if (updating) return;
	updating = true;

	selection->setValue (selector->currentFullName ());
	objectname->setValue (selector->currentBaseName ());
	parent->setObjectValue (selector->rootObject ());
	active->setBoolValue ((!groupbox->isCheckable()) || groupbox->isChecked());

	updating = false;
	update ();
}

bool RKPluginSaveObject::isValid () {
	RK_TRACE (PLUGIN);

	if (groupbox->isCheckable () && (!groupbox->isChecked ())) return true;
	return (RKComponent::isValid () && selector->isOk ());
}

QVariant RKPluginSaveObject::value (const QString& modifier) {
//	RK_TRACE (PLUGIN);

	return (selection->value (modifier));
}

#include "rkpluginsaveobject.moc"
