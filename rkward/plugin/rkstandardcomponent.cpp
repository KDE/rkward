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
#include <qtabwidget.h>
#include <qsplitter.h>
//#include <qwidgetstack.h>
#include <qlabel.h>
#include <qtimer.h>
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

// component widgets
#include "rkvarselector.h"
#include "rkvarslot.h"
#include "rkformula.h"
#include "rkradio.h"
#include "rkcheckbox.h"
#include "rkpluginspinbox.h"
#include "rkinput.h"
#include "rkpluginbrowser.h"
#include "rktext.h"

#include "../rkglobals.h"

#include "../debug.h"


RKStandardComponent::RKStandardComponent (RKComponent *parent_component, QWidget *parent_widget, const QString &filename) : RKComponent (parent_component, parent_widget) {
	RK_TRACE (PLUGIN);

	RKStandardComponent::filename = filename;
	backend = 0;
	gui = 0;
	created = false;
	addChild ("code", code = new RKComponentPropertyCode (this, true));
	
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
	connect (backend, SIGNAL (idle ()), this, SLOT (backendIdle ()));
	connect (backend, SIGNAL (requestValue (const QString&)), this, SLOT (getValue (const QString&)));
//	connect (backend, SIGNAL (requestRCall (const QString&)), this, SLOT (doRCall (const QString&)));
//	connect (backend, SIGNAL (requestRVector (const QString&)), this, SLOT (getRVector (const QString&)));
	connect (backend, SIGNAL (haveError ()), this, SLOT (tryDestruct ()));
	if (!backend->initialize (dummy, code)) return;

	connect (qApp, SIGNAL (aboutToQuit ()), this, SLOT (tryDestruct ()));
	
/*	update_timer = new QTimer (this);
	connect (update_timer, SIGNAL (timeout ()), this, SLOT (doChangeUpdate ())); */

// construct the GUI
	// if top-level, construct standard elements
	if (!parent_widget) {
		gui = new RKStandardComponentGUI (this, code);
		parent_widget = gui->mainWidget ();
	}

	// create a builder
	RKComponentBuilder *builder = new RKComponentBuilder (this);

	// go
	// TODO: this is wrong! wizard/dialog
	builder->buildElement (xml->getChildElement (doc_element, "dialog", DL_ERROR), parent_widget);

	// initialize
	builder->makeConnections ();

	// done!
	delete builder;
	created = true;
	if (gui) gui->show ();
	changed ();
}

RKStandardComponent::~RKStandardComponent () {
	RK_TRACE (PLUGIN);

	delete error_dialog;
	delete backend;
}

void RKStandardComponent::switchInterfaces () {
	RK_TRACE (PLUGIN);
}

void RKStandardComponent::changed () {
	RK_TRACE (PLUGIN);

	if (!created) return;

	backend->preprocess (0);
	backend->calculate (0);
	backend->printout (0);
	backend->cleanup (0);

	if (gui) {
		gui->updateCode ();
		gui->enableSubmit (isSatisfied ());
	}

	RKComponent::changed ();
}

bool RKStandardComponent::isReady () {
	RK_TRACE (PLUGIN);
	RK_ASSERT (backend);

	return (!(backend->isBusy ()));
};

void RKStandardComponent::backendIdle () {
	RK_TRACE (PLUGIN);

	if (gui) {
		gui->updateCode ();
		gui->enableSubmit (isSatisfied ());
	}
}

void RKStandardComponent::getValue (const QString &id) {
	RK_TRACE (PLUGIN);
	RK_ASSERT (backend);

	backend->writeData (fetchStringValue (id));
}

/////////////////////////////////////// RKComponentBuilder /////////////////////////////////////////


RKComponentBuilder::RKComponentBuilder (RKComponent *parent_component) {
	RK_TRACE (PLUGIN);
	parent = parent_component;
}

RKComponentBuilder::~RKComponentBuilder () {
	RK_TRACE (PLUGIN);
}

