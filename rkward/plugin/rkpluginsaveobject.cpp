/*
rkpluginsaveobject - This file is part of RKWard (https://rkward.kde.org). Created: Tue Jan 30 2007
SPDX-FileCopyrightText: 2007-2014 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "rkpluginsaveobject.h"

#include <QVBoxLayout>
#include <QGroupBox>

#include <KLocalizedString>

#include "../misc/xmlhelper.h"
#include "../misc/rksaveobjectchooser.h"

#include "../debug.h"

RKPluginSaveObject::RKPluginSaveObject (const QDomElement &element, RKComponent *parent_component, QWidget *parent_widget) : RKComponent (parent_component, parent_widget) {
	RK_TRACE (PLUGIN);

	// read settings
	XMLHelper *xml = parent_component->xmlHelper ();

	bool checkable = xml->getBoolAttribute (element, "checkable", false, DL_INFO);
	bool checked = xml->getBoolAttribute (element, "checked", false, DL_INFO);
	bool required = xml->getBoolAttribute (element, "required", true, DL_INFO);
	QString label = xml->i18nStringAttribute (element, "label", i18n ("Save to:"), DL_INFO);
	QString initial = xml->getStringAttribute (element, "initial", i18n ("my.data"), DL_INFO);

	// create and add properties
	addChild ("selection", selection = new RKComponentPropertyBase (this, required));
	connect (selection, &RKComponentPropertyBase::valueChanged, this, &RKPluginSaveObject::externalChange);
	selection->setInternal (true);	// the two separate properties "parent" and "objectname" are used for (re-)storing.
	addChild ("parent", parent = new RKComponentPropertyRObjects (this, false));
	connect (parent, &RKComponentPropertyBase::valueChanged, this, &RKPluginSaveObject::externalChange);
	addChild ("objectname", objectname = new RKComponentPropertyBase (this, false));
	connect (objectname, &RKComponentPropertyBase::valueChanged, this, &RKPluginSaveObject::externalChange);
	addChild ("active", active = new RKComponentPropertyBool (this, false, false, "1", "0"));
	connect (active, &RKComponentPropertyBase::valueChanged, this, &RKPluginSaveObject::externalChange);
	if (!checkable) active->setInternal (true);

	// create GUI
	groupbox = new QGroupBox (label, this);
	groupbox->setCheckable (checkable);
	if (checkable) groupbox->setChecked (checked);
	connect (groupbox, &QGroupBox::toggled, this, &RKPluginSaveObject::internalChange);

	selector = new RKSaveObjectChooser (groupbox, initial);
	connect (selector, &RKSaveObjectChooser::changed, this, &RKPluginSaveObject::internalChange);

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

	if (isSatisfied ()) selector->setStyleSheet(QString(""));
	else selector->setStyleSheet(QString("background: red; color: black"));
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

	// call internalChange, now, in case one or more settings could not be applied
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

QStringList RKPluginSaveObject::getUiLabelPair () const {
	RK_TRACE (PLUGIN);

	QStringList ret (stripAccelerators (groupbox->title ()));
	ret.append (selection->value ().toString ());
	return ret;
}

