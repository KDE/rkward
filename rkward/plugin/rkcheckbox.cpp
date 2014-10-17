/***************************************************************************
                          rkcheckbox  -  description
                             -------------------
    begin                : Fri Jul 30 2004
    copyright            : (C) 2004, 2006 by Thomas Friedrichsmeier
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
#include "rkcheckbox.h"

#include <qcheckbox.h>
#include <QVBoxLayout>

#include "../rkglobals.h"
#include "../misc/xmlhelper.h"
#include "../debug.h"

RKCheckBox::RKCheckBox (const QDomElement &element, RKComponent *parent_component, QWidget *parent_widget) : RKComponent (parent_component, parent_widget) {
	RK_TRACE (PLUGIN);

	// get xml-helper
	XMLHelper *xml = parent_component->xmlHelper ();

	// create and add property
	addChild ("state", state = new RKComponentPropertyBool (this, true, xml->getBoolAttribute (element, "checked", false, DL_INFO), xml->getStringAttribute (element, "value", "1", DL_INFO), xml->getStringAttribute (element, "value_unchecked", QString::null, DL_INFO)));
	connect (state, SIGNAL (valueChanged (RKComponentPropertyBase *)), this, SLOT (changedState (RKComponentPropertyBase *)));

	// create checkbox
	QVBoxLayout *vbox = new QVBoxLayout (this);
	vbox->setContentsMargins (0, 0, 0, 0);
	checkbox = new QCheckBox (xml->getStringAttribute (element, "label", QString::null, DL_WARNING), this);
	vbox->addWidget (checkbox);
	checkbox->setChecked (xml->getBoolAttribute (element, "checked", false, DL_INFO));
	connect (checkbox, SIGNAL (stateChanged (int)), this, SLOT (changedState (int)));

	// initialize
	updating = false;
	changedState (0);
}

RKCheckBox::~RKCheckBox () {
	RK_TRACE (PLUGIN);
}

void RKCheckBox::changedState (RKComponentPropertyBase *) {
	RK_TRACE (PLUGIN);

	if (updating) return;
	updating = true;
	checkbox->setChecked (state->boolValue ());
	updating = false;

	changed ();
}

void RKCheckBox::changedState (int) {
	RK_TRACE (PLUGIN);

	state->setBoolValue (checkbox->isChecked ());
}

#include "rkcheckbox.moc"
