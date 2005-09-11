/***************************************************************************
                          rkcomponentmap.cpp  -  description
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

#include "rkcomponentmap.h"

#include <qfileinfo.h>

#include <klocale.h>

#include "../misc/xmlhelper.h"
#include "../misc/rkmenu.h"
#include "../misc/rkmenulist.h"
#include "../debug.h"
#include "../rkglobals.h"
#include "../rkward.h"
#include "rkpluginhandle.h"

RKComponentMap::RKComponentMap () {
	RK_TRACE (PLUGIN);
}

RKComponentMap::~RKComponentMap () {
	RK_TRACE (PLUGIN);
}

int RKComponentMap::addPluginMap (const QString& plugin_map_file) {
	RK_TRACE (PLUGIN);

	XMLHelper* xml = XMLHelper::getStaticHelper ();

	QDomElement document_element = xml->openXMLFile (plugin_map_file, DL_ERROR);
	if (xml->highestError () >= DL_ERROR) return (0);

	QString prefix = QFileInfo (plugin_map_file).dirPath (true) + "/" + xml->getStringAttribute(document_element, "base_prefix", "", DL_WARNING);
	QString cnamespace = xml->getStringAttribute(document_element, "namespace", "rkward", DL_WARNING) + "::";

	// step 1: create list of components
	QDomElement element = xml->getChildElement (document_element, "components", DL_ERROR);
	XMLChildList list = xml->getChildElements (element, "component", DL_ERROR);

	for (XMLChildList::const_iterator it=list.begin (); it != list.end (); ++it) {
		QString filename = prefix + xml->getStringAttribute((*it), "file", "", DL_WARNING);
		QString id = cnamespace + xml->getStringAttribute((*it), "id", "", DL_WARNING);
		int type = xml->getMultiChoiceAttribute ((*it), "type", "standard", 0, DL_WARNING);

		if (components.contains (id)) {
			RK_DO (qDebug ("RKComponentMap already contains a component with id \"%s\". Ignoring second entry.", id.latin1 ()), PLUGIN, DL_WARNING);
		} else if (!QFileInfo (filename).isReadable ()) {
			RK_DO (qDebug ("Specified file '%s' for component id \"%s\" does not exist or is not readable. Ignoring.", filename.latin1 (), id.latin1 ()), PLUGIN, DL_ERROR);
		} else {
			components.insert (id, RKComponentHandle::createComponentHandle (filename, (RKComponentType) type));
		}
	}

	// step 2: create / insert into menus
	element = xml->getChildElement (document_element, "hierarchy", DL_ERROR);
	list = xml->getChildElements (element, "menu", DL_ERROR);

	int counter = 0;
	for (XMLChildList::const_iterator it=list.begin (); it != list.end (); ++it) {
		counter += addSubMenu (0, (*it), cnamespace);
	}
	return counter;
}

void RKComponentMap::clear () {
	RK_TRACE (PLUGIN);

	RKGlobals::rkApp ()->getMenuList ()->clear ();
	for (ComponentMap::iterator it = components.begin (); it != components.end (); ++it) {
		delete (it.data ());
	}
	components.clear ();
}

RKComponentHandle* RKComponentMap::getComponentHandle (const QString &id) {
	RK_TRACE (PLUGIN);

	return (components[id]);
}

int RKComponentMap::addSubMenu (RKMenu* parent, const QDomElement& element, const QString& cnamespace) {
	RK_TRACE (PLUGIN);

	int counter = 0;
	XMLHelper* xml = XMLHelper::getStaticHelper ();

	// 1: create new menu
	RKMenu* menu = 0;
	if (!parent) {
		menu = RKGlobals::rkApp ()->getMenuList ()->createMenu (xml->getStringAttribute (element, "id", "none", DL_WARNING), xml->getStringAttribute (element, "label", i18n ("(no label)"), DL_WARNING), xml->getIntAttribute (element, "index", 4, DL_INFO));
	} else {
		menu = parent->addSubMenu (xml->getStringAttribute (element, "id", "none", DL_WARNING), xml->getStringAttribute (element, "label", i18n ("(no label)"), DL_WARNING), xml->getIntAttribute (element, "index", -1, DL_INFO));
	}

	// 2: recurse into submenus
	XMLChildList list = xml->getChildElements (element, "menu", DL_INFO);

	for (XMLChildList::const_iterator it=list.begin (); it != list.end (); ++it) {
		counter += addSubMenu (menu, (*it), cnamespace);
	}

	// 3: add entries
	list = xml->getChildElements (element, "entry", DL_INFO);
	for (XMLChildList::const_iterator it=list.begin (); it != list.end (); ++it) {
		QString id = cnamespace + xml->getStringAttribute ((*it), "component", "#invalid#", DL_ERROR);

		RKComponentHandle* handle = components[id];

		if ((!handle) || (!handle->isPlugin ())) {
			RK_DO (qDebug ("No such component found while creating menu-entries or component is not a standalone plugin: \"%s\". No entry created.", id.latin1 ()), PLUGIN, DL_ERROR);
		} else {
			menu->addEntry (id, static_cast<RKPluginHandle*> (handle), xml->getStringAttribute ((*it), "label", i18n ("(no label)"), DL_WARNING), xml->getIntAttribute ((*it), "index", -1, DL_INFO));
			counter++;
		}
	}
	return counter;
}

///########################### END RKComponentMap ###############################
///########################### BEGIN RKComponentHandle ############################

RKComponentHandle::RKComponentHandle (const QString &filename, RKComponentType type) {
	RK_TRACE (PLUGIN);

	RKComponentHandle::type = type;
	RKComponentHandle::filename = filename;
}

RKComponentHandle::~RKComponentHandle () {
	RK_TRACE (PLUGIN);
}

//static 
RKComponentHandle* RKComponentHandle::createComponentHandle (const QString &filename, RKComponentType type) {
	if (type == (int) Standard) {
		return (new RKPluginHandle (filename, type));
	}
	// TODO: create an RKPluginHandle instead!

	// TODO: more ifs, special handling for sepcial components

	RK_ASSERT (false);
	return 0;
}

bool RKComponentHandle::isPlugin () {
	if (type != Standard) {
		return false;
	}
	return true;
}
