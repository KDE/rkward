/***************************************************************************
                          rkcheckbox  -  description
                             -------------------
    begin                : Fri Jul 30 2004
    copyright            : (C) 2004 by Thomas Friedrichsmeier
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
#include "rkcheckbox.h"

#include <qdom.h>
#include <qcheckbox.h>

#include "rkplugin.h"

RKCheckBox::RKCheckBox (const QDomElement &element, QWidget *parent, RKPlugin *plugin, QLayout *layout) : RKPluginWidget (element, parent, plugin, layout) {
	qDebug ("creating checkbox");

	checkbox = new QCheckBox (element.attribute ("label"), parent);
	
	value_if_checked = element.attribute ("value", "1");
	value_if_unchecked = element.attribute ("value_unchecked", "");
	
	if (element.attribute ("checked") == true) {
		checkbox->setChecked (true);
	}

	connect (checkbox, SIGNAL (stateChanged (int)), this, SLOT (changed (int)));
	
	addWidget (checkbox);
}

RKCheckBox::~RKCheckBox () {
}

QString RKCheckBox::value () {
	if (checkbox->isChecked ()) {
		return value_if_checked;
	} else {
		return value_if_unchecked;
	}
}

void RKCheckBox::changed (int id) {
	plugin ()->changed ();
}

