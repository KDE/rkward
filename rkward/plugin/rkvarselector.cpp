/***************************************************************************
                          rkvarselector.cpp  -  description
                             -------------------
    begin                : Thu Nov 7 2002
    copyright            : (C) 2002,2006 by Thomas Friedrichsmeier
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

#include <qlayout.h>
#include <qlabel.h>
//Added by qt3to4:
#include <Q3VBoxLayout>

#include "../misc/xmlhelper.h"
#include "../core/rcontainerobject.h"
#include "../core/rkvariable.h"
#include "../rkglobals.h"
#include "../misc/rkobjectlistview.h"
#include "../core/robjectlist.h"

#include "../debug.h"

RKVarSelector::RKVarSelector (const QDomElement &element, RKComponent *parent_component, QWidget *parent_widget) : RKComponent (parent_component, parent_widget) {
	RK_TRACE (PLUGIN);

// TODO: read filter settings
	addChild ("available", available = new RKComponentPropertyRObjects (this, false));
	addChild ("selected", selected = new RKComponentPropertyRObjects (this, false));

	Q3VBoxLayout  *vbox = new Q3VBoxLayout (this, RKGlobals::spacingHint ());
	
	QLabel *label = new QLabel (element.attribute ("label", "Select Variable(s)"), this);
	vbox->addWidget (label);

	list_view = new RKObjectListView (this);
	list_view->setSelectionMode (Q3ListView::Extended);
	connect (list_view, SIGNAL (listChanged ()), this, SLOT (objectListChanged ()));
	connect (list_view, SIGNAL (selectionChanged ()), this, SLOT (objectSelectionChanged ()));

	vbox->addWidget (list_view);
	list_view->getSettings ()->setSetting (RKObjectListViewSettings::ShowObjectsAllEnvironments, RKObjectListViewSettings::No);
	list_view->initialize ();
}

RKVarSelector::~RKVarSelector () {
	RK_TRACE (PLUGIN);
}

void RKVarSelector::objectListChanged () {
	RK_TRACE (PLUGIN);

	available->setFromListView (list_view);
	selected->setFromListView (list_view, true);
}

void RKVarSelector::objectSelectionChanged () {
	RK_TRACE (PLUGIN);

	selected->setFromListView (list_view, true);
	RK_DO (qDebug ("selected in varselector: %s", selected->value ().toLatin1 ()), PLUGIN, DL_DEBUG);
}

#include "rkvarselector.moc"
