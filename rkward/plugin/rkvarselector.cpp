/*
rkvarselector.cpp - This file is part of RKWard (https://rkward.kde.org). Created: Thu Nov 7 2002
SPDX-FileCopyrightText: 2002-2018 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "rkvarselector.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QAction>
#include <QMenu>
#include <QToolButton>

#include <KLocalizedString>

#include "../misc/xmlhelper.h"
#include "../misc/rkobjectlistview.h"
#include "../misc/rkstandardicons.h"
#include "../misc/rkcommonfunctions.h"
#include "../core/robjectlist.h"
#include "../core/renvironmentobject.h"

#include "../debug.h"

bool RKVarSelector::expanded = true;

RKVarSelector::RKVarSelector (const QDomElement &element, RKComponent *parent_component, QWidget *parent_widget) : RKComponent (parent_component, parent_widget) {
	RK_TRACE (PLUGIN);

	XMLHelper *xml = parent_component->xmlHelper ();

// TODO: read filter settings
	addChild ("selected", selected = new RKComponentPropertyRObjects (this, false));
	selected->setInternal (true);
	addChild ("root", root = new RKComponentPropertyRObjects (this, false));
	connect (root, &RKComponentPropertyBase::valueChanged, this, &RKVarSelector::rootChanged);
	root->setInternal (true);

	QVBoxLayout *vbox = new QVBoxLayout (this);
	vbox->setContentsMargins (0, 0, 0, 0);

	QHBoxLayout *hbox = new QHBoxLayout ();
	hbox->setContentsMargins (0, 0, 0, 0);
	vbox->addLayout (hbox);
	QLabel *label = new QLabel (xml->i18nStringAttribute (element, "label", i18n ("Select Variable(s)"), DL_INFO), this);
	hbox->addWidget (label);
	expand_collapse_button = new QToolButton (this);
	expand_collapse_button->setPopupMode (QToolButton::InstantPopup);
	expand_collapse_button->setAutoRaise (true);
	connect (expand_collapse_button, &QToolButton::clicked, this, &RKVarSelector::toggleLevel1);
	hbox->addWidget (expand_collapse_button);
	QToolButton *advanced_button = new QToolButton (this);
	advanced_button->setIcon (RKStandardIcons::getIcon (RKStandardIcons::ActionConfigureGeneric));
	advanced_button->setPopupMode (QToolButton::InstantPopup);
	advanced_button->setAutoRaise (true);
	hbox->addWidget (advanced_button);

	// TODO: Or should these actions be moved to RKObjectListView, non-tool-window-mode?
	show_all_envs_action = new QAction (i18n ("Show all environments"), this);
	show_all_envs_action->setCheckable (true);
	show_all_envs_action->setToolTip (i18n ("Show objects in all environments on the <i>search()</i> path, instead of just those in <i>.GlobalEnv</i>. Check this, if you want to select objects from a loaded package."));
	connect (show_all_envs_action, &QAction::toggled, this, &RKVarSelector::rootChanged);

	filter_widget = nullptr;
	filter_widget_placeholder = new QVBoxLayout ();
	filter_widget_placeholder->setContentsMargins (0, 0, 0, 0);
	vbox->addLayout (filter_widget_placeholder);
	show_filter_action = new QAction (i18n ("Show filter options"), this);
	show_filter_action->setCheckable (true);
	show_filter_action->setShortcut (QKeySequence ("Ctrl+F"));
	show_filter_action->setIcon (RKStandardIcons::getIcon (RKStandardIcons::ActionSearch));
	connect (show_filter_action, &QAction::toggled, this, &RKVarSelector::showFilterWidget);

	list_view = new RKObjectListView (false, this);
	list_view->setSelectionMode (QAbstractItemView::ExtendedSelection);
	list_view->initialize ();
	vbox->addWidget (list_view);
	connect (list_view->selectionModel (), &QItemSelectionModel::selectionChanged, this, &RKVarSelector::objectSelectionChanged);

	QAction* sep = list_view->contextMenu ()->insertSeparator (list_view->contextMenu ()->actions ().value (0));
	list_view->contextMenu ()->insertAction (sep, show_filter_action);
	list_view->contextMenu ()->insertAction (sep, show_all_envs_action);
	advanced_button->setMenu (list_view->contextMenu ());

	rootChanged ();
	expanded = !expanded; // toggled right back, below
	toggleLevel1 ();
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
	if (show_filter_action->isChecked ()) {
		filter_widget->show ();
	} else {
		filter_widget->hide ();
		list_view->getSettings ()->resetFilters ();
	}
}

void RKVarSelector::rootChanged () {
	RK_TRACE (PLUGIN);

	RObject* object = root->objectValue ();
	if (!object) {
		if (!show_all_envs_action->isChecked ()) object = RObjectList::getGlobalEnv ();
	}
	list_view->setRootObject (object);
}

void RKVarSelector::toggleLevel1 () {
	RK_TRACE (PLUGIN);

	if (expanded) {
		expand_collapse_button->setIcon (RKStandardIcons::getIcon (RKStandardIcons::ActionExpandDown));
		RKCommonFunctions::setTips (i18n ("Expand all root level objects (show columns of data.frames)"), expand_collapse_button);
		list_view->collapseAll ();
	} else {
		expand_collapse_button->setIcon (RKStandardIcons::getIcon (RKStandardIcons::ActionCollapseUp));
		RKCommonFunctions::setTips (i18n ("Collapse all root level objects (hide columns of data.frames)"), expand_collapse_button);
		list_view->expandToDepth (1);
	}
	expanded = !expanded;
}

void RKVarSelector::objectSelectionChanged () {
	RK_TRACE (PLUGIN);

	selected->setObjectList (list_view->selectedObjects ());
	RK_DEBUG (PLUGIN, DL_DEBUG, "selected in varselector: %s", qPrintable (fetchStringValue (selected)));
}

