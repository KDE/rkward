/***************************************************************************
                          rkpluginsaveobject  -  description
                             -------------------
    begin                : Tue Jan 30 2007
    copyright            : (C) 2007, 2010, 2012, 2014 by Thomas Friedrichsmeier
    email                : thomas.friedrichsmeier@kdemail.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef RKPLUGINSAVEOBJECT_H
#define RKPLUGINSAVEOBJECT_H

#include "rkcomponent.h"

#include "rkcomponentproperties.h"

class RKSaveObjectChooser;
class QDomElement;
class QGroupBox;

/** RKComponent to select an R object name to save something to

@author Thomas Friedrichsmeier
*/

class RKPluginSaveObject : public RKComponent {
	Q_OBJECT
public:
	RKPluginSaveObject (const QDomElement &element, RKComponent *parent_component, QWidget *parent_widget);
	~RKPluginSaveObject ();

	QVariant value (const QString &modifier=QString ()) override;
	QStringList getUiLabelPair () const override;
	int type () override { return ComponentSaveObject; };
	bool isValid () override;
public slots:
	void externalChange ();
	void internalChange ();
private:
	RKSaveObjectChooser *selector;
	void update ();
	bool updating;

	QGroupBox* groupbox;

	RKComponentPropertyBase *selection;
	RKComponentPropertyBase *objectname;
	RKComponentPropertyRObjects *parent;
	RKComponentPropertyBool *active;
};

#endif
