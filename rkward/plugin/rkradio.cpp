/*
rkradio.cpp - This file is part of RKWard (https://rkward.kde.org). Created: Thu Nov 7 2002
SPDX-FileCopyrightText: 2002-2024 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "rkradio.h"

#include <QVBoxLayout>
#include <qdom.h>
#include <qlabel.h>

#include <KLocalizedString>

#include "../misc/rkradiogroup.h"
#include "../misc/xmlhelper.h"

#include "../debug.h"

RKRadio::RKRadio(const QDomElement &element, RKComponent *parent_component, QWidget *parent_widget) : RKAbstractOptionSelector(parent_component, parent_widget) {
	RK_TRACE(PLUGIN);

	// get xml-helper
	XMLHelper *xml = parent_component->xmlHelper();

	// create layout
	QVBoxLayout *vbox = new QVBoxLayout(this);
	vbox->setContentsMargins(0, 0, 0, 0);

	// create ButtonGroup
	group_box = new RKRadioGroup(xml->i18nStringAttribute(element, QStringLiteral("label"), i18n("Select one:"), DL_INFO));

	addOptionsAndInit(element);

	vbox->addWidget(group_box);
	connect(group_box->group(), &QButtonGroup::idClicked, this, &RKRadio::itemSelected);
}

RKRadio::~RKRadio() {
	RK_TRACE(PLUGIN);
}

void RKRadio::setItemInGUI(int id) {
	RK_TRACE(PLUGIN);

	QAbstractButton *button = group_box->group()->button(id);
	if (button) button->setChecked(true);
}

void RKRadio::addOptionToGUI(const QString &label, int id) {
	RK_TRACE(PLUGIN);

	group_box->addButton(label, id);
}

void RKRadio::setItemEnabledInGUI(int id, bool enabled) {
	RK_TRACE(PLUGIN);

	QAbstractButton *button = group_box->group()->button(id);
	RK_ASSERT(button);
	button->setEnabled(enabled);
}

QStringList RKRadio::getUiLabelPair() const {
	RK_TRACE(PLUGIN);

	QStringList ret(stripAccelerators(group_box->title()));
	ret.append(stripAccelerators(group_box->group()->checkedButton()->text()));
	return ret;
}
