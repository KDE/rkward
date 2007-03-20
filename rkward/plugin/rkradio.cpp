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
#include <qlayout.h>
#include <qvbuttongroup.h>
#include <qradiobutton.h>

#include <klocale.h>

#include "../rkglobals.h"
#include "../misc/xmlhelper.h"
#include "../debug.h"

RKRadio::RKRadio (const QDomElement &element, RKComponent *parent_component, QWidget *parent_widget) : RKAbstractOptionSelector (parent_component, parent_widget) {
	RK_TRACE (PLUGIN);

	// get xml-helper
	XMLHelper *xml = XMLHelper::getStaticHelper ();

	// create layout
	QVBoxLayout *vbox = new QVBoxLayout (this, RKGlobals::spacingHint ());

	// create ButtonGroup
	group = new QVButtonGroup (xml->getStringAttribute (element, "label", i18n ("Select one:"), DL_INFO), this);

	// adjust internal layout for the buttons in the ButtonGroup
	RK_ASSERT (group->layout ());
	group->layout()->setSpacing (RKGlobals::spacingHint ());
	group->layout()->setMargin (RKGlobals::marginHint ());

	addOptionsAndInit (element);

	vbox->addWidget (group);
	connect (group, SIGNAL (clicked (int)), this, SLOT (itemSelected (int)));
}

RKRadio::~RKRadio(){
	RK_TRACE (PLUGIN);
}

void RKRadio::setItemInGUI (int id) {
	RK_TRACE (PLUGIN);

	group->setButton (id);
}

void RKRadio::addOptionToGUI (const QString &label, int id) {
	RK_TRACE (PLUGIN);

	QRadioButton *button = new QRadioButton (label, group);
	group->insert (button, id);
}

void RKRadio::setItemEnabledInGUI (int id, bool enabled) {
	RK_TRACE (PLUGIN);

	QButton *button = group->find (id);
	RK_ASSERT (button);
	button->setEnabled (enabled);
}

#include "rkradio.moc"
