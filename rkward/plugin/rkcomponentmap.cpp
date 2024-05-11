/*
rkcomponentmap.cpp - This file is part of RKWard (https://rkward.kde.org). Created: Thu May 12 2005
SPDX-FileCopyrightText: 2005-2022 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "rkcomponentmap.h"

#include <qfileinfo.h>
#include <qdir.h>
#include <QElapsedTimer>
#include <QObjectCleanupHandler>
#include <QSet>
#include <QGuiApplication>

#include <KLocalizedString>
#include <kactioncollection.h>
#include <kmessagebox.h>

#include "rkcomponentcontext.h"
#include "rkstandardcomponent.h"
#include "../misc/xmlhelper.h"
#include "../misc/rkcommonfunctions.h"
#include "../debug.h"
#include "../rkward.h"
#include "../settings/rksettingsmoduleplugins.h"
#include "../rbackend/rksessionvars.h"
#include "../misc/rkparsedversion.h"
#include "../dialogs/rkerrordialog.h"

QString RKPluginMapFile::makeFileName (const QString &filename) const {
	return QDir::cleanPath (QDir (basedir).filePath (filename));
}

QString RKPluginMapFile::parseId (const QDomElement& e, XMLHelper &xml) {
	RK_TRACE (PLUGIN);

	return (xml.getStringAttribute (e, "namespace", "rkward", DL_WARNING) + "::" + xml.getStringAttribute (e, "id", QString (), DL_INFO));
}

RKComponentAboutData RKPluginMapFile::getAboutData () {
	RK_TRACE (PLUGIN);

	if (about) return *about;
	XMLHelper xml (filename);
	QDomElement element = xml.openXMLFile (DL_ERROR);
	about = new RKComponentAboutData (xml.getChildElement (element, "about", DL_INFO), xml);	// might be empty, but not null
	return *about;
}

RKComponentGUIXML::RKComponentGUIXML (const QString &context_id) {
	RK_TRACE (PLUGIN);

	context = context_id;
	clearGUIDescription ();
}

RKComponentGUIXML::~RKComponentGUIXML () {
	RK_TRACE (PLUGIN);

	toplevel_menu.clear ();
}

void RKComponentGUIXML::clearGUIDescription () {
	RK_TRACE (PLUGIN);

	gui_xml.setContent (QString ("<!DOCTYPE kpartgui>\n<kpartgui name=\"rkwardcomponents\" version=\"063\">\n<MenuBar>\n\n</MenuBar>\n</kpartgui>"));
	toplevel_menu.clear ();
	component_menus.clear ();
}

bool compareMenuEntries (const RKComponentGUIXML::Entry *a, const RKComponentGUIXML::Entry *b) {
	return (QString::localeAwareCompare (a->label, b->label) < 0);
}

void RKComponentGUIXML::resolveComponentLabelsAndSortMenu (Menu *menu, const QString &menu_path) {
	RK_TRACE (PLUGIN);

	for (int i = 0; i < menu->groups.size (); ++i) {
		Group *group = menu->groups[i];
		for (int j = 0; j < group->entries.size (); ++j) {
			Entry *entry = group->entries[j];
			if (!entry->is_menu) {
				RKComponentHandle* handle = RKComponentMap::getComponentHandle (entry->id);
				if (!handle) {
					RK_DEBUG (PLUGIN, DL_ERROR, "No such component found while creating menu-entries or component is not a standalone plugin: \"%s\". No entry created.", qPrintable (entry->id));
					delete (group->entries.takeAt (j));
					--j;
					continue;
				} else {
					// The reason that handling of label is delayed to this point is that if a plugin is overridden, we want to use the label specified for the effective plugin (which might have changed WRT the overridden plugin, too).
					entry->label = handle->getLabel ();
					addedEntry (entry->id, handle);
					component_menus.insert (entry->id, menu_path);
				}
			} else {
				resolveComponentLabelsAndSortMenu (static_cast<Menu*> (entry), menu_path.isEmpty () ? entry->label : menu_path + '\t' + entry->label);
			}
		}
		std::sort(group->entries.begin(), group->entries.end(), compareMenuEntries);
	}
}

void RKComponentGUIXML::menuItemsToXml (const RKComponentGUIXML::Menu *menu, QDomElement &xml) {
	RK_TRACE (PLUGIN);

	// ok, we could really do simply text-pasting, instead of using QDom in this function, but I'm afraid of not getting all character escapes right.
	for (int i = 0; i < menu->groups.size (); ++i) {
		const RKComponentGUIXML::Group *group = menu->groups[i];
		if (group->separated) xml.appendChild (gui_xml.createElement ("Separator"));
		for (int j = 0; j < group->entries.size (); ++j) {
			const RKComponentGUIXML::Entry *entry = group->entries[j];
			if (entry->is_menu) {
				const RKComponentGUIXML::Menu *submenu = static_cast<const RKComponentGUIXML::Menu*> (entry);
				if (submenu->groups.isEmpty ()) {
					continue;	// just skip over empty menus
				}

				QDomElement elem = gui_xml.createElement ("Menu");
				elem.setAttribute ("name", submenu->id);
				QDomElement text = gui_xml.createElement ("text");
				text.appendChild (gui_xml.createTextNode (submenu->label));
				elem.appendChild (text);
				menuItemsToXml (submenu, elem);
				xml.appendChild (elem);
			} else {
				QDomElement action = gui_xml.createElement ("Action");
				action.setAttribute ("name", entry->id);
				xml.appendChild (action);
			}
		}
		if (group->separated) xml.appendChild (gui_xml.createElement ("Separator"));
	}
}

int RKComponentGUIXML::createMenus (XMLHelper &xml, const QDomElement& hierarchy_description, const QString& cnamespace) {
	RK_TRACE (PLUGIN);

	return (addEntries (&toplevel_menu, xml, hierarchy_description, cnamespace));
}

void RKComponentGUIXML::finalize () {
	RK_TRACE (PLUGIN);

	QDomElement xmlgui_menubar_element = gui_xml.documentElement ().firstChildElement ("MenuBar");
	resolveComponentLabelsAndSortMenu (&toplevel_menu);
	menuItemsToXml (&toplevel_menu, xmlgui_menubar_element);
	toplevel_menu.clear ();	// no longer needed
}

RKComponentGUIXML::Group::~Group () {
	for (int i = 0; i < entries.size (); ++i) {
		if (entries[i]->is_menu) delete (static_cast<RKComponentGUIXML::Menu*> (entries[i]));   // NOTE: No virtual d'tor in base-class Entry
		else delete (entries[i]);
	}
}

void RKComponentGUIXML::Menu::clear () {
	for (int i = 0; i < groups.size (); ++i) {
		delete (groups[i]);
	}
	groups.clear ();
}

RKComponentGUIXML::Group *findOrCreateGroup (RKComponentGUIXML::Menu *parent, const QString &group) {
	// try to find group
	int bottom_index = -1;
	QList<RKComponentGUIXML::Group*> &list = parent->groups;
	for (int i = 0; i < list.size (); ++i) {
		if (list[i]->id == group) {
			return list[i];
		}
		if (list[i]->id == QLatin1String ("bottom")) {
			bottom_index = i;
		}
	}

	RK_TRACE (PLUGIN);

	RKComponentGUIXML::Group *new_group = new RKComponentGUIXML::Group ();
	new_group->id = group;
	if (group == QLatin1String ("top")) {
		list.insert (0, new_group);
	} else if (group == QLatin1String ("bottom")) {
		list.append (new_group);
	} else {
		if (bottom_index < 0) {	// no "bottom" is defined, yet
			list.append (new_group);
		} else {	// a bottom group already exists, add new group _above_ that
			list.insert (bottom_index, new_group);
		}
	}
	return new_group;
}

void insertGroup (RKComponentGUIXML::Menu *parent, const QString &group_id, bool separated, const QString &after_group) {
	RK_TRACE (PLUGIN);
	RK_ASSERT (parent);

	// does the group already exist?
	QList<RKComponentGUIXML::Group*> &groups = parent->groups;
	for (int i = 0; i < groups.size (); ++i) {
		if (groups[i]->id == group_id) {
			if (separated) {
				// if group was defined, implicitly, before, it will be lacking a separator
				groups[i]->separated = true;
			}
			return;
		}
	}

	RKComponentGUIXML::Group* found = findOrCreateGroup (parent, after_group);
	if (after_group != group_id) {         // after_group == group_id may happen for implicitly created group "", for example
		RKComponentGUIXML::Group* group = new RKComponentGUIXML::Group ();
		group->id = group_id;
		group->separated = separated;
		groups.insert (parent->groups.indexOf (found) + 1, group);
	} else if (separated) {
		found->separated = true;
	}
}

void insertEntry (RKComponentGUIXML::Menu *parent, RKComponentGUIXML::Entry *entry, const QString &in_group) {
	RK_TRACE (PLUGIN);
	RK_ASSERT (parent);

	RKComponentGUIXML::Group *group = findOrCreateGroup (parent, in_group);
	group->entries.append (entry);
	if (!entry->is_menu) {
		parent->components.insert (entry->id);
	}
}

RKComponentGUIXML::Menu *findMenu (RKComponentGUIXML::Menu *parent, const QString &id) {
	RK_TRACE (PLUGIN);

	RK_ASSERT (parent);
	QList<RKComponentGUIXML::Group*> list = parent->groups;
	for (int i = 0; i < list.size (); ++i) {
		QList<RKComponentGUIXML::Entry*> group_list = list[i]->entries;
		for (int j = 0; j < group_list.size (); ++j) {
			RKComponentGUIXML::Entry *e = group_list[j];
			if (e->is_menu && (e->id == id)) return static_cast<RKComponentGUIXML::Menu*> (e);
		}
	}
	return nullptr;
}

int RKComponentGUIXML::addEntries (RKComponentGUIXML::Menu *menu, XMLHelper &xml, const QDomElement &description, const QString& cnamespace) {
	RK_TRACE (PLUGIN);

	int leaves = 0;
	XMLChildList list = xml.getChildElements (description, QString (), DL_INFO);
	for (int i = 0; i < list.size (); ++i) {
		const QString add_to = xml.getStringAttribute (list[i], "group", QString (), DL_INFO);
		if (list[i].tagName () == "menu") {
			QString sub_id = xml.getStringAttribute (list[i], "id", "none", DL_ERROR);

			Menu *found = findMenu (menu, sub_id);
			if (found) {
				leaves += addEntries (found, xml, list[i], cnamespace);
			} else {
				Menu *sub = new Menu ();
				sub->id = sub_id;
				sub->label = xml.i18nStringAttribute (list[i], "label", i18n ("(no label)"), DL_WARNING);
				leaves += addEntries (sub, xml, list[i], cnamespace);
				insertEntry (menu, sub, add_to);
			}
		} else if (list[i].tagName () == "entry") {
			QString id = cnamespace + xml.getStringAttribute (list[i], "component", "#invalid#", DL_ERROR);
			if (menu->components.contains (id)) {
				RK_DEBUG (PLUGIN, DL_INFO, "Component %s is registered more than once in menu %s. Keeping one entry, only", qPrintable (id), qPrintable (menu->id));
				continue;
			}

			// check if there is an override hiding this plugin (TODO: what if there is more than one override?)
			bool hidden = false;
			OverrideMap::const_iterator ov = overrides.constFind (id);
			while (ov != overrides.constEnd () && ov.key () == id) {
				const ComponentOverride &over = ov.value ();
				if (over.context.isEmpty () || over.context == context) {
					if (over.hidden) {
						hidden = true;
						break;
					}
				}
				++ov;
			}
			if (hidden) continue;

			Entry *plug = new Entry ();
			plug->id = id;
			insertEntry (menu, plug, add_to);
			++leaves;
		} else if (list[i].tagName () == "group") {
			insertGroup (menu, xml.getStringAttribute (list[i], "id", "none", DL_ERROR), xml.getBoolAttribute (list[i], "separated", false, DL_INFO), add_to);
		} else {
			RK_ASSERT (false);
		}
	}
	return leaves;
}

// static
QMultiMap<QString, RKComponentGUIXML::ComponentOverride> RKComponentGUIXML::overrides;
void RKComponentGUIXML::addOverride (const QString& id, const QString& context, bool visible) {

	OverrideMap::iterator ov = overrides.find (id);
	while (ov != overrides.end () && ov.key () == id) {
		const ComponentOverride &over = ov.value ();
		if (over.context == context) {
			overrides.erase (ov);
			break;
		}
		++ov;
	}

	ComponentOverride over;
	over.context = context;
	over.hidden = !visible;
	overrides.insert (id, over);
}

void RKComponentGUIXML::clearOverrides () {
	overrides.clear ();
}

/////////////////////////// END RKComponentXMLGUIClient /////////////////////////////////
////////////////////////////// Bhttp://apps.sourceforge.net/mediawiki/rkward/nfs/project/r/rk/rkward/6/6d/RKWardApplicationDetached.pngEGIN RKComponentMap /////////////////////////////////////

// static members
RKComponentMap *RKComponentMap::component_map = nullptr;

void RKComponentMap::initialize () {
	RK_TRACE (PLUGIN);

	RK_ASSERT (component_map == nullptr);
	component_map = new RKComponentMap ();
}

RKComponentMap::RKComponentMap () : QObject (), RKComponentGUIXML ("global"), KXMLGUIClient () {
	RK_TRACE (PLUGIN);

	setComponentName (QCoreApplication::applicationName (), QGuiApplication::applicationDisplayName ());
	actionCollection ()->setConfigGroup ("Plugin Shortcuts");
	contexts.insert ("global", this);
}

RKComponentMap::~RKComponentMap () {
	RK_TRACE (PLUGIN);

	clearAll ();
}

void RKComponentMap::clearAll () {
	RK_TRACE (PLUGIN);

	actionCollection ()->clear ();
	for (ComponentMap::const_iterator it = components.constBegin (); it != components.constEnd (); ++it) {
		delete (it.value ());
/* TODO: this is not technically correct, as there may be several actions for this id, and we're only deleting one. But practically this should not really be relevant. */
		delete (actionCollection ()->action (it.key ()));
	}
	components.clear ();

	for (int i = 0; i < pluginmapfiles.size (); ++i) {
		delete (pluginmapfiles[i]);
	}
	pluginmapfiles.clear ();
	component_attributes.clear ();
	component_dependencies.clear ();
	for (RKComponentContextMap::const_iterator it = contexts.constBegin (); it != contexts.constEnd (); ++it) {
		if (it.value () != this) delete (it.value ());
	}
	contexts.clear ();
	contexts.insert ("global", this);

	clearGUIDescription ();

	setXMLGUIBuildDocument (gui_xml);
}

