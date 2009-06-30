/***************************************************************************
                          rkcomponentmap.cpp  -  description
                             -------------------
    begin                : Thu May 12 2005
    copyright            : (C) 2005, 2006, 2007, 2009 by Thomas Friedrichsmeier
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

#include "rkcomponentmap.h"

#include <qfileinfo.h>
#include <qdir.h>

#include <klocale.h>
#include <kactioncollection.h>
#include <kmessagebox.h>

#include "rkcomponentcontext.h"
#include "rkstandardcomponent.h"
#include "../misc/xmlhelper.h"
#include "../debug.h"
#include "../rkglobals.h"
#include "../rkward.h"

QString RKPluginMapFile::makeFileName (const QString &filename) {
	return QDir::cleanPath (QDir (basedir).filePath (filename));
}

RKComponentGUIXML::RKComponentGUIXML () {
	RK_TRACE (PLUGIN);

	clearGUIDescription ();
}

RKComponentGUIXML::~RKComponentGUIXML () {
	RK_TRACE (PLUGIN);
}

void RKComponentGUIXML::clearGUIDescription () {
	RK_TRACE (PLUGIN);

	gui_xml.setContent (QString ("<!DOCTYPE kpartgui>\n<kpartgui name=\"rkwardcomponents\" version=\"0.3.4\">\n<MenuBar>\n\n</MenuBar>\n</kpartgui>"));
}

int RKComponentGUIXML::createMenus (QDomElement& parent, const QDomElement& hierarchy_description, const QString& cnamespace) {
	RK_TRACE (PLUGIN);

	XMLHelper* xml = XMLHelper::getStaticHelper ();
	XMLChildList list = xml->getChildElements (hierarchy_description, "menu", DL_INFO);
	int counter = 0;
	for (XMLChildList::const_iterator it=list.begin (); it != list.end (); ++it) {
		counter += addSubMenu (parent, (*it), cnamespace);
	}
	return counter;
}

QDomElement RKComponentGUIXML::findOrCreateElement (QDomElement& parent, const QString& tagname, const QString& name, const QString& label, int index) {
	RK_TRACE (PLUGIN);

	XMLHelper* xml = XMLHelper::getStaticHelper ();
	XMLChildList list = xml->getChildElements (parent, QString::null, DL_INFO);		// we need to look at all children, so we get the order right
	QDomElement insert_after_element;
	for (XMLChildList::const_iterator it=list.begin (); it != list.end (); ++it) {
		if ((tagname == (*it).tagName ()) && (name == xml->getStringAttribute ((*it), "name", "", DL_ERROR))) {
			return (*it);
		} else {
			if (index >= 0) {
				if (index > xml->getIntAttribute ((*it), "index", -1, DL_INFO)) {
					insert_after_element = *it;
				}
			}
		}
	}

	// element not found. Create a new one instead
	QDomElement ret = gui_xml.createElement (tagname);
	ret.setAttribute ("name", name);
	ret.setAttribute ("index", index);
	if (!label.isEmpty ()) {
		QDomElement text = gui_xml.createElement ("text");
		text.appendChild (gui_xml.createTextNode (label));
		ret.appendChild (text);
	}
	parent.insertAfter (ret, insert_after_element);	// if index_after_element.isNull, this add the new element as the last child of parent!

	return ret;
}

int RKComponentGUIXML::addSubMenu (QDomElement& parent, const QDomElement& description, const QString& cnamespace) {
	RK_TRACE (PLUGIN);

	int counter = 0;
	XMLHelper* xml = XMLHelper::getStaticHelper ();

	// 1: check whether menu already exists, and create new menu otherwise
	QDomElement menu_element = findOrCreateElement (parent, "Menu", xml->getStringAttribute (description, "id", "none", DL_ERROR), xml->getStringAttribute (description, "label", i18n ("(no label)"), DL_WARNING), xml->getIntAttribute (description, "index", -1, DL_INFO));

	// 2: recurse into submenus (of element to add!)
	XMLChildList list = xml->getChildElements (description, "menu", DL_INFO);
	for (XMLChildList::const_iterator it=list.begin (); it != list.end (); ++it) {
		counter += addSubMenu (menu_element, (*it), cnamespace);
	}

	// 3: add entries
	list = xml->getChildElements (description, "entry", DL_INFO);
	for (XMLChildList::const_iterator it=list.begin (); it != list.end (); ++it) {
		QString id = cnamespace + xml->getStringAttribute ((*it), "component", "#invalid#", DL_ERROR);

		RKComponentHandle* handle = RKComponentMap::getComponentHandle (id);
		if ((!handle) || (!handle->isPlugin ())) {
			RK_DO (qDebug ("No such component found while creating menu-entries or component is not a standalone plugin: \"%s\". No entry created.", id.toLatin1 ().data ()), PLUGIN, DL_ERROR);
		} else {
			findOrCreateElement (menu_element, "Action", id, QString::null, xml->getIntAttribute ((*it), "index", -1, DL_INFO));
			addedEntry (id, handle);
			counter++;
		}
	}
	return counter;
}

/////////////////////////// END RKComponentXMLGUIClient /////////////////////////////////
////////////////////////////// Bhttp://apps.sourceforge.net/mediawiki/rkward/nfs/project/r/rk/rkward/6/6d/RKWardApplicationDetached.pngEGIN RKComponentMap /////////////////////////////////////

// static members
RKComponentMap *RKComponentMap::component_map = 0;

void RKComponentMap::initialize () {
	RK_TRACE (PLUGIN);

	RK_ASSERT (component_map == 0);
	component_map = new RKComponentMap ();
}

RKComponentMap::RKComponentMap () : RKComponentGUIXML (), KXMLGUIClient () {
	RK_TRACE (PLUGIN);

	setComponentData (KGlobal::mainComponent ());
	actionCollection ()->setConfigGroup ("Plugin Shortcuts");
}

RKComponentMap::~RKComponentMap () {
	RK_TRACE (PLUGIN);

	clearLocal ();
}

void RKComponentMap::clearLocal () {
	RK_TRACE (PLUGIN);

	actionCollection ()->clear ();
	for (ComponentMap::iterator it = components.begin (); it != components.end (); ++it) {
		delete (it.value ());
/* TODO: this is not technically correct, as there may be several actions for this id, and we're only deleting one. But practically this should not really be relevant. */
		delete (actionCollection ()->action (it.key ()));
	}
	components.clear ();

	for (PluginMapFileMap::const_iterator it = pluginmapfiles.constBegin (); it != pluginmapfiles.constEnd (); ++it) {
		delete (it.value ());
	}
	pluginmapfiles.clear ();

	clearGUIDescription ();

	setXMLGUIBuildDocument (gui_xml);
}

