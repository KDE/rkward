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

	gui = new QWidget (0);
	gui->setCaption (_label);
	QGridLayout *layout = new QGridLayout (gui, 1, 1, 11, 6);

	if ((element.tagName () == "row") || (element.tagName () == "column")) {
		layout->addLayout (buildStructure (element), 0, 0);
	} else {
    	layout->addLayout (buildWidget (element), 0, 0);
	}

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
