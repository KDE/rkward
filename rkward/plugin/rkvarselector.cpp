/***************************************************************************
                          rkvarselector.cpp  -  description
                             -------------------
    begin                : Thu Nov 7 2002
    copyright            : (C) 2002, 2006, 2009, 2010, 2012 by Thomas Friedrichsmeier
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

#include "rkvarselector.h"

#include <QVBoxLayout>
#include <qlabel.h>

#include "../misc/xmlhelper.h"
#include "../rkglobals.h"
#include "../misc/rkobjectlistview.h"
#include "../core/robjectlist.h"

#include "../debug.h"

RKVarSelector::RKVarSelector (const QDomElement &element, RKComponent *parent_component, QWidget *parent_widget) : RKComponent (parent_component, parent_widget) {
	RK_TRACE (PLUGIN);

// TODO: read filter settings
	addChild ("selected", selected = new RKComponentPropertyRObjects (this, false));
	selected->setInternal (true);
	addChild ("root", root = new RKComponentPropertyRObjects (this, false));
	connect (root, SIGNAL (valueChanged(RKComponentPropertyBase*)), this, SLOT (rootChanged()));
	root->setInternal (true);

	QVBoxLayout *vbox = new QVBoxLayout (this);
	vbox->setContentsMargins (0, 0, 0, 0);
	
	QLabel *label = new QLabel (element.attribute ("label", "Select Variable(s)"), this);
	vbox->addWidget (label);

	list_view = new RKObjectListView (this);
	list_view->setSelectionMode (QAbstractItemView::ExtendedSelection);
	connect (list_view, SIGNAL (selectionChanged ()), this, SLOT (objectSelectionChanged ()));

	vbox->addWidget (list_view);
	list_view->getSettings ()->setSetting (RKObjectListViewSettings::ShowObjectsAllEnvironments, false);
	list_view->initialize ();
	rootChanged ();
}

RKVarSelector::~RKVarSelector () {
	RK_TRACE (PLUGIN);
}

void RKVarSelector::rootChanged () {
	RK_TRACE (PLUGIN);

	RObject* object = root->objectValue ();
	if (!object) object = RObjectList::getObjectList ();
	list_view->setRootObject (object);
}

void RKVarSelector::objectSelectionChanged () {
	RK_TRACE (PLUGIN);

	selected->setObjectList (list_view->selectedObjects ());
	RK_DEBUG (PLUGIN, DL_DEBUG, "selected in varselector: %s", qPrintable (fetchStringValue (selected)));
}

#include "rkvarselector.moc"