RKComponentGUIXML *RKComponentMap::getContext (const QString &id) {
	RK_TRACE (PLUGIN);

	RKComponentGUIXML *context = getMap ()->contexts.value (id);
	if (context) return context;

	RK_DEBUG (PLUGIN, DL_WARNING, "no such context %s", id.toLatin1 ().data ());
	return nullptr;
}

RKComponentHandle* RKComponentMap::getComponentHandle (const QString &id) {
	RK_TRACE (PLUGIN);

	RKComponentHandle *handle = getMap ()->getComponentHandleLocal (id);
	if (handle) return handle;

	RK_DEBUG (PLUGIN, DL_WARNING, "no such component %s", id.toLatin1 ().data ());
	return nullptr;
}

RKComponentHandle* RKComponentMap::getComponentHandleLocal (const QString &id) {
	RK_TRACE (PLUGIN);

	QList<RKComponentHandle*> candidates = components.values (id);
	if (candidates.isEmpty ()) return nullptr;
	if (candidates.length () == 1) return candidates.first ();

	RK_DEBUG (PLUGIN, DL_INFO, "Looking for latest version of component %s, among %d candidates", qPrintable (id), candidates.size ());
	RKComponentHandle* candidate = candidates.first ();
	auto vera = RKParsedVersion(candidate->getAboutData().version);
	for (int i = 1; i < candidates.size (); ++i) {
		auto verb = RKParsedVersion(candidates[i]->getAboutData ().version);
		if (verb > vera) {
			candidate = candidates[i];
			vera = verb;
		}
	}
	// purge inferior components to avoid future version lookups
	RK_DEBUG (PLUGIN, DL_INFO, "Latest version is '%s'. Forgetting about the others.", qPrintable (candidate->getFilename ()));
	components.remove (id);
	components.insert (id, candidate);
	for (int i = 0; i < candidates.size (); ++i) {
		if (candidates[i] != candidate) delete candidates[i];
	}

	return candidate;
}

