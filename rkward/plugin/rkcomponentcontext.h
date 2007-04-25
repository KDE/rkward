/***************************************************************************
                          rkcomponentcontext  -  description
                             -------------------
    begin                : Mon Jan 22 2007
    copyright            : (C) 2007 by Thomas Friedrichsmeier
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

#ifndef RKCOMPONENTCONTEXT_H
#define RKCOMPONENTCONTEXT_H

#include <qobject.h>

#include "rkcomponentmap.h"
#include "rkcomponent.h"

class QDomElement;
class RKContextHandler;

/** This class keeps a list of components that are useable in a certain context (e.g. x11 device). It also keeps a description of the menu structure(s) that should be used for that context. Note that in order to use the XML-GUI, you should create an RKContextHandler using makeContextHandler().

@author Thomas Friedrichsmeier
*/
class RKContextMap : public RKComponentGUIXML {
public:
/** constructor
@param id The id of the context */
	RKContextMap (const QString &id);
/** destructor */
	~RKContextMap ();
/** A menu entries to the context map from a .pluginmap file */
	int create (const QDomElement &context_element, const QString &component_namespace);
/** Create a context handler for this context. */
	RKContextHandler *makeContextHandler (QObject *parent, bool create_actions=true);
	QStringList components () { return component_ids; };
protected:
	void addedEntry (const QString &id, RKComponentHandle * /* handle */);
private:
	QStringList component_ids;
	QString id;
};


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
friend class RKContextMap;
public:
	void invokeComponent (RKComponentHandle *handle);
	int type () { return ComponentContextHandler; };
protected:
/** constructor. Protected. Use RKContextMap::makeContextHandler() instead. */
	RKContextHandler (QObject *parent, const QDomDocument &gui_xml, const QString &id);
/** desctructor. Should be called automatically, as this is QObject */
	~RKContextHandler ();
/** add a KAction to the context. To be called from RKContext::makeContextHandler(). */
	void addAction (const QString &id, RKComponentHandle *handle);
private slots:
/** slot to handle plugin activation */
	void componentActionActivated ();
private:
	typedef QMap<const KAction *, RKComponentHandle *> ActionMap;
	ActionMap action_map;
};

#endif
