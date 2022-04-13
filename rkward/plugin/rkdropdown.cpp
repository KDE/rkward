/*
rkdropdown.h - This file is part of RKWard (https://rkward.kde.org). Created: Fri Jan 12 2007
SPDX-FileCopyrightText: 2007-2014 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "rkdropdown.h"

#include <qdom.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qcombobox.h>
#include <QListWidget>

#include <KLocalizedString>

#include "../misc/xmlhelper.h"
#include "../debug.h"

RKDropDown::RKDropDown (const QDomElement &element, RKComponent *parent_component, QWidget *parent_widget) : RKAbstractOptionSelector (parent_component, parent_widget) {
	RK_TRACE (PLUGIN);

	// get xml-helper
	XMLHelper *xml = parent_component->xmlHelper ();

	// create layout
	QVBoxLayout *vbox = new QVBoxLayout (this);
	vbox->setContentsMargins (0, 0, 0, 0);

	label = new QLabel (xml->i18nStringAttribute (element, "label", i18n ("Select one:"), DL_INFO), this);
	vbox->addWidget (label);

	// create ComboBox
	box = new QComboBox (this);
	box->setEditable (false);
	listwidget = new QListWidget (box);
	box->setModel (listwidget->model ());
	box->setView (listwidget);

	addOptionsAndInit (element);

	vbox->addWidget (box);
	connect (box, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), this, &RKDropDown::comboItemActivated);
}

RKDropDown::~RKDropDown(){
	RK_TRACE (PLUGIN);
}

void RKDropDown::comboItemActivated (int id) {
	RK_TRACE (PLUGIN);

	itemSelected (id);
}

void RKDropDown::setItemInGUI (int id) {
	RK_TRACE (PLUGIN);

	box->setCurrentIndex (id);
}

void RKDropDown::addOptionToGUI (const QString &label, int id) {
	RK_TRACE (PLUGIN);

	box->insertItem (id, label);
}

void RKDropDown::setItemEnabledInGUI (int id, bool enabled) {
	RK_TRACE (PLUGIN);

	QListWidgetItem* item = listwidget->item (id);
	RK_ASSERT (item);
	int flags = item->flags ();
	if (enabled) flags |= Qt::ItemIsEnabled | Qt::ItemIsSelectable;
	else flags -= flags & (Qt::ItemIsEnabled | Qt::ItemIsSelectable);
	item->setFlags ((Qt::ItemFlags) flags);
}

QStringList RKDropDown::getUiLabelPair () const {
	RK_TRACE (PLUGIN);

	QStringList ret (stripAccelerators (label->text ()));
	ret.append (stripAccelerators (box->currentText ()));
	return ret;
}

