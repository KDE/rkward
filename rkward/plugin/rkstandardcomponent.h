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

#include <qdom.h>

/** The standard type of component (i.e. stand-alone), previously known as "plugin". This is the type of component described by an XML-file */
class RKStandardComponent : public RKComponent {
	Q_OBJECT
public:
/** constructor.
@param parent_component Parent component (or 0, if this is going to be a top-level component)
@param parent_widget Parent widget (typically 0, if this is going to be a top-level component)
@param filename Filename of the XML-file to construct this component from */
	RKStandardComponent (RKComponent *parent_component, QWidget *parent_widget, const QString &filename);
/** destructor */
	~RKStandardComponent ();
// TODO: should these be moved to a separate RKStandardComponentGUI-class?
public slots:
	void ok ();
	void back ();
	void cancel ();
	void toggleCode ();
	void switchInterfaces ();
	void help ();
private:
/** The property holding the generated code. TODO: maybe, de facto, this property should be controlled (but not owned) by the scriptbackend. This way, we'd need less twisted logic inside this class. */
	RKComponentPropertyCode *code;
	QString filename;
};

/** A helper class used to build and initialize an RKComponent. Most importantly this will keep track of the properties yet to be connected. Used at least by RKStandardComponent.

Notes: How does building work?
- Builder builds the components. Simple components are built by the same builder. For embedded components, a sub-builder is invoked.
- Simple components register their (property) connection wishes to the the builder during construction
- Builder takes care of connecting properties

Important: For built components with non-zero parent components should call parent_component->addChild () to register them! As an exception, this may be omitted for passive components (e.g. layouting components) that do not specify an id

Reminder to the twisted brain: Typically inside a standard-component, *all* child components, even if nested in layouting components, etc. have the same standard-component as parent! Only embedded full-fledged components are a truely separate unit!
*/
class RKComponentBuilder {
public:
	RKComponentBuilder (RKComponent *parent_component, QWidget *parent_widget);
	~RKComponentBuilder ();
	void buildElement (QWidget *parent, const QDomElement &element);
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
