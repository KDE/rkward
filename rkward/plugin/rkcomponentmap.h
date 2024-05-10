/*
rkcomponentmap.h - This file is part of the RKWard project. Created: Thu May 12 2005
SPDX-FileCopyrightText: 2005-2015 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef RKCOMPONENTMAP_H
#define RKCOMPONENTMAP_H

#include <qstring.h>
#include <QSet>

#include "rkcomponentmeta.h"

class RKMessageCatalog;
class XMLHelper;
/** very simple helper class to keep track of .pluginmap files */
class RKPluginMapFile {
public:
	RKPluginMapFile (const QString &filename, const QString &basedir, const RKMessageCatalog *_catalog) : basedir(basedir), filename(filename), catalog(_catalog), about(nullptr) {};
	~RKPluginMapFile () { delete about; };

	QString getBaseDir () const { return basedir; };
	QString getFileName () const { return filename; };
	QString makeFileName (const QString &filename) const;
	QList<RKComponentDependency> getDependencies () const { return dependencies; };
	static QString parseId (const QDomElement &e, XMLHelper &xml);
	const RKMessageCatalog *messageCatalog () const { return catalog; };
	/** Get the about data for this pluginmap. */
	RKComponentAboutData getAboutData ();
private:
friend class RKComponentMap;
	QString basedir;
	QString filename;
	QString id;
	QList<RKComponentDependency> dependencies;
	const RKMessageCatalog *catalog;
	RKComponentAboutData *about;
};

/** enum of different types of RKComponent */
enum RKComponentType {
	Standard=0		/// the only type available so far. Classifies a component that can be used standalone, and is not special in any way. Of course, as long as there is only one category of component, this is fairly meaningless. It's meant for future features.
};

#include <qobject.h>
#include <QPair>
#include <QMap>
#include <QHash>

class RKComponent;
class RKComponentMap;
class RCommandChain;
class RKStandardComponent;
class QWidget;
class KActionCollection;
/** This simple class keeps the most basic information about a component in RKWard. Most work is done in RKComponentMap.

@author Thomas Friedrichsmeier
*/
class RKComponentHandle {
public:
	RKComponentHandle (RKPluginMapFile *pluginmap, const QString &rel_filename, const QString &label);
	virtual ~RKComponentHandle ();

	QString getFilename () const { return plugin_map->makeFileName (filename); };
	QString getLabel () const { return label; };
	QString getPluginmapFilename () const;

	RKStandardComponent *invoke (RKComponent *parent_component, QWidget *parent_widget);

/** Gets the dependencies of this plugin. This *includes* the dependencies of the parent pluginmap */
	QList<RKComponentDependency> getDependencies ();
/** Adds dependencies for this plugin. */
	void addDependencies (const QList<RKComponentDependency> & deps);

	QString getAttributeValue (const QString &attribute_id);
	QString getAttributeLabel (const QString &attribute_id);
	void addAttribute (const QString &id, const QString &value, const QString &label);
	void setAccessible (bool accessible) { is_accessible = accessible; };
/** Returns whether this component is accessible from the menu, somewhere (else it might be in a context) */
	bool isAccessible () const { return is_accessible; };
	const RKMessageCatalog *messageCatalog () const { return plugin_map->messageCatalog (); };
/** Returns the about data for this component (likely that of the containing pluginmap). Note that in contrast to RKPluginMapFile::getAboutData (), the parsed
 *  about data is not cached, as access to this data should be infrequent. */
	RKComponentAboutData getAboutData ();
protected:
/** The plugin map where this component was declared */
	RKPluginMapFile *plugin_map;
/** The filename relative to the pluginmap file */
	QString filename;
	QString label;
	RKComponentType type;
private:
	bool is_accessible;
};

#include <QMultiMap>
#include <QDomDocument>

#include <kxmlguiclient.h>

class QDomElement;
class XMLHelper;
class RKContextHandler;

/** This class keeps a QDomDocument that is a representation of the GUI using KDEs XML-GUI format (a ui.rc). Use createMenus () to have it parse the menu descriptions from a .pluginmap file. It will adjust the XML description accordingly. When done, you can use to generated gui_xml to set it as the xmlGUIBuildDocument of a KXMLGUIClient. 

One instance of this class is generally around, persistently: The RKComonentMap (with context "global"). Further instances are around one for each context.

@author Thomas Friedrichsmeier
*/
class RKComponentGUIXML {
public:
	class Entry {
	public:
		Entry () { is_menu = false; }
		QString label;
		QString id;
		bool is_menu;
	};
	class Group {
	public:
		~Group ();
		QString id;
		bool separated;
		QList<Entry*> entries;
	};
	class Menu : public Entry {
	public:
		Menu () { is_menu = true; }
		~Menu () { clear (); }
		void clear ();
		QList<Group*> groups;
		QSet<QString> components;
	} toplevel_menu;
/** build XMLGUI menus
@param hierarchy_description the QDomElement containing the description for the new menu hierarchy
@returns number of plugins/menu-entries added successfully */
	int createMenus (XMLHelper &xml, const QDomElement& hierarchy_description, const QString& cnamespace);
	void finalize ();

/** Create a context handler for this context. */
	RKContextHandler *makeContextHandler (QObject *parent, bool create_actions=true);
	QStringList components () { return component_menus.keys (); };
protected:
	explicit RKComponentGUIXML(const QString &context_id);
	virtual ~RKComponentGUIXML();

/** reset the xml file */
	void clearGUIDescription ();

/** an entry was added to the menu(s) somewhere. Reimplement, if you want to e.g. create a QAction for this */
	virtual void addedEntry (const QString & /* id */, RKComponentHandle * /* handle */) {};

/** The generated XML GUI description in KDEs ui.rc format */
	QDomDocument gui_xml;
friend class RKComponentMap;
	QMap<QString, QString> component_menus;

/** Add a component override, e.g. hiding component "t_test" in context "global". Overrides are persistent per session. */
	static void addOverride (const QString &id, const QString &context, bool visible);
/** Clear component overrides (see addOverride()). */
	static void clearOverrides ();
private:
	int addEntries (RKComponentGUIXML::Menu *menu, XMLHelper &xml, const QDomElement &description, const QString& cnamespace);
	void menuItemsToXml (const RKComponentGUIXML::Menu *menu, QDomElement &xml);
	void resolveComponentLabelsAndSortMenu (Menu *menu, const QString &menu_path=QString ());
	struct ComponentOverride {
		QString context;
		bool hidden;
	};
	typedef QMultiMap<QString, ComponentOverride> OverrideMap;
	static OverrideMap overrides;
	QString context;

