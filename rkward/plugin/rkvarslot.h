/***************************************************************************
                          rkvarslot.h  -  description
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

#ifndef RKVARSLOT_H
#define RKVARSLOT_H

#include <rkcomponent.h>

#include <qmap.h>

class QLineEdit;
class QPushButton;
class QListView;
class QListViewItem;
class QDomElement;

class RKVariable;
class RContainerObject;

/** An RKVarSlot typically takes one or more variable(s) from an RKVarSelector.
  *@author Thomas Friedrichsmeier
  */

class RKVarSlot : public RKComponent {
	Q_OBJECT
public: 
	RKVarSlot (const QDomElement &element, RKComponent *parent_component, QWidget *parent_widget);
	~RKVarSlot ();
	int type () {return ComponentVarSlot; };
	QString value (const QString &modifier) { return (available->value (modifier)); };
public slots:
/** Called when the select-button is pressed */
	void selectPressed ();
	void listSelectionChanged ();
	void availablePropertyChanged (RKComponentPropertyBase *);
private:
/** change the select button to left/right / add/remove
@param add if true, button shows arrow right, or signifies more values would be added. Else the other way around */
	void setSelectButton (bool add);
	bool add_mode;
	bool multi;

/** the available objects (typically a copy of the property of the varselector) */
	RKComponentPropertyRObjects *source;
/** the objects in the varslot */
	RKComponentPropertyRObjects *available;
/** of the objects in the varslot, those that are marked */
	RKComponentPropertyRObjects *selected;

	QListView *list;
	QPushButton *select;
	typedef QMap<QListViewItem*, RObject*> ItemMap;
	ItemMap item_map;
};

#endif