//static
QString RKComponentMap::getComponentId (RKComponentHandle* by_component) {
	RK_TRACE (PLUGIN);

	return (getMap ()->getComponentIdLocal (by_component));
}

QString RKComponentMap::getComponentIdLocal (RKComponentHandle* component) {
	RK_TRACE (PLUGIN);

	QString component_found = components.key (component, QString ());
	RK_ASSERT (!component_found.isNull ());
	return (component_found);
}

bool RKComponentMap::isPluginMapLoaded (const QString& abs_filename) const {
	RK_TRACE (PLUGIN);

	for (int i = 0; i < pluginmapfiles.size (); ++i) {
		if (pluginmapfiles[i]->filename == abs_filename) return true;
	}
	return false;
}

//static
bool RKComponentMap::invokeComponent (const QString &component_id, const QStringList &serialized_settings, ComponentInvocationMode submit_mode, QString *message, RCommandChain *in_chain) {
	RK_TRACE (PLUGIN);

	QString _message;
	RKComponentHandle *handle = getComponentHandle (component_id);
	if (!handle) {
		_message = i18n ("You tried to invoke a plugin called '%1', but that plugin is currently unknown. Probably you need to load the corresponding PluginMap (Settings->Configure RKWard->Plugins), or perhaps the plugin was renamed.", component_id);
		if (message) *message = _message;
		else KMessageBox::error(RKWardMainWindow::getMain(), _message, i18n("No such plugin"));
		return false;
	}

	RKStandardComponent *component = handle->invoke (nullptr, nullptr);
	RK_ASSERT (component);

	RKComponent::PropertyValueMap state;
	bool format_ok = RKComponent::stringListToValueMap (serialized_settings, &state);
	if (!format_ok) {
		_message = i18n ("Bad serialization format while trying to invoke plugin '%1'. In general, this should not happen, unless you modified the parameters by hand. Please consider reporting this issue.", component_id);
		if (message) *message = _message;
		else RKErrorDialog::reportableErrorMessage (component, _message, QString (), i18n ("Bad serialization format"), "invoke_comp_deserialization_error");
		return false;
	}
	component->applyState (state);

	// now wait for the component to settle
	// the call to processEvents(), below, is quite dangerous, as the component self-destructs on errors. This helps us prevent crashes.
	QObjectCleanupHandler chandler;
	chandler.add (component);
	// if the plugin takes longer than 5 seconds to settle, than that really is sort of buggy...
	const int max_wait = 5000;

	QElapsedTimer t;
	t.start();
	RKComponentBase::ComponentStatus status;
	while ((!chandler.isEmpty()) && ((status = component->recursiveStatus()) == RKComponentBase::Processing) && (t.elapsed() < max_wait)) {
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
		else KMessageBox::error(RKWardMainWindow::getMain(), _message, i18n("Could not submit"));

		return (submit_mode != AutoSubmitOrFail);
	}

	return true;
}

