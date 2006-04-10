/***************************************************************************
                          rkcommonfunctions  -  description
                             -------------------
    begin                : Mon Oct 17 2005
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
#include "rkcommonfunctions.h"

#include <qstringlist.h>
#include <qdom.h>

#include <kxmlguiclient.h>

namespace RKCommonFunctions {
	void removeNamedElementsRecursive (const QStringList &names, QDomNode &parent) {
		QDomNode nchild;

		for (QDomNode child = parent.firstChild (); !child.isNull (); child = nchild) {
			removeNamedElementsRecursive (names, child);

			nchild = child.nextSibling ();		// need to fetch next sibling here, as we might remove the child below
			if (child.isElement ()) {
				QDomElement e = child.toElement ();
				if (names.contains (e.attribute ("name"))) {
					parent.removeChild (child);
				}
			}
		}
	}
	
	void removeContainers (KXMLGUIClient *from, const QStringList &names, bool recursive) {
		QDomDocument doc = from->xmlguiBuildDocument ();
		if  (doc.documentElement ().isNull ()) doc = from->domDocument ();
	
		QDomElement e = doc.documentElement ();
		removeNamedElementsRecursive (names, e);
		from->setXMLGUIBuildDocument (doc);
	
		if (recursive) {
			QPtrList <KXMLGUIClient> *children = const_cast<QPtrList <KXMLGUIClient> *> (from->childClients ());
			if (children) {
				for (KXMLGUIClient *child = children->first (); child; child = children->next ()) {
					removeContainers (child, names, true);
				}
			}
		}
	}

	void moveContainer (KXMLGUIClient *client, const QString &tagname, const QString &name, const QString &to_name, bool recursive) {
		QDomDocument doc = client->xmlguiBuildDocument ();
		if  (doc.documentElement ().isNull ()) doc = client->domDocument ();
	
		// find the given elements
		QDomElement e = doc.documentElement ();

		QDomElement from_elem;
		QDomElement to_elem;

		QDomNodeList list = e.elementsByTagName (tagname);
		int count = list.count ();
		for (int i = 0; i < count; ++i) {
			QDomElement elem = list.item (i).toElement ();
			if (elem.isNull ()) continue;
			if (elem.attribute ("name") == name) {
				from_elem = elem;
			} else if (elem.attribute ("name") == to_name) {
				to_elem = elem;
			}
		}

		// move
		from_elem.parentNode ().removeChild (from_elem);
		to_elem.appendChild (from_elem);

		// set result
		client->setXMLGUIBuildDocument (doc);

		// recurse
		if (recursive) {
			QPtrList <KXMLGUIClient> *children = const_cast<QPtrList <KXMLGUIClient> *> (client->childClients ());
			if (children) {
				for (KXMLGUIClient *child = children->first (); child; child = children->next ()) {
					moveContainer (child, tagname, name, to_name, true);
				}
			}
		}
	}

}	// namespace
