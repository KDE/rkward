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
class QString;
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
/** move container (action, menu, etc.) with tagname "tagname" and attribute 'name="..."' to be a child node of the tag with tagname=tagname and attribute name=to_name. Can be used to make a top-level menu a sub-menu of another menu instead */
	void moveContainer (KXMLGUIClient *client, const QString &tagname, const QString &name, const QString &to_name, bool recursive);

/** given the context line, find what looks like an R symbol */
	QString getCurrentSymbol (const QString &context_line, int cursor_pos, bool strict=true);
/** like get current symbol, but merely returns the start and end position of the current symbol */
	void getCurrentSymbolOffset (const QString &context_line, int cursor_pos, bool strict, int *start, int *end);
};

#endif
