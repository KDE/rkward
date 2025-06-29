/*
rkstandardcomponent - This file is part of RKWard (https://rkward.kde.org). Created: Sun Feb 19 2006
SPDX-FileCopyrightText: 2006-2018 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "rkstandardcomponent.h"

#include <QGroupBox>
#include <QVBoxLayout>
#include <qapplication.h>
#include <qfileinfo.h>
#include <qlabel.h>
#include <qtabwidget.h>
#include <qtimer.h>

#include <KLocalizedString>
#include <kmessagebox.h>

#include "../misc/xmlhelper.h"
#include "../scriptbackends/qtscriptbackend.h"
#include "../scriptbackends/rkcomponentscripting.h"
#include "../scriptbackends/simplebackend.h"
#include "../settings/rksettingsmoduleplugins.h"
#include "../windows/rkmdiwindow.h"
#include "../windows/rkworkplace.h"
#include "rkcomponentmap.h"
#include "rkstandardcomponentgui.h"

// component widgets
#include "rkcheckbox.h"
#include "rkdropdown.h"
#include "rkformula.h"
#include "rkinput.h"
#include "rkmatrixinput.h"
#include "rkoptionset.h"
#include "rkpluginbrowser.h"
#include "rkpluginframe.h"
#include "rkpluginsaveobject.h"
#include "rkpluginspinbox.h"
#include "rkpreviewbox.h"
#include "rkradio.h"
#include "rktabpage.h"
#include "rktext.h"
#include "rkvalueselector.h"
#include "rkvarselector.h"
#include "rkvarslot.h"

#include "../debug.h"

RKStandardComponent::RKStandardComponent(RKComponent *parent_component, QWidget *parent_widget, const QString &filename, const QString &id) : RKComponent(parent_component, parent_widget), filename(filename), id(id) {
	RK_TRACE(PLUGIN);

	command_chain = nullptr;
	backend = nullptr;
	scripting = nullptr;
	gui = nullptr;
	wizard = nullptr;
	xml = nullptr;
	created = false;
	killed = false;
	addChild(QStringLiteral("code"), code = new RKComponentPropertyCode(this, true)); // do not change this name!
	code->setInternal(true);

	RKMDIWindow *w = RKWorkplace::mainWorkplace()->activeWindow(RKMDIWindow::AnyWindowState);
	if (w) {
		const auto props = w->globalContextProperties();
		for (const auto [key, value] : props.asKeyValueRange()) {
			RKComponentPropertyBase *prop;
			if (key == QLatin1String("current_object") || key == QLatin1String("current_dataframe")) { // TODO: find cleaner solution than this special casing
				prop = new RKComponentPropertyRObjects(this, false);
			} else {
				prop = new RKComponentPropertyBase(this, false);
			}
			prop->setInternal(true);
			prop->setValue(value);
			addChild(key, prop);
		}
	}

	// open the main description file for parsing
	XMLHelper *xml = getXmlHelper();
	QDomElement doc_element = xml->openXMLFile(DL_ERROR);
	if (doc_element.isNull()) {
		KMessageBox::error(this, i18n("There has been an error while trying to parse the description of this plugin ('%1'). Please refer to stdout for details.", filename), i18n("Could not create plugin"));
		kill();
		return;
	}

	// initialize the script backend with the code-template
	QDomElement element = xml->getChildElement(doc_element, QStringLiteral("code"), DL_WARNING);
	if (element.hasAttribute(QStringLiteral("file"))) {
		QString dummy = QFileInfo(filename).path() + u'/' + xml->getStringAttribute(element, QStringLiteral("file"), QStringLiteral("code.js"), DL_WARNING);

		backend = new QtScriptBackend(dummy, xml->messageCatalog());
	} else {
		SimpleBackend *back = new SimpleBackend();
		back->setPreprocessTemplate(xml->getStringAttribute(element, QStringLiteral("preprocess"), QString(), DL_INFO));
		back->setPrintoutTemplate(xml->getStringAttribute(element, QStringLiteral("printout"), QString(), DL_INFO));
		back->setCalculateTemplate(xml->getStringAttribute(element, QStringLiteral("calculate"), QString(), DL_INFO));
		back->setPreviewTemplate(xml->getStringAttribute(element, QStringLiteral("preview"), QString(), DL_INFO));
		backend = back;
	}
	connect(backend, &ScriptBackend::idle, this, &RKStandardComponent::backendIdle);
	connect(backend, &ScriptBackend::requestValue, this, &RKStandardComponent::getValue);
	connect(backend, &ScriptBackend::haveError, this, &RKStandardComponent::kill);
	if (!backend->initialize(code, parent_component == nullptr)) return;

	// check for existence of help file
	element = xml->getChildElement(doc_element, QStringLiteral("help"), DL_WARNING);
	QString dummy = QFileInfo(filename).path() + u'/' + xml->getStringAttribute(element, QStringLiteral("file"), QStringLiteral("::nosuchfile::"), DL_INFO);
	have_help = QFileInfo::exists(dummy);

	update_pending = false;

	// construct the GUI
	if (!parent_component) { // top-level
		if (!createTopLevel(doc_element)) {
			RK_ASSERT(false);
			kill();
			return; // should never happen
		}
	} else if (!parent_widget) { // we have a parent component, but should still have a separate GUI
		int force_mode = 1;
		if (parentComponent()->isWizardish()) force_mode = 2;
		if (!createTopLevel(doc_element, force_mode, true)) {
			RK_ASSERT(false);
			kill();
			return; // should never happen
		}
	} else {
		bool build_wizard = false;
		QDomElement gui_element;
		if (parent_component->isWizardish()) {
			build_wizard = true;
			gui_element = xml->getChildElement(doc_element, QStringLiteral("wizard"), DL_WARNING);
			if (gui_element.isNull()) {
				gui_element = xml->getChildElement(doc_element, QStringLiteral("dialog"), DL_WARNING);
				build_wizard = false;

				QWidget *fake_page = parent_component->addPage();
				QVBoxLayout *l = new QVBoxLayout(fake_page);
				l->setContentsMargins(0, 0, 0, 0);
				parent_widget = fake_page;
			}
		} else {
			gui_element = xml->getChildElement(doc_element, QStringLiteral("dialog"), DL_WARNING);
			if (gui_element.isNull()) {
				xml->displayError(&doc_element, QStringLiteral("Cannot embed a wizard into a dialog, and no dialog definition available"), DL_ERROR);
				kill();
				return;
			}
		}
		buildAndInitialize(doc_element, gui_element, parent_widget, build_wizard);
	}
#ifdef JSBACKEND_PERFORMANCE_TEST
	QtScriptBackend::_performanceTest();
#endif
}

RKStandardComponent::~RKStandardComponent() {
	RK_TRACE(PLUGIN);

	if (gui) delete gui;             // NOTE: *NOT* using gui->deleteLater (). Destructing the GUI immediately is necessary to get rid of child components, immediately. Otherwise these could try to access their (destroyed) parent, e.g. if they have a timer running that gets triggered before the deletion event arrives.
	if (backend) backend->destroy(); // it will self-destruct, when it has closed the process.
	delete xml;
}

void RKStandardComponent::kill() {
	RK_TRACE(PLUGIN);

	if (killed) return;
	killed = true;
	if (parentComponent()) topmostStandardComponent()->kill();
	if (gui) gui->deleteLater();
}

RKComponentScriptingProxy *RKStandardComponent::scriptingProxy() {
	RK_TRACE(PLUGIN);

	if (!scripting) {
		scripting = new RKComponentScriptingProxy(this);
		connect(scripting, &RKComponentScriptingProxy::haveError, this, &RKStandardComponent::kill);
	}
	return scripting;
}

void RKStandardComponent::hide() {
	RK_TRACE(PLUGIN);

	if (gui) gui->hide();
	RKComponent::hide();
}

void RKStandardComponent::showGUI() {
	RK_TRACE(PLUGIN);

	if (!gui) {
		RK_ASSERT(false);
		return;
	}
	gui->show();
	gui->raise();
}

void RKStandardComponent::setCaption(const QString &caption) {
	RK_TRACE(PLUGIN);

	if (!gui) return;
	gui->setWindowTitle(caption);
}

XMLHelper *RKStandardComponent::getXmlHelper() {
	RK_TRACE(PLUGIN);

	if (!xml) {
		RKComponentHandle *handle = RKComponentMap::getComponentHandle(id);
		xml = new XMLHelper(filename, handle ? handle->messageCatalog() : nullptr);
	}
	return xml;
}

bool RKStandardComponent::createTopLevel(const QDomElement &doc_element, int force_mode, bool enslaved) {
	RK_TRACE(PLUGIN);

	XMLHelper *xml = getXmlHelper();
	bool build_wizard = false;
	QDomElement dialog_element;
	QDomElement wizard_element;

	dialog_element = xml->getChildElement(doc_element, QStringLiteral("dialog"), DL_INFO);
	wizard_element = xml->getChildElement(doc_element, QStringLiteral("wizard"), DL_INFO);

	if (force_mode == 0) {
		if (wizard_element.isNull()) build_wizard = false;
		else if (dialog_element.isNull()) build_wizard = true;
		else { // both are given
			if (RKSettingsModulePlugins::getInterfacePreference() == RKSettingsModulePlugins::PreferDialog) {
				build_wizard = false;
			} else if (RKSettingsModulePlugins::getInterfacePreference() == RKSettingsModulePlugins::PreferWizard) {
				build_wizard = true;
			} else {
				build_wizard = xml->getBoolAttribute(wizard_element, QStringLiteral("recommended"), false, DL_INFO);
			}
		}
	} else if (force_mode == 1) {
		build_wizard = false;
		if (dialog_element.isNull()) {
			build_wizard = true;
			xml->displayError(&doc_element, QStringLiteral("Dialog mode forced, but no dialog element given"), DL_INFO);
		}
	} else if (force_mode == 2) {
		build_wizard = true;
		if (wizard_element.isNull()) {
			build_wizard = false;
			xml->displayError(&doc_element, QStringLiteral("Wizard mode forced, but no wizard element given"), DL_INFO);
		}
	}

	if (build_wizard) {
		gui = new RKStandardComponentWizard(this, code, enslaved);
		static_cast<RKStandardComponentWizard *>(gui)->createWizard(!dialog_element.isNull());
		wizard = static_cast<RKStandardComponentWizard *>(gui)->getStack();
		buildAndInitialize(doc_element, wizard_element, gui->mainWidget(), true, enslaved);
	} else {
		gui = new RKStandardComponentGUI(this, code, enslaved);
		gui->createDialog(!wizard_element.isNull());
		buildAndInitialize(doc_element, dialog_element, gui->mainWidget(), false, enslaved);
	}
	gui->finalize();

	return true;
}

void RKStandardComponent::switchInterface() {
	RK_TRACE(PLUGIN);

	RK_ASSERT(gui); // this should only ever happen on top level

	// open the main description file for parsing (again)
	XMLHelper *xml = getXmlHelper();
	QDomElement doc_element = xml->openXMLFile(DL_ERROR);
	int force_mode = 2;
	if (isWizardish()) force_mode = 1;

	QMap<QString, QString> value_save; // fetch current GUI settings
	fetchPropertyValuesRecursive(&value_save);

	discard();

	createTopLevel(doc_element, force_mode);

	// set old GUI settings. For this purpose, we'll temporarily disable updates in the script backend
	created = false;
	setPropertyValues(&value_save);
	created = true;
	changed(); // now we can update
}

void RKStandardComponent::discard() {
	RK_TRACE(PLUGIN);

	created = false;
	gui->hide();
	gui->deleteLater();
	gui = nullptr;
	wizard = nullptr;

	// clear all properties. Not the code property, as the script backend relies on it
	for (auto it = child_map.constBegin(); it != child_map.constEnd(); ++it) {
		if (it.value() != code) {
			if (it.value()->isProperty()) {
				static_cast<RKComponentPropertyBase *>(it.value())->deleteLater();
			} else {
				static_cast<RKComponent *>(it.value())->deleteLater();
			}
		}
	}
	child_map.clear();
	addChild(QStringLiteral("code"), code);

	createDefaultProperties(); // enabledness, requiredness, visibility
}

void RKStandardComponent::buildAndInitialize(const QDomElement &doc_element, const QDomElement &gui_element, QWidget *parent_widget, bool build_wizard, bool enslaved) {
	RK_TRACE(PLUGIN);

	XMLHelper *xml = getXmlHelper();

	// create a builder
	RKComponentBuilder *builder = new RKComponentBuilder(this, doc_element);

	// go
	builder->buildElement(gui_element, *xml, parent_widget, build_wizard);
	builder->parseLogic(xml->getChildElement(doc_element, QStringLiteral("logic"), DL_INFO), *xml);
	setCaption(xml->i18nStringAttribute(gui_element, QStringLiteral("label"), QString(), DL_WARNING));

	// initialize
	builder->makeConnections();

	// done!
	delete builder;
	created = true;
	if (gui && (!enslaved)) {
		// somehow, when switching the interface, and we show before the old GUI has been fully deleted (it is deleted via deleteLater (), then there may be strange graphical glitches until the GUI is first redrawn completely.
		// Likely a difficult bug in Qt. Delaying the show until the next event loop solves the problem.
		QTimer::singleShot(0, gui, [this]() { gui->show(); });
	}
	changed();
	Q_EMIT standardInitializationComplete();
}

RKXMLGUIPreviewArea *RKStandardComponent::addDockedPreview(RKComponentPropertyBool *controller, const QString &label, RKPreviewManager *manager) {
	RK_TRACE(PLUGIN);

	RK_ASSERT(gui);
	if (!gui) return nullptr;
	return gui->addDockedPreview(controller, label, manager);
}

RKComponentBase::ComponentStatus RKStandardComponent::recursiveStatus() {
	RK_TRACE(PLUGIN);

	if (killed) return Dead;
	if (backend->isBusy() || update_pending) return Processing;
	return (RKComponentBase::recursiveStatus());
}

bool RKStandardComponent::submit(RCommandChain *in_chain) {
	RK_TRACE(PLUGIN);

	if (!isSatisfied()) return false;

	RCommandChain *prev_chain = command_chain;
	command_chain = in_chain;
	gui->ok();
	command_chain = prev_chain;
	return true;
}

void RKStandardComponent::close() {
	RK_TRACE(PLUGIN);

	if (gui && (!parentComponent())) {
		QTimer::singleShot(0, gui, [this]() { gui->close(); });
	} else {
		RK_ASSERT(false);
	}
}

void RKStandardComponent::changed() {
	RK_TRACE(PLUGIN);

	if (!created) return;
	if (gui) gui->enableSubmit(false);

	// don't trigger update twice
	if (!update_pending) {
		update_pending = true;
		QTimer::singleShot(0, this, [this]() { handleChange(); });
	}
}

void RKStandardComponent::handleChange() {
	RK_TRACE(PLUGIN);

	if (killed) return;
	update_pending = false;
	backend->preprocess(0);
	backend->calculate(0);
	backend->printout(0);
	backend->preview(0);

	if (gui) {
		gui->updateCode(); // will read "processing, please wait", or similar
	}

	RKComponent::changed();
}

void RKStandardComponent::backendIdle() {
	RK_TRACE(PLUGIN);

	if (gui) {
		gui->updateCode();
		gui->enableSubmit(isSatisfied());
	}

	RKComponent::changed(); // notify parent, if any
}

void RKStandardComponent::getValue(const QString &id, const int hint) {
	RK_TRACE(PLUGIN);
	RK_ASSERT(backend);

	backend->writeData(fetchValue(id, hint));
}

bool RKStandardComponent::isWizardish() {
	RK_TRACE(PLUGIN);

	if (gui) return (wizard != nullptr);
	if (parentComponent()) return (parentComponent()->isWizardish());
	return false;
}

bool RKStandardComponent::havePage(bool next) {
	RK_TRACE(PLUGIN);
	RK_ASSERT(wizard);

	return (wizard->havePage(next));
}

void RKStandardComponent::movePage(bool next) {
	RK_TRACE(PLUGIN);
	RK_ASSERT(wizard);

	wizard->movePage(next);
}

bool RKStandardComponent::currentPageSatisfied() {
	RK_TRACE(PLUGIN);
	RK_ASSERT(wizard);

	return (wizard->currentPageSatisfied());
}

RKComponent *RKStandardComponent::addPage() {
	RK_TRACE(PLUGIN);

	if (wizard) {
		RKComponent *page = wizard->addPage(this);
		return page;
	} else if (parentComponent()) {
		RK_ASSERT(!gui);
		return parentComponent()->addPage();
	}

	RK_ASSERT(false);
	return RKComponent::addPage();
}

void RKStandardComponent::addChild(const QString &id, RKComponentBase *child) {
	RK_TRACE(PLUGIN);

	if (wizard || (parentComponent() && parentComponent()->isWizardish())) {
		if (!child->isProperty()) {
			addComponentToCurrentPage(static_cast<RKComponent *>(child));
		}
	}

	RKComponent::addChild(id, child);
}

void RKStandardComponent::addComponentToCurrentPage(RKComponent *component) {
	RK_TRACE(PLUGIN);

	if (wizard) {
		wizard->addComponentToCurrentPage(component);
	} else if (parentComponent() && parentComponent()->isWizardish()) {
		parentComponent()->addComponentToCurrentPage(component);
	} else {
		RK_ASSERT(false);
	}
}

/////////////////////////////////////// RKComponentBuilder /////////////////////////////////////////

#include <qpushbutton.h>

RKComponentBuilder::RKComponentBuilder(RKComponent *parent_component, const QDomElement &document_element) {
	RK_TRACE(PLUGIN);
	parent = parent_component;
	doc_elem = document_element;
	while (!doc_elem.parentNode().isNull()) {
		doc_elem = doc_elem.parentNode().toElement();
	}
}

RKComponentBuilder::~RKComponentBuilder() {
	RK_TRACE(PLUGIN);
}

QDomElement RKComponentBuilder::doElementCopy(const QString &id, XMLHelper &xml, const QDomElement &copy) {
	RK_TRACE(PLUGIN);

	QDomElement res;

	if (id.isEmpty()) {
		xml.displayError(&copy, QStringLiteral("no id given for copy element"), DL_ERROR, DL_ERROR);
		return res; // null
	}

	// find matching element to copy from
	XMLChildList candidates = xml.findElementsWithAttribute(doc_elem, QStringLiteral("id"), id, true, DL_ERROR);
	XMLChildList::const_iterator it;
	for (it = candidates.constBegin(); it != candidates.constEnd(); ++it) {
		if ((*it).tagName() == QLatin1String("copy")) continue;
		res = (*it).cloneNode().toElement();
		break;
	}
	if (res.isNull()) {
		xml.displayError(&copy, QStringLiteral("no matching element found to copy from"), DL_ERROR, DL_ERROR);
		return res;
	}

	// copy overridden attributes
	QDomNamedNodeMap attribs = copy.attributes();
	int len = attribs.count();
	for (int i = 0; i < len; ++i) {
		QDomAttr attr = attribs.item(i).toAttr();
		if (attr.name() == QLatin1String("copy_element_tag_name")) res.setTagName(attr.value());
		else res.setAttribute(attr.name(), attr.value());
	}

	return res;
}

void RKComponentBuilder::buildElement(const QDomElement &element, XMLHelper &xml, QWidget *parent_widget, bool allow_pages) {
	RK_TRACE(PLUGIN);

	XMLChildList children = xml.getChildElements(element, QString(), DL_ERROR);

	XMLChildList::const_iterator it;
	for (it = children.constBegin(); it != children.constEnd(); ++it) {
		bool add_to_layout = true;
		RKComponent *widget = nullptr;
		QDomElement e = *it; // shorthand
		QString id = xml.getStringAttribute(e, QStringLiteral("id"), QString(), DL_INFO);

		if (e.tagName() == QLatin1String("copy")) {
			e = doElementCopy(id, xml, e);
		} // no else, here. e may be changed to some entirely different element, now.

		if (allow_pages && (e.tagName() == QLatin1String("page"))) {
			widget = component()->addPage();
			add_to_layout = false; // For wizards, that's done inside addPage()
			QVBoxLayout *l = new QVBoxLayout(widget);
			l->setContentsMargins(0, 0, 0, 0);
			buildElement(e, xml, widget, false);
		} else if (e.tagName() == QLatin1String("row")) {
			widget = new RKComponent(component(), parent_widget); // wrapping this (and column, below) inside an RKComponent has the benefit, that it can have an id, and hence can be set to visibile/hidden, enabled/disabled
			QHBoxLayout *layout = new QHBoxLayout(widget);
			layout->setContentsMargins(0, 0, 0, 0);
			buildElement(e, xml, widget, false);
		} else if (e.tagName() == QLatin1String("stretch")) {
			QWidget *stretch = new QWidget(parent_widget);
			stretch->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
			parent_widget->layout()->addWidget(stretch);
			QBoxLayout *box = dynamic_cast<QBoxLayout *>(parent_widget->layout());
			// RK_ASSERT (box);  <- NO, also meaningful in a <frame>
			if (box) box->setStretchFactor(stretch, 100);
		} else if (e.tagName() == QLatin1String("column")) {
			widget = new RKComponent(component(), parent_widget);
			QVBoxLayout *layout = new QVBoxLayout(widget);
			layout->setContentsMargins(0, 0, 0, 0);
			buildElement(e, xml, widget, false);
		} else if (e.tagName() == QLatin1String("frame")) {
			RKPluginFrame *frame = new RKPluginFrame(e, component(), parent_widget);
			widget = frame;
			buildElement(e, xml, frame->getPage(), false);
		} else if (e.tagName() == QLatin1String("tabbook")) {
			QTabWidget *tabbook = new QTabWidget(parent_widget);
			// this is not an RKComponent, so we need to add it, manually
			parent_widget->layout()->addWidget(tabbook);
			QDomNodeList tabs = e.childNodes();
			for (int t = 0; t < tabs.count(); ++t) {
				QDomElement tab_e = tabs.item(t).toElement();
				if (tab_e.tagName() == QLatin1String("tab")) {
					RKTabPage *tabpage = new RKTabPage(tab_e, component(), tabbook);
					buildElement(tab_e, xml, tabpage->getPage(), false);
					QString tab_id = xml.getStringAttribute(tab_e, QStringLiteral("id"), QString(), DL_INFO);
					if (!tab_id.isNull()) {
						parent->addChild(tab_id, tabpage);
					}
				}
			}
		} else if (e.tagName() == QLatin1String("varselector")) {
			widget = new RKVarSelector(e, component(), parent_widget);
		} else if ((e.tagName() == QLatin1String("varslot")) || (e.tagName() == QLatin1String("valueslot"))) {
			widget = new RKVarSlot(e, component(), parent_widget);
			QString source = xml.getStringAttribute(e, QStringLiteral("source_property"), QString(), DL_INFO);
			if (source.isEmpty()) source = xml.getStringAttribute(e, QStringLiteral("source"), QStringLiteral("#noid#"), DL_WARNING) + u".selected"_s;
			addConnection(id, QStringLiteral("source"), source, QString(), false, e);
		} else if ((e.tagName() == QLatin1String("valueselector")) || (e.tagName() == QLatin1String("select"))) {
			widget = new RKValueSelector(e, component(), parent_widget);
		} else if (e.tagName() == QLatin1String("formula")) {
			widget = new RKFormula(e, component(), parent_widget);
			addConnection(id, QStringLiteral("dependent"), xml.getStringAttribute(e, QStringLiteral("dependent"), QStringLiteral("#noid#"), DL_INFO), QStringLiteral("available"), false, e);
			addConnection(id, QStringLiteral("fixed_factors"), xml.getStringAttribute(e, QStringLiteral("fixed_factors"), QStringLiteral("#noid#"), DL_INFO), QStringLiteral("available"), false, e);
		} else if (e.tagName() == QLatin1String("radio")) {
			widget = new RKRadio(e, component(), parent_widget);
		} else if (e.tagName() == QLatin1String("dropdown")) {
			widget = new RKDropDown(e, component(), parent_widget);
		} else if (e.tagName() == QLatin1String("checkbox")) {
			widget = new RKCheckBox(e, component(), parent_widget);
		} else if (e.tagName() == QLatin1String("spinbox")) {
			widget = new RKPluginSpinBox(e, component(), parent_widget);
		} else if (e.tagName() == QLatin1String("matrix")) {
			widget = new RKMatrixInput(e, component(), parent_widget);
		} else if (e.tagName() == QLatin1String("input")) {
			widget = new RKInput(e, component(), parent_widget);
		} else if (e.tagName() == QLatin1String("browser")) {
			widget = new RKPluginBrowser(e, component(), parent_widget);
		} else if (e.tagName() == QLatin1String("text")) {
			widget = new RKText(e, component(), parent_widget);
		} else if (e.tagName() == QLatin1String("preview")) {
			QWidget *pwidget = parent_widget;
			RKStandardComponent *uicomp = parent->topmostStandardComponent();
			if (uicomp && !uicomp->isWizardish()) {
				parent_widget = static_cast<RKStandardComponent *>(uicomp)->gui->custom_preview_buttons_area;
			}
			widget = new RKPreviewBox(e, component(), parent_widget);
			parent_widget->layout()->addWidget(widget);
			parent_widget = pwidget;
			add_to_layout = false;
		} else if (e.tagName() == QLatin1String("saveobject")) {
			widget = new RKPluginSaveObject(e, component(), parent_widget);
		} else if (e.tagName() == QLatin1String("embed")) {
			QString component_id = xml.getStringAttribute(e, QStringLiteral("component"), QString(), DL_ERROR);
			RKComponentHandle *handle = RKComponentMap::getComponentHandle(component_id);
			if (handle) {
				if (xml.getBoolAttribute(e, QStringLiteral("as_button"), false, DL_INFO)) {
					RKStandardComponent *swidget = handle->invoke(component(), nullptr);
					widget = swidget;
					QString dummy = xml.i18nStringAttribute(e, QStringLiteral("label"), i18n("Options"), DL_WARNING);
					swidget->setCaption(dummy);
					// TODO we should use a specialized pushbutton, that changes color if the corresponding component is dissatisfied!
					QPushButton *button = new QPushButton(dummy, parent_widget);
					component()->connect(button, &QPushButton::clicked, swidget, &RKStandardComponent::showGUI);
					add_to_layout = false;
					parent_widget->layout()->addWidget(button);
				} else {
					widget = handle->invoke(component(), parent_widget);
				}
			} else {
				xml.displayError(&e, QStringLiteral("Could not embed component '%1'. Not found").arg(component_id), DL_ERROR);
			}
		} else if (e.tagName() == QLatin1String("optionset")) {
			widget = new RKOptionSet(e, component(), parent_widget);
		} else if (e.tagName() == QLatin1String("optiondisplay")) {
			// TODO: Remove after grace period. Last release to support optiondisplay: 0.6.3
			xml.displayError(&e, QStringLiteral("<optiondisplay> element is obsolete. Ignoring."), DL_WARNING);
			/*		} else if (e.tagName () == QLatin1String ("scriptable")) {
			 * TODO: We used to have some purely experimental code, here, to support fully custom elements (scripted via Kross->Forms). We will want
			 * to have something like that, eventually. After porting to KF5, Qt5, and thus dropping legacy support, the natural way to add this will
			 * be QML.
			 */
		} else {
			xml.displayError(&e, QStringLiteral("Invalid tagname '%1'").arg(e.tagName()), DL_ERROR);
		}

		if (widget) {
			if (add_to_layout) parent_widget->layout()->addWidget(widget);
			if (!id.isNull()) parent->addChild(id, widget);
		}
	}
}