void RKComponentMap::clearAll () {
	RK_TRACE (PLUGIN);

	getMap ()->clearLocal ();
}

RKContextMap *RKComponentMap::getContext (const QString &id) {
	RK_TRACE (PLUGIN);

	RKContextMap *context = getMap ()->getContextLocal (id);
	if (context) return context;

	RK_DO (qDebug ("no such context %s", id.toLatin1 ().data ()), PLUGIN, DL_WARNING);
	return (0);
}

RKContextMap *RKComponentMap::getContextLocal (const QString &id) {
	RK_TRACE (PLUGIN);

	if (contexts.contains (id)) return (contexts[id]);
	return 0;
}

RKComponentHandle* RKComponentMap::getComponentHandle (const QString &id) {
	RK_TRACE (PLUGIN);

	RKComponentHandle *handle = getMap ()->getComponentHandleLocal (id);
	if (handle) return handle;

	RK_DO (qDebug ("no such component %s", id.toLatin1 ().data ()), PLUGIN, DL_WARNING);
	return (0);
}

RKComponentHandle* RKComponentMap::getComponentHandleLocal (const QString &id) {
	RK_TRACE (PLUGIN);

	if (components.contains (id)) return (components[id]);
	return 0;
}

//static
QString RKComponentMap::getComponentId (RKComponentHandle* by_component) {
	RK_TRACE (PLUGIN);

	return (getMap ()->getComponentIdLocal (by_component));
}

QString RKComponentMap::getComponentIdLocal (RKComponentHandle* component) {
	RK_TRACE (PLUGIN);

	for (ComponentMap::iterator it = components.begin (); it != components.end (); ++it) {
		if (it.value () == component) {
			return it.key ();
		}
	}

	RK_ASSERT (false);
	return (QString ());
}

