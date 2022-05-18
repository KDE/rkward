/*
rkpluginspinbox - This file is part of RKWard (https://rkward.kde.org). Created: Wed Aug 11 2004
SPDX-FileCopyrightText: 2004-2014 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "rkpluginspinbox.h"

#include <qlayout.h>
#include <qlabel.h>

#include <KLocalizedString>

#include "../misc/rkspinbox.h"
#include "../misc/xmlhelper.h"

#include "../debug.h"

RKPluginSpinBox::RKPluginSpinBox (const QDomElement &element, RKComponent *parent_component, QWidget *parent_widget) : RKComponent (parent_component, parent_widget) {
	RK_TRACE (PLUGIN);
	// get xml-helper
	XMLHelper *xml = parent_component->xmlHelper ();

	// first question: int or real
	intmode = (xml->getMultiChoiceAttribute (element, "type", "integer;real", 1, DL_INFO) == 0);

	// create and add properties
	addChild ("int", intvalue = new RKComponentPropertyInt (this, intmode, 0));
	intvalue->setInternal (true);
	addChild ("real", realvalue = new RKComponentPropertyDouble (this, !intmode, 0));

	// layout and label
	QVBoxLayout *vbox = new QVBoxLayout (this);
	vbox->setContentsMargins (0, 0, 0, 0);
	label = new QLabel (xml->i18nStringAttribute (element, "label", i18n ("Enter value:"), DL_WARNING), this);
	vbox->addWidget (label);

	// create spinbox and read settings
	spinbox = new RKSpinBox (this);
	if (!intmode) {
		double min = xml->getDoubleAttribute (element, "min", -FLT_MAX, DL_INFO);
		double max = xml->getDoubleAttribute (element, "max", FLT_MAX, DL_INFO);
		double initial = xml->getDoubleAttribute (element, "initial", qMin (max, qMax (min, double (qreal(0.0)))), DL_INFO);
		int default_precision = xml->getIntAttribute (element, "default_precision", 2, DL_INFO);
		int max_precision = xml->getIntAttribute (element, "max_precision", 8, DL_INFO);

		spinbox->setRealMode (min, max, initial, default_precision, max_precision);

		realvalue->setMin (min);
		realvalue->setMax (max);
		realvalue->setPrecision (default_precision);
	} else {
		int min = xml->getIntAttribute (element, "min", INT_MIN, DL_INFO);
		int max = xml->getIntAttribute (element, "max", INT_MAX, DL_INFO);
		int initial = xml->getIntAttribute (element, "initial", qMin (max, qMax (min, 0)), DL_INFO);

		spinbox->setIntMode (min, max, initial);

		intvalue->setMin (min);
		intvalue->setMax (max);
	}

	// connect
	connect (spinbox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &RKPluginSpinBox::valueChangedFromUi);
	connect (intvalue, &RKComponentPropertyBase::valueChanged, this, &RKPluginSpinBox::valueChanged);
	connect (realvalue, &RKComponentPropertyBase::valueChanged, this, &RKPluginSpinBox::valueChanged);
	updating = false;

	// finish layout
	vbox->addWidget (spinbox);
	vbox->addStretch (1);		// make sure label remains attached to spinbox
	if (xml->getStringAttribute (element, "size", "normal", DL_INFO) == "small") {
		spinbox->setFixedWidth (100);
	}

	// initialize
	valueChangedFromUi ();
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
		spinbox->setIntValue (intvalue->intValue ());
	} else {
		spinbox->setRealValue (realvalue->doubleValue ());
	}

	updating = false;
}

void RKPluginSpinBox::valueChangedFromUi () {
	RK_TRACE (PLUGIN);

	if (intmode) {
		intvalue->setIntValue (spinbox->intValue ());
	} else {
		// this may be ugly, but we have to set via text to make sure we get the exact same display
		realvalue->setValue (spinbox->text ());
	}

	changed ();
}

QVariant RKPluginSpinBox::value (const QString &modifier) {
	RK_TRACE (PLUGIN);

	if (intmode) {
		return intvalue->value (modifier);
	} else {
		if (modifier.isEmpty ()) return realvalue->value ("formatted");
		return realvalue->value (modifier);
	}
}

QStringList RKPluginSpinBox::getUiLabelPair () const {
	RK_TRACE (PLUGIN);

	QStringList ret (stripAccelerators (label->text ()));
	ret.append (const_cast<RKPluginSpinBox *> (this)->value ().toString ());
	return ret;
}

