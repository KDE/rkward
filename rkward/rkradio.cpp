/***************************************************************************
                          rkradio.cpp  -  description
                             -------------------
    begin                : Thu Nov 7 2002
    copyright            : (C) 2002 by Thomas Friedrichsmeier
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
#include <qbuttongroup.h>
#include <qradiobutton.h>

RKRadio::RKRadio(const QDomElement &element, QWidget *parent, RKPlugin *plugin) : RKPluginWidget (element, parent, plugin) {
	qDebug ("creating radio");

	// create label	
	label = new QLabel (element.attribute ("label", "Variable:"), parent);
	addWidget (label);

	// create ButtonGroup
	group = new QButtonGroup (parent);

	// create internal layout for the buttons in the ButtonGroup
	group->setColumnLayout(0, Qt::Vertical );
	group->layout()->setSpacing( 6 );
	group->layout()->setMargin( 11 );
	QVBoxLayout *group_layout = new QVBoxLayout(group->layout());

	// create all the options
	QDomNodeList children = element.elementsByTagName("option");
	bool checked_one = false;	
	for (unsigned int i=0; i < children.count (); i++) {
		qDebug ("option");
		QDomElement child = children.item (i).toElement ();

		QRadioButton *button = new QRadioButton (child.attribute ("label"), group);
		options.insert (button, child.attribute ("value"));
		group_layout->addWidget (button);

		if (child.attribute ("checked") == true) {
			button->setChecked (true);
			checked_one = true;
		}
	}
	// if none was set to checked, check the first
	if (!checked_one) {
		group->setButton (0);
	}

	addWidget (group);
}

RKRadio::~RKRadio(){
}
