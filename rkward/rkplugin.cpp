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
#include <qfileinfo.h>
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
#include "phpbackend.h"
#include "rkpluginguiwidget.h"

// plugin-widgets
#include "rkpluginwidget.h"
#include "rkvarselector.h"
#include "rkvarslot.h"
#include "rktext.h"
#include "rkradio.h"

RKPlugin::RKPlugin(RKwardApp *parent, const QString &label, const QString &filename) {
	app = parent;
	RKPlugin::filename = filename;
	_label = label;
	backend = 0;
}

RKPlugin::~RKPlugin(){
	qDebug ("Implement destructor in RKPlugin");
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

	// keep it alive
	should_destruct = false;
	should_updatecode = true;
	
	// initialize the PHP-backend with the code-template
	dummy = QFileInfo (f).dirPath () + "/code.php";
	backend = new PHPBackend ();
	if (!backend->initTemplate (dummy, this)) return;
	
	// initialize code/warn-views
	changed ();
}

void RKPlugin::buildGUI (const QDomElement &layout_element) {
	// layout-section may only contain one top-level component
	QDomNodeList children = layout_element.childNodes ();
	QDomElement element = children.item (0).toElement ();

	gui = new RKPluginGUIWidget (this);
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
	getApp ()->r_inter->issueCommand (new RCommand (codeDisplay->text (), RCommand::Plugin));
	backend->callFunction ("printout (); cleanup ();", true);
}

void RKPlugin::cancel () {
	try_destruct ();
}

void RKPlugin::toggleCode () {
	if (codeDisplay->isVisible ()) {
		codeDisplay->hide ();
	} else {
		codeDisplay->show ();
	}
}

void RKPlugin::newOutput () {
	app->newOutput();
}

void RKPlugin::try_destruct () {
	qDebug ("try_destruct");
	if (!backend->isBusy ()) {
		delete gui;
		delete backend;
		backend = 0;
	} else {
		gui->hide ();
		should_destruct = true;
	}
}

bool RKPlugin::backendIdle () {
	qDebug ("backendIdle");
	if (should_destruct) {
		try_destruct ();
		return true;
	}

	if (should_updatecode) {
		codeDisplay->setText ("");
		backend->callFunction ("preprocess (); calculate ();");
		should_updatecode = false;
		warnDisplay->setText ("Processing. Please wait.");
		okButton->setEnabled (false);
		return false;	// backend is not idle anymore, now
	}

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
	
	return false;
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
	// the entire GUI and all widgets get deleted automatically!
	qDebug ("plugin cleaned");
}

void RKPlugin::changed () {
	// trigger update for code-display
	should_updatecode = true;
	
	if (!backend->isBusy ()) {
		backendIdle ();
	} else {
		okButton->setEnabled (false);
		warnDisplay->setText ("Processing. Please wait.");
	}
}

void RKPlugin::updateCode (const QString &text) {
	codeDisplay->insert (text);
}

void RKPlugin::doRCall (const QString &call) {
	getApp ()->r_inter->issueCommand (new RCommand (call, RCommand::Plugin | RCommand::PluginCom, "", this, SLOT (gotRResult (RCommand *))));
}

void RKPlugin::gotRResult (RCommand *command) {
// since R-calls are (will be) asynchronous, we need to expect incoming data after the backend has been torn down
	if (backend) backend->gotRCallResult (command->output());
}

QString RKPlugin::getVar (const QString &id) {
	QString ident = id.section (".", 0, 0);
	RKPluginWidget *widget;
	if (widgets.contains (ident)) {
		widget = widgets[ident];
	} else {
		widget = 0;
		qDebug ("Couldn't find value for $" + id +"$!");
		return ("#unavailable#");
	}

	QString modifier = id.section (".", 1, 1);

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
			return (quoted.replace (QRegExp ("\""), "\\\""));
		} else {
			return (widget->value ());
		}
	}
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
