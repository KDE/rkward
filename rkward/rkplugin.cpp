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
#include <qtabwidget.h>
#include <qsplitter.h>

#include <klocale.h>

#include "rkward.h"
#include "rkwarddoc.h"
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
	php_backend_chain = 0;
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
	
	QGridLayout *main_grid = new QGridLayout (gui, 1, 1, 11, 6);
	QSplitter *splitter = new QSplitter (QSplitter::Vertical, gui);
	main_grid->addWidget (splitter, 0, 0);
	QWidget *upper_widget = new QWidget (splitter);
	
	QGridLayout *grid = new QGridLayout (upper_widget, 1, 3, 11, 6);
	QVBoxLayout *vbox = new QVBoxLayout (grid, 6);

	// default layout is in vertical	
	buildStructure (layout_element, vbox, upper_widget);

	// build standard elements
	// lines
	QFrame *line;
	line = new QFrame (upper_widget);
	line->setFrameShape (QFrame::VLine);
	line->setFrameShadow (QFrame::Plain);	
	grid->addWidget (line, 0, 1);

	// buttons
	vbox = new QVBoxLayout (0, 0, 6);
	okButton = new QPushButton ("Submit", upper_widget);
	connect (okButton, SIGNAL (clicked ()), this, SLOT (ok ()));
	cancelButton = new QPushButton ("Close", upper_widget);
	connect (cancelButton, SIGNAL (clicked ()), this, SLOT (cancel ()));
	helpButton = new QPushButton ("Help", upper_widget);
	connect (helpButton, SIGNAL (clicked ()), this, SLOT (help ()));
	toggleCodeButton = new QPushButton ("Code", upper_widget);
	toggleCodeButton->setToggleButton (true);
	toggleCodeButton->setOn (true);
	connect (toggleCodeButton, SIGNAL (clicked ()), this, SLOT (toggleCode ()));
	toggleWarnButton = new QPushButton ("Problems", upper_widget);
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
	QWidget *lower_widget = new QWidget (splitter);
	
	vbox = new QVBoxLayout (lower_widget, 6);
	codeDisplay = new QTextEdit (lower_widget);
	codeDisplay->setMinimumHeight (40);
	codeDisplay->setReadOnly (true);
	codeDisplay->setWordWrap (QTextEdit::NoWrap);
	warnDisplay = new QTextEdit (lower_widget);
	warnDisplay->setMinimumHeight (40);
	warnDisplay->hide ();
	warnDisplay->setReadOnly (true);
	vbox->addWidget (codeDisplay);
	vbox->addWidget (warnDisplay);

	gui->show ();
	connect (gui, SIGNAL (destroyed ()), this, SLOT (discard ()));
}

void RKPlugin::buildStructure (const QDomElement &element, QBoxLayout *playout, QWidget *pwidget) {
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
			QWidget *dummy = new QWidget (box);		// cumbersome workaround. Can this be done in a more straightforward way?
			QVBoxLayout *ilayout = new QVBoxLayout (dummy, 6);
			buildStructure (e, ilayout, dummy);
		} else if (e.tagName () == "tabbook") {
			QTabWidget *tabbook = new QTabWidget (pwidget);
			playout->addWidget (tabbook);
			QDomNodeList tabs = e.childNodes ();
			for (unsigned int t=0; t < tabs.count (); ++t) {
				QDomElement tab_e = tabs.item (t).toElement ();
				if (tab_e.tagName () == "tab") {
					QWidget *tabwidget = new QWidget (tabbook);
					QVBoxLayout *ilayout = new QVBoxLayout (tabwidget);
					buildStructure (tab_e, ilayout, tabwidget);
					tabbook->addTab (tabwidget, tab_e.attribute ("label"));
				}
			}
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
	php_backend_chain = getApp ()->r_inter->startChain ();
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
		getApp ()->r_inter->closeChain (php_backend_chain);
		php_backend_chain = 0;
	} else {
		gui->hide ();
		should_destruct = true;
	}
}

bool RKPlugin::backendIdle () {
	qDebug ("backendIdle");
	getApp ()->r_inter->closeChain (php_backend_chain);
	php_backend_chain = 0;
	
	if (should_destruct) {
		try_destruct ();
		return true;
	}

	if (should_updatecode) {
		codeDisplay->setText ("");
		php_backend_chain = getApp ()->r_inter->startChain ();
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
	getApp ()->r_inter->issueCommand (new RCommand (call, RCommand::Plugin | RCommand::PluginCom, "", this, SLOT (gotRResult (RCommand *)), FOR_PHP_FLAG), php_backend_chain);
}

void RKPlugin::getRVector (const QString &call) {
	getApp ()->r_inter->issueCommand (new RCommand (call, RCommand::Plugin | RCommand::PluginCom | RCommand::GetStringVector, "", this, SLOT (gotRResult (RCommand *)), FOR_PHP_FLAG), php_backend_chain);
}

void RKPlugin::gotRResult (RCommand *command) {
	if (command->hasError()) {
		error_dialog->newError (command->error());
	}
	if (command->getFlags() & FOR_PHP_FLAG) {
		if (!backend) return;
		if ((command->type () & RCommand::GetStringVector)) {
			QString temp;
// TODO: can this painful process be optimized?
			for (int i = 0; i < command->stringVectorLength (); ++i) {
				if (i) {
					temp.append ("\t");
				}
				temp.append (command->getStringVector ()[i]);
			}
			backend->gotRCallResult (temp);
		} else {
			// since R-calls are (will be) asynchronous, we need to expect incoming data after the backend has been torn down
			backend->gotRCallResult (command->output());
		}
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

	return (widget->value (id.section (".", 1, 1)));
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