//static
bool RKComponentMap::invokeComponent (const QString &component_id, const QStringList &serialized_settings, ComponentInvocationMode submit_mode, QString *message, RCommandChain *in_chain) {
	RK_TRACE (PLUGIN);

	QString _message;
	RKComponentHandle *handle = getComponentHandle (component_id);
	if (!handle) {
		_message = i18n ("You tried to invoke a plugin called '%1', but that plugin is currently unknown. Probably you need to load the corresponding PluginMap (Settings->Configure RKWard->Plugins), or perhaps the plugin was renamed.").arg (component_id);
		if (message) *message = _message;
		else KMessageBox::sorry (RKWardMainWindow::getMain (), _message, i18n ("No such plugin"));
		return false;
	}

	RKStandardComponent *component = handle->invoke (0, 0);
	RK_ASSERT (component);

	RKComponent::UnserializeError error = component->unserializeState (serialized_settings);
	if (error == RKComponent::BadFormat) {
		_message = i18n ("Bad serialization format while trying to invoke plugin '%1'. Please contact the RKWard team (Help->About RKWard->Authors).").arg (component_id);
		if (message) *message = _message;
		else KMessageBox::error (component, _message, i18n ("Bad serialization format"));
		return false;
	}
	if (error == RKComponent::NotAllSettingsApplied) {
		_message = i18n ("Not all specified settings could be applied. Most likely this is because some R objects are no longer present in your current workspace.");
		if (message) *message = _message;
		else KMessageBox::information (component, _message, i18n ("Not all settings applied"));
		// TODO: Don't show again-box?
		// not considered an error
	}

	// Auto-Submit
	if (submit_mode != ManualSubmit) {
#ifndef Q_OS_WIN
		// if the plugin takes longer than 5 seconds to settle, than that really is sort of buggy...
		bool submit_ok = component->submit (5000, in_chain);
#else
#warning Temporary workaround. Remove this.
		// (yet on windows, the PHP backend is *real* slow. We give it a bit longer as long as we still use it...
		bool submit_ok = component->submit (50000, in_chain);
#endif
		if (submit_ok || (submit_mode == AutoSubmitOrFail)) component->close ();
		if (!submit_ok) {
			_message.append (i18n ("\nThe plugin could not be auto-submitted with these settings."));
			if (message) *message = _message;
			else KMessageBox::sorry (RKWardMainWindow::getMain (), _message, i18n ("Could not submit"));

			return (submit_mode != AutoSubmitOrFail);
		}
	}

	return true;
}

int RKComponentMap::addPluginMap (const QString& plugin_map_file) {
	RK_TRACE (PLUGIN);

	return getMap()->addPluginMapLocal (plugin_map_file);
}

int RKComponentMap::addPluginMapLocal (const QString& plugin_map_file) {
	RK_TRACE (PLUGIN);

	QString plugin_map_file_abs = QFileInfo (plugin_map_file).absoluteFilePath ();
	if (pluginmapfiles.contains (plugin_map_file_abs)) {
		RK_DO (qDebug ("Plugin map file '%s' already loaded", plugin_map_file.toLatin1().data ()), PLUGIN, DL_INFO);
		return 0;
	}

	XMLHelper* xml = XMLHelper::getStaticHelper ();
	QDomElement element;
	XMLChildList list;

	QDomElement document_element = xml->openXMLFile (plugin_map_file_abs, DL_ERROR);
	if (xml->highestError () >= DL_ERROR) return (0);

	QString prefix = QFileInfo (plugin_map_file_abs).absolutePath() + '/' + xml->getStringAttribute (document_element, "base_prefix", QString::null, DL_INFO);
	QString cnamespace = xml->getStringAttribute (document_element, "namespace", "rkward", DL_INFO) + "::";

	RKPluginMapFile *pluginmap_file_desc = new RKPluginMapFile (prefix);
	pluginmapfiles.insert (QFileInfo (plugin_map_file).absoluteFilePath (), pluginmap_file_desc);

	// step 1: include required files
	int counter = 0;
	QStringList includelist;
	list = xml->getChildElements (document_element, "require", DL_INFO);
	for (XMLChildList::const_iterator it=list.constBegin (); it != list.constEnd (); ++it) {
		QString file = pluginmap_file_desc->makeFileName (xml->getStringAttribute (*it, "file", QString::null, DL_ERROR));
		if (QFileInfo (file).isReadable ()) {
			includelist.append (file);
		} else {
			RK_DO (qDebug ("Specified required file '%s' does not exist or is not readable. Ignoring.", file.toLatin1 ().data ()), PLUGIN, DL_ERROR);
		}
	}
	for (QStringList::const_iterator it = includelist.constBegin (); it != includelist.constEnd (); ++it) {
		counter += addPluginMapLocal (*it);
	}

	// step 2: create (list of) components
	element = xml->getChildElement (document_element, "components", DL_INFO);
	list = xml->getChildElements (element, "component", DL_INFO);

	for (XMLChildList::const_iterator it=list.begin (); it != list.end (); ++it) {
		QString filename = xml->getStringAttribute((*it), "file", QString::null, DL_WARNING);
		QString id = cnamespace + xml->getStringAttribute((*it), "id", QString::null, DL_WARNING);
		int type = xml->getMultiChoiceAttribute ((*it), "type", "standard", 0, DL_WARNING);
		QString label = xml->getStringAttribute ((*it), "label", i18n ("(no label)"), DL_WARNING);

		if (components.contains (id)) {
			RK_DO (qDebug ("RKComponentMap already contains a component with id \"%s\". Ignoring second entry.", id.toLatin1 ().data ()), PLUGIN, DL_WARNING);
		} else if (!QFileInfo (pluginmap_file_desc->makeFileName (filename)).isReadable ()) {
			RK_DO (qDebug ("Specified file '%s' for component id \"%s\" does not exist or is not readable. Ignoring.", filename.toLatin1 ().data (), id.toLatin1 ().data ()), PLUGIN, DL_ERROR);
		} else {
			// create and initialize component handle
			RKComponentHandle *handle = new RKComponentHandle (pluginmap_file_desc, filename, label, (RKComponentType) type);
			XMLChildList attributes_list = xml->getChildElements (*it, "attribute", DL_DEBUG);
			for (XMLChildList::const_iterator ait=attributes_list.begin (); ait != attributes_list.end (); ++ait) {
				handle->addAttribute (xml->getStringAttribute (*ait, "id", "noid", DL_WARNING), xml->getStringAttribute (*ait, "value", QString::null, DL_ERROR), xml->getStringAttribute (*ait, "label", QString::null, DL_ERROR));
			}
			components.insert (id, handle);
		}
	}

	// step 3: create / insert into menus
	QDomElement xmlgui_menubar_element = xml->getChildElement (gui_xml.documentElement (), "MenuBar", DL_ERROR);
	counter += createMenus (xmlgui_menubar_element, xml->getChildElement (document_element, "hierarchy", DL_INFO), cnamespace);

	// step 4: create and register contexts
	list = xml->getChildElements (document_element, "context", DL_INFO);
	for (XMLChildList::const_iterator it=list.constBegin (); it != list.constEnd (); ++it) {
		QString id = xml->getStringAttribute (*it, "id", QString::null, DL_ERROR);

		RKContextMap *context = getContextLocal (id);
		if (!context) {
			context = new RKContextMap (id);
			contexts.insert (id, context);
		}
		counter += context->create (*it, cnamespace);
	}

	setXMLGUIBuildDocument (gui_xml);
	actionCollection ()->readSettings ();
	return counter;
}

