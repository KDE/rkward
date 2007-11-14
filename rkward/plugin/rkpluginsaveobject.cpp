/***************************************************************************
                          rkpluginsaveobject  -  description
                             -------------------
    begin                : Tue Jan 30 2007
    copyright            : (C) 2007 by Thomas Friedrichsmeier
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

#include <klocale.h>

#include "../misc/xmlhelper.h"
#include "../misc/rksaveobjectchooser.h"
#include "../rkglobals.h"
#include "../debug.h"

RKPluginSaveObject::RKPluginSaveObject (const QDomElement &element, RKComponent *parent_component, QWidget *parent_widget) : RKComponent (parent_component, parent_widget) {
	RK_TRACE (PLUGIN);

	// get xml-helper
	XMLHelper *xml = XMLHelper::getStaticHelper ();

	// create and add property
	addChild ("selection", selection = new RKComponentPropertyBase (this, xml->getBoolAttribute (element, "required", true, DL_INFO)));
	connect (selection, SIGNAL (valueChanged (RKComponentPropertyBase *)), this, SLOT (selectionChanged (RKComponentPropertyBase *)));

	QVBoxLayout *vbox = new QVBoxLayout (this);
	vbox->setContentsMargins (0, 0, 0, 0);

	selector = new RKSaveObjectChooser (this, xml->getStringAttribute (element, "initial", i18n ("my.data"), DL_INFO), xml->getStringAttribute (element, "label", i18n ("Save to:"), DL_INFO));
	connect (selector, SIGNAL (changed ()), SLOT (selectionChanged ()));
	connect (selector, SIGNAL (okStatusChanged (bool)), SLOT (selectionChanged (bool)));

	vbox->addWidget (selector);

	// initialize
	updating = false;
	selectionChanged ();
}

RKPluginSaveObject::~RKPluginSaveObject () {
	RK_TRACE (PLUGIN);
}

void RKPluginSaveObject::selectionChanged (RKComponentPropertyBase *) {
	RK_TRACE (PLUGIN);

	if (updating) return;
	updating = true;

	selector->setObjectName (selection->value ());

	updating = false;
	if (isSatisfied ()) selector->setBackgroundColor (QColor (255, 255, 255));
	else selector->setBackgroundColor (QColor (255, 0, 0));
	changed ();
}

void RKPluginSaveObject::selectionChanged () {
	RK_TRACE (PLUGIN);

	selection->setValue (selector->validizedSelectedObjectName ());
}

void RKPluginSaveObject::selectionChanged (bool) {
	RK_TRACE (PLUGIN);

	selectionChanged ();
}

bool RKPluginSaveObject::isValid () {
	RK_TRACE (PLUGIN);

	return (RKComponent::isValid () && selector->isOk ());
}

#include "rkpluginsaveobject.moc"
