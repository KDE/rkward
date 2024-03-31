/*
rkcheckbox - This file is part of RKWard (https://rkward.kde.org). Created: Fri Jul 30 2004
SPDX-FileCopyrightText: 2004-2014 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "rkcheckbox.h"

#include <qcheckbox.h>
#include <QVBoxLayout>
#include <KLocalizedString>

#include "../misc/xmlhelper.h"
#include "../debug.h"

RKCheckBox::RKCheckBox (const QDomElement &element, RKComponent *parent_component, QWidget *parent_widget) : RKComponent (parent_component, parent_widget) {
	RK_TRACE (PLUGIN);

	// get xml-helper
	XMLHelper *xml = parent_component->xmlHelper ();

	// create and add property
	addChild ("state", state = new RKComponentPropertyBool (this, true, xml->getBoolAttribute (element, "checked", false, DL_INFO), xml->getStringAttribute (element, "value", "1", DL_INFO), xml->getStringAttribute (element, "value_unchecked", QString (), DL_INFO)));
	connect (state, &RKComponentPropertyBase::valueChanged, this, &RKCheckBox::changedState);

	// create checkbox
	QVBoxLayout *vbox = new QVBoxLayout (this);
	vbox->setContentsMargins (0, 0, 0, 0);
	checkbox = new QCheckBox (xml->i18nStringAttribute (element, "label", QString (), DL_WARNING), this);
	vbox->addWidget (checkbox);
	checkbox->setChecked (xml->getBoolAttribute (element, "checked", false, DL_INFO));
	connect (checkbox, &QCheckBox::stateChanged, this, &RKCheckBox::changedStateFromUi);

	// initialize
	updating = false;
	changedState(nullptr);
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

void RKCheckBox::changedStateFromUi (int) {
	RK_TRACE (PLUGIN);

	state->setBoolValue (checkbox->isChecked ());
}

QStringList RKCheckBox::getUiLabelPair () const {
	RK_TRACE (PLUGIN);

	QStringList ret (stripAccelerators (checkbox->text ()));
	ret.append (checkbox->isChecked () ? i18n ("yes") : i18n ("no"));
	return ret;
}

