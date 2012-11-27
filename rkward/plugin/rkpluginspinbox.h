/***************************************************************************
                          rkpluginspinbox  -  description
                             -------------------
    begin                : Wed Aug 11 2004
    copyright            : (C) 2004, 2006, 2012 by Thomas Friedrichsmeier
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
#ifndef RKPLUGINSPINBOX_H
#define RKPLUGINSPINBOX_H

#include "rkcomponent.h"

#include "rkcomponentproperties.h"

class RKSpinBox;
class QDomElement;

/** RKComponent for numeric input represented as a spinbox
TODO: rename file and class to RKComponentSpinBox
@author Thomas Friedrichsmeier
*/
class RKPluginSpinBox : public RKComponent {
	Q_OBJECT
public:
	RKPluginSpinBox (const QDomElement &element, RKComponent *parent_component, QWidget *parent_widget);

	~RKPluginSpinBox ();
	QVariant value (const QString &modifier=QString ());
	int type () { return ComponentSpinBox; };

	RKComponentPropertyInt *intvalue;
	RKComponentPropertyDouble *realvalue;
public slots:
	void valueChanged (int);
	void valueChanged (RKComponentPropertyBase *property);
private:
	RKSpinBox *spinbox;
	bool intmode;
	bool updating;
};

#endif
