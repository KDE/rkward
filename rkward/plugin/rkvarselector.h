/*
rkvarselector.h - This file is part of the RKWard project. Created: Thu Nov 7 2002
SPDX-FileCopyrightText: 2002-2018 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef RKVARSELECTOR_H
#define RKVARSELECTOR_H

#include "rkcomponent.h"
#include "rkcomponentproperties.h"

class RKObjectListView;
class QDomElement;
class QAction;
class QVBoxLayout;
class QToolButton;

/** This is an especially important RK-plugin-widget. It provides a list of variables, that
can be selected for statistical analysis.
It's major counterpart is the RKVarSlot-class, which "takes" variables selected from
this list. Most of the time, you'll only have a single RKVarSelector in a GUI, but
you can have more than one, e.g. for serving conceptionally different RKVarSlots.

@author Thomas Friedrichsmeier
*/
class RKVarSelector : public RKComponent {
	Q_OBJECT
public: 
	RKVarSelector (const QDomElement &element, RKComponent *parent_component, QWidget *parent_widget);
	~RKVarSelector ();
	int type () override { return ComponentVarSelector; };
private Q_SLOTS:
	void objectSelectionChanged ();
	void rootChanged ();
	void showFilterWidget ();
	void toggleLevel1 ();
private:
	RKObjectListView *list_view;
	RKComponentPropertyRObjects *selected;
	RKComponentPropertyRObjects *root;
	QAction *show_filter_action;
	QAction *show_all_envs_action;
	QWidget *filter_widget;
	QVBoxLayout *filter_widget_placeholder;
	QToolButton *expand_collapse_button;
	static bool expanded;
};

#endif
