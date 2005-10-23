/***************************************************************************
                          rkcomponentmap.h  -  description
                             -------------------
    begin                : Thu May 12 2005
    copyright            : (C) 2005 by Thomas Friedrichsmeier
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

/** enum of different types of RKComponent */
enum RKComponentType {
	Standard=0		/// the only type available so far. Classifies a component that can be used standalone, and is not special in any way. Of course, as long as there is only one category of component, this is fairly meaningless. It's meant for future features.
};

#include <qstring.h>

/** This simple class keeps the most basic information about a component in RKWard. Most work is done in RKComponentMap.
Note that standard components (i.e. components which can act as regular top-level dialogs) have an additional counterpart in RKPluginHandle

TODO: no! RKPluginHandle should inherit from RKComponentHandle!


@author Thomas Friedrichsmeier
*/

class RKComponentHandle {
public:
	RKComponentHandle (const QString &filename, RKComponentType type);

	virtual ~RKComponentHandle ();

	QString getFilename () { return filename; };
	RKComponentType getType () { return type; };
	bool isPlugin ();

	static RKComponentHandle* createComponentHandle (const QString &filename, RKComponentType type, const QString& id);
private:
/** The filename of the description file for this comonent */
	QString filename;
	RKComponentType type;
};

#include <qmap.h>
#include <kxmlguiclient.h>

class QDomElement;
class RKMenu;

/** This class (only a single instance should ever be needed) keeps a list of named components, which can be made accessible via the menu-structure
or included in other plugins. What this class does is rather simple: It basically maps a two piece name (namespace, component name) to a short description of the component (RKComponentHandle). The most important part of that description is the filename where a more elaborate definition of
the component can be retrieved.

The RKComponentMap provides convenience functions for adding or removing a .pluginmap-file to/from the list of components, and looking up RKComponentHandle for a given component name.

// TODO: maybe we do not need to keep a map at all, but only create the handles! Think about this one day!

@author Thomas Friedrichsmeier
*/
class RKComponentMap : public KXMLGUIClient {
public:
	RKComponentMap ();

	~RKComponentMap ();

/** adds all Plugins / components in a .pluginmap-file. Also takes care of creating the menu-items, etc.
@returns number of plugins (i.e. stand-alone components/menu-entries) added successfully */
	int addPluginMap (const QString& plugin_map_file);
//	int addPluginMapNew (const QString& plugin_map_file);
/** clears out (and deletes) all components / plugins */
	void clear ();

/** returns the component identified by id */
	RKComponentHandle* getComponentHandle (const QString &id);
private:
/** recurse into a lower menu-level 
@param parent the parent menu
@param element the QDomElement containing the description for the new submenu
@returns number of plugins/menu-entries added successfully */
//	int addSubMenu (RKMenu* parent, const QDomElement& element, const QString& cnamespace);
	int addSubMenu (QDomElement& parent, const QDomElement& description, const QString& cnamespace);

	QDomElement findOrCreateElement (QDomElement& parent, const QString& tagname, const QString& name, const QString& label, int index);

/** typedef for easy reference to iterator */
	typedef QMap<QString, RKComponentHandle*> ComponentMap;
/** the actual map of components */
	ComponentMap components;
};

#endif