void RKComponentBuilder::parseLogic(const QDomElement &element, XMLHelper &xml, bool allow_script_tag) {
	RK_TRACE(PLUGIN);

	if (element.isNull()) return;

	QHash<RKComponentPropertyBase *, QStringList> switch_convert_sources;

	const XMLChildList children = xml.getChildElements(element, QString(), DL_ERROR);
	for (int i = 0; i < children.size(); ++i) {
		const QDomElement &cel = children[i];
		const QString tagName = cel.tagName();
		if (tagName == QLatin1String("connect")) {
			addConnection(xml.getStringAttribute(cel, QStringLiteral("client"), QStringLiteral("#noid#"), DL_WARNING), QString(), xml.getStringAttribute(cel, QStringLiteral("governor"), QStringLiteral("#noid#"), DL_WARNING), QString(), xml.getBoolAttribute(cel, QStringLiteral("reconcile"), false, DL_INFO), element);
		} else if (tagName == QLatin1String("dependency_check")) {
			RKComponentPropertyBool *dep = new RKComponentPropertyBool(component(), false);
			dep->setInternal(true);
			dep->setBoolValue(RKComponentDependency::isRKWardVersionCompatible(cel) && RKComponentDependency::isRVersionCompatible(cel));
			component()->addChild(xml.getStringAttribute(cel, QStringLiteral("id"), QStringLiteral("#noid#"), DL_WARNING), dep);
		} else if (tagName == QLatin1String("external")) {
			QString id = xml.getStringAttribute(cel, QStringLiteral("id"), QStringLiteral("#noid#"), DL_WARNING);
			RKComponentPropertyBase *prop = new RKComponentPropertyBase(component(), xml.getBoolAttribute(cel, QStringLiteral("required"), false, DL_INFO));
			component()->addChild(id, prop);
			prop->setInternal(true);
			component()->connect(prop, &RKComponentPropertyBase::valueChanged, component(), &RKComponent::outsideValueChanged);

			QString dummy = xml.getStringAttribute(cel, QStringLiteral("default"), QString(), DL_INFO);
			if (!dummy.isNull()) {
				initial_values.insert(id, dummy);
			}
		} else if (tagName == QLatin1String("set")) {
			// NOTE: It is by design that if there are several initializations for a single id, the latest one takes precedence. Useful in some cases of inclusion.
			initial_values.insert(xml.getStringAttribute(cel, QStringLiteral("id"), QStringLiteral("#noid#"), DL_WARNING), xml.getStringAttribute(cel, QStringLiteral("to"), QStringLiteral("false"), DL_WARNING));
		} else if (tagName == QLatin1String("i18n")) {
			QString id = xml.getStringAttribute(cel, QStringLiteral("id"), QStringLiteral("#noid#"), DL_WARNING);
			RKComponentPropertyBase *prop = new RKComponentPropertyBase(component(), false);
			component()->addChild(id, prop);
			prop->setInternal(true);
			initial_values.insert(id, xml.i18nStringAttribute(cel, QStringLiteral("label"), QString(), DL_WARNING));
		} else if (tagName == QLatin1String("convert")) {
			RKComponentPropertyConvert *convert = new RKComponentPropertyConvert(component());
			convert->setInternal(true);
			QString id = xml.getStringAttribute(cel, QStringLiteral("id"), QStringLiteral("#noid#"), DL_WARNING);
			int mode = xml.getMultiChoiceAttribute(cel, QStringLiteral("mode"), convert->convertModeOptionString(), 0, DL_WARNING);
			convert->setMode((RKComponentPropertyConvert::ConvertMode)mode);
			if ((mode == RKComponentPropertyConvert::Equals) || (mode == RKComponentPropertyConvert::NotEquals)) {
				convert->setStandard(xml.getStringAttribute(cel, QStringLiteral("standard"), QString(), DL_WARNING));
			} else if (mode == RKComponentPropertyConvert::Range) {
				convert->setRange(xml.getDoubleAttribute(cel, QStringLiteral("min"), -FLT_MAX, DL_INFO), xml.getDoubleAttribute(cel, QStringLiteral("max"), FLT_MAX, DL_INFO));
			}
			switch_convert_sources.insert(convert, xml.getStringAttribute(cel, QStringLiteral("sources"), QString(), DL_WARNING).split(u';'));
			convert->setRequireTrue(xml.getBoolAttribute(cel, QStringLiteral("require_true"), false, DL_INFO));
			component()->addChild(id, convert);
		} else if (tagName == QLatin1String("switch")) {
			QDomElement t = xml.getChildElement(cel, QStringLiteral("true"), DL_INFO);
			QDomElement f = xml.getChildElement(cel, QStringLiteral("false"), DL_INFO);
			if (t.isNull() != f.isNull()) {
				xml.displayError(&(cel), QStringLiteral("One of <true> / <false> was provided for boolean <switch>, but not the other. Skipping switch."), DL_ERROR);
				continue;
			}

			XMLChildList case_elems = xml.getChildElements(cel, QStringLiteral("case"), DL_INFO);
			QDomElement default_elem = xml.getChildElement(cel, QStringLiteral("default"), DL_INFO);
			if (!default_elem.isNull()) case_elems.append(default_elem);

			if (t.isNull()) {
				if (case_elems.isEmpty()) {
					xml.displayError(&cel, QStringLiteral("Neither <true> / <false> nor <case> / <default> were provided. Skipping switch."), DL_ERROR);
					continue;
				}
			} else {
				if (!case_elems.isEmpty()) {
					xml.displayError(&cel, QStringLiteral("One <true> / <false> *or* <case> / <default> may be provided a <switch>. Proceeding with boolean switch."), DL_ERROR);
					case_elems.clear();
				}
				case_elems.append(f);
				case_elems.append(t);
			}

			QStringList def_strings;
			QStringList standards;
			QStringList sources;
			sources.append(xml.getStringAttribute(cel, QStringLiteral("condition"), QString(), DL_ERROR)); // store condition prop as first "source"

			for (XMLChildList::const_iterator cit = case_elems.constBegin(); cit != case_elems.constEnd(); ++cit) {
				def_strings.append(xml.getStringAttribute(*cit, QStringLiteral("fixed_value"), QString(), DL_INFO));
				sources.append(xml.getStringAttribute(*cit, QStringLiteral("dynamic_value"), QString(), DL_INFO));
				if ((*cit).tagName() == QLatin1String("case")) standards.append(xml.getStringAttribute(*cit, QStringLiteral("standard"), QString(), DL_WARNING));
			}

			QString id = xml.getStringAttribute(cel, QStringLiteral("id"), QStringLiteral("#noid#"), DL_WARNING);
			RKComponentPropertySwitch *switchel = new RKComponentPropertySwitch(component(), def_strings, standards);
			switchel->setInternal(true);
			switch_convert_sources.insert(switchel, sources);
			component()->addChild(id, switchel);
		}
	}

	// resolve source properties for switch and convert elements, *after* all properties have been created
	for (auto it = switch_convert_sources.constBegin(); it != switch_convert_sources.constEnd(); ++it) {
		if (it.key()->type() == RKComponentBase::PropertyConvert) {
			static_cast<RKComponentPropertyConvert *>(it.key())->setSources(it.value());
		} else {
			RK_ASSERT(it.key()->type() == RKComponentBase::PropertySwitch);
			QStringList sources = it.value();
			if (sources.isEmpty()) {
				RK_ASSERT(!sources.isEmpty());
				continue;
			}
			static_cast<RKComponentPropertySwitch *>(it.key())->setSources(sources.takeFirst(), sources);
		}
	}

	QDomElement e = xml.getChildElement(element, QStringLiteral("script"), DL_INFO);
	if (!e.isNull() && allow_script_tag) {
		QString file = xml.getStringAttribute(e, QStringLiteral("file"), QString(), DL_INFO);
		QString inline_command = e.text();
		parent->standardComponent()->scriptingProxy()->initialize(file, inline_command);
	} else if (!e.isNull()) {
		xml.displayError(&e, QStringLiteral("<script> element is not allowed inside this <logic> section."), DL_ERROR);
	}
}

