/***************************************************************************
                          rkpluginspinbox  -  description
                             -------------------
    begin                : Wed Aug 11 2004
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
#include "rkpluginspinbox.h"

#include <qwidget.h>
#include <qlabel.h>
#include <qdom.h>

#include "../misc/rkspinbox.h"
#include "rkplugin.h"

RKPluginSpinBox::RKPluginSpinBox (const QDomElement &element, QWidget *parent, RKPlugin *plugin, QLayout *layout) : RKPluginWidget(element, parent, plugin, layout) {
	label = new QLabel (element.attribute ("label", "Enter value:"), parent);
	addWidget (label);

	spinbox = new RKSpinBox (parent);
	if (element.attribute ("type") != "integer") {
		spinbox->setRealMode (element.attribute ("min", "0").toFloat (), element.attribute ("max", "1").toFloat (), element.attribute ("initial", "0").toFloat (), element.attribute ("default_precision", "2").toInt (), element.attribute ("max_precision", "4").toInt ());
	} else {
		spinbox->setIntMode (element.attribute ("min", "0").toInt (), element.attribute ("max", "100").toInt (), element.attribute ("initial", "0").toInt ());
	}
	connect (spinbox, SIGNAL (valueChanged (int)), this, SLOT (valueChanged (int)));
	addWidget (spinbox);
}

RKPluginSpinBox::~RKPluginSpinBox () {
}

QString RKPluginSpinBox::value (const QString &) {
	return (spinbox->text ());
}

void RKPluginSpinBox::valueChanged (int) {
	plugin ()->changed ();
}
