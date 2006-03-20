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

#include <qfileinfo.h>
#include <qlayout.h>
#include <qvbox.h>
#include <qhbox.h>
#include <qgroupbox.h>
#include <qframe.h>
#include <qtabwidget.h>
#include <qlabel.h>
#include <qapplication.h>

#include <klocale.h>
#include <kmessagebox.h>

#include "rkstandardcomponentgui.h"
#include "../scriptbackends/phpbackend.h"
#include "../misc/rkerrordialog.h"
#include "../misc/xmlhelper.h"
#include "../settings/rksettingsmoduleplugins.h"

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
	wizard = 0;
	created = false;
	addChild ("code", code = new RKComponentPropertyCode (this, true));
	
	// open the main description file for parsing
	XMLHelper* xml = XMLHelper::getStaticHelper ();
	QDomElement doc_element = xml->openXMLFile (filename, DL_ERROR);
	if (xml->highestError () >= DL_ERROR) {
		KMessageBox::error (this, i18n ("There has been an error while trying to parse the description of this pluign ('%1'). Please refer to stdout for details.").arg (filename), i18n("Could not create plugin"));
		deleteLater ();
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
	connect (backend, SIGNAL (haveError ()), this, SLOT (deleteLater ()));
	if (!backend->initialize (dummy, code)) return;

	connect (qApp, SIGNAL (aboutToQuit ()), this, SLOT (deleteLater ()));

// construct the GUI
	if (!parent_component) {					// top-level
		if (!createTopLevel (doc_element)) {
			RK_ASSERT (false);
			deleteLater ();
			return;		// should never happen
		}
	} else {
		bool build_wizard = false;
		QDomElement gui_element;
		if (parent_component->isWizardish ()) {
			build_wizard = true;
			gui_element = xml->getChildElement (doc_element, "wizard", DL_WARNING);
			if (gui_element.isNull ()) {
				gui_element = xml->getChildElement (doc_element, "dialog", DL_WARNING);
				build_wizard = false;
			}
		} else {
			QDomElement gui_element = xml->getChildElement (doc_element, "dialog", DL_WARNING);
			if (gui_element.isNull ()) {
				xml->displayError (&doc_element, "Cannot embed a wizard into a dialog, and no dialog definition available", DL_ERROR);
				deleteLater ();
				return;
			}
		}
		buildAndInitialize (doc_element, gui_element, parent_widget, build_wizard);
	}
}

RKStandardComponent::~RKStandardComponent () {
	RK_TRACE (PLUGIN);

	delete error_dialog;
	delete backend;
}

bool RKStandardComponent::createTopLevel (const QDomElement &doc_element, int force_mode) {
	RK_TRACE (PLUGIN);

	XMLHelper* xml = XMLHelper::getStaticHelper ();
	bool build_wizard = false;
	QDomElement dialog_element;
	QDomElement wizard_element;

	dialog_element = xml->getChildElement (doc_element, "dialog", DL_INFO);
	wizard_element = xml->getChildElement (doc_element, "wizard", DL_INFO);
	if (!wizard_element.isNull ()) {
		build_wizard = xml->getBoolAttribute (wizard_element, "recommended", false, DL_INFO);
	}

	if (force_mode == 0) {
		if (RKSettingsModulePlugins::getInterfacePreference () == RKSettingsModulePlugins::PreferDialog) {
			if (!dialog_element.isNull ()) build_wizard = false;
		} else if (RKSettingsModulePlugins::getInterfacePreference () == RKSettingsModulePlugins::PreferWizard) {
			if (!wizard_element.isNull ()) build_wizard = true;
		}
	} else if (force_mode == 1) {
		build_wizard = false;
		if (dialog_element.isNull ()) {
			xml->displayError (&doc_element, "Dialog mode forced, but no dialog element given", DL_ERROR);
			deleteLater ();
			return false;
		}
	} else if (force_mode == 2) {
		build_wizard = true;
		if (wizard_element.isNull ()) {
			xml->displayError (&doc_element, "Wizard mode forced, but no wizard element given", DL_ERROR);
			deleteLater ();
			return false;
		}
	}

	if (build_wizard) {
		gui = new RKStandardComponentWizard (this, code);
		static_cast<RKStandardComponentWizard *> (gui)->createWizard (!dialog_element.isNull ());
		wizard = static_cast<RKStandardComponentWizard *> (gui)->getStack ();
		buildAndInitialize (doc_element, wizard_element, gui->mainWidget (), true);
		static_cast<RKStandardComponentWizard *> (gui)->addLastPage ();
	} else {
		gui = new RKStandardComponentGUI (this, code);
		gui->createDialog (!wizard_element.isNull ());
		buildAndInitialize (doc_element, dialog_element, gui->mainWidget (), false);
	}

	return true;
}

void RKStandardComponent::switchInterface () {
	RK_TRACE (PLUGIN);

	RK_ASSERT (gui);		// this should only ever happen on top level

	// open the main description file for parsing (again)
	XMLHelper* xml = XMLHelper::getStaticHelper ();
	QDomElement doc_element = xml->openXMLFile (filename, DL_ERROR);
	int force_mode = 2;
	if (isWizardish ()) force_mode = 1;

	discard ();

	createTopLevel (doc_element, force_mode);
}

