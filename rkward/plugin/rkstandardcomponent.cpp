/***************************************************************************
                          rkstandardcomponent  -  description
                             -------------------
    begin                : Sun Feb 19 2006
    copyright            : (C) 2006 by Thomas Friedrichsmeier
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

#include "rkstandardcomponent.h"

//#include <qdom.h>
//#include <qfile.h>
#include <qfileinfo.h>
//#include <qdialog.h>
#include <qlayout.h>
#include <qgroupbox.h>
//#include <qmap.h>
#include <qframe.h>
#include <qpushbutton.h>
//#include <qregexp.h>
//#include <qtabwidget.h>
#include <qsplitter.h>
//#include <qwidgetstack.h>
#include <qlabel.h>
//#include <qtimer.h>
#include <qapplication.h>

#include <klocale.h>

#include "../rkcommandeditor.h"
#include "../scriptbackends/phpbackend.h"
#include "../misc/rkerrordialog.h"
#include "../misc/xmlhelper.h"
/*#include "../rkward.h"
#include "../rkeditormanager.h"
#include "../rkcommandeditor.h"
#include "../settings/rksettingsmoduleplugins.h"
#include "../rbackend/rinterface.h"*/

// plugin-widgets
/*#include "rkpluginwidget.h"
#include "rkvarselector.h"
#include "rkvarslot.h"
#include "rktext.h"
#include "rkradio.h"
#include "rkcheckbox.h"
#include "rkpluginspinbox.h"
#include "rkformula.h"
#include "rknote.h"
#include "rkinput.h"
#include "rkpluginbrowser.h" */

#include "../rkglobals.h"

#include "../debug.h"


RKStandardComponent::RKStandardComponent (RKComponent *parent_component, QWidget *parent_widget, const QString &filename) : RKComponent (parent_component, parent_widget) {
	RKStandardComponent::filename = filename;
	backend = 0;
	gui = 0;

	// create an error-dialog
	error_dialog = new RKErrorDialog (i18n ("The R-backend has reported one or more error(s) while processing the plugin '%1'. This may lead to an incorrect ouput and is likely due to a bug in the plugin.\nA transcript of the error message(s) is shown below.").arg (filename), i18n ("R-Error"), false);
	
	// open the main description file for parsing
	XMLHelper* xml = XMLHelper::getStaticHelper ();
	QDomElement doc_element = xml->openXMLFile (filename, DL_ERROR);
	if (xml->highestError () >= DL_ERROR) {
		// TODO: inform user
		return;
	}

	// initialize the PHP-backend with the code-template
	QDomElement element = xml->getChildElement (doc_element, "code", DL_WARNING);
	QString dummy = QFileInfo (filename).dirPath () + "/" + xml->getStringAttribute (element, "file", "code.php", DL_WARNING);
	backend = new PHPBackend ();
	connect (backend, SIGNAL (commandDone (int)), this, SLOT (backendCommandDone (int)));
	connect (backend, SIGNAL (idle ()), this, SLOT (backendIdle ()));
	connect (backend, SIGNAL (requestValue (const QString&)), this, SLOT (getValue (const QString&)));
//	connect (backend, SIGNAL (requestRCall (const QString&)), this, SLOT (doRCall (const QString&)));
//	connect (backend, SIGNAL (requestRVector (const QString&)), this, SLOT (getRVector (const QString&)));
	connect (backend, SIGNAL (haveError ()), this, SLOT (tryDestruct ()));
	if (!backend->initialize (dummy)) return;

	connect (qApp, SIGNAL (aboutToQuit ()), this, SLOT (tryDestruct ()));
	
/*	update_timer = new QTimer (this);
	connect (update_timer, SIGNAL (timeout ()), this, SLOT (doChangeUpdate ())); */

// construct the GUI
	// if top-level, construct standard elements
	if (!parent_widget) {
		parent_widget = gui = new RKStandardComponentGUI (this);
	}

	// create a builder
	RKComponentBuilder *builder = new RKComponentBuilder (this);

	// go
	// TODO: this is wrong! wizard/dialog
	builder->buildElement (parent_widget, doc_element);
}

RKStandardComponent::~RKStandardComponent () {
}

void RKStandardComponent::tryDestruct () {
	if (gui) {
		gui->hide ();
	}
	destroyed = true;

	// TODO!
}

void RKStandardComponent::switchInterfaces () {
}







/////////////////////////////////////// RKStandardComponentGUI ////////////////////////////////////////////////

RKStandardComponentGUI::RKStandardComponentGUI (RKStandardComponent *component) {
	RKStandardComponentGUI::component = component;

	QGridLayout *main_grid = new QGridLayout (this, 1, 1);
	QSplitter *splitter = new QSplitter (QSplitter::Vertical, this);
	main_grid->addWidget (splitter, 0, 0);
	QWidget *main_widget = new QWidget (splitter);
	
	QHBoxLayout *hbox = new QHBoxLayout (main_widget, RKGlobals::marginHint (), RKGlobals::spacingHint ());
	QVBoxLayout *vbox = new QVBoxLayout (hbox, RKGlobals::spacingHint ());

	// build standard elements
	// lines
	QFrame *line;
	line = new QFrame (main_widget);
	line->setFrameShape (QFrame::VLine);
	line->setFrameShadow (QFrame::Plain);	
	hbox->addWidget (line);

	// buttons
	vbox = new QVBoxLayout (hbox, RKGlobals::spacingHint ());
	okButton = new QPushButton ("Submit", main_widget);
	connect (okButton, SIGNAL (clicked ()), this, SLOT (ok ()));
	vbox->addWidget (okButton);
	
	cancelButton = new QPushButton ("Close", main_widget);
	connect (cancelButton, SIGNAL (clicked ()), this, SLOT (cancel ()));
	vbox->addWidget (cancelButton);
	vbox->addStretch (1);
	
	helpButton = new QPushButton ("Help", main_widget);
	connect (helpButton, SIGNAL (clicked ()), this, SLOT (help ()));
	vbox->addWidget (helpButton);
	
/*	if (wizard_available) {
		switchButton = new QPushButton ("Use Wizard", main_widget);
		connect (switchButton, SIGNAL (clicked ()), this, SLOT (switchInterfaces ()));
		vbox->addWidget (switchButton);
	} */
	vbox->addStretch (2);
	
	toggleCodeButton = new QPushButton ("Code", main_widget);
	toggleCodeButton->setToggleButton (true);
	toggleCodeButton->setOn (true);
	connect (toggleCodeButton, SIGNAL (clicked ()), this, SLOT (toggleCode ()));
	vbox->addWidget (toggleCodeButton);
	
	// text-fields
	QWidget *lower_widget = new QWidget (splitter);
	
	vbox = new QVBoxLayout (lower_widget, RKGlobals::spacingHint ());
	codeDisplay = new RKCommandEditor (lower_widget, true);
	vbox->addWidget (codeDisplay);
}

RKStandardComponentGUI::~RKStandardComponentGUI () {
}

void RKStandardComponentGUI::ok () {
}

void RKStandardComponentGUI::back () {
}

void RKStandardComponentGUI::cancel () {
}

void RKStandardComponentGUI::toggleCode () {
}

void RKStandardComponentGUI::help () {
}

void RKStandardComponentGUI::closeEvent (QCloseEvent *e) {
	RK_TRACE (PLUGIN);

	e->accept ();
	component->tryDestruct ();
}



#include "rkstandardcomponent.moc"
