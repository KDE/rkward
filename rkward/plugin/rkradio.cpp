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
#include <qlayout.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>

#include "../rkglobals.h"

RKRadio::RKRadio(const QDomElement &element, QWidget *parent, RKPlugin *plugin) : RKPluginWidget (element, parent, plugin) {
	QVBoxLayout *vbox = new QVBoxLayout (this, RKGlobals::spacingHint ());

	// create label
	label = new QLabel (element.attribute ("label", "Select one:"), this);
	vbox->addWidget (label);
	depend = element.attribute ("depend", "");

	// create ButtonGroup
	group = new QButtonGroup (this);

	// create internal layout for the buttons in the ButtonGroup
	group->setColumnLayout (0, Qt::Vertical);
	group->layout()->setSpacing (RKGlobals::spacingHint ());
	group->layout()->setMargin (RKGlobals::marginHint ());
	QVBoxLayout *group_layout = new QVBoxLayout (group->layout(), RKGlobals::spacingHint ());

	// create all the options
	QDomNodeList children = element.elementsByTagName("option");
	bool checked_one = false;	
	for (unsigned int i=0; i < children.count (); i++) {
		qDebug ("option");
		QDomElement child = children.item (i).toElement ();

		QRadioButton *button = new QRadioButton (child.attribute ("label"), group);
		options.insert (button, child.attribute ("value"));
		group_layout->addWidget (button);

		if (child.attribute ("checked") == "true") {
			button->setChecked (true);
			checked_one = true;
		}
	}
	// if none was set to checked, check the first
	if (!checked_one) {
		group->setButton (0);
	}

	connect (group, SIGNAL (clicked (int)), this, SLOT (buttonClicked (int)));

	vbox->addWidget (group);
}

RKRadio::~RKRadio(){
}

QString RKRadio::value (const QString &) {
	OptionsMap::Iterator it;
	for (it = options.begin(); it != options.end(); ++it) {
		if (it.key()->isChecked ()) {
			return (it.data ());
		}
	}

	return "";
}

void RKRadio::buttonClicked (int) {
	emit (changed ());
}

void RKRadio::setEnabled(bool checked){
  group->setEnabled(checked);
  label->setEnabled(checked);
  }


void RKRadio::slotActive(){
bool isOk = group->isEnabled();
group->setEnabled(! isOk) ;
label->setEnabled(! isOk) ;
}

void RKRadio::slotActive(bool isOk){
group->setEnabled(isOk) ;
label->setEnabled(isOk) ;
}

QRadioButton * RKRadio::findLabel (QString lab) {
  QRadioButton * sol = 0 ;
  OptionsMap::iterator findlab;
      for (findlab = options.begin(); findlab != options.end(); ++findlab) {
        if (findlab != options.end ()  ) { 
//          qDebug ("looking  : %s", findlab.data().latin1 ()) ;
        if ( findlab.data() == lab) { return findlab.key() ;};
          };
        };
   return  sol ;
}  

bool RKRadio::isOk(QString val) {
  QString sol ;
  OptionsMap::Iterator it;
	for (it = options.begin(); it != options.end(); ++it) {
		if (it.key()->isChecked ()) {
         sol   = it.data() ;
		};
    };
    if (sol == val) return true ;
    else return false;

 }