void RKComponentBuilder::buildElement (const QDomElement &element, QWidget *parent_widget) {
	RK_TRACE (PLUGIN);

	XMLHelper* xml = XMLHelper::getStaticHelper ();
	XMLChildList children = xml->getChildElements (element, "", DL_ERROR);
	
	XMLChildList::const_iterator it;
	for (it = children.begin (); it != children.end (); ++it) {
		RKComponent *widget = 0;
		QDomElement e = *it;		// shorthand
		QString id = xml->getStringAttribute (e, "id", "#noid#", DL_INFO);

	    if (e.tagName () == "row") {
			QHBox *box = new QHBox (parent_widget);
			box->setSpacing (RKGlobals::spacingHint ());
			buildElement (e, box);
		} else if (e.tagName () == "column") {
			QVBox *box = new QVBox (parent_widget);
			box->setSpacing (RKGlobals::spacingHint ());
			buildElement (e, box);
		} else if (e.tagName () == "frame") {
			QGroupBox *box = new QGroupBox (1, Qt::Horizontal, e.attribute ("label"), parent_widget);
			box->setInsideSpacing (RKGlobals::spacingHint ());
			buildElement (e, box);
		} else if (e.tagName () == "tabbook") {
			QTabWidget *tabbook = new QTabWidget (parent_widget);
			QDomNodeList tabs = e.childNodes ();
			for (unsigned int t=0; t < tabs.count (); ++t) {
				QDomElement tab_e = tabs.item (t).toElement ();
				if (tab_e.tagName () == "tab") {
					QVBox *tabpage = new QVBox (tabbook);
					tabpage->setSpacing (RKGlobals::spacingHint ());
					buildElement (tab_e, tabpage);
					tabbook->addTab (tabpage, tab_e.attribute ("label"));
				}
			}
		} else if (e.tagName () == "varselector") {
			widget = new RKVarSelector (e, component (), parent_widget);
		} else if (e.tagName () == "varslot") {
			widget = new RKVarSlot (e, component (), parent_widget);
			addConnection (id, "source", xml->getStringAttribute (e, "source", "#noid#", DL_INFO), "selected", false, e);
		} else if (e.tagName () == "formula") {
			widget = new RKFormula (e, component (), parent_widget);
			addConnection (id, "dependent", xml->getStringAttribute (e, "dependent", "#noid#", DL_INFO), "available", false, e);
			addConnection (id, "fixed_factors", xml->getStringAttribute (e, "fixed_factors", "#noid#", DL_INFO), "available", false, e);
		} else if (e.tagName () == "radio") {
			widget = new RKRadio (e, component (), parent_widget);
		} else if (e.tagName () == "checkbox") {
			widget = new RKCheckBox (e, component (), parent_widget);
		} else if (e.tagName () == "spinbox") {
			widget = new RKPluginSpinBox (e, component (), parent_widget);
		} else if (e.tagName () == "input") {
			widget = new RKInput (e, component (), parent_widget);
		} else if (e.tagName () == "browser") {
			widget = new RKPluginBrowser (e, component (), parent_widget);
		} else if (e.tagName () == "text") {
			widget = new RKText (e, component (), parent_widget);
		} else {
			xml->displayError (&e, QString ("Invalid tagname '%1'").arg (e.tagName ()), DL_ERROR);
		}

		if (widget) {
			parent->addChild (id, widget);
			// TODO: deal with (multi-page) wizards
			// TODO: parse connections
		}
	}
}

void RKComponentBuilder::addConnection (const QString &client_id, const QString &client_property, const QString &governor_id, const QString &governor_property, bool reconcile, const QDomElement &origin) {
	RK_TRACE (PLUGIN);

	RKComponentPropertyConnection conn;
	conn.client_property = client_id + "::" + client_property;
	conn.governor_property = governor_id + "::" + governor_property;
	conn.reconcile = reconcile;
	conn.origin = origin;
	connection_list.append (conn);
}

void RKComponentBuilder::makeConnections () {
	RK_TRACE (PLUGIN);
	XMLHelper *xml = XMLHelper::getStaticHelper ();

	for (ConnectionList::const_iterator it = connection_list.begin (); it != connection_list.end (); ++it) {
		RK_DO (qDebug ("Connecting '%s' to '%s'", (*it).client_property.latin1 (), (*it).governor_property.latin1 ()), PLUGIN, DL_DEBUG);

		QString dummy;
		RKComponentBase *client = parent->lookupComponent ((*it).client_property, &dummy);
		if ((!client) || (!dummy.isEmpty ()) || (!client->isProperty ())) {
			xml->displayError (&((*it).origin), QString ("Invalid client identifier '%1'").arg ((*it).client_property), DL_ERROR);
			continue;
		}
		RKComponentBase *governor = parent->lookupComponent ((*it).governor_property, &dummy);
		if ((!governor) || (!governor->isProperty ())) {
			xml->displayError (&((*it).origin), QString ("Invalid governor identifier '%1'").arg ((*it).governor_property), DL_ERROR);
			continue;
		}

		static_cast<RKComponentPropertyBase *> (client)->connectToGovernor (static_cast<RKComponentPropertyBase *> (governor), dummy, (*it).reconcile);
	}
}




