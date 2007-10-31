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
class QListWidget;

/** This RKPluginWidget provides a drop down list of options for use in plugins
@author Thomas Friedrichsmeier
*/
class RKDropDown : public RKAbstractOptionSelector {
	Q_OBJECT
public: 
	RKDropDown (const QDomElement &element, RKComponent *parent_component, QWidget *parent_widget);
	~RKDropDown ();
	int type () { return ComponentDropDown; };
public slots:
	// workaround for a qt bug (3.3.7)
	void comboItemActivated (int id);
protected:
	void setItemInGUI (int id);
	void addOptionToGUI (const QString &label, int id);
	void setItemEnabledInGUI (int id, bool enabled);
private:
	QComboBox *box;
	QListWidget *listwidget;
};

#endif