void RKComponentBuilder::addConnection(const QString &client_id, const QString &client_property, const QString &governor_id, const QString &governor_property,
                                       bool reconcile, const QDomElement &origin) {
	RK_TRACE(PLUGIN);

	RKComponentPropertyConnection conn;
	conn.client_property = client_id;
	if (!client_property.isEmpty()) conn.client_property += u'.' + client_property;
	conn.governor_property = governor_id;
	if (!governor_property.isEmpty()) conn.governor_property += u'.' + governor_property;
	conn.reconcile = reconcile;
	conn.origin = origin;
	connection_list.append(conn);
}

void RKComponentBuilder::makeConnections() {
	RK_TRACE(PLUGIN);

	component()->setPropertyValues(&initial_values);

	XMLHelper *xml = component()->xmlHelper();

	for (ConnectionList::const_iterator it = connection_list.constBegin(); it != connection_list.constEnd(); ++it) {
		RK_DEBUG(PLUGIN, DL_DEBUG, "Connecting '%s' to '%s'", (*it).client_property.toLatin1().data(), (*it).governor_property.toLatin1().data());

		QString dummy;
		RKComponentBase *client = parent->lookupComponent((*it).client_property, &dummy);
		if ((!client) || (!dummy.isEmpty()) || (!client->isProperty())) {
			xml->displayError(&((*it).origin), QStringLiteral("Invalid client identifier '%1'").arg((*it).client_property), DL_ERROR);
			continue;
		}
		RKComponentBase *governor = parent->lookupComponent((*it).governor_property, &dummy);
		if ((!governor) || (!governor->isProperty())) {
			xml->displayError(&((*it).origin), QStringLiteral("Invalid governor identifier '%1'").arg((*it).governor_property), DL_ERROR);
			continue;
		}

		static_cast<RKComponentPropertyBase *>(client)->connectToGovernor(static_cast<RKComponentPropertyBase *>(governor), dummy, (*it).reconcile);
	}

	// save some RAM
	connection_list.clear();
	initial_values.clear();
}
