/***************************************************************************
                          rkcomponentmap.cpp  -  description
                             -------------------
    begin                : Thu May 12 2005
    copyright            : (C) 2005-2014 by Thomas Friedrichsmeier
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
#include <QTime>
#include <QObjectCleanupHandler>
#include <QSet>

#include <klocale.h>
#include <kactioncollection.h>
#include <kmessagebox.h>

#include "rkcomponentcontext.h"
#include "rkstandardcomponent.h"
#include "../misc/xmlhelper.h"
#include "../misc/rkcommonfunctions.h"
#include "../debug.h"
#include "../rkglobals.h"
#include "../rkward.h"
#include "../settings/rksettingsmoduleplugins.h"

QString RKPluginMapFile::makeFileName (const QString &filename) {
	return QDir::cleanPath (QDir (basedir).filePath (filename));
}

QString RKPluginMapFile::parseId (const QDomElement& e, XMLHelper &xml) {
	RK_TRACE (PLUGIN);

	return (xml.getStringAttribute (e, "namespace", "rkward", DL_WARNING) + "::" + xml.getStringAttribute (e, "id", QString (), DL_INFO));
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

	gui_xml.setContent (QString ("<!DOCTYPE kpartgui>\n<kpartgui name=\"rkwardcomponents\" version=\"063\">\n<MenuBar>\n\n</MenuBar>\n</kpartgui>"));
	toplevel_menu.subentries.clear ();
	toplevel_menu.type = MenuEntry::Menu;
}

void RKComponentGUIXML::menuItemsToXml (const QList<RKComponentGUIXML::MenuEntry> &entries, QDomElement &xml) {
	// ok, we could really do simply text-pasting, instead of using QDom in this function, but I'm afraid of not getting all character escapes right.
	for (int i = 0; i < entries.size (); ++i) {
		const RKComponentGUIXML::MenuEntry &entry = entries[i];
		if (entry.type == RKComponentGUIXML::MenuEntry::Group) {
			if (entry.label == "-") xml.appendChild (gui_xml.createElement ("Separator"));
			menuItemsToXml (entry.subentries, xml);
			if (entry.label == "-") xml.appendChild (gui_xml.createElement ("Separator"));
		} else if (entry.type == RKComponentGUIXML::MenuEntry::Menu) {
			if (entry.subentries.isEmpty ()) {
				continue;	// just skip over empty menus
			}
			QDomElement submenu = gui_xml.createElement ("Menu");
			submenu.setAttribute ("name", entry.id);
			QDomElement text = gui_xml.createElement ("text");
			text.appendChild (gui_xml.createTextNode (entry.label));
			submenu.appendChild (text);
			menuItemsToXml (entry.subentries, submenu);
			xml.appendChild (submenu);
		} else {
			QDomElement action = gui_xml.createElement ("Action");
			action.setAttribute ("name", entry.id);
			xml.appendChild (action);
		}
	}
}

int RKComponentGUIXML::createMenus (XMLHelper &xml, const QDomElement& hierarchy_description, const QString& cnamespace) {
	RK_TRACE (PLUGIN);

	return (addEntries (&toplevel_menu, xml, hierarchy_description, cnamespace));
}

void RKComponentGUIXML::finalize () {
	RK_TRACE (PLUGIN);

	QDomElement xmlgui_menubar_element = gui_xml.documentElement ().firstChildElement ("MenuBar");
	menuItemsToXml (toplevel_menu.subentries, xmlgui_menubar_element);
}

void insertEntry (RKComponentGUIXML::MenuEntry *parent, const RKComponentGUIXML::MenuEntry &entry) {
	RK_ASSERT (parent && (parent->type == RKComponentGUIXML::MenuEntry::Menu));

	// try to find group
	QList<RKComponentGUIXML::MenuEntry> *list = &(parent->subentries);
	for (int i = 0; i < list->size (); ++i) {
		RKComponentGUIXML::MenuEntry &g = (*list)[i];
		if (g.id == entry.group) {
			if (entry.type == RKComponentGUIXML::MenuEntry::Group) {
				list->insert (++i, entry);
			} else {
				QList<RKComponentGUIXML::MenuEntry> *group_list = &(g.subentries);
				for (int j = 0; j < group_list->size (); ++j) {
					if (QString::localeAwareCompare (group_list->at (j).label, entry.label) > 0) {
						group_list->insert (j, entry);
						return;
					}
				}
				group_list->append (entry);
			}
			return;
		}
	}

	// group not found: Declare it, implicitly, and try again.
	RKComponentGUIXML::MenuEntry new_group;
	new_group.type = RKComponentGUIXML::MenuEntry::Group;
	new_group.id = entry.group;
	if (entry.group == QLatin1String ("top")) {
		list->insert (0, new_group);
	} else if (entry.group == QLatin1String ("bottom")) {
		list->append (new_group);
	} else {
		if (list->isEmpty () || list->last ().group != QLatin1String ("bottom")) {
			list->append (new_group);
		} else {
			list->insert (list->size () - 1, new_group);
		}
	}
	insertEntry (parent, entry);
}

RKComponentGUIXML::MenuEntry *findMenu (RKComponentGUIXML::MenuEntry *parent, const QString id) {
	RK_ASSERT (parent && parent->type == RKComponentGUIXML::MenuEntry::Menu);
	QList<RKComponentGUIXML::MenuEntry> *list = &(parent->subentries);
	for (int i = 0; i < list->size (); ++i) {
		QList<RKComponentGUIXML::MenuEntry> *group_list = &((*list)[i].subentries);
		for (int j = 0; j < group_list->size (); ++j) {
			RKComponentGUIXML::MenuEntry *g = &((*group_list)[j]);
			if (g->id == id) return g;
		}
	}
	return 0;
}

int RKComponentGUIXML::addEntries (RKComponentGUIXML::MenuEntry *menu, XMLHelper &xml, const QDomElement description, const QString& cnamespace) {
	RK_TRACE (PLUGIN);

	int leaves = 0;
	XMLChildList list = xml.getChildElements (description, QString (), DL_INFO);
	for (int i = 0; i < list.size (); ++i) {
		const QString add_to = xml.getStringAttribute (list[i], "group", QString (), DL_INFO);
		if (list[i].tagName () == "menu") {
			QString sub_id = xml.getStringAttribute (list[i], "id", "none", DL_ERROR);
			MenuEntry *found = findMenu (menu, sub_id);
			if (found) {
				leaves += addEntries (found, xml, list[i], cnamespace);
			} else {
				MenuEntry sub;
				sub.id = sub_id;
				sub.label = xml.i18nStringAttribute (list[i], "label", i18n ("(no label)"), DL_WARNING);
				sub.group = add_to;
				sub.type = MenuEntry::Menu;
				leaves += addEntries (&sub, xml, list[i], cnamespace);
				insertEntry (menu, sub);
			}
		} else if (list[i].tagName () == "entry") {
			QString id = cnamespace + xml.getStringAttribute (list[i], "component", "#invalid#", DL_ERROR);

			RKComponentHandle* handle = RKComponentMap::getComponentHandle (id);
			if ((!handle) || (!handle->isPlugin ())) {
				RK_DEBUG (PLUGIN, DL_ERROR, "No such component found while creating menu-entries or component is not a standalone plugin: \"%s\". No entry created.", id.toLatin1 ().data ());
			} else {
				MenuEntry plug;
				plug.id = id;
				plug.label = handle->getLabel ();
				plug.group = add_to;
				plug.type = MenuEntry::Entry;
				insertEntry (menu, plug);
				addedEntry (id, handle);
				++leaves;
			}
		} else if (list[i].tagName () == "group") {
			MenuEntry group;
			group.id = xml.getStringAttribute (list[i], "id", "none", DL_ERROR);
			group.group = add_to;
			if (xml.getBoolAttribute (list[i], "separated", false, DL_INFO)) group.label = "-";
			group.type = MenuEntry::Group;
			insertEntry (menu, group);
		} else {
			RK_ASSERT (false);
		}
	}
	return leaves;
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

RKComponentMap::RKComponentMap () : QObject (), RKComponentGUIXML (), KXMLGUIClient () {
	RK_TRACE (PLUGIN);

	setComponentData (KGlobal::mainComponent ());
	actionCollection ()->setConfigGroup ("Plugin Shortcuts");
}

RKComponentMap::~RKComponentMap () {
	RK_TRACE (PLUGIN);

	clearAll ();
}

void RKComponentMap::clearAll () {
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
	component_attributes.clear ();
	component_dependencies.clear ();

	clearGUIDescription ();

	setXMLGUIBuildDocument (gui_xml);
}

RKContextMap *RKComponentMap::getContext (const QString &id) {
	RK_TRACE (PLUGIN);

	RKContextMap *context = getMap ()->getContextLocal (id);
	if (context) return context;

	RK_DEBUG (PLUGIN, DL_WARNING, "no such context %s", id.toLatin1 ().data ());
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

	RK_DEBUG (PLUGIN, DL_WARNING, "no such component %s", id.toLatin1 ().data ());
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

	RKComponent::PropertyValueMap state;
	bool format_ok = RKComponent::stringListToValueMap (serialized_settings, &state);
	if (!format_ok) {
		_message = i18n ("Bad serialization format while trying to invoke plugin '%1'. Please contact the RKWard team (Help->About RKWard->Authors).").arg (component_id);
		if (message) *message = _message;
		else KMessageBox::error (component, _message, i18n ("Bad serialization format"));
		return false;
	}
	component->applyState (state);

	// now wait for the component to settle
	// the call to processEvents(), below, is quite dangerous, as the component self-destructs on errors. This helps us prevent crashes.
	QObjectCleanupHandler chandler;
	chandler.add (component);
	// if the plugin takes longer than 5 seconds to settle, than that really is sort of buggy...
	const int max_wait = 5000;

	QTime t;
	t.start ();
	RKComponentBase::ComponentStatus status;
	while ((!chandler.isEmpty ()) && ((status = component->recursiveStatus ()) == RKComponentBase::Processing) && (t.elapsed () < max_wait)) {
		QCoreApplication::processEvents (QEventLoop::ExcludeUserInputEvents, (max_wait / 2));
	}
	if (chandler.isEmpty ()) status = RKComponentBase::Dead;
	chandler.remove (component);	// otherwise it would auto-delete the component, later!

	if (status == RKComponentBase::Dead) {
		if (message) {
			_message.append (i18n ("\nThe plugin has crashed."));
			*message = _message;
		}
		return false;
	}

	QStringList problems = component->matchAgainstState (state);
	if (!problems.isEmpty ()) {
		_message = i18n ("Not all specified settings could be applied. Most likely this is because some R objects are no longer present in your current workspace.");
		RK_DEBUG (PLUGIN, DL_WARNING, "%s", qPrintable (problems.join ("\n")));	// TODO: make failures available to backend
		if (message) *message = _message;
		else KMessageBox::informationList (component, _message, problems, i18n ("Not all settings applied"));
		// TODO: Don't show again-box?
		// not considered an error
	}

	if (submit_mode == ManualSubmit) return true;

	if (status == RKComponentBase::Satisfied) {
		bool ok = component->submit (in_chain);
		RK_ASSERT (ok);
	} else {
		if (submit_mode == AutoSubmitOrFail) component->kill ();

		_message.append (i18n ("\nThe plugin could not be auto-submitted with these settings."));
		if (message) *message = _message;
		else KMessageBox::sorry (RKWardMainWindow::getMain (), _message, i18n ("Could not submit"));

		return (submit_mode != AutoSubmitOrFail);
	}

	return true;
}

void RKPluginMapParseResult::addAndPrintError (int level, const QString message) {
	detailed_problems.append (message);
	RK_DEBUG (PLUGIN, level, qPrintable (message));
}

void RKComponentMap::finalizeAll () {
	RK_TRACE (PLUGIN);

	finalize ();
	setXMLGUIBuildDocument (gui_xml);
	actionCollection ()->readSettings ();
	foreach (RKContextMap *ctx, getMap()->contexts) {
		ctx->finalize ();
	}
}

RKPluginMapParseResult RKComponentMap::addPluginMap (const QString& plugin_map_file) {
	RK_TRACE (PLUGIN);

	RKPluginMapParseResult ret;

	QString plugin_map_file_abs = QFileInfo (plugin_map_file).absoluteFilePath ();
	if (pluginmapfiles.contains (plugin_map_file_abs)) {
		RK_DEBUG (PLUGIN, DL_INFO, "Plugin map file '%s' already loaded", plugin_map_file.toLatin1().data ());
		return ret;
	}

	XMLHelper xml (plugin_map_file_abs);
	QDomElement element;
	XMLChildList list;

	QDomElement document_element = xml.openXMLFile (DL_ERROR);
	if (document_element.isNull ()) {
		ret.addAndPrintError (DL_ERROR, i18n ("Could not open plugin map file %1. (Is not readble, or failed to parse)", plugin_map_file_abs));
		return ret;
	}

	QString prefix = QFileInfo (plugin_map_file_abs).absolutePath() + '/' + xml.getStringAttribute (document_element, "base_prefix", QString::null, DL_INFO);
	QString cnamespace = xml.getStringAttribute (document_element, "namespace", "rkward", DL_INFO) + "::";

	RKPluginMapFile *pluginmap_file_desc = new RKPluginMapFile (prefix, xml.messageCatalog ());
	pluginmap_file_desc->id = RKPluginMapFile::parseId (document_element, xml);
	pluginmapfiles.insert (QFileInfo (plugin_map_file).absoluteFilePath (), pluginmap_file_desc);

	// step 0: check dependencies, parse about, and initialize
	QDomElement dependencies = xml.getChildElement (document_element, "dependencies", DL_INFO);
	if (!dependencies.isNull ()) {
		if (!RKComponentDependency::isRKWardVersionCompatible (dependencies)) {
			ret.addAndPrintError (DL_WARNING, i18n ("Skipping plugin map file '%1': Not compatible with this version of RKWard", plugin_map_file_abs));
			return ret;
		}
		pluginmap_file_desc->dependencies = RKComponentDependency::parseDependencies (dependencies, xml);
	}

	// step 1: include required files
	QStringList includelist;
	list = xml.getChildElements (document_element, "require", DL_INFO);
	for (XMLChildList::const_iterator it=list.constBegin (); it != list.constEnd (); ++it) {
		if ((*it).hasAttribute ("file")) {
			QString file = pluginmap_file_desc->makeFileName (xml.getStringAttribute (*it, "file", QString (), DL_ERROR));
			if (QFileInfo (file).isReadable ()) {
				includelist.append (file);
			} else {
				ret.addAndPrintError (DL_ERROR, i18n ("Specified required file '%1' does not exist or is not readable. Ignoring.", file));
			}
		} else {
			QString map_id = xml.getStringAttribute (*it, "map", QString (), DL_ERROR);
			// Try to locate the map among the already loaded files, first
			QString file;
			for (PluginMapFileMap::const_iterator pmit = pluginmapfiles.constBegin (); pmit != pluginmapfiles.constEnd (); ++pmit) {
				if (pmit.value ()->id == map_id) {
					file = pmit.key ();
					break;
				}
			}
			// If the map is not among the loaded files, try to look it up among all known pluginmaps
			if (file.isEmpty ()) file = RKSettingsModulePlugins::findPluginMapById (map_id);
			if (!file.isEmpty ()) {
				RK_DEBUG (PLUGIN, DL_INFO, "Resolving plugin map specification %s to filename %s", qPrintable (map_id), qPrintable (file));
				includelist.append (file);
			} else {
				ret.addAndPrintError (DL_ERROR, i18n ("Could not resolve specified required pluginmap '%1'. You may have to install additional packages.", map_id));
			}
		}
	}
	for (QStringList::const_iterator it = includelist.constBegin (); it != includelist.constEnd (); ++it) {
		ret.add (addPluginMap (*it));
	}

	// step 2: create (list of) components
	element = xml.getChildElement (document_element, "components", DL_INFO);
	list = xml.getChildElements (element, "component", DL_INFO);
	// Plugins that depend on a specific version of RKWard can be specified in several alternative version.
	// It is not an error, unless *none* of the specified alternatives can be loaded.
	QSet<QString> local_components;
	QSet<QString> depfailed_local_components;

	for (XMLChildList::const_iterator it=list.begin (); it != list.end (); ++it) {
		QString id = cnamespace + xml.getStringAttribute((*it), "id", QString::null, DL_WARNING);

		// check dependencies, first
		QDomElement cdependencies = xml.getChildElement (*it, "dependencies", DL_INFO);
		if (!cdependencies.isNull ()) {
			if (!RKComponentDependency::isRKWardVersionCompatible (cdependencies)) {
				RK_DEBUG (PLUGIN, DL_INFO, "Skipping component '%1': Not compatible with this version of RKWard", qPrintable (id));
				depfailed_local_components.insert (id);
				continue;
			}
		}

		QString filename = xml.getStringAttribute((*it), "file", QString (), DL_WARNING);
		int type = xml.getMultiChoiceAttribute ((*it), "type", "standard", 0, DL_WARNING);
		QString label = xml.i18nStringAttribute ((*it), "label", i18n ("(no label)"), DL_WARNING);

		if (components.contains (id)) {
			ret.addAndPrintError (DL_WARNING, i18n ("RKComponentMap already contains a component with id \"%1\". Ignoring second entry.", id));
		} else if (!QFileInfo (pluginmap_file_desc->makeFileName (filename)).isReadable ()) {
			ret.addAndPrintError (DL_ERROR, i18n ("Specified file '%1' for component id \"%2\" does not exist or is not readable. Ignoring.", filename, id));
		} else {
			// create and initialize component handle
			RKComponentHandle *handle = new RKComponentHandle (pluginmap_file_desc, filename, label, (RKComponentType) type);
			XMLChildList attributes_list = xml.getChildElements (*it, "attribute", DL_DEBUG);
			for (XMLChildList::const_iterator ait=attributes_list.begin (); ait != attributes_list.end (); ++ait) {
				handle->addAttribute (xml.getStringAttribute (*ait, "id", "noid", DL_WARNING), xml.getStringAttribute (*ait, "value", QString (), DL_ERROR), xml.i18nStringAttribute (*ait, "label", QString (), DL_ERROR));
			}
			if (!cdependencies.isNull ()) handle->addDependencies (RKComponentDependency::parseDependencies (cdependencies, xml));
			components.insert (id, handle);
			local_components.insert (id);
		}
	}

	foreach (const QString &id, depfailed_local_components) {
		if (local_components.contains (id)) continue;
		ret.addAndPrintError (DL_ERROR, i18n ("Component '%1' is not available in a version compatible with this version of RKWard", id));
	}

	// step 3: create / insert into menus
	ret.valid_plugins += createMenus (xml, xml.getChildElement (document_element, "hierarchy", DL_INFO), cnamespace);

	// step 4: create and register contexts
	list = xml.getChildElements (document_element, "context", DL_INFO);
	for (XMLChildList::const_iterator it=list.constBegin (); it != list.constEnd (); ++it) {
		QString id = xml.getStringAttribute (*it, "id", QString::null, DL_ERROR);

		RKContextMap *context = getContextLocal (id);
		if (!context) {
			context = new RKContextMap (id);
			contexts.insert (id, context);
		}
		ret.valid_plugins += context->createMenus (xml, *it, cnamespace);
	}

	return ret;
}

void RKComponentMap::activateComponent () {
	RK_TRACE (PLUGIN);

	if (!sender ()) {
		RK_ASSERT (sender ());
		return;
	}
	RKComponentHandle *handle = getComponentHandleLocal (sender ()->objectName ());
	if (!handle) {
		RK_ASSERT (handle);
		return;
	}
	handle->invoke (0, 0);
}

void RKComponentMap::addedEntry (const QString &id, RKComponentHandle *handle) {
	RK_TRACE (PLUGIN);

	if (handle->isPlugin ()) {
		handle->setAccessible (true);
		KAction *action = actionCollection ()->addAction (id, this, SLOT (activateComponent()));
		action->setText (handle->getLabel ());
		action->setShortcutConfigurable (true);
	}
}

///########################### END RKComponentMap ###############################
///########################### BEGIN RKComponentHandle ############################

#include "rkstandardcomponent.h"

RKComponentHandle::RKComponentHandle (RKPluginMapFile *pluginmap, const QString &rel_filename, const QString &label, RKComponentType type) {
	RK_TRACE (PLUGIN);

	RKComponentHandle::type = type;
	RKComponentHandle::filename = rel_filename;
	RKComponentHandle::label = label;
	RKComponentHandle::plugin_map = pluginmap;

	is_accessible = false;
}

RKComponentHandle::~RKComponentHandle () {
	RK_TRACE (PLUGIN);
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

QString RKComponentHandle::getAttributeValue (const QString &attribute_id) {
	RK_TRACE (PLUGIN);

	QMap<QString, RKComponentMap::AttributeValueMap>::const_iterator it = RKComponentMap::getMap ()->component_attributes.find (attribute_id);
	if (it == RKComponentMap::getMap ()->component_attributes.constEnd ()) return QString ();
	return (*it).valuemap.value (this);
}

QString RKComponentHandle::getAttributeLabel (const QString &attribute_id) {
	RK_TRACE (PLUGIN);

	QMap<QString, RKComponentMap::AttributeValueMap>::const_iterator it = RKComponentMap::getMap ()->component_attributes.find (attribute_id);
	if (it == RKComponentMap::getMap ()->component_attributes.constEnd ()) return QString ();
	return (*it).labelmap.value (this);
}

void RKComponentHandle::addAttribute (const QString &id, const QString &value, const QString &label) {
	RK_TRACE (PLUGIN);

	RKComponentMap::AttributeValueMap & map = RKComponentMap::getMap ()->component_attributes[id];	// NOTE: auto-created, if needed
	map.valuemap.insert (this, value);
	map.labelmap.insert (this, label);
}

void RKComponentHandle::addDependencies (const QList<RKComponentDependency>& deps) {
	if (deps.isEmpty ()) return;
	RK_TRACE (PLUGIN);

	RKComponentMap::getMap ()->component_dependencies[this].append (deps);
}

QList <RKComponentDependency> RKComponentHandle::getDependencies () {
	RK_TRACE (PLUGIN);

	QList <RKComponentDependency> ret = plugin_map->getDependencies ();
	QHash<RKComponentHandle*, QList<RKComponentDependency> >::const_iterator it = RKComponentMap::getMap ()->component_dependencies.find (this);
	if (it == RKComponentMap::getMap ()->component_dependencies.constEnd ()) return ret;
	return (ret + (*it));
}

QString RKComponentHandle::getPluginmapFilename () {
	RK_TRACE (PLUGIN);

	return RKComponentMap::getMap ()->pluginmapfiles.key (plugin_map);
}

///########################### END RKComponentHandle ###############################

#include "rkcomponentmap.moc"
