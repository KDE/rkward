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
#include <qregexp.h>

#include "rkward.h"
#include "rkwarddoc.h"
#include "rinterface.h"
#include "rcommand.h"

// plugin-widgets
#include "rkpluginwidget.h"
#include "rkvarselector.h"
#include "rkvarslot.h"
#include "rktext.h"
#include "rkradio.h"

RKPlugin::RKPlugin(RKwardApp *parent, const QDomElement &element, QString filename) {
	app = parent;
	RKPlugin::filename = filename;
	_label = element.attribute ("label", "untitled");
}

RKPlugin::~RKPlugin(){
}

void RKPlugin::activated () {
	qDebug ("activated plugin: " + filename);

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

	// construct the GUI
	buildGUI (element);

	// retrieve other relevant information from XML-file
	element = doc.documentElement ();
	children = element.elementsByTagName("code");
	if (children.length ()) {
		element = children.item (0).toElement ();

		QStringList lines = lines.split ("\n", element.text (), false);
		for (unsigned int i=0; i < lines.count (); i++) {
			QString line = lines[i].stripWhiteSpace ();
			if (line != "") {
				code_template.append (line + "\n");
			}
		}

		// strip final newline
		code_template.truncate (code_template.length () -1);
	}

	// initialize code/warn-views
	changed ();
}

void RKPlugin::buildGUI (const QDomElement &layout_element) {
	// layout-section may only contain one top-level component
	QDomNodeList children = layout_element.childNodes ();
	QDomElement element = children.item (0).toElement ();

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
	okButton = new QPushButton ("Submit", gui);
	connect (okButton, SIGNAL (clicked ()), this, SLOT (ok ()));
	cancelButton = new QPushButton ("Close", gui);
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
	connect (gui, SIGNAL (destroyed ()), this, SLOT (discard ()));
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
	} else if (element.tagName () == "varslot") {
		widget = new RKVarSlot (element, gui, this);
	} else {
		widget = new RKText (element, gui, this);
	}

	widgets.insert (element.attribute ("id", "#noid#"), widget);

	return widget;
}

void RKPlugin::ok () {
	getApp ()->getDocument ()->syncToR ();
	getApp ()->r_inter.issueCommand (new RCommand (codeDisplay->text (), RCommand::Plugin));
}

void RKPlugin::cancel () {
	delete gui;
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
	// TODO
}

void RKPlugin::discard () {
	code_template = "";
	// the entire GUI and all widgets get deleted automatically!
	qDebug ("plugin cleaned");
}

void RKPlugin::changed () {
	// update code-display
	// TODO: make more flexible (allow "$$" etc);
	QString code;
	QString section;
	int i = 0;
	while ((section = code_template.section ("$", i, i)) != "") {
		if (i % 2) {
			QString ident = section.section ("_", 0, 0);
			RKPluginWidget *widget;
			if (widgets.contains (ident)) {
				widget = widgets[ident];
			} else {
				widget = 0;
				qDebug ("Couldn't find value for $" + section +"$!");
			}

			QString modifier = section.section ("_", 1, 1);

			if (widget) {
				if (modifier != "") {
					QString quoted;
					if (modifier == "label") {
						int col = getApp ()->getDocument ()->lookUp (widget->value ());
						if (col >= 0) {
							quoted = getApp ()->getDocument ()->label (col);
						}
					} else if (modifier == "name") {
						quoted = widget->value ();
					}
					code.append (quoted.replace (QRegExp ("\""), "\\\""));
				} else {
					code.append (widget->value ());
				}
			}
		} else {
			code.append (section);
		}
		i++;
	}
	codeDisplay->setText (code);

	// check whether everything is satisfied and fetch any complaints
	bool ok = true;
	QString warn;
	WidgetsMap::Iterator it;
	for (it = widgets.begin(); it != widgets.end(); ++it) {
		if (!it.data ()->isSatisfied ()) {
			ok = false;
		}
		warn.append (it.data ()->complaints ());
	}

	okButton->setEnabled (ok);
	warn.truncate (warn.length () -1);
	warnDisplay->setText (warn);	
}

/** Returns a pointer to the varselector by that name (0 if not available) */
RKVarSelector *RKPlugin::getVarSelector (const QString &id) {
	RKPluginWidget *selector = widgets[id];
	if (selector->isVarSelector ()) {
		return (RKVarSelector *) selector;
	}

	qDebug ("failed to find varselector!");
	return 0;
}
