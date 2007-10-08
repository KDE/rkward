/***************************************************************************
                          rkstandardcomponent  -  description
                             -------------------
    begin                : Sun Feb 19 2006
    copyright            : (C) 2006, 2007 by Thomas Friedrichsmeier
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
#include <q3vbox.h>
#include <q3hbox.h>
#include <q3groupbox.h>
#include <q3frame.h>
#include <qtabwidget.h>
#include <qlabel.h>
#include <qapplication.h>
#include <qtimer.h>
//Added by qt3to4:
#include <Q3VBoxLayout>

#include <klocale.h>
#include <kmessagebox.h>

#include "rkstandardcomponentgui.h"
#include "../scriptbackends/phpbackend.h"
#include "../scriptbackends/simplebackend.h"
#include "../misc/xmlhelper.h"
#include "../settings/rksettingsmoduleplugins.h"

// component widgets
#include "rkvarselector.h"
#include "rkvarslot.h"
#include "rkformula.h"
#include "rkradio.h"
#include "rkdropdown.h"
#include "rkcheckbox.h"
#include "rkpluginspinbox.h"
#include "rkinput.h"
#include "rkpluginbrowser.h"
#include "rkpluginsaveobject.h"
#include "rkpreviewbox.h"
#include "rktext.h"
#include "rktabpage.h"

#include "../rkglobals.h"

#include "../debug.h"


RKStandardComponent::RKStandardComponent (RKComponent *parent_component, QWidget *parent_widget, const QString &filename, RKComponentHandle *handle) : RKComponent (parent_component, parent_widget) {
	RK_TRACE (PLUGIN);

	RKStandardComponent::filename = filename;
	RKStandardComponent::handle = handle;
	backend = 0;
	gui = 0;
	wizard = 0;
	created = false;
	addChild ("code", code = new RKComponentPropertyCode (this, true));		// do not change this name!
	
	// open the main description file for parsing
	XMLHelper* xml = XMLHelper::getStaticHelper ();
	QDomElement doc_element = xml->openXMLFile (filename, DL_ERROR);
	if (xml->highestError () >= DL_ERROR) {
		KMessageBox::error (this, i18n ("There has been an error while trying to parse the description of this plugin ('%1'). Please refer to stdout for details.").arg (filename), i18n ("Could not create plugin"));
		removeFromParent ();
		deleteLater ();
		return;
	}

	// initialize the PHP-backend with the code-template
	QDomElement element = xml->getChildElement (doc_element, "code", DL_WARNING);
	if (element.hasAttribute ("file")) {
		QString dummy = QFileInfo (filename).dirPath () + '/' + xml->getStringAttribute (element, "file", "code.php", DL_WARNING);
		backend = new PHPBackend (dummy);
	} else {
		SimpleBackend *back = new SimpleBackend ();
		back->setPreprocessTemplate (xml->getStringAttribute (element, "preprocess", QString::null, DL_INFO));
		back->setPrintoutTemplate (xml->getStringAttribute (element, "printout", QString::null, DL_INFO));
		back->setCalculateTemplate (xml->getStringAttribute (element, "calculate", QString::null, DL_INFO));
		back->setPreviewTemplate (xml->getStringAttribute (element, "preview", QString::null, DL_INFO));
		backend = back;
	}
	connect (backend, SIGNAL (idle ()), this, SLOT (backendIdle ()));
	connect (backend, SIGNAL (requestValue (const QString&)), this, SLOT (getValue (const QString&)));
	connect (backend, SIGNAL (haveError ()), this, SLOT (hide ()));
	connect (backend, SIGNAL (haveError ()), this, SLOT (removeFromParent ()));
	connect (backend, SIGNAL (haveError ()), this, SLOT (deleteLater ()));
	if (!backend->initialize (code, parent_component == 0)) return;

	// check for existance of help file
	element = xml->getChildElement (doc_element, "help", DL_WARNING);
	QString dummy = QFileInfo (filename).dirPath () + '/' + xml->getStringAttribute (element, "file", "::nosuchfile::", DL_INFO);
	have_help = QFileInfo (dummy).exists ();

	handle_change_timer = new QTimer (this);
	connect (handle_change_timer, SIGNAL (timeout ()), this, SLOT (handleChange ()));

// construct the GUI
	if (!parent_component) {					// top-level
		if (!createTopLevel (doc_element)) {
			RK_ASSERT (false);
			deleteLater ();
			return;		// should never happen
		}
	} else if (!parent_widget) {				// we have a parent component, but should still have a separate GUI
		int force_mode = 1;
		if (parentComponent ()->isWizardish ()) force_mode = 2;
		if (!createTopLevel (doc_element, force_mode, true)) {
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

				QWidget *fake_page = parent_component->addPage ();
				Q3VBoxLayout *layout = new Q3VBoxLayout (fake_page);	
				Q3VBox *box = new Q3VBox (fake_page);
				box->setSpacing (RKGlobals::spacingHint ());
				layout->addWidget (box);
				parent_widget = box;
			}
		} else {
			gui_element = xml->getChildElement (doc_element, "dialog", DL_WARNING);
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

	if (gui) gui->deleteLater ();
	delete backend;
}

void RKStandardComponent::hide () {
	RK_TRACE (PLUGIN);

	if (gui) gui->hide ();
	RKComponent::hide ();
}

void RKStandardComponent::showGUI () {
	RK_TRACE (PLUGIN);

	if (!gui) {
		RK_ASSERT (false);
		return;
	}
	gui->show ();
	gui->raise ();
}

void RKStandardComponent::setCaption (const QString &caption) {
	RK_TRACE (PLUGIN);

	if (!gui) return;
	gui->setCaption (caption);
}

bool RKStandardComponent::createTopLevel (const QDomElement &doc_element, int force_mode, bool enslaved) {
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
			build_wizard = true;
			xml->displayError (&doc_element, "Dialog mode forced, but no dialog element given", DL_INFO);
		}
	} else if (force_mode == 2) {
		build_wizard = true;
		if (wizard_element.isNull ()) {
			build_wizard = false;
			xml->displayError (&doc_element, "Wizard mode forced, but no wizard element given", DL_INFO);
		}
	}

	if (build_wizard) {
		gui = new RKStandardComponentWizard (this, code, enslaved);
		static_cast<RKStandardComponentWizard *> (gui)->createWizard (!dialog_element.isNull ());
		wizard = static_cast<RKStandardComponentWizard *> (gui)->getStack ();
		buildAndInitialize (doc_element, wizard_element, gui->mainWidget (), true, enslaved);
		static_cast<RKStandardComponentWizard *> (gui)->addLastPage ();
	} else {
		gui = new RKStandardComponentGUI (this, code, enslaved);
		gui->createDialog (!wizard_element.isNull ());
		buildAndInitialize (doc_element, dialog_element, gui->mainWidget (), false, enslaved);
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

	QMap<QString, QString> value_save;		// fetch current GUI settings
	fetchPropertyValuesRecursive (&value_save);

	discard ();

	createTopLevel (doc_element, force_mode);

	// set old GUI settings. For this purpose, we'll temporarily disable updates in the phpbackend
	created = false;
	setPropertyValues (&value_save);
	created = true;
	changed ();		// now we can update
}

void RKStandardComponent::discard () {
	RK_TRACE (PLUGIN);

	created = false;
	gui->hide ();
	gui->deleteLater ();
	gui = 0;
	wizard = 0;

	// clear all properties. Not the code property, as the script backend relies on it
	for (Q3DictIterator<RKComponentBase> it (child_map); it.current (); ++it) {
		if (it.current () != code) {
			if (it.current ()->isProperty ()) {
				static_cast<RKComponentPropertyBase *> (it.current ())->deleteLater ();
			} else {
				static_cast<RKComponent *> (it.current ())->deleteLater ();
			}
		}
	}
	child_map.clear ();
	addChild ("code", code);

	createDefaultProperties ();	// enabledness, requiredness, visibility
}

void RKStandardComponent::buildAndInitialize (const QDomElement &doc_element, const QDomElement &gui_element, QWidget *parent_widget, bool build_wizard, bool enslaved) {
	RK_TRACE (PLUGIN);

	XMLHelper* xml = XMLHelper::getStaticHelper ();

	// create a builder
	RKComponentBuilder *builder = new RKComponentBuilder (this, doc_element);

	// go
	builder->buildElement (gui_element, parent_widget, build_wizard);
	builder->parseLogic (xml->getChildElement (doc_element, "logic", DL_INFO));
	setCaption (xml->getStringAttribute (gui_element, "label", QString::null, DL_WARNING));

	// initialize
	builder->makeConnections ();

	// done!
	delete builder;
	created = true;
	if (gui && (!enslaved)) {
		// somehow, when switching the interface, and we show before the old GUI has been fully deleted (it is deleted via deleteLater (), then there may be strange graphical glitches until the GUI is first redrawn completely.
		// Likely a difficult bug in Qt. Delaying the show until the next event loop solves the problem.
		QTimer::singleShot (0, gui, SLOT (show ()));
	}
	changed ();
}


void RKStandardComponent::changed () {
	RK_TRACE (PLUGIN);

	if (!created) return;
	if (gui) gui->enableSubmit (false);

	// delay actual handling, until all changes have run up
	handle_change_timer->start (10, true);
}

void RKStandardComponent::handleChange () {
	RK_TRACE (PLUGIN);

	backend->preprocess (0);
	backend->calculate (0);
	backend->printout (0);
	backend->preview (0);

	if (gui) {
		gui->updateCode ();	// will read "processing, please wait", or similar
	}

	RKComponent::changed ();
}

bool RKStandardComponent::isReady () {
	RK_TRACE (PLUGIN);
	RK_ASSERT (backend);

	return (!(backend->isBusy ()));
}

void RKStandardComponent::backendIdle () {
	RK_TRACE (PLUGIN);

	if (gui) {
		gui->updateCode ();
		gui->enableSubmit (isSatisfied ());
	}

	RKComponent::changed ();		// notify parent, if any
}

void RKStandardComponent::getValue (const QString &id) {
	RK_TRACE (PLUGIN);
	RK_ASSERT (backend);

	backend->writeData (fetchStringValue (id));
}

bool RKStandardComponent::isWizardish () {
	RK_TRACE (PLUGIN);

	if (gui) return (wizard != 0);
	if (parentComponent ()) return (parentComponent ()->isWizardish ());
	return false;
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

	if (wizard) {
		RKComponent *page = wizard->addPage (this);
		return page;
	} else if (parentComponent ()) {
		RK_ASSERT (!gui);
		return parentComponent ()->addPage ();
	}

	RK_ASSERT (false);
	return RKComponent::addPage ();
}

void RKStandardComponent::addChild (const QString &id, RKComponentBase *child) {
	RK_TRACE (PLUGIN);

	if (wizard || (parentComponent () && parentComponent ()->isWizardish ())) {
		if (!child->isProperty ()) {
			addComponentToCurrentPage (static_cast<RKComponent *>(child));
		}
	}

	RKComponent::addChild (id, child);
}

void RKStandardComponent::addComponentToCurrentPage (RKComponent *component) {
	RK_TRACE (PLUGIN);

	if (wizard) {
		wizard->addComponentToCurrentPage (component);
	} else if (parentComponent () && parentComponent ()->isWizardish ()) {
		parentComponent ()->addComponentToCurrentPage (component);
	} else {
		RK_ASSERT (false);
	}
}


/////////////////////////////////////// RKComponentBuilder /////////////////////////////////////////

#include "rkcomponentmap.h"

#include <qpushbutton.h>

RKComponentBuilder::RKComponentBuilder (RKStandardComponent *parent_component, const QDomElement &document_element) {
	RK_TRACE (PLUGIN);
	parent = parent_component;
	doc_elem = document_element;
}

RKComponentBuilder::~RKComponentBuilder () {
	RK_TRACE (PLUGIN);
}

QDomElement RKComponentBuilder::doElementCopy (const QString id, const QDomElement &copy) {
	RK_TRACE (PLUGIN);

	XMLHelper* xml = XMLHelper::getStaticHelper ();
	QDomElement res;

	if (id.isEmpty ()) {
		xml->displayError (&copy, "no id given for copy element", DL_ERROR, DL_ERROR);
		return res;	// null
	}

	// find matching element to copy from
	XMLChildList candidates = xml->findElementsWithAttribute (doc_elem, "id", id, true, DL_ERROR);
	XMLChildList::const_iterator it;
	for (it = candidates.constBegin (); it != candidates.constEnd (); ++it) {
		if ((*it).tagName () == "copy") continue;
		res = (*it).cloneNode ().toElement ();
		break;
	}
	if (res.isNull ()) {
		xml->displayError (&copy, "no matching element found to copy from", DL_ERROR, DL_ERROR);
		return res;
	}

	// copy overridden attributes
	QDomNamedNodeMap attribs = copy.attributes ();
	int len = attribs.count ();
	for (int i=0; i < len; ++i) {
		QDomAttr attr = attribs.item (i).toAttr ();
		if (attr.name () == "copy_element_tag_name") res.setTagName (attr.value ());
		else res.setAttribute (attr.name (), attr.value ());
	}

	return res;
}

void RKComponentBuilder::buildElement (const QDomElement &element, QWidget *parent_widget, bool allow_pages) {
	RK_TRACE (PLUGIN);

	XMLHelper* xml = XMLHelper::getStaticHelper ();
	XMLChildList children = xml->getChildElements (element, QString::null, DL_ERROR);
	
	XMLChildList::const_iterator it;
	for (it = children.constBegin (); it != children.constEnd (); ++it) {
		RKComponent *widget = 0;
		QDomElement e = *it;		// shorthand
		QString id = xml->getStringAttribute (e, "id", QString::null, DL_INFO);

		if (e.tagName () == "copy") {
			e = doElementCopy (id, e);
		}	// no else, here. e may be changed to some entirely different element, now.

		if (allow_pages && (e.tagName () == "page")) {
			widget = component ()->addPage ();
			Q3VBoxLayout *layout = new Q3VBoxLayout (widget);
			Q3VBox *box = new Q3VBox (widget);
			box->setSpacing (RKGlobals::spacingHint ());
			layout->addWidget (box);
			buildElement (e, box, false);
		} else if (e.tagName () == "row") {
			widget = new RKComponent (component (), parent_widget);		// wrapping this (and column, frame below) inside an RKComponent has the benefit, that it can have an id, and hence can be set to visibile/hidden, enabled/disabled
			Q3VBoxLayout *layout = new Q3VBoxLayout (widget);
			Q3HBox *box = new Q3HBox (widget);
			box->setSpacing (RKGlobals::spacingHint ());
			layout->addWidget (box);
			buildElement (e, box, false);
		} else if (e.tagName () == "stretch") {
			QWidget *stretch = new QWidget (parent_widget);
			stretch->setSizePolicy (QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
			Q3HBox *box = dynamic_cast<Q3HBox *> (parent_widget);
			// RK_ASSERT (box);  <- NO, also meaningful in a <frame>
			if (box) box->setStretchFactor (stretch, 100);
		} else if (e.tagName () == "column") {
			widget = new RKComponent (component (), parent_widget);
			Q3VBoxLayout *layout = new Q3VBoxLayout (widget);
			Q3VBox *box = new Q3VBox (widget);
			box->setSpacing (RKGlobals::spacingHint ());
			layout->addWidget (box);
			buildElement (e, box, false);
		} else if (e.tagName () == "frame") {
			widget = new RKComponent (component (), parent_widget);
			Q3VBoxLayout *layout = new Q3VBoxLayout (widget);
			Q3GroupBox *box = new Q3GroupBox (1, Qt::Horizontal, e.attribute ("label"), widget);
			box->setInsideSpacing (RKGlobals::spacingHint ());
			layout->addWidget (box);
			buildElement (e, box, false);
		} else if (e.tagName () == "tabbook") {
			QTabWidget *tabbook = new QTabWidget (parent_widget);
			QDomNodeList tabs = e.childNodes ();
			for (unsigned int t=0; t < tabs.count (); ++t) {
				QDomElement tab_e = tabs.item (t).toElement ();
				if (tab_e.tagName () == "tab") {
					RKTabPage *tabpage = new RKTabPage (tab_e, component (), tabbook);
					buildElement (tab_e, tabpage->getPage (), false);
					QString tab_id = xml->getStringAttribute (tab_e, "id", QString::null, DL_INFO);
					if (!tab_id.isNull ()) {
						parent->addChild (tab_id, tabpage);
					}
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
		} else if (e.tagName () == "dropdown") {
			widget = new RKDropDown (e, component (), parent_widget);
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
		} else if (e.tagName () == "preview") {
			widget = new RKPreviewBox (e, component (), parent_widget);
		} else if (e.tagName () == "saveobject") {
			widget = new RKPluginSaveObject (e, component (), parent_widget);
		} else if (e.tagName () == "embed") {
			QString component_id = xml->getStringAttribute (e, "component", QString::null, DL_ERROR);
			RKComponentHandle *handle = RKComponentMap::getComponentHandle (component_id);
			if (handle) {
				if (xml->getBoolAttribute (e, "as_button", false, DL_INFO)) {
					widget = handle->invoke (component (), 0);
					QString dummy = xml->getStringAttribute (e, "label", "Options", DL_WARNING);
					widget->setCaption (dummy);
// TODO we should use a specialized pushbutton, that changes color if the corresponding component is dissatisfied!
					QPushButton *button = new QPushButton (dummy, parent_widget);
					component ()->connect (button, SIGNAL (clicked ()), widget, SLOT (showGUI ()));
				} else {
					widget = handle->invoke (component (), parent_widget);
				}
			} else {
				xml->displayError (&e, QString ("Could not embed component '%1'. Not found").arg (component_id), DL_ERROR);
			}
		} else {
			xml->displayError (&e, QString ("Invalid tagname '%1'").arg (e.tagName ()), DL_ERROR);
		}

		if (widget && (!(id.isNull ()))) {
			parent->addChild (id, widget);
		}
	}
}

void RKComponentBuilder::parseLogic (const QDomElement &element) {
	RK_TRACE (PLUGIN);

	if (element.isNull ()) return;

	// find connect elements
	XMLHelper* xml = XMLHelper::getStaticHelper ();
	XMLChildList children = xml->getChildElements (element, "connect", DL_INFO);

	XMLChildList::const_iterator it;
	for (it = children.constBegin (); it != children.constEnd (); ++it) {
		addConnection (xml->getStringAttribute (*it, "client", "#noid#", DL_WARNING), QString::null, xml->getStringAttribute (*it, "governor", "#noid#", DL_WARNING), QString::null, xml->getBoolAttribute (*it, "reconcile", false, DL_INFO), element);
	}

	// find initialize elements
	children = xml->getChildElements (element, "set", DL_INFO);
	for (it = children.constBegin (); it != children.constEnd (); ++it) {
		initial_values.insert (xml->getStringAttribute (*it, "id", "#noid#", DL_WARNING), xml->getStringAttribute (*it, "to", "false", DL_WARNING));
	}

	// find outside elements
	children = xml->getChildElements (element, "external", DL_INFO);
	for (it = children.constBegin (); it != children.constEnd (); ++it) {
		QString id = xml->getStringAttribute (*it, "id", "#noid#", DL_WARNING);
		RKComponentPropertyBase *prop = new RKComponentPropertyBase (component (), xml->getBoolAttribute (*it, "required", false, DL_INFO));
		component ()->addChild (id, prop);
		component ()->connect (prop, SIGNAL (valueChanged (RKComponentPropertyBase *)), component (), SLOT (outsideValueChanged (RKComponentPropertyBase *)));

		QString dummy = xml->getStringAttribute (*it, "default", QString::null, DL_INFO);
		if (!dummy.isNull ()) {
			initial_values.insert (id, dummy);
		}
		// TODO add more options
	}

	// find convert elements
	children = xml->getChildElements (element, "convert", DL_INFO);
	for (it = children.constBegin (); it != children.constEnd (); ++it) {
		RKComponentPropertyConvert *convert = new RKComponentPropertyConvert (component ());
		QString id = xml->getStringAttribute (*it, "id", "#noid#", DL_WARNING);
		int mode = xml->getMultiChoiceAttribute (*it, "mode", convert->convertModeOptionString (), 0, DL_WARNING);
		QString sources = xml->getStringAttribute (*it, "sources", QString::null, DL_WARNING);
		convert->setMode ((RKComponentPropertyConvert::ConvertMode) mode);
		convert->setSources (sources);
		if ((mode == RKComponentPropertyConvert::Equals) || (mode == RKComponentPropertyConvert::NotEquals)) {
			convert->setStandard (xml->getStringAttribute (*it, "standard", QString::null, DL_WARNING));
		} else if (mode == RKComponentPropertyConvert::Range) {
			convert->setRange (xml->getDoubleAttribute (*it, "min", -FLT_MAX, DL_INFO), xml->getDoubleAttribute (*it, "max", FLT_MAX, DL_INFO));
		}
		convert->setRequireTrue (xml->getBoolAttribute (*it, "require_true", false, DL_INFO));
		component ()->addChild (id, convert);
	}
}

void RKComponentBuilder::addConnection (const QString &client_id, const QString &client_property, const QString &governor_id, const QString &governor_property, bool reconcile, const QDomElement &origin) {
	RK_TRACE (PLUGIN);

	RKComponentPropertyConnection conn;
	conn.client_property = client_id;
	if (!client_property.isEmpty ()) conn.client_property += '.' + client_property;
	conn.governor_property = governor_id;
	if (!governor_property.isEmpty ()) conn.governor_property += '.' + governor_property;
	conn.reconcile = reconcile;
	conn.origin = origin;
	connection_list.append (conn);
}

void RKComponentBuilder::makeConnections () {
	RK_TRACE (PLUGIN);

	component ()->setPropertyValues (&initial_values);

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

	// save some RAM
	connection_list.clear ();
	initial_values.clear ();
}


#include "rkstandardcomponent.moc"
