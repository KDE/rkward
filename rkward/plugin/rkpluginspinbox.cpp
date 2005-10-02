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

#include <qlayout.h>
#include <qlabel.h>
#include <qdom.h>

#include "../misc/rkspinbox.h"
#include "../rkglobals.h"
#include "../debug.h"
#include "rkplugin.h"

RKPluginSpinBox::RKPluginSpinBox (const QDomElement &element, QWidget *parent, RKPlugin *plugin) : RKPluginWidget(element, parent, plugin) {
		RK_TRACE (PLUGIN);
	QVBoxLayout *vbox = new QVBoxLayout (this, RKGlobals::spacingHint ());
	
	label = new QLabel (element.attribute ("label", "Enter value:"), this);
	vbox->addWidget (label);
	depend = element.attribute ("depend", QString::null);

	spinbox = new RKSpinBox (this);
	if (element.attribute ("type") != "integer") {
		spinbox->setRealMode (element.attribute ("min", "0").toFloat (), element.attribute ("max", "1").toFloat (), element.attribute ("initial", "0").toFloat (), element.attribute ("default_precision", "2").toInt (), element.attribute ("max_precision", "4").toInt ());
	} else {
		spinbox->setIntMode (element.attribute ("min", "0").toInt (), element.attribute ("max", "100").toInt (), element.attribute ("initial", "0").toInt ());
	}
	connect (spinbox, SIGNAL (valueChanged (int)), this, SLOT (valueChanged (int)));
	vbox->addWidget (spinbox);
	size = element.attribute ("size", "small");
	if (size=="small"){
	spinbox->setMaximumWidth(100);
	spinbox->setMinimumWidth(100);
	}
}

RKPluginSpinBox::~RKPluginSpinBox () {
	RK_TRACE (PLUGIN);
}

void RKPluginSpinBox::setEnabled(bool checked){
	RK_TRACE (PLUGIN);
  spinbox->setEnabled(checked);
  label->setEnabled(checked);
  }

QString RKPluginSpinBox::value (const QString &) {
	RK_TRACE (PLUGIN);
	return (spinbox->text ());
}

void RKPluginSpinBox::valueChanged (int) {
	RK_TRACE (PLUGIN);
	plugin ()->changed ();
}

void RKPluginSpinBox::slotActive(){
	RK_TRACE (PLUGIN);
bool isOk = spinbox->isEnabled();
spinbox->setEnabled(! isOk) ;
label->setEnabled(! isOk);
}

void RKPluginSpinBox::slotActive(bool isOk){
	RK_TRACE (PLUGIN);
spinbox->setEnabled( isOk) ;
label->setEnabled(isOk);
}
void RKPluginSpinBox::adjust(int longueur, int largeur){
	RK_TRACE (PLUGIN);
spinbox->resize(longueur, largeur);
}

#include "rkpluginspinbox.moc"
