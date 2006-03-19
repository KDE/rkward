/***************************************************************************
                          rkradio.h  -  description
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

#ifndef RKRADIO_H
#define RKRADIO_H

#include "rkcomponent.h"
#include "rkcomponentproperties.h"

#include <qmap.h>

class QButtonGroup;
class QRadioButton;
class QDomElement;

/** This RKPluginWidget provides a group of radio-buttons.
  *@author Thomas Friedrichsmeier
  */

class RKRadio : public RKComponent {
	Q_OBJECT
public: 
	RKRadio (const QDomElement &element, RKComponent *parent_component, QWidget *parent_widget);
	~RKRadio ();
	int type () { return ComponentRadio; };
/** Find the option number with the corresponding string. If not found, returns -1
@param option_string the option string to search for
@returns the id (0, 1, 2...) of the corresponding option, or -1 if not found */
	int findOption (const QString &option_string);
	QString value (const QString &modifier) { return (string->value (modifier)); };
public slots:
	void buttonClicked (int id);
	void propertyChanged (RKComponentPropertyBase *property);
private:
	RKComponentPropertyBase *string;
	RKComponentPropertyInt *number;

	bool updating;		// prevent recursion
	QButtonGroup *group;
	typedef QMap<int, QString> OptionsMap;
	OptionsMap options;
};

#endif
