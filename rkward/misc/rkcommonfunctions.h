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
#ifndef RKCOMMONFUNCTIONS_H
#define RKCOMMONFUNCTIONS_H

class QStringList;
class QDomNode;
class KXMLGUIClient;

/** Some common static helper functions that don't really belong to any class in particular. If ever we have more than a dozen or so functions in here,
we should probably split this file up. Until then, there's no real need.

@author Thomas Friedrichsmeier
*/
namespace RKCommonFunctions {
/** remove QDomElements with attribute 'name="..."' from QDomNode parent, where "..." is any of the strings in names */
	void removeNamedElementsRecursive (const QStringList &names, QDomNode &parent);
/** remove containers (actions, menus, etc.) with attribute 'name="..."' from KXMLGUIClient from s XML gui, where "..." is any of the strings in names. If recursive, also removes those containers from child clients. */
	void removeContainers (KXMLGUIClient *from, const QStringList &names, bool recursive);
};

#endif
