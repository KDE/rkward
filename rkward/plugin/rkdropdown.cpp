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
#include <QListWidget>

#include <klocale.h>

#include "../rkglobals.h"
#include "../misc/xmlhelper.h"
#include "../debug.h"

RKDropDown::RKDropDown (const QDomElement &element, RKComponent *parent_component, QWidget *parent_widget) : RKAbstractOptionSelector (parent_component, parent_widget) {
	RK_TRACE (PLUGIN);

	// get xml-helper
	XMLHelper *xml = parent_component->xmlHelper ();

	// create layout
	QVBoxLayout *vbox = new QVBoxLayout (this);
	vbox->setContentsMargins (0, 0, 0, 0);

	QLabel *label = new QLabel (xml->i18nStringAttribute (element, "label", i18n ("Select one:"), DL_INFO), this);
	vbox->addWidget (label);

	// create ComboBox
	box = new QComboBox (this);
	box->setEditable (false);
	listwidget = new QListWidget (box);
	box->setModel (listwidget->model ());
	box->setView (listwidget);

	addOptionsAndInit (element);

	vbox->addWidget (box);
	connect (box, SIGNAL (activated (int)), this, SLOT (comboItemActivated (int)));
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

#include "rkdropdown.moc"
