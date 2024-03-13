/*
rkvarslot.h - This file is part of the RKWard project. Created: Thu Nov 7 2002
SPDX-FileCopyrightText: 2002-2018 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

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
	int type () override {return ComponentVarSlot; };
	QVariant value (const QString &modifier=QString ()) override { return (available->value (modifier)); };
	QStringList getUiLabelPair () const override;
/** reimplemented to call updateLook (), in case enabledness changed */
	void changed () override { updateLook (); RKComponent::changed (); };
	void updateLook ();
public Q_SLOTS:
/** Called when the select-button is pressed */
	void selectPressed ();
	void removePressed ();
	void listSelectionChanged ();
	void availablePropertyChanged (RKComponentPropertyBase *);
protected:
/** Calls updateLook (), when enabledness changes */
	void changeEvent (QEvent *event) override;
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
