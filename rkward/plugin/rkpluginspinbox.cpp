/***************************************************************************
                          rkpluginspinbox  -  description
                             -------------------
    begin                : Wed Aug 11 2004
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
#include "rkpluginspinbox.h"

#include <qlayout.h>
#include <qlabel.h>

#include <klocale.h>

#include "../misc/rkspinbox.h"
#include "../misc/xmlhelper.h"
#include "../rkglobals.h"
#include "../debug.h"

RKPluginSpinBox::RKPluginSpinBox (const QDomElement &element, RKComponent *parent_component, QWidget *parent_widget) : RKComponent (parent_component, parent_widget) {
	RK_TRACE (PLUGIN);
	// get xml-helper
	XMLHelper *xml = XMLHelper::getStaticHelper ();

	// first question: int or real
	intmode = (xml->getMultiChoiceAttribute (element, "type", "integer;real", 0, DL_INFO) == 0);

	// create and add properties
	addChild ("int", intvalue = new RKComponentPropertyInt (this, intmode, 0));
	addChild ("real", realvalue = new RKComponentPropertyDouble (this, !intmode, 0));

	// layout and label
	QVBoxLayout *vbox = new QVBoxLayout (this, RKGlobals::spacingHint ());
	QLabel *label = new QLabel (xml->getStringAttribute (element, "label", i18n ("Enter value:"), DL_WARNING), this);
	vbox->addWidget (label);

	// create spinbox and read settings
	spinbox = new RKSpinBox (this);
	if (!intmode) {
		double min = xml->getDoubleAttribute (element, "min", FLT_MIN, DL_INFO);
		double max = xml->getDoubleAttribute (element, "max", FLT_MAX, DL_INFO);
		double initial = xml->getDoubleAttribute (element, "initial", min, DL_INFO);
		int default_precision = xml->getIntAttribute (element, "default_precision", 2, DL_INFO);
		int max_precision = xml->getIntAttribute (element, "max_precision", 4, DL_INFO);

		spinbox->setRealMode (min, max, initial, default_precision, max_precision);

		realvalue->setMin (min);
		realvalue->setMax (max);
		realvalue->setPrecision (max_precision);
		intmode = false;
	} else {
		int min = xml->getIntAttribute (element, "min", INT_MIN, DL_INFO);
		int max = xml->getIntAttribute (element, "max", INT_MAX, DL_INFO);
		int initial = xml->getIntAttribute (element, "initial", min, DL_INFO);

		spinbox->setIntMode (min, max, initial);

		intvalue->setMin (min);
		intvalue->setMax (max);
		intmode = true;
	}

	// connect
	connect (spinbox, SIGNAL (valueChanged (int)), this, SLOT (valueChanged (int)));
	connect (intvalue, SIGNAL (valueChanged (RKComponentPropertyBase *)), this, SLOT (valueChanged (RKComponentPropertyBase *)));
	connect (realvalue, SIGNAL (valueChanged (RKComponentPropertyBase *)), this, SLOT (valueChanged (RKComponentPropertyBase *)));
	updating = false;

	// finish layout
	vbox->addWidget (spinbox);
	if (xml->getStringAttribute (element, "size", "normal", DL_INFO) == "small") {
		spinbox->setFixedWidth (100);
	}

	// initialize
	valueChanged (1);
}

RKPluginSpinBox::~RKPluginSpinBox () {
	RK_TRACE (PLUGIN);
}

void RKPluginSpinBox::valueChanged (RKComponentPropertyBase *property) {
	RK_TRACE (PLUGIN);

	if (updating) return;
	updating = true;

	// sync the two properties
	if (property == intvalue) {
		realvalue->setDoubleValue ((double) intvalue->intValue ());
	} else {
		intvalue->setIntValue ((int) realvalue->doubleValue ());
	}

	// update GUI
	if (intmode) {
		spinbox->setValue (intvalue->intValue ());
	} else {
		spinbox->setRealValue (realvalue->doubleValue ());
	}

	updating = false;
}

void RKPluginSpinBox::valueChanged (int) {
	RK_TRACE (PLUGIN);

	if (intmode) {
		intvalue->setIntValue (spinbox->value ());
	} else {
		// this may be ugly, but we have to set via text to make sure we get the exact same display
		realvalue->setValue (spinbox->text ());
	}

	changed ();
}

#include "rkpluginspinbox.moc"