void RKComponentMap::addedEntry (const QString &id, RKComponentHandle *handle) {
	RK_TRACE (PLUGIN);

	if (handle->isPlugin ()) {
		handle->setAccessible (true);
		KAction *action = actionCollection ()->addAction (id, handle, SLOT (activated()));
		action->setText (handle->getLabel ());
		action->setShortcutConfigurable (true);
	}
}

///########################### END RKComponentMap ###############################
///########################### BEGIN RKComponentHandle ############################

#include "rkstandardcomponent.h"

RKComponentHandle::RKComponentHandle (RKPluginMapFile *pluginmap, const QString &rel_filename, const QString &label, RKComponentType type) : QObject (RKWardMainWindow::getMain ()) {
	RK_TRACE (PLUGIN);

	RKComponentHandle::type = type;
	RKComponentHandle::filename = rel_filename;
	RKComponentHandle::label = label;
	RKComponentHandle::plugin_map = pluginmap;

	attributes = 0;
	is_accessible = false;
}

RKComponentHandle::~RKComponentHandle () {
	RK_TRACE (PLUGIN);

	delete attributes;
}

bool RKComponentHandle::isPlugin () {
	if (type != Standard) {
		return false;
	}
	return true;
}

RKStandardComponent *RKComponentHandle::invoke (RKComponent *parent_component, QWidget *parent_widget) {
	RK_TRACE (PLUGIN);
	RK_ASSERT (isPlugin ());

	return (new RKStandardComponent (parent_component, parent_widget, getFilename (), this));
}

void RKComponentHandle::activated () {
	RK_TRACE (PLUGIN);

	invoke (0, 0);
}

QString RKComponentHandle::getAttributeValue (const QString &attribute_id) {
	RK_TRACE (PLUGIN);

	if (!attributes) return QString ();
	AttributeMap::const_iterator it = attributes->find (attribute_id);
	if (it == attributes->constEnd ()) return QString ();
	return ((*it).first);
}

QString RKComponentHandle::getAttributeLabel (const QString &attribute_id) {
	RK_TRACE (PLUGIN);

	if (!attributes) return QString ();
	AttributeMap::const_iterator it = attributes->find (attribute_id);
	if (it == attributes->constEnd ()) return QString ();
	return ((*it).second);
}

bool RKComponentHandle::hasAttribute (const QString &attribute_id) {
	RK_TRACE (PLUGIN);

	if (!attributes) return false;
	return (attributes->contains (attribute_id));
}

void RKComponentHandle::addAttribute (const QString &id, const QString &value, const QString &label) {
	RK_TRACE (PLUGIN);

	if (!attributes) {
		attributes = new AttributeMap;
	}
	AttributeValue value_p (value, label);
	attributes->insert (id, value_p);
}

///########################### END RKComponentHandle ###############################

#include "rkcomponentmap.moc"
