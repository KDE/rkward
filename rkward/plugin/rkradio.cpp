/*
rkradio.cpp - This file is part of RKWard (https://rkward.kde.org). Created: Thu Nov 7 2002
SPDX-FileCopyrightText: 2002-2014 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "rkradio.h"

#include <qdom.h>
#include <qlabel.h>
#include <QButtonGroup>
#include <QGroupBox>
#include <qradiobutton.h>
#include <QVBoxLayout>

#include <KLocalizedString>

#include "../misc/xmlhelper.h"
#include "../debug.h"

RKRadio::RKRadio (const QDomElement &element, RKComponent *parent_component, QWidget *parent_widget) : RKAbstractOptionSelector (parent_component, parent_widget) {
	RK_TRACE (PLUGIN);

	// get xml-helper
	XMLHelper *xml = parent_component->xmlHelper ();

	// create layout
	QVBoxLayout *vbox = new QVBoxLayout (this);
	vbox->setContentsMargins (0, 0, 0, 0);

	// create ButtonGroup
	group = new QButtonGroup (this);
	group_box = new QGroupBox (xml->i18nStringAttribute (element, "label", i18n ("Select one:"), DL_INFO), this);
	new QVBoxLayout (group_box);

	addOptionsAndInit (element);

	vbox->addWidget (group_box);
	connect (group, &QButtonGroup::idClicked, this, &RKRadio::itemSelected);
}

RKRadio::~RKRadio(){
	RK_TRACE (PLUGIN);
}

void RKRadio::setItemInGUI (int id) {
	RK_TRACE (PLUGIN);

	QAbstractButton *button = group->button (id);
	if (button) button->setChecked (true);
}

void RKRadio::addOptionToGUI (const QString &label, int id) {
	RK_TRACE (PLUGIN);

	QRadioButton *button = new QRadioButton (label, group_box);
	group->addButton (button, id);
	group_box->layout ()->addWidget (button);
}

void RKRadio::setItemEnabledInGUI (int id, bool enabled) {
	RK_TRACE (PLUGIN);

	QAbstractButton *button = group->button (id);
	RK_ASSERT (button);
	button->setEnabled (enabled);
}

QStringList RKRadio::getUiLabelPair () const {
	RK_TRACE (PLUGIN);

	QStringList ret (stripAccelerators (group_box->title ()));
	ret.append (stripAccelerators (group->checkedButton ()->text ()));
	return ret;
}