void RKPluginMapParseResult::addAndPrintError (int level, const QString &message) {
	detailed_problems.append (message);
	RK_DEBUG (PLUGIN, level, qPrintable (message));
}

void RKComponentMap::finalizeAll () {
	RK_TRACE (PLUGIN);

	finalize ();
	setXMLGUIBuildDocument (gui_xml);
	actionCollection ()->readSettings ();
	const auto contexts = getMap()->contexts;
	for (RKComponentGUIXML *ctx : contexts) {
		ctx->finalize ();
	}
}

RKPluginMapParseResult RKComponentMap::addPluginMap (const QString& plugin_map_file) {
	RK_TRACE (PLUGIN);

	RKPluginMapParseResult ret;

	QString plugin_map_file_abs = QFileInfo (plugin_map_file).absoluteFilePath ();
	if (isPluginMapLoaded (plugin_map_file_abs)) {
		RK_DEBUG (PLUGIN, DL_INFO, "Plugin map file '%s' already loaded", plugin_map_file.toLatin1().data ());
		return ret;
	}

	XMLHelper xml (plugin_map_file_abs);
	QDomElement element;
	XMLChildList list;

	QDomElement document_element = xml.openXMLFile (DL_ERROR);
	if (document_element.isNull ()) {
		ret.addAndPrintError (DL_ERROR, i18n ("Could not open plugin map file %1. (Is not readable, or failed to parse)", plugin_map_file_abs));
		return ret;
	}

	QString prefix = QFileInfo (plugin_map_file_abs).absolutePath() + '/' + xml.getStringAttribute (document_element, "base_prefix", QString (), DL_INFO);
	QString cnamespace = xml.getStringAttribute (document_element, "namespace", "rkward", DL_INFO) + "::";

	RKPluginMapFile* pluginmap_file_desc = new RKPluginMapFile (QFileInfo (plugin_map_file).absoluteFilePath (), prefix, xml.messageCatalog ());
	pluginmap_file_desc->id = RKPluginMapFile::parseId (document_element, xml);
	pluginmapfiles.append (pluginmap_file_desc);

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
			for (int i = 0; i < pluginmapfiles.size (); ++i) {
				if (pluginmapfiles[i]->id == map_id) {
					file = pluginmapfiles[i]->filename;
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

	for (XMLChildList::const_iterator it=list.cbegin (); it != list.cend (); ++it) {
		QString id = cnamespace + xml.getStringAttribute((*it), "id", QString (), DL_WARNING);

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
		xml.getMultiChoiceAttribute ((*it), "type", "standard", 0, DL_WARNING);	// unused, but documented for future extension; TODO: remove?
		QString label = xml.i18nStringAttribute ((*it), "label", i18n ("(no label)"), DL_WARNING);

		if (local_components.contains (id)) {
			ret.addAndPrintError (DL_WARNING, i18n ("Duplicate declaration of component id \"%1\" within pluginmap file \"%2\".", id, plugin_map_file_abs));
		}
		if (components.contains (id)) {
			RK_DEBUG (PLUGIN, DL_INFO, "RKComponentMap already contains a component with id \"%1\". Highest version will be picked at runtime.", qPrintable (id));
		}
		if (!QFileInfo (pluginmap_file_desc->makeFileName (filename)).isReadable ()) {
			ret.addAndPrintError (DL_ERROR, i18n ("Specified file '%1' for component id \"%2\" does not exist or is not readable. Ignoring.", filename, id));
		} else {
			// create and initialize component handle
			RKComponentHandle *handle = new RKComponentHandle (pluginmap_file_desc, filename, label);
			XMLChildList attributes_list = xml.getChildElements (*it, "attribute", DL_DEBUG);
			for (XMLChildList::const_iterator ait=attributes_list.cbegin (); ait != attributes_list.cend (); ++ait) {
				handle->addAttribute (xml.getStringAttribute (*ait, "id", "noid", DL_WARNING), xml.getStringAttribute (*ait, "value", QString (), DL_ERROR), xml.i18nStringAttribute (*ait, "label", QString (), DL_ERROR));
			}
			if (!cdependencies.isNull ()) handle->addDependencies (RKComponentDependency::parseDependencies (cdependencies, xml));
			components.insert (id, handle);
			local_components.insert (id);
		}
	}

	for (const QString &id : std::as_const(depfailed_local_components)) {
		if (local_components.contains (id)) continue;
		ret.addAndPrintError (DL_ERROR, i18n ("Component '%1' is not available in a version compatible with this version of RKWard", id));
	}

	// step 3: create / insert into menus
	ret.valid_plugins += createMenus (xml, xml.getChildElement (document_element, "hierarchy", DL_INFO), cnamespace);

	// step 4: create and register contexts
	list = xml.getChildElements (document_element, "context", DL_INFO);
	for (XMLChildList::const_iterator it=list.constBegin (); it != list.constEnd (); ++it) {
		QString id = xml.getStringAttribute (*it, "id", QString (), DL_ERROR);

		RKComponentGUIXML *context = contexts.value (id);
		if (!context) {
			context = new RKComponentGUIXML (id);
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
	handle->invoke(nullptr, nullptr);
}

void RKComponentMap::addedEntry (const QString &id, RKComponentHandle *handle) {
	RK_TRACE (PLUGIN);

	handle->setAccessible (true);
	QAction *action = actionCollection()->addAction(id, this, &RKComponentMap::activateComponent);
	action->setText (handle->getLabel ());
	actionCollection ()->setShortcutsConfigurable (action, true);
}

void RKComponentGUIXML::appendPluginToList (const QString& id, QStringList* list) {
	list->append (id);
	list->append (context);
	list->append (component_menus.value (id));
	RKComponentHandle *handle = RKComponentMap::getComponentHandle (id);
	if (handle) list->append (handle->getLabel ());
	else {
		RK_ASSERT (handle);
		list->append (QString ());
	}
}

QStringList RKComponentMap::listPlugins () {
	RK_TRACE (PLUGIN);

	QStringList ret;
	ret.reserve (components.size () * 4);
	for (ComponentMap::const_iterator it = components.constBegin (); it != components.constEnd (); ++it) {
		// RKComponentMap (in contrast to other contexts) will also contain plugins not added to the menu,
		// and is listed separately, for this reason
		getMap ()->appendPluginToList (it.key (), &ret);
	}
	for (RKComponentContextMap::const_iterator ctx = contexts.constBegin (); ctx != contexts.constEnd (); ++ctx) {
		RKComponentGUIXML *context = ctx.value ();
		if (context == getMap ()) continue;

		QStringList ids = ctx.value ()->components ();
		for (int i = 0; i < ids.size (); ++i) {
			context->appendPluginToList (ids[i], &ret);
		}
	}
	return ret;
}

void RKComponentMap::setPluginStatus (const QStringList& ids, const QStringList& _contexts, const QStringList& _visible) {
	RK_TRACE (PLUGIN);
	RK_ASSERT (ids.size () == _contexts.size ());
	RK_ASSERT (_contexts.size () == _visible.size ());

	for (int i = 0; i < ids.size (); ++i) {
		addOverride (ids[i], _contexts[i], (_visible[i] == "1"));
	}

	RKWardMainWindow::getMain ()->initPlugins ();
}


///########################### END RKComponentMap ###############################
///########################### BEGIN RKComponentHandle ############################

RKComponentHandle::RKComponentHandle (RKPluginMapFile *pluginmap, const QString &rel_filename, const QString &label) {
	RK_TRACE (PLUGIN);

	RKComponentHandle::filename = rel_filename;
	RKComponentHandle::label = label;
	RKComponentHandle::plugin_map = pluginmap;

	is_accessible = false;
}

RKComponentHandle::~RKComponentHandle () {
	RK_TRACE (PLUGIN);
}

RKStandardComponent *RKComponentHandle::invoke (RKComponent *parent_component, QWidget *parent_widget) {
	RK_TRACE (PLUGIN);

	return (new RKStandardComponent (parent_component, parent_widget, getFilename (), RKComponentMap::getComponentId (this)));
}

QString RKComponentHandle::getAttributeValue (const QString &attribute_id) {
	RK_TRACE (PLUGIN);

	QMap<QString, RKComponentMap::AttributeValueMap>::const_iterator it = RKComponentMap::getMap ()->component_attributes.constFind (attribute_id);
	if (it == RKComponentMap::getMap ()->component_attributes.constEnd ()) return QString ();
	return (*it).valuemap.value (this);
}

QString RKComponentHandle::getAttributeLabel (const QString &attribute_id) {
	RK_TRACE (PLUGIN);

	QMap<QString, RKComponentMap::AttributeValueMap>::const_iterator it = RKComponentMap::getMap ()->component_attributes.constFind (attribute_id);
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
	QHash<RKComponentHandle*, QList<RKComponentDependency> >::const_iterator it = RKComponentMap::getMap ()->component_dependencies.constFind (this);
	if (it == RKComponentMap::getMap ()->component_dependencies.constEnd ()) return ret;
	return (ret + (*it));
}

QString RKComponentHandle::getPluginmapFilename () const {
	RK_TRACE (PLUGIN);

	if (!plugin_map) {
		RK_ASSERT (plugin_map);
		return QString ();
	}
	return plugin_map->getFileName ();
}

RKComponentAboutData RKComponentHandle::getAboutData () {
	RK_TRACE (PLUGIN);

	// NOTE: In order to determine the message catalog to use, we have to open the pluginmap file...
	XMLHelper pluginmap_xml (getPluginmapFilename ());
	QDomElement pluginmap_doc = pluginmap_xml.openXMLFile (DL_ERROR);

	XMLHelper component_xml (getFilename (), pluginmap_xml.messageCatalog ());
	QDomElement component_doc = component_xml.openXMLFile (DL_ERROR);
	QDomElement about = component_xml.getChildElement (component_doc, "about", DL_INFO);
	if (!about.isNull ()) return RKComponentAboutData (about, component_xml);

	return (plugin_map->getAboutData ());
}

///########################### END RKComponentHandle ###############################

