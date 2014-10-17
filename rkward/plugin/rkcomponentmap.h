/***************************************************************************
                          rkcomponentmap.h  -  description
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

#ifndef RKCOMPONENTMAP_H
#define RKCOMPONENTMAP_H

#include <qstring.h>

#include "rkcomponentmeta.h"

class XMLHelper;
/** very simple helper class to keep track of .pluginmap files */
class RKPluginMapFile {
public:
	RKPluginMapFile (const QString &basedir) { RKPluginMapFile::basedir = basedir; };
	~RKPluginMapFile () {};

	QString getBaseDir () { return basedir; };
	QString makeFileName (const QString &filename);
	QList<RKComponentDependency> getDependencies () { return dependencies; };
	static QString parseId (const QDomElement &e, XMLHelper &xml);
private:
friend class RKComponentMap;
	QString basedir;
	QString id;
	QList<RKComponentDependency> dependencies;
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
class RKComponentHandle : public QObject {
	Q_OBJECT
public:
	RKComponentHandle (RKPluginMapFile *pluginmap, const QString &rel_filename, const QString &label, RKComponentType type);
	virtual ~RKComponentHandle ();

	QString getFilename () { return plugin_map->makeFileName (filename); };
	QString getLabel () { return label; };
	RKComponentType getType () { return type; };
	bool isPlugin ();
	QString getPluginmapFilename ();

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
public slots:
/** Slot called, when the menu-item for this component is selected. Responsible for creating the GUI. */
	void activated ();
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

#include <qmap.h>
#include <QDomDocument>

#include <kxmlguiclient.h>

class QDomElement;
class XMLHelper;

/** This class keeps a QDomDocument that is a represenation of the GUI using KDEs XML-GUI format (a ui.rc). Use createMenus () to have it parse the menu descriptions from a .pluginmap file. It will adjust the XML description accordingly. When done, you can use to generated gui_xml to set it as the xmlGUIBuildDocument of a KXMLGUIClient. 

This class represents the common functionality between RKComponentMap and RKContextMap.

@author Thomas Friedrichsmeier
*/
class RKComponentGUIXML {
protected:
	RKComponentGUIXML ();
	virtual ~RKComponentGUIXML ();

/** reset the xml file */
	void clearGUIDescription ();

/** build XMLGUI menus
@param parent the parent menu (or tag) (in the KXMLGUI)
@param hierarchy_description the QDomElement containing the description for the new menu hierarchy
@returns number of plugins/menu-entries added successfully */
	int createMenus (QDomElement& parent, XMLHelper &xml, const QDomElement& hierarchy_description, const QString& cnamespace);

/** recurse into a lower menu-level 
@param parent the parent menu (in the KXMLGUI)
@param description the QDomElement containing the description for the new submenu
@returns number of plugins/menu-entries added successfully */
	int addSubMenu (QDomElement& parent, XMLHelper &xml, const QDomElement& description, const QString& cnamespace);

/** helper function: Find a specified element, and return it. If the element could not be found, it is created instead. The first three parameters are used as search parameters (all have to match). The additional two parameters only take effect, if a new element is created.
@param parent the QDomElement whose children to search through
@param tagname the tagname to look for
@param name value of the "name"-attribute to look for
@param label the label to assign to the new element (if no existing match could be found)
@param index the index position where to insert the new element in the list of children (if no existing match could be found). -1 means insert at the end of the list. */
	QDomElement findOrCreateElement (QDomElement& parent, XMLHelper &xml, const QString& tagname, const QString& name, const QString& label, int index);

/** an entry was added to the menu(s) somewhere. Reimplement, if you want to e.g. create a KAction for this */
	virtual void addedEntry (const QString & /* id */, RKComponentHandle * /* handle */) {};

/** The generated XML GUI description in KDEs ui.rc format */
	QDomDocument gui_xml;
};


class RKContextMap;

class RKPluginMapParseResult {
public:
	RKPluginMapParseResult () : valid_plugins (0) {};
	void add (const RKPluginMapParseResult &other) {
		detailed_problems.append (other.detailed_problems);
		valid_plugins += other.valid_plugins;
	};
	void addAndPrintError (int level, const QString message);
	QStringList detailed_problems;
	int valid_plugins;
};

/** This class (only a single instance should ever be needed) keeps a list of named components, which can be made accessible via the menu-structure
or included in other plugins. What this class does is rather simple: It basically maps a two piece name (namespace, component name) to a short description of the component (RKComponentHandle). The most important part of that description is the filename where a more elaborate definition of
the component can be retrieved.

The RKComponentMap provides convenience functions for adding or removing a .pluginmap-file to/from the list of components, and looking up RKComponentHandle for a given component name.

@author Thomas Friedrichsmeier
*/
class RKComponentMap : public RKComponentGUIXML, public KXMLGUIClient {
public:
	RKComponentMap ();

	~RKComponentMap ();

/** adds all Plugins / components in a .pluginmap-file. Also takes care of creating the menu-items, etc.
@returns status info of number of plugins (i.e. stand-alone components/menu-entries) added successfully / failed */
	static RKPluginMapParseResult addPluginMap (const QString& plugin_map_file);

/** clears out (and deletes) all components / plugins */
	static void clearAll ();

/** returns the component identified by id, 0 if not found */
	static RKComponentHandle* getComponentHandle (const QString &id);
/** look up the id of a component, empty string, if not found */
	static QString getComponentId (RKComponentHandle* by_component);
	static RKComponentMap *getMap () { return component_map; };
	static void initialize ();
/** returns the context identified by id */
	static RKContextMap *getContext (const QString &id);

	enum ComponentInvocationMode {
		ManualSubmit,
		AutoSubmit,
		AutoSubmitOrFail
	};
/** invokes the specified component as toplevel
@param message If a non-null pointer to QString is given, error messages are written into this string *instead* of being displayed */
	static bool invokeComponent (const QString &component_id, const QStringList &serialized_settings, ComponentInvocationMode submit_mode = ManualSubmit, QString *message=0, RCommandChain *in_chain = 0);
/** @returns a list of all currently registered component ids */
	QStringList allComponentIds () { return components.keys(); };
	bool isPluginMapLoaded (const QString& abs_filename) { return pluginmapfiles.contains (abs_filename); };
private:
/** typedef for easy reference to iterator */
	typedef QMap<QString, RKComponentHandle*> ComponentMap;
/** the actual map of components */
	ComponentMap components;

	RKComponentHandle* getComponentHandleLocal (const QString &id);
	QString getComponentIdLocal (RKComponentHandle* component);
	RKContextMap *getContextLocal (const QString &id);
	RKPluginMapParseResult addPluginMapLocal (const QString& plugin_map_file);

	void clearLocal ();

	typedef QMap<QString, RKContextMap*> RKComponentContextMap;
	RKComponentContextMap contexts;

	typedef QMap<QString, RKPluginMapFile*> PluginMapFileMap;
	PluginMapFileMap pluginmapfiles;

	static RKComponentMap *component_map;
friend class RKComponentHandle;
	// most components have neither attributes specific dependencies (other than dependencies shared by all plugins in a pluginmap).
	// therefore, it saves a few bytes to store attributes and specific dependencies in a central map, rather than keeping structures
	// per plugin
	struct AttributeValueMap {
		QHash<RKComponentHandle*, QString> valuemap;
		QHash<RKComponentHandle*, QString> labelmap;
	};
	QMap<QString, AttributeValueMap> component_attributes;
	QHash<RKComponentHandle*, QList<RKComponentDependency> > component_dependencies;
protected:
	void addedEntry (const QString &id, RKComponentHandle *handle);
};

#endif