void RKStandardComponent::discard () {
	RK_TRACE (PLUGIN);

	created = false;
	gui->hide ();
	gui->deleteLater ();
	gui = 0;
	wizard = 0;

	for (QDictIterator<RKComponentBase> it (child_map); it.current (); ++it) {
		if (it.current () != code) {
			if (it.current ()->isProperty ()) {
				static_cast<RKComponentPropertyBase *> (it.current ())->deleteLater ();
			} else {
				static_cast<RKComponent *> (it.current ())->deleteLater ();
			}
		}
	}
	child_map.clear ();
	createDefaultProperties ();
}

void RKStandardComponent::buildAndInitialize (const QDomElement &doc_element, const QDomElement &gui_element, QWidget *parent_widget, bool build_wizard) {
	RK_TRACE (PLUGIN);

	XMLHelper* xml = XMLHelper::getStaticHelper ();

	// create a builder
	RKComponentBuilder *builder = new RKComponentBuilder (this);

	// go
	builder->buildElement (gui_element, parent_widget, build_wizard);
	builder->parseLogic (xml->getChildElement (doc_element, "logic", DL_INFO));

	// initialize
	builder->makeConnections ();

	// done!
	delete builder;
	created = true;
	if (gui) gui->show ();
	changed ();
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

bool RKStandardComponent::isWizardish () {
	RK_TRACE (PLUGIN);

	return (wizard != 0);
}

bool RKStandardComponent::havePage (bool next) {
	RK_TRACE (PLUGIN);
	RK_ASSERT (wizard);

	return (wizard->havePage (next));
}

void RKStandardComponent::movePage (bool next) {
	RK_TRACE (PLUGIN);
	RK_ASSERT (wizard);

	wizard->movePage (next);
}

bool RKStandardComponent::currentPageSatisfied () {
	RK_TRACE (PLUGIN);
	RK_ASSERT (wizard);

	return (wizard->currentPageSatisfied ());
}

RKComponent *RKStandardComponent::addPage () {
	RK_TRACE (PLUGIN);
	RK_ASSERT (wizard);

	RKComponent *page = wizard->addPage (this);
	return page;
}

void RKStandardComponent::addChild (const QString &id, RKComponentBase *child) {
	RK_TRACE (PLUGIN);

	if (wizard) {
		if (!child->isProperty ()) {
			wizard->addComponentToCurrentPage (static_cast<RKComponent *>(child));
		}
	}

	RKComponent::addChild (id, child);
}




/////////////////////////////////////// RKComponentBuilder /////////////////////////////////////////


RKComponentBuilder::RKComponentBuilder (RKStandardComponent *parent_component) {
	RK_TRACE (PLUGIN);
	parent = parent_component;
}

RKComponentBuilder::~RKComponentBuilder () {
	RK_TRACE (PLUGIN);
}

void RKComponentBuilder::buildElement (const QDomElement &element, QWidget *parent_widget, bool allow_pages) {
	RK_TRACE (PLUGIN);

	XMLHelper* xml = XMLHelper::getStaticHelper ();
	XMLChildList children = xml->getChildElements (element, QString::null, DL_ERROR);
	
	XMLChildList::const_iterator it;
	for (it = children.begin (); it != children.end (); ++it) {
		RKComponent *widget = 0;
		QDomElement e = *it;		// shorthand
		QString id = xml->getStringAttribute (e, "id", "#noid#", DL_INFO);

		if (allow_pages && (e.tagName () == "page")) {
			widget = component ()->addPage ();
			QVBoxLayout *layout = new QVBoxLayout (widget);
			QVBox *box = new QVBox (widget);
			box->setSpacing (RKGlobals::spacingHint ());
			layout->addWidget (box);
			buildElement (e, box, false);
		} else if (e.tagName () == "row") {
			RKComponent *widget = new RKComponent (component (), parent_widget);		// wrapping this (and column, frame below) inside an RKComponent has the benefit, that it can have an id, and hence can be set to visibile/hidden, enabled/disabled
			QVBoxLayout *layout = new QVBoxLayout (widget);
			QHBox *box = new QHBox (widget);
			box->setSpacing (RKGlobals::spacingHint ());
			layout->addWidget (box);
			buildElement (e, box, false);
		} else if (e.tagName () == "column") {
			RKComponent *widget = new RKComponent (component (), parent_widget);
			QVBoxLayout *layout = new QVBoxLayout (widget);
			QVBox *box = new QVBox (widget);
			box->setSpacing (RKGlobals::spacingHint ());
			layout->addWidget (box);
			buildElement (e, box, false);
		} else if (e.tagName () == "frame") {
			RKComponent *widget = new RKComponent (component (), parent_widget);
			QVBoxLayout *layout = new QVBoxLayout (widget);
			QGroupBox *box = new QGroupBox (1, Qt::Horizontal, e.attribute ("label"), widget);
			box->setInsideSpacing (RKGlobals::spacingHint ());
			layout->addWidget (box);
			buildElement (e, box, false);
		} else if (e.tagName () == "tabbook") {
			QTabWidget *tabbook = new QTabWidget (parent_widget);
			QDomNodeList tabs = e.childNodes ();
			for (unsigned int t=0; t < tabs.count (); ++t) {
				QDomElement tab_e = tabs.item (t).toElement ();
				if (tab_e.tagName () == "tab") {
					QVBox *tabpage = new QVBox (tabbook);
					tabpage->setSpacing (RKGlobals::spacingHint ());
					buildElement (tab_e, tabpage, false);
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
		}
	}
}

void RKComponentBuilder::parseLogic (const QDomElement &element) {
	RK_TRACE (PLUGIN);

	// TODO
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


#include "rkstandardcomponent.moc"
