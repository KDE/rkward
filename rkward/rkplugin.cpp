/***************************************************************************
                          rkplugin.cpp  -  description
                             -------------------
    begin                : Wed Nov 6 2002
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

#include "rkplugin.h"

#include <qdom.h>
#include <qfile.h>
#include <qdialog.h>
#include <qlayout.h>
#include <qmap.h>
#include <qframe.h>
#include <qpushbutton.h>
#include <qtextedit.h>

#include "rkmenu.h"
#include "rkpluginwidget.h"
#include "rkvarselector.h"
#include "rkvarslot.h"
#include "rkradio.h"

RKPlugin::RKPlugin(RKMenu *parent, const QDomElement &element, QString filename) {
	RKPlugin::parent = parent;
	RKPlugin::filename = filename;
	_label = element.attribute ("label", "untitled");
}

RKPlugin::~RKPlugin(){
}

void RKPlugin::activated () {
	qDebug ("activated plugin: " + filename);
	buildGUI ();
}

void RKPlugin::buildGUI () {
	// open XML-file (TODO: remove code-duplication)
	int error_line, error_column;
	QString error_message, dummy;
	QDomDocument doc;
	QFile f(filename);
	if (!f.open(IO_ReadOnly))
		qDebug ("Could not open file for reading: " + filename);
	if (!doc.setContent(&f, false, &error_message, &error_line, &error_column)) {
		f.close();
		qDebug ("parsing-error in: " + filename);
		qDebug ("Message: " + error_message);
		qDebug ("Line: %d", error_line);
		qDebug ("Column: %d", error_column);
		return;
	}
	f.close();

	// find layout-section
	QDomElement element = doc.documentElement ();
	QDomNodeList children = element.elementsByTagName("layout");
	element = children.item (0).toElement ();

	// layout-section may only contain one top-level component
	children = element.childNodes ();
	element = children.item (0).toElement ();

	gui = new QWidget (0, "", Qt::WDestructiveClose);
	gui->setCaption (_label);
	QGridLayout *layout = new QGridLayout (gui, 4, 3, 11, 6);

	if ((element.tagName () == "row") || (element.tagName () == "column")) {
		layout->addLayout (buildStructure (element), 0, 0);
	} else {
    	layout->addLayout (buildWidget (element), 0, 0);
	}

	// build standard elements
	// lines
	QFrame *line;
	line = new QFrame (gui);
	line->setFrameShape (QFrame::VLine);
	line->setFrameShadow (QFrame::Plain);	
	layout->addWidget (line, 0, 1);
	line = new QFrame (gui);
	line->setFrameShape (QFrame::HLine);
	line->setFrameShadow (QFrame::Plain);
	layout->addMultiCellWidget (line, 1, 1, 0, 2);

	// buttons
	QVBoxLayout *vbox;
	vbox = new QVBoxLayout (0, 0, 6);
	okButton = new QPushButton ("Ok", gui);
	connect (okButton, SIGNAL (clicked ()), this, SLOT (ok ()));
	cancelButton = new QPushButton ("Cancel", gui);
	connect (cancelButton, SIGNAL (clicked ()), this, SLOT (cancel ()));
	helpButton = new QPushButton ("Help", gui);
	connect (helpButton, SIGNAL (clicked ()), this, SLOT (help ()));
	toggleCodeButton = new QPushButton ("Code", gui);
	toggleCodeButton->setToggleButton (true);
	toggleCodeButton->setOn (true);
	connect (toggleCodeButton, SIGNAL (clicked ()), this, SLOT (toggleCode ()));
	toggleWarnButton = new QPushButton ("Problems", gui);
	toggleWarnButton->setToggleButton (true);
	connect (toggleWarnButton, SIGNAL (clicked ()), this, SLOT (toggleWarn ()));
	vbox->addWidget (okButton);
	vbox->addWidget (cancelButton);
	vbox->addStretch (1);
	vbox->addWidget (helpButton);
	vbox->addStretch (2);
	vbox->addWidget (toggleCodeButton);
	vbox->addWidget (toggleWarnButton);
	layout->addLayout (vbox, 0, 2);
	
	// text-fields
	codeDisplay = new QTextEdit (gui);
	codeDisplay->setMinimumHeight (40);
	codeDisplay->setReadOnly (true);
	warnDisplay = new QTextEdit (gui);
	warnDisplay->setMinimumHeight (40);
	warnDisplay->hide ();
	warnDisplay->setReadOnly (true);
	layout->addMultiCellWidget (codeDisplay, 3, 3, 0, 2);
	layout->addMultiCellWidget (warnDisplay, 4, 4, 0, 2);

	layout->setRowStretch (0, 4);

	gui->show ();
}

QBoxLayout *RKPlugin::buildStructure (const QDomElement &element) {
	QBoxLayout *layout;
    if (element.tagName () == "row") {
		layout = new QHBoxLayout (0, 0, 6);
	} else {
		layout = new QVBoxLayout (0, 0, 6);
	}

	QDomNodeList children = element.childNodes ();

	for (unsigned int i=0; i < children.count (); i++) {
		QDomElement child = children.item (i).toElement ();

		if ((child.tagName () == "row") || (child.tagName () == "column")) {
			layout->addLayout (buildStructure (child));
		} else {
			// it's a widget
			layout->addLayout (buildWidget (child));
		}
	}

	return layout;	
}

RKPluginWidget *RKPlugin::buildWidget (const QDomElement &element) {
	RKPluginWidget *widget;
	qDebug ("creating RKPluginWidget " + element.tagName ());
	if (element.tagName () == "varselector") {
		widget = new RKVarSelector (element, gui, this);
	} else if (element.tagName () == "radio") {
		widget = new RKRadio (element, gui, this);
	} else {
		// TODO: to be changed, of course
		widget = new RKVarSlot (element, gui, this);
	}

	widgets.insert (element.attribute ("id"), widget);

	return widget;
}

void RKPlugin::ok () {
}

void RKPlugin::cancel () {
}

void RKPlugin::toggleCode () {
	if (codeDisplay->isVisible ()) {
		codeDisplay->hide ();
	} else {
		codeDisplay->show ();
	}
}

void RKPlugin::toggleWarn () {
	if (warnDisplay->isVisible ()) {
		warnDisplay->hide ();
	} else {
		warnDisplay->show ();
	}
}

void RKPlugin::help () {
}
