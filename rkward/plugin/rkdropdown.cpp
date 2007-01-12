/***************************************************************************
                          rkdropdown.h  -  description
                             -------------------
    begin                : Fri Jan 12 2007
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

#include "rkdropdown.h"

#include <qdom.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qcombobox.h>

#include <klocale.h>

#include "../rkglobals.h"
#include "../misc/xmlhelper.h"
#include "../debug.h"

RKDropDown::RKDropDown (const QDomElement &element, RKComponent *parent_component, QWidget *parent_widget) : RKComponent (parent_component, parent_widget) {
	RK_TRACE (PLUGIN);

	// create and register properties
	addChild ("string", string = new RKComponentPropertyBase (this, false));
	connect (string, SIGNAL (valueChanged (RKComponentPropertyBase *)), this, SLOT (propertyChanged (RKComponentPropertyBase *)));
	addChild ("number", number = new RKComponentPropertyInt (this, true, -1));
	connect (number, SIGNAL (valueChanged (RKComponentPropertyBase *)), this, SLOT (propertyChanged (RKComponentPropertyBase *)));

	// get xml-helper
	XMLHelper *xml = XMLHelper::getStaticHelper ();

	// create layout
	QVBoxLayout *vbox = new QVBoxLayout (this, RKGlobals::spacingHint ());

	QLabel *label = new QLabel (xml->getStringAttribute (element, "label", i18n ("Select one:"), DL_INFO), this);
	vbox->addWidget (label);

	// create ComboBox
	box = new QComboBox (false, this);

	// create all the options
	XMLChildList option_elements = xml->getChildElements (element, "option", DL_ERROR);	
	int selected = 0;
	int i = 0;
	for (XMLChildList::const_iterator it = option_elements.begin (); it != option_elements.end (); ++it) {
		box->insertItem (xml->getStringAttribute (*it, "label", QString::null, DL_ERROR));
		options.insert (i, xml->getStringAttribute (*it, "value", QString::null, DL_WARNING));

		if (xml->getBoolAttribute (*it, "checked", false, DL_INFO)) {
			selected = i;
		}

		++i;
	}

	updating = false;
	number->setIntValue (selected);			// will also take care of activating the corret item
	number->setMin (0);
	number->setMax (i-1);

	vbox->addWidget (box);
	connect (box, SIGNAL (activated (int)), this, SLOT (itemSelected (int)));
}

RKDropDown::~RKDropDown(){
	RK_TRACE (PLUGIN);
}

void RKDropDown::propertyChanged (RKComponentPropertyBase *property) {
	RK_TRACE (PLUGIN);

	if (updating) return;

	int new_id = -1;
	if (property == string) {
		new_id = findOption (string->value ());
	} else if (property == number) {
		new_id = number->intValue ();
	} else {
		RK_ASSERT (false);
	}

	updating = true;
	box->setCurrentItem (new_id);
	updating = false;

	changed ();
}

void RKDropDown::itemSelected (int id) {
	RK_TRACE (PLUGIN);

	string->setValue (options[id]);
	number->setIntValue (id);
}

int RKDropDown::findOption (const QString &option_string) {
	RK_TRACE (PLUGIN);

	for (OptionsMap::const_iterator it = options.begin(); it != options.end(); ++it) {
		if (it.data () == option_string) return (it.key ());
	}
	return -1;
}  

#include "rkdropdown.moc"
