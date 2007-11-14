/***************************************************************************
                          rkradio.cpp  -  description
                             -------------------
    begin                : Thu Nov 7 2002
    copyright            : (C) 2002, 2006, 2007 by Thomas Friedrichsmeier
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
#include <QButtonGroup>
#include <QGroupBox>
#include <qradiobutton.h>
#include <QVBoxLayout>

#include <klocale.h>

#include "../rkglobals.h"
#include "../misc/xmlhelper.h"
#include "../debug.h"

RKRadio::RKRadio (const QDomElement &element, RKComponent *parent_component, QWidget *parent_widget) : RKAbstractOptionSelector (parent_component, parent_widget) {
	RK_TRACE (PLUGIN);

	// get xml-helper
	XMLHelper *xml = XMLHelper::getStaticHelper ();

	// create layout
	QVBoxLayout *vbox = new QVBoxLayout (this);
	vbox->setContentsMargins (0, 0, 0, 0);

	// create ButtonGroup
	group = new QButtonGroup (this);
	group_box = new QGroupBox (xml->getStringAttribute (element, "label", i18n ("Select one:"), DL_INFO), this);
	new QVBoxLayout (group_box);

	addOptionsAndInit (element);

	vbox->addWidget (group_box);
	connect (group, SIGNAL (buttonClicked (int)), this, SLOT (itemSelected (int)));
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

#include "rkradio.moc"