/////////////////////////////////////// RKStandardComponentGUI ////////////////////////////////////////////////

RKStandardComponentGUI::RKStandardComponentGUI (RKStandardComponent *component, RKComponentPropertyCode *code_property) {
	RK_TRACE (PLUGIN);

	RKStandardComponentGUI::component = component;
	RKStandardComponentGUI::code_property = code_property;
	connect (code_property, SIGNAL (valueChanged (RKComponentPropertyBase *)), this, SLOT (codeChanged (RKComponentPropertyBase *)));

	QGridLayout *main_grid = new QGridLayout (this, 1, 1);
	QSplitter *splitter = new QSplitter (QSplitter::Vertical, this);
	main_grid->addWidget (splitter, 0, 0);
	QWidget *upper_widget = new QWidget (splitter);
	
	QHBoxLayout *hbox = new QHBoxLayout (upper_widget, RKGlobals::marginHint (), RKGlobals::spacingHint ());
	QVBoxLayout *vbox = new QVBoxLayout (hbox, RKGlobals::spacingHint ());

	// build standard elements
	main_widget = new QVBox (upper_widget);
	hbox->addWidget (main_widget);

	// lines
	QFrame *line;
	line = new QFrame (upper_widget);
	line->setFrameShape (QFrame::VLine);
	line->setFrameShadow (QFrame::Plain);	
	hbox->addWidget (line);

	// buttons
	vbox = new QVBoxLayout (hbox, RKGlobals::spacingHint ());
	okButton = new QPushButton ("Submit", upper_widget);
	connect (okButton, SIGNAL (clicked ()), this, SLOT (ok ()));
	vbox->addWidget (okButton);
	
	cancelButton = new QPushButton ("Close", upper_widget);
	connect (cancelButton, SIGNAL (clicked ()), this, SLOT (cancel ()));
	vbox->addWidget (cancelButton);
	vbox->addStretch (1);
	
	helpButton = new QPushButton ("Help", upper_widget);
	connect (helpButton, SIGNAL (clicked ()), this, SLOT (help ()));
	vbox->addWidget (helpButton);
	
/*	if (wizard_available) {
		switchButton = new QPushButton ("Use Wizard", upper_widget);
		connect (switchButton, SIGNAL (clicked ()), this, SLOT (switchInterfaces ()));
		vbox->addWidget (switchButton);
	} */
	vbox->addStretch (2);
	
	toggleCodeButton = new QPushButton ("Code", upper_widget);
	toggleCodeButton->setToggleButton (true);
	toggleCodeButton->setOn (true);
	connect (toggleCodeButton, SIGNAL (clicked ()), this, SLOT (toggleCode ()));
	vbox->addWidget (toggleCodeButton);
	
	// text-fields
	QWidget *lower_widget = new QWidget (splitter);
	
	vbox = new QVBoxLayout (lower_widget, RKGlobals::spacingHint ());
	codeDisplay = new RKCommandEditor (lower_widget, true);
	vbox->addWidget (codeDisplay);

	// code update timer
	code_update_timer = new QTimer (this);
	connect (code_update_timer, SIGNAL (timeout ()), this, SLOT (updateCodeNow ()));
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
	hide ();
	component->deleteLater ();
}

void RKStandardComponentGUI::enableSubmit (bool enable) {
	RK_TRACE (PLUGIN);

	okButton->setEnabled (enable);
}

void RKStandardComponentGUI::codeChanged (RKComponentPropertyBase *) {
	RK_TRACE (PLUGIN);

	updateCode ();
}

void RKStandardComponentGUI::updateCode () {
	RK_TRACE (PLUGIN);

	code_update_timer->start (0, true);
}

void RKStandardComponentGUI::updateCodeNow () {
	RK_TRACE (PLUGIN);

	if (!code_property->isValid ()) {
		codeDisplay->setText (i18n ("Processing. Please wait"));
		RK_DO (qDebug ("code not ready to be displayed: pre %d, cal %d, pri %d, cle %d", !code_property->preprocess ().isNull (), !code_property->calculate ().isNull (), !code_property->printout ().isNull (), !code_property->cleanup ().isNull ()), PLUGIN, DL_DEBUG);
	} else {
		codeDisplay->setText (code_property->preprocess () + code_property->calculate () + code_property->printout () + code_property->cleanup ());
	}
}


#include "rkstandardcomponent.moc"
