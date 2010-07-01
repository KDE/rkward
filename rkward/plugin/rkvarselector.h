/***************************************************************************
                          rkvarselector.h  -  description
                             -------------------
    begin                : Thu Nov 7 2002
    copyright            : (C) 2002, 2006, 2010 by Thomas Friedrichsmeier
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

#ifndef RKVARSELECTOR_H
#define RKVARSELECTOR_H

#include "rkcomponent.h"
#include "rkcomponentproperties.h"

class RKObjectListView;
class QDomElement;

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
	int type () { return ComponentVarSelector; };
private slots:
	void objectSelectionChanged ();
	void rootChanged ();
private:
	RKObjectListView *list_view;
	RKComponentPropertyRObjects *selected;
	RKComponentPropertyRObjects *root;
};

#endif
