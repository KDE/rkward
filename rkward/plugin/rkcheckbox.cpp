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
#include <qlayout.h>
#include <qcheckbox.h>

#include "rkplugin.h"
#include "../rkglobals.h"

RKCheckBox::RKCheckBox (const QDomElement &element, QWidget *parent, RKPlugin *plugin) : RKPluginWidget (element, parent, plugin) {
	qDebug ("creating checkbox");

  QVBoxLayout *vbox = new QVBoxLayout (this, RKGlobals::spacingHint ());
	checkbox = new QCheckBox (element.attribute ("label"), this);
	vbox->addWidget (checkbox);
  isOk =false;
	value_if_checked = element.attribute ("value", "1");
	value_if_unchecked = element.attribute ("value_unchecked", "");
	depend = element.attribute ("depend", "");
	
	if (element.attribute ("checked") == "true") {
		checkbox->setChecked (true);
    isOk =true ;
	}

	connect (checkbox, SIGNAL (stateChanged (int)), this, SLOT (changedState (int)));
  connect(checkbox,SIGNAL(clicked()),  SIGNAL(clicked()) );
}

RKCheckBox::~RKCheckBox () {
}


void RKCheckBox::setEnabled(bool checked){
  checkbox->setEnabled(checked);
  }
  

QString RKCheckBox::value (const QString &) {
	if (checkbox->isChecked ()) {
		return value_if_checked;
	} else {
		return value_if_unchecked;
	}
}


void RKCheckBox::changedState (int) {
	emit (changed ());
}

void RKCheckBox::slotActive(bool isOk){
checkbox->setEnabled(isOk) ;
}


void RKCheckBox::slotActive(){
bool isOk = checkbox->isEnabled();
checkbox->setEnabled(! isOk) ;
}

