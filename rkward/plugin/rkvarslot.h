/***************************************************************************
                          rkvarslot.h  -  description
                             -------------------
    begin                : Thu Nov 7 2002
    copyright            : (C) 2002 - 2015 by Thomas Friedrichsmeier
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
class QTreeWidget;
class QTreeWidgetItem;
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
	QVariant value (const QString &modifier=QString ()) { return (available->value (modifier)); };
	QStringList getUiLabelPair () const;
/** reimplemented to call updateLook (), in case enabledness changed */
	void changed () { updateLook (); RKComponent::changed (); };
	void updateLook ();
public slots:
/** Called when the select-button is pressed */
	void selectPressed ();
	void removePressed ();
	void listSelectionChanged ();
	void availablePropertyChanged (RKComponentPropertyBase *);
protected:
/** Calls updateLook (), when enabledness changes */
	void enabledChange (bool old) { updateLook (); QWidget::enabledChange (old); };
private:
	void addOrRemove (bool add);
	enum {
		Varslot,
		Valueslot
	} mode;

	bool multi;
	bool updating;
	QString label_string;

/** the available objects (typically a copy of the property of the varselector) */
	RKComponentPropertyAbstractList *source;
/** the objects in the varslot */
	RKComponentPropertyAbstractList *available;
/** of the objects in the varslot, those that are marked */
	RKComponentPropertyAbstractList *selected;

	QTreeWidget *list;
	QPushButton *select_button;
	QPushButton *remove_button;
};

#endif
