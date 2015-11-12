/***************************************************************************
                          rkvarselector.cpp  -  description
                             -------------------
    begin                : Thu Nov 7 2002
    copyright            : (C) 2002-2015 by Thomas Friedrichsmeier
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

#include "rkvarselector.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QAction>
#include <QMenu>

#include <klocale.h>

#include "../misc/xmlhelper.h"
#include "../rkglobals.h"
#include "../misc/rkobjectlistview.h"
#include "../misc/rkstandardicons.h"
#include "../core/robjectlist.h"
#include "../core/renvironmentobject.h"

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
	
	QLabel *label = new QLabel (element.attribute ("label", i18n ("Select Variable(s)")), this);
	vbox->addWidget (label);

	// TODO: Or should these actions be moved to RKObjectListView, non-tool-window-mode?
	show_all_envs_action = new QAction (i18n ("Show all environments"), this);
	show_all_envs_action->setCheckable (true);
	show_all_envs_action->setToolTip (i18n ("Show objects in all environments on the <i>search()</i> path, instead of just those in <i>.GlobalEnv</i>. Check this, if you want to select objects from a loaded package."));
	connect (show_all_envs_action, SIGNAL (toggled(bool)), this, SLOT (rootChanged()));

	filter_widget = 0;
	filter_widget_placeholder = new QVBoxLayout (this);
	filter_widget_placeholder->setContentsMargins (0, 0, 0, 0);
	vbox->addLayout (filter_widget_placeholder);
	show_filter_action = new QAction (i18n ("Show filter options"), this);
	show_filter_action->setCheckable (true);
	show_filter_action->setShortcut (QKeySequence ("Ctrl+F"));
	show_filter_action->setIcon (RKStandardIcons::getIcon (RKStandardIcons::ActionSearch));
	connect (show_filter_action, SIGNAL (toggled(bool)), this, SLOT(showFilterWidget()));

	list_view = new RKObjectListView (false, this);
	list_view->setSelectionMode (QAbstractItemView::ExtendedSelection);
	list_view->initialize ();
	vbox->addWidget (list_view);
	connect (list_view, SIGNAL (selectionChanged()), this, SLOT (objectSelectionChanged()));

	QAction* sep = list_view->contextMenu ()->insertSeparator (list_view->contextMenu ()->actions ().value (0));
	list_view->contextMenu ()->insertAction (sep, show_filter_action);
	list_view->contextMenu ()->insertAction (sep, show_all_envs_action);

	rootChanged ();
}

RKVarSelector::~RKVarSelector () {
	RK_TRACE (PLUGIN);
}

void RKVarSelector::showFilterWidget () {
	RK_TRACE (PLUGIN);

	if (!filter_widget) {
		filter_widget = list_view->getSettings ()->filterWidget (this);
		filter_widget_placeholder->addWidget (filter_widget);
	}
	filter_widget->setShown (show_filter_action->isChecked ());
}

void RKVarSelector::rootChanged () {
	RK_TRACE (PLUGIN);

	RObject* object = root->objectValue ();
	if (!object) {
		if (!show_all_envs_action->isChecked ()) object = RObjectList::getGlobalEnv ();
	}
	list_view->setRootObject (object);
}

void RKVarSelector::objectSelectionChanged () {
	RK_TRACE (PLUGIN);

	selected->setObjectList (list_view->selectedObjects ());
	RK_DEBUG (PLUGIN, DL_DEBUG, "selected in varselector: %s", qPrintable (fetchStringValue (selected)));
}

#include "rkvarselector.moc"
