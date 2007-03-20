/***************************************************************************
                          rkdropdown.h  -  description
                             -------------------
    begin                : Fri Jan 12 2007
    copyright            : (C) 2007 by Thomas Friedrichsmeier
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

#ifndef RKDROPDOWN_H
#define RKDROPDOWN_H

#include "rkabstractoptionselector.h"

class QComboBox;

/** This RKPluginWidget provides a group of radio-buttons.
@author Thomas Friedrichsmeier
*/
class RKDropDown : public RKAbstractOptionSelector {
	Q_OBJECT
public: 
	RKDropDown (const QDomElement &element, RKComponent *parent_component, QWidget *parent_widget);
	~RKDropDown ();
	int type () { return ComponentDropDown; };
public slots:
	// workaround for a qt bug
	void comboItemActivated (int id);
protected:
	void setItemInGUI (int id);
	void addOptionToGUI (const QString &label, int id);
	void setItemEnabledInGUI (int id, bool enabled);
private:
	QComboBox *box;
};

#include <qlistbox.h>

#define ID_RKDROPDOWNLISTITEM 1001

/** Item used in RKDropDown. The difference to a regular QListBoxText is that the item looks different when disabled */
class RKDropDownListItem : public QListBoxText {
public:
	RKDropDownListItem (QListBox *listbox, const QString &text);
	~RKDropDownListItem () {};
	int rtti () const { return ID_RKDROPDOWNLISTITEM; };
protected:
	void paint (QPainter *painter);
};

#endif
