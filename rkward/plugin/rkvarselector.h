/***************************************************************************
                          rkvarselector.h  -  description
                             -------------------
    begin                : Thu Nov 7 2002
    copyright            : (C) 2002, 2006 by Thomas Friedrichsmeier
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

#include "rkstandardcomponent.h"
#include "rkcomponentproperties.h"

class RKObjectListView;

/** This is an especially important RK-plugin-widget. It provides a list of variables
(derived from the document's table), that can be selected for statistical analysis.
It's major counterpart is the RKVarSlot-class, which "takes" variables selected from
this list. Most of the time, you'll only have a single VarSelector in a GUI, but
you can have more than one, e.g. for serving conceptionally different VarSlots.

@author Thomas Friedrichsmeier
  */


class RKVarSelector : public RKComponent {
   Q_OBJECT
public: 
	RKVarSelector(const QDomElement &element, RKComponent *parent_component, QWidget *parent_widget);
	~RKVarSelector();
	int type () { return ComponentVarSelector; };
public slots:
	void objectListChanged ();
	void objectSelectionChanged ();
private:
	RKObjectListView *list_view;
	RKComponentPropertyRObjects *selected;
	RKComponentPropertyRObjects *available;
};

#endif
