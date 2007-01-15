/***************************************************************************
                          rkradio.cpp  -  description
                             -------------------
    begin                : Thu Nov 7 2002
    copyright            : (C) 2002, 2006 by Thomas Friedrichsmeier
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

#include "rkradio.h"

#include <qdom.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>

#include <klocale.h>

#include "../rkglobals.h"
#include "../misc/xmlhelper.h"
#include "../debug.h"

RKRadio::RKRadio (const QDomElement &element, RKComponent *parent_component, QWidget *parent_widget) : RKComponent (parent_component, parent_widget) {
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

	// create ButtonGroup
	group = new QButtonGroup (xml->getStringAttribute (element, "label", i18n ("Select one:"), DL_INFO), this);

	// create internal layout for the buttons in the ButtonGroup
	group->setColumnLayout (0, Qt::Vertical);
	group->layout()->setSpacing (RKGlobals::spacingHint ());
	group->layout()->setMargin (RKGlobals::marginHint ());
	QVBoxLayout *group_layout = new QVBoxLayout (group->layout(), RKGlobals::spacingHint ());

	// create all the options
	XMLChildList option_elements = xml->getChildElements (element, "option", DL_ERROR);	
	int checked = 0;
	int i = 0;
	for (XMLChildList::const_iterator it = option_elements.begin (); it != option_elements.end (); ++it) {
		QRadioButton *button = new QRadioButton (xml->getStringAttribute (*it, "label", QString::null, DL_ERROR), group);
		options.insert (i, xml->getStringAttribute (*it, "value", QString::null, DL_WARNING));
		group_layout->addWidget (button);

		if (xml->getBoolAttribute (*it, "checked", false, DL_INFO)) {
			button->setChecked (true);
			checked = i;
		}

		++i;
	}
	updating = false;
	number->setIntValue (checked);			// will also take care of checking the correct button
	number->setMin (0);
	number->setMax (i-1);

	vbox->addWidget (group);
	connect (group, SIGNAL (clicked (int)), this, SLOT (buttonClicked (int)));

	// initialize
	buttonClicked (group->selectedId ());
}

RKRadio::~RKRadio(){
	RK_TRACE (PLUGIN);
}

void RKRadio::propertyChanged (RKComponentPropertyBase *property) {
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
	group->setButton (new_id);
	buttonClicked (new_id);		// unfortunately, this slot is not called when the option is changed programatically!
	updating = false;

	changed ();
}

void RKRadio::buttonClicked (int id) {
	RK_TRACE (PLUGIN);

	string->setValue (options[id]);
	number->setIntValue (id);
}

int RKRadio::findOption (const QString &option_string) {
	RK_TRACE (PLUGIN);

	for (OptionsMap::const_iterator it = options.begin(); it != options.end(); ++it) {
		if (it.data () == option_string) return (it.key ());
	}
	return -1;
}  

#include "rkradio.moc"
