/***************************************************************************
                          rkcomponentcontext  -  description
                             -------------------
    begin                : Mon Jan 22 2007
    copyright            : (C) 2007, 2014, 2015 by Thomas Friedrichsmeier
    email                : thomas.friedrichsmeier@kdemail.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef RKCOMPONENTCONTEXT_H
#define RKCOMPONENTCONTEXT_H

#include <qobject.h>

#include "rkcomponentmap.h"
#include "rkcomponent.h"

class QDomElement;

/** An RKContextHandler can be thought of as an instance of a context. E.g. the x11 device context by itself is just an abstract description. To actually use the context for a given device, an RKContextHandler is used. The normal way to construct an instance of this class is using RKContextMap::makeContextHandler().

The context handler is responsible for
a) providing a KXMLGUIClient to insert into the GUI
b) provide and handle KActions to invoke the plugins in the context
c) provide invoked plugins with context information.

The last part (providing context information) is the most important one in this class. Providing context information works like this: When a plugin is invoked, we look for an RKComponentPropertyBase called "incontext" in the plugin. If it has one, it will be set to the id of this context. If not, it will be ignored. Further context properties can be added to the context handler by calling RKComponentBase::addChild(). These will be set in the invoked plugin in the same way.

@author Thomas Friedrichsmeier
*/
class RKContextHandler : public QObject, public RKComponentBase, public KXMLGUIClient {
	Q_OBJECT
friend class RKComponentGUIXML;
public:
	void invokeComponent (RKComponentHandle *handle);
	int type () { return ComponentContextHandler; };
protected:
/** constructor. Protected. Use RKContextMap::makeContextHandler() instead. */
	RKContextHandler (QObject *parent, const QDomDocument &gui_xml, const QString &id);
/** desctructor. Should be called automatically, as this is QObject */
	~RKContextHandler ();
/** add a QAction to the context. To be called from RKContext::makeContextHandler(). */
	void addAction (const QString &id, RKComponentHandle *handle);
private slots:
/** slot to handle plugin activation */
	void componentActionActivated ();
private:
// KDE4: TODO: This can probably be made more straight-forward by using QAction::setData()
	typedef QMap<const QAction *, RKComponentHandle *> ActionMap;
	ActionMap action_map;
};

#endif
