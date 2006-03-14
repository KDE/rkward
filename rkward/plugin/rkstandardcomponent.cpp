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
#include <qvbox.h>
#include <qhbox.h>
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
#include <kmessagebox.h>

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
	RK_TRACE (PLUGIN);

	RKStandardComponent::filename = filename;
	backend = 0;
	gui = 0;
	
	// open the main description file for parsing
	XMLHelper* xml = XMLHelper::getStaticHelper ();
	QDomElement doc_element = xml->openXMLFile (filename, DL_ERROR);
	if (xml->highestError () >= DL_ERROR) {
		KMessageBox::error (this, i18n ("There has been an error while trying to parse the description of this pluign ('%1'). Please refer to stdout for details.").arg (filename), i18n("Could not create plugin"));
		return;
	}

	// create an error-dialog
	error_dialog = new RKErrorDialog (i18n ("The R-backend has reported one or more error(s) while processing the plugin '%1'. This may lead to an incorrect ouput and is likely due to a bug in the plugin.\nA transcript of the error message(s) is shown below.").arg (filename), i18n ("R-Error"), false);

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

	// initialize
	builder->makeConnections ();

	// done!
	delete builder;
}

RKStandardComponent::~RKStandardComponent () {
	RK_TRACE (PLUGIN);

	delete error_dialog;
	delete backend;
}

void RKStandardComponent::tryDestruct () {
	RK_TRACE (PLUGIN);

	if (gui) {
		gui->hide ();
	}
	destroyed = true;

	// TODO!
}

void RKStandardComponent::switchInterfaces () {
	RK_TRACE (PLUGIN);
}

/////////////////////////////////////// RKComponentBuilder /////////////////////////////////////////


RKComponentBuilder::RKComponentBuilder (RKComponent *parent_component) {
	RK_TRACE (PLUGIN);
	parent = parent_component;
}

RKComponentBuilder::~RKComponentBuilder () {
	RK_TRACE (PLUGIN);
}

void RKComponentBuilder::buildElement (QWidget *parent, const QDomElement &element) {
	RK_TRACE (PLUGIN);

	RK_TRACE (PLUGIN);

	XMLHelper* xml = XMLHelper::getStaticHelper ();
	XMLChildList children = xml->getChildElements (element, "", DL_ERROR);
	
	XMLChildList::const_iterator it;
	for (it = children.begin (); it != children.end (); ++it) {
		RKPluginWidget *widget = 0;
		QDomElement e = *it;		// shorthand
		
	    if (e.tagName () == "row") {
			QHBox box = new QHBox (parent);
			box->setSpacing (RKGlobals::spacingHint ());
			buildElement (e, box);
		} else if (e.tagName () == "column") {
			QVBox box = new QVBox (parent);
			box->setSpacing (RKGlobals::spacingHint ());
			buildElement (e, box);
		} else if (e.tagName () == "frame") {
			QGroupBox *box = new QGroupBox (1, Qt::Vertical, e.attribute ("label"), parent);
			box->setSpacing (RKGlobals::spacingHint ());
			buildElement (e, box);
		} else if (e.tagName () == "tabbook") {
			QTabWidget *tabbook = new QTabWidget (parent);
			QDomNodeList tabs = e.childNodes ();
			for (unsigned int t=0; t < tabs.count (); ++t) {
				QDomElement tab_e = tabs.item (t).toElement ();
				if (tab_e.tagName () == "tab") {
					QVBox *tabpage = new QWidget (tabbook);
					tabpage->setSpacing (RKGlobals::spacingHint ());
					buildStructure (tab_e, tabpage);
					tabbook->addTab (tabpage, tab_e.attribute ("label"));
				}
			}
		} else if (e.tagName () == "varselector") {
			widget = new RKVarSelector (e, parent, this);
		} else if (e.tagName () == "radio") {
			widget = new RKRadio (e, parent, this);
		} else if (e.tagName () == "checkbox") {
			widget = new RKCheckBox (e, parent, this);
		} else if (e.tagName () == "spinbox") {
			widget = new RKPluginSpinBox (e, parent, this);
		} else if (e.tagName () == "varslot") {
			widget = new RKVarSlot (e, paretn, this);
		} else if (e.tagName () == "formula") {
			widget = new RKFormula (e, parent, this);
/*		} else if (e.tagName () == "note") {		//TODO: remove corresonding class
			widget = new RKNote (e, parent, this); */
		} else if (e.tagName () == "browser") {
			widget = new RKPluginBrowser (e, parent, this);
		} else if (e.tagName () == "input") {
			widget = new RKInput (e, parent, this);
		} else if (e.tagName () == "text") {
			widget = new RKText (e, parent, this);
		} else {
			xml->displayError (e, "Invalid tagname '%'".arg (e.tagname ()), DL_ERROR);
		}

		if (widget) {
			registerWidget (widget, e.attribute ("id", "#noid#"), e.attribute ("depend","#free#"), num_pages);
		}
	}
}

void RKComponentBuilder::makeConnections () {
	RK_TRACE (PLUGIN);
}


/////////////////////////////////////// RKStandardComponentGUI ////////////////////////////////////////////////

RKStandardComponentGUI::RKStandardComponentGUI (RKStandardComponent *component) {
	RK_TRACE (PLUGIN);

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
	RK_TRACE (PLUGIN);
}

void RKStandardComponentGUI::ok () {
	RK_TRACE (PLUGIN);
}

void RKStandardComponentGUI::back () {
	RK_TRACE (PLUGIN);
}

void RKStandardComponentGUI::cancel () {
	RK_TRACE (PLUGIN);
}

void RKStandardComponentGUI::toggleCode () {
	RK_TRACE (PLUGIN);
}

void RKStandardComponentGUI::help () {
	RK_TRACE (PLUGIN);
}

void RKStandardComponentGUI::closeEvent (QCloseEvent *e) {
	RK_TRACE (PLUGIN);

	e->accept ();
	component->tryDestruct ();
}



#include "rkstandardcomponent.moc"
