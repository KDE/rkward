/***************************************************************************
                          rkcomponentmap.cpp  -  description
                             -------------------
    begin                : Thu May 12 2005
    copyright            : (C) 2005, 2006 by Thomas Friedrichsmeier
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

RKComponentMap::RKComponentMap () : KXMLGUIClient () {
	RK_TRACE (PLUGIN);
}

RKComponentMap::~RKComponentMap () {
	RK_TRACE (PLUGIN);

	clear ();
}

void RKComponentMap::clear () {
	RK_TRACE (PLUGIN);

	actionCollection ()->clear ();
	for (ComponentMap::iterator it = components.begin (); it != components.end (); ++it) {
		delete (it.data ());
	}
	components.clear ();

	// TODO!
	QDomDocument doc;
	doc.setContent (QString ("<!DOCTYPE kpartgui>\n<kpartgui name=\"rkwardcomponents\" version=\"0.3.4\">\n<MenuBar>\n\n</MenuBar>\n</kpartgui>"));
/*<Menu name=\"file\"><text>&amp;File</text></Menu>\n<Menu name=\"analysis\"><text>&amp;Analysis</text>\n</Menu>*/
	setXMLGUIBuildDocument (doc);
}

RKComponentHandle* RKComponentMap::getComponentHandle (const QString &id) {
	RK_TRACE (PLUGIN);

	return (components[id]);
}

//static
QDomElement RKComponentMap::findOrCreateElement (QDomElement& parent, const QString& tagname, const QString& name, const QString& label, int index) {
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
	QDomElement ret = xmlguiBuildDocument ().createElement (tagname);
	ret.setAttribute ("name", name);
	ret.setAttribute ("index", index);
	if (!label.isEmpty ()) {
		QDomElement text = xmlguiBuildDocument ().createElement ("text");
		text.appendChild (xmlguiBuildDocument ().createTextNode (label));
		ret.appendChild (text);
	}
	parent.insertAfter (ret, insert_after_element);	// if index_after_element.isNull, this add the new element as the last child of parent!

	return ret;
}

int RKComponentMap::addSubMenu (QDomElement& parent, const QDomElement& description, const QString& cnamespace) {
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

		RKComponentHandle* handle = components[id];

		if ((!handle) || (!handle->isPlugin ())) {
			RK_DO (qDebug ("No such component found while creating menu-entries or component is not a standalone plugin: \"%s\". No entry created.", id.latin1 ()), PLUGIN, DL_ERROR);
		} else {
			findOrCreateElement (menu_element, "Action", id, QString::null, xml->getIntAttribute ((*it), "index", -1, DL_INFO));
			counter++;
		}
	}
	return counter;
}

int RKComponentMap::addPluginMap (const QString& plugin_map_file) {
	RK_TRACE (PLUGIN);

	XMLHelper* xml = XMLHelper::getStaticHelper ();
	QDomElement element;
	XMLChildList list;

	QDomElement document_element = xml->openXMLFile (plugin_map_file, DL_ERROR);
	if (xml->highestError () >= DL_ERROR) return (0);

	QString prefix = QFileInfo (plugin_map_file).dirPath (true) + "/" + xml->getStringAttribute (document_element, "base_prefix", QString::null, DL_INFO);
	QString cnamespace = xml->getStringAttribute (document_element, "namespace", "rkward", DL_INFO) + "::";

	// step 1: create (list of) components
	element = xml->getChildElement (document_element, "components", DL_INFO);
	list = xml->getChildElements (element, "component", DL_INFO);

	for (XMLChildList::const_iterator it=list.begin (); it != list.end (); ++it) {
		QString filename = prefix + xml->getStringAttribute((*it), "file", QString::null, DL_WARNING);
		QString id = cnamespace + xml->getStringAttribute((*it), "id", QString::null, DL_WARNING);
		int type = xml->getMultiChoiceAttribute ((*it), "type", "standard", 0, DL_WARNING);
		QString label = xml->getStringAttribute ((*it), "label", i18n ("(no label)"), DL_WARNING);

		if (components.contains (id)) {
			RK_DO (qDebug ("RKComponentMap already contains a component with id \"%s\". Ignoring second entry.", id.latin1 ()), PLUGIN, DL_WARNING);
		} else if (!QFileInfo (filename).isReadable ()) {
			RK_DO (qDebug ("Specified file '%s' for component id \"%s\" does not exist or is not readable. Ignoring.", filename.latin1 (), id.latin1 ()), PLUGIN, DL_ERROR);
		} else {
			components.insert (id, RKComponentHandle::createComponentHandle (filename, (RKComponentType) type, id, label));
		}
	}

	// step 2: create / insert into menus
	QDomElement xmlgui_menubar_elem = xml->getChildElement (xmlguiBuildDocument ().documentElement (), "MenuBar", DL_ERROR);

	element = xml->getChildElement (document_element, "hierarchy", DL_INFO);
	list = xml->getChildElements (element, "menu", DL_INFO);
	int counter = 0;
	for (XMLChildList::const_iterator it=list.begin (); it != list.end (); ++it) {
		counter += addSubMenu (xmlgui_menubar_elem, (*it), cnamespace);
	}

	// step 3: included files
	QStringList includelist;
	list = xml->getChildElements (document_element, "include", DL_INFO);
	for (XMLChildList::const_iterator it=list.constBegin (); it != list.constEnd (); ++it) {
		QString file = prefix + xml->getStringAttribute (*it, "file", QString::null, DL_ERROR);
		if (QFileInfo (file).isReadable ()) {
			includelist.append (file);
		} else {
			RK_DO (qDebug ("Specified include file '%s' does not exist or is not readable. Ignoring.", file.latin1 ()), PLUGIN, DL_ERROR);
		}
	}
	for (QStringList::const_iterator it = includelist.constBegin (); it != includelist.constEnd (); ++it) {
		counter += addPluginMap (*it);
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
RKComponentHandle* RKComponentHandle::createComponentHandle (const QString &filename, RKComponentType type, const QString& id, const QString& label) {
	if (type == (int) Standard) {
		RKStandardComponentHandle *ret = new RKStandardComponentHandle (filename, type);
		new KAction (label, 0, ret, SLOT (activated ()), RKGlobals::componentMap ()->actionCollection (), id);
		return (ret);
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


///########################### END RKComponentHandle ###############################
///########################### BEGIN RKStandardComponentHandle ############################

#include "rkstandardcomponent.h"

RKStandardComponentHandle::RKStandardComponentHandle (const QString &filename, RKComponentType type) : QObject (RKGlobals::rkApp ()), RKComponentHandle (filename, type) {
	RK_TRACE (PLUGIN);
}

RKStandardComponentHandle::~RKStandardComponentHandle () {
	RK_TRACE (PLUGIN);
}

RKComponent *RKStandardComponentHandle::invoke (RKComponent *parent_component, QWidget *parent_widget) {
	RK_TRACE (PLUGIN);

	return (new RKStandardComponent (parent_component, parent_widget, getFilename ()));
}

void RKStandardComponentHandle::activated () {
	RK_TRACE (PLUGIN);

	invoke (0, 0);
}

#include "rkcomponentmap.moc"
