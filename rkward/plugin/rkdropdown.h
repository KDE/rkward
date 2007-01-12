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

#include "rkcomponent.h"
#include "rkcomponentproperties.h"

#include <qmap.h>

class QComboBox;
class QDomElement;

/** This RKPluginWidget provides a group of radio-buttons.
  *@author Thomas Friedrichsmeier
  */

class RKDropDown : public RKComponent {
	Q_OBJECT
public: 
	RKDropDown (const QDomElement &element, RKComponent *parent_component, QWidget *parent_widget);
	~RKDropDown ();
	int type () { return ComponentDropDown; };
/** Find the option number with the corresponding string. If not found, returns -1
@param option_string the option string to search for
@returns the id (0, 1, 2...) of the corresponding option, or -1 if not found */
	int findOption (const QString &option_string);
	QString value (const QString &modifier) { return (string->value (modifier)); };
public slots:
	void itemSelected (int id);
	void propertyChanged (RKComponentPropertyBase *property);
private:
	RKComponentPropertyBase *string;
	RKComponentPropertyInt *number;

	bool updating;		// prevent recursion
	QComboBox *box;
	typedef QMap<int, QString> OptionsMap;
	OptionsMap options;
};

#endif