	void appendPluginToList (const QString &id, QStringList *list);
};


class RKPluginMapParseResult {
public:
	RKPluginMapParseResult () : valid_plugins (0) {};
	void add (const RKPluginMapParseResult &other) {
		detailed_problems.append (other.detailed_problems);
		valid_plugins += other.valid_plugins;
	};
	void addAndPrintError (int level, const QString &message);
	QStringList detailed_problems;
	int valid_plugins;
};

/** This class keeps a list of named components, which can be made accessible via the menu-structure
or included in other plugins. What this class does is rather simple: It basically maps a two piece name (namespace, component name) to a short description of the component (RKComponentHandle). The most important part of that description is the filename where a more elaborate definition of
the component can be retrieved.

The RKComponentMap provides convenience functions for adding or removing a .pluginmap-file to/from the list of components, and looking up RKComponentHandle for a given component name.

Usually there is exactly one instance of this class, accessible with RKComonentMap::getMain(), and some other static functions. However, temporary instances can be created for parsing
individual .pluginmap files in order to list / display the plugins contained in these.

@author Thomas Friedrichsmeier
*/
class RKComponentMap : public QObject, public RKComponentGUIXML, public KXMLGUIClient {
	Q_OBJECT
public:
	RKComponentMap ();
	~RKComponentMap ();

/** adds all Plugins / components in a .pluginmap-file. Also takes care of creating the menu-items, etc.
@returns status info of number of plugins (i.e. stand-alone components/menu-entries) added successfully / failed */
	RKPluginMapParseResult addPluginMap (const QString& plugin_map_file);
	void finalizeAll ();

/** clears out (and deletes) all components / plugins */
	void clearAll ();

/** returns true, if no plugin maps are loaded. */
	bool isEmpty() const { return pluginmapfiles.isEmpty(); }

/** returns the component identified by id, 0 if not found */
	static RKComponentHandle* getComponentHandle (const QString &id);
/** look up the id of a component, empty string, if not found */
	static QString getComponentId (RKComponentHandle* by_component);
	static RKComponentMap *getMap () { return component_map; };
	static void initialize ();
/** returns the context identified by id */
	static RKComponentGUIXML *getContext (const QString &id);

	enum ComponentInvocationMode {
		ManualSubmit,
		AutoSubmit,
		AutoSubmitOrFail
	};
/** invokes the specified component as toplevel
@param message If a non-null pointer to QString is given, error messages are written into this string *instead* of being displayed */
	static bool invokeComponent (const QString &component_id, const QStringList &serialized_settings, ComponentInvocationMode submit_mode = ManualSubmit, QString *message = nullptr, RCommandChain *in_chain = nullptr);
/** @returns for rk.list.plugins(): Return a list of all currently registered component ids, their context, menu, and label (i.e. current four strings per component) */
	QStringList listPlugins ();
	void setPluginStatus (const QStringList &ids, const QStringList &contexts, const QStringList& visible);
	bool isPluginMapLoaded (const QString& abs_filename) const;
public Q_SLOTS:
/** Slot called, when a menu-item for a component is selected. Responsible for creating the GUI. */
	void activateComponent ();
private:
/** typedef for easy reference to iterator */
	typedef QMultiMap<QString, RKComponentHandle*> ComponentMap;
/** the actual map of components */
	ComponentMap components;

	RKComponentHandle* getComponentHandleLocal (const QString &id);
	QString getComponentIdLocal (RKComponentHandle* component);

	typedef QMap<QString, RKComponentGUIXML*> RKComponentContextMap;
	RKComponentContextMap contexts;

	QList<RKPluginMapFile*> pluginmapfiles;

	static RKComponentMap *component_map;
friend class RKComponentHandle;
	// most components have neither attributes nor specific dependencies (other than dependencies shared by all plugins in a pluginmap).
	// therefore, it saves a few bytes to store attributes and specific dependencies in a central map, rather than keeping structures
	// per plugin
	struct AttributeValueMap {
		QHash<RKComponentHandle*, QString> valuemap;
		QHash<RKComponentHandle*, QString> labelmap;
	};
	QMap<QString, AttributeValueMap> component_attributes;
	QHash<RKComponentHandle*, QList<RKComponentDependency> > component_dependencies;
protected:
	void addedEntry (const QString &id, RKComponentHandle *handle) override;
};

#endif
