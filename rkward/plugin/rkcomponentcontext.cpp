/***************************************************************************
                          rkcomponentcontext  -  description
                             -------------------
    begin                : Mon Jan 22 2007
    copyright            : (C) 2007, 2014, 2015 by Thomas Friedrichsmeier
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

#include "rkcomponentcontext.h"

#include <kaction.h>
#include <kactioncollection.h>

#include "rkstandardcomponent.h"
#include "../misc/xmlhelper.h"

#include "../debug.h"

RKContextHandler *RKComponentGUIXML::makeContextHandler (QObject *parent, bool create_actions) {
	RK_TRACE (PLUGIN);

	RKContextHandler *handler = new RKContextHandler (parent, gui_xml, context);
	if (create_actions) {
		const QStringList ids = components ();
		for (int i = 0; i < ids.size (); ++i) {
			RKComponentHandle *handle = RKComponentMap::getComponentHandle (ids[i]);
			handler->addAction (ids[i], handle);
		}
	}
	return handler;
}

RKContextHandler::RKContextHandler (QObject *parent, const QDomDocument &gui_xml, const QString &id) : QObject (parent), RKComponentBase (), KXMLGUIClient () {
	RK_TRACE (PLUGIN);

	setXMLGUIBuildDocument (gui_xml);

	RKComponentPropertyBase *incontext = new RKComponentPropertyBase (this, false);
	incontext->setValue (id);
	addChild ("context", incontext);
}

RKContextHandler::~RKContextHandler () {
	RK_TRACE (PLUGIN);
}

void RKContextHandler::addAction (const QString &id, RKComponentHandle *handle) {
	RK_TRACE (PLUGIN);

	QAction *action = actionCollection ()->addAction (id, this, SLOT (componentActionActivated()));
	action->setText (handle->getLabel ());
	action_map.insert (action, handle);
}

void RKContextHandler::componentActionActivated () {
	RK_TRACE (PLUGIN);

	// find handle that triggered action
	RKComponentHandle *handle = 0;
	const QAction *action = dynamic_cast<const QAction *> (sender ());
	if (action_map.contains (action)) handle = action_map[action];
	if (!handle) {
		RK_ASSERT (false);
		return;
	}

	invokeComponent (handle);
}

void RKContextHandler::invokeComponent (RKComponentHandle *handle) {
	RK_TRACE (PLUGIN);
	RK_ASSERT (handle);

	// create component
	RKComponent *component = handle->invoke (0, 0);

	// set context values
	for (QHash<QString, RKComponentBase*>::const_iterator it = child_map.constBegin (); it != child_map.constEnd (); ++it) {
		if (it.key () != "#noid#") {
			QString id = it.key ();
			QString remainder;
			RKComponentBase *client = component->lookupComponent (id, &remainder);

			RK_ASSERT (it.value ()->isProperty ());
			if (!(client && remainder.isEmpty () && client->isProperty () && it.value ()->isProperty ())) {
				RK_DEBUG (PLUGIN, DL_INFO, "Could not set context property %s", id.toLatin1 ().data ());
				continue;
			}

			static_cast<RKComponentPropertyBase *> (client)->connectToGovernor (static_cast<RKComponentPropertyBase *> (it.value ()), QString::null, false);
		}
	}
}

#include "rkcomponentcontext.moc"
