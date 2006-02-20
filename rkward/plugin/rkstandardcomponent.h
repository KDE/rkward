/***************************************************************************
                          rkstandardcomponent  -  description
                             -------------------
    begin                : Sun Feb 19 2006
    copyright            : (C) 2006 by Thomas Friedrichsmeier
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

#ifndef RKSTANDARDCOMPONENT_H
#define RKSTANDARDCOMPONENT_H

#include "rkcomponent.h"

/** The standard type of component (i.e. stand-alone), previously known as "plugin" */
class RKStandardComponent : public RKComponent {
public:
	RKStandardComponent ();
	~RKStandardComponent ();
};

/** A helper class used to build and initialize an RKComponent. Most importantly this will keep track of the properties yet to be connected. Used at least by RKStandardComponent. */
class RKComponentBuilder {
public:
	RKComponentBuilder (RKComponent *parent);
	~RKComponentBuilder ();
	void buildElement (const QDomElement &element);
	void makeConnections ();
private:
	RKComponent *parent;
	struct RKComponentPropertyConnection {
		QString governor_property;
		QString client_property;
		bool reconcile;
	};
	QValueList <RKComponentPropertyConnection *> connection_list;
};

#endif
