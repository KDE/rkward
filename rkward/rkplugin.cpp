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
#include <qgroupbox.h>
#include <qmap.h>
#include <qframe.h>
#include <qpushbutton.h>
#include <qtextedit.h>
#include <qregexp.h>

#include <klocale.h>

#include "rkward.h"
#include "rkwarddoc.h"
#include "rinterface.h"
#include "rcommand.h"
#include "phpbackend.h"
#include "rkpluginguiwidget.h"
#include "rkerrordialog.h"

// plugin-widgets
#include "rkpluginwidget.h"
#include "rkvarselector.h"
#include "rkvarslot.h"
#include "rktext.h"
#include "rkradio.h"
#include "rkcheckbox.h"

#define FOR_PHP_FLAG 1

RKPlugin::RKPlugin(RKwardApp *parent, const QString &label, const QString &filename) {
	app = parent;
	RKPlugin::filename = filename;
	_label = label;
	backend = 0;
	gui = 0;
}

RKPlugin::~RKPlugin(){
	qDebug ("Implement destructor in RKPlugin");
}

void RKPlugin::activated () {
	// can't activate the same plugin twice!
	if (gui) {
		gui->raise ();
		return;
	}
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
	
	// create an error-dialog
	error_dialog = new RKErrorDialog (i18n ("The R-backend has reported one or more error(s) while processing the plugin ") + gui->caption () + i18n (". This may lead to an incorrect ouput and is likely due to a bug in the plugin.\nA transcript of the error message(s) is shown below."), i18n ("R-Error"), false);
	
	// initialize the PHP-backend with the code-template
	dummy = QFileInfo (f).dirPath () + "/code.php";
	backend = new PHPBackend ();
	if (!backend->initTemplate (dummy, this)) return;
	
	// initialize code/warn-views
	changed ();
}

void RKPlugin::buildGUI (const QDomElement &layout_element) {
	gui = new RKPluginGUIWidget (this);
	gui->setCaption (_label);
	QGridLayout *grid = new QGridLayout (gui, 4, 3, 11, 6);
	QVBoxLayout *vbox = new QVBoxLayout (grid, 6);

	// default layout is in vertical	
	buildStructure (layout_element, vbox, gui);

	// build standard elements
	// lines
	QFrame *line;
	line = new QFrame (gui);
	line->setFrameShape (QFrame::VLine);
	line->setFrameShadow (QFrame::Plain);	
	grid->addWidget (line, 0, 1);
	line = new QFrame (gui);
	line->setFrameShape (QFrame::HLine);
	line->setFrameShadow (QFrame::Plain);
	grid->addMultiCellWidget (line, 1, 1, 0, 2);

	// buttons
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
	grid->addLayout (vbox, 0, 2);
	
	// text-fields
	codeDisplay = new QTextEdit (gui);
	codeDisplay->setMinimumHeight (40);
	codeDisplay->setReadOnly (true);
	warnDisplay = new QTextEdit (gui);
	warnDisplay->setMinimumHeight (40);
	warnDisplay->hide ();
	warnDisplay->setReadOnly (true);
	grid->addMultiCellWidget (codeDisplay, 3, 3, 0, 2);
	grid->addMultiCellWidget (warnDisplay, 4, 4, 0, 2);

	grid->setRowStretch (0, 4);

	gui->show ();
	connect (gui, SIGNAL (destroyed ()), this, SLOT (discard ()));
}

void RKPlugin::buildStructure (const QDomElement &element, QLayout *playout, QWidget *pwidget) {
	RKPluginWidget *widget = 0;

	QDomNodeList children = element.childNodes ();
	
	for (unsigned int i=0; i < children.count (); i++) {
		QDomElement e = children.item (i).toElement ();
		
	    if (e.tagName () == "row") {
			buildStructure (e, new QHBoxLayout (playout, 6), pwidget);
		} else if (e.tagName () == "column") {
			buildStructure (e, new QVBoxLayout (playout, 6), pwidget);
		} else if (e.tagName () == "frame") {
			QVBoxLayout *layout = new QVBoxLayout (playout, 6);	// just a container
			QGroupBox *box = new QGroupBox (1, Qt::Vertical, e.attribute ("label"), pwidget);
			layout->addWidget (box);
			buildStructure (e, box->layout (), box);
		} else if (e.tagName () == "varselector") {
			widget = new RKVarSelector (e, pwidget, this, playout);
		} else if (e.tagName () == "radio") {
			widget = new RKRadio (e, pwidget, this, playout);
		} else if (e.tagName () == "checkbox") {
			widget = new RKCheckBox (e, pwidget, this, playout);
		} else if (e.tagName () == "varslot") {
			widget = new RKVarSlot (e, pwidget, this, playout);
		} else {
			widget = new RKText (e, pwidget, this, playout);
		}
		
		if (widget) {
			widgets.insert (e.attribute ("id", "#noid#"), widget);
		}
	}
}

void RKPlugin::ok () {
	getApp ()->getDocument ()->syncToR ();
	getApp ()->r_inter->issueCommand (new RCommand (codeDisplay->text (), RCommand::Plugin, "", this, SLOT (gotRResult (RCommand *))));
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
		gui = 0;
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
	delete error_dialog;
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
	getApp ()->r_inter->issueCommand (new RCommand (call, RCommand::Plugin | RCommand::PluginCom, "", this, SLOT (gotRResult (RCommand *)), FOR_PHP_FLAG));
}

void RKPlugin::gotRResult (RCommand *command) {
	if (command->hasError()) {
		error_dialog->newError (command->error());
	}
	if (command->getFlags() & FOR_PHP_FLAG) {
		// since R-calls are (will be) asynchronous, we need to expect incoming data after the backend has been torn down
		if (backend) backend->gotRCallResult (command->output());
	}
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
