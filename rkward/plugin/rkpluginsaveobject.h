/*
rkpluginsaveobject - This file is part of the RKWard project. Created: Tue Jan 30 2007
SPDX-FileCopyrightText: 2007-2014 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

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
public Q_SLOTS:
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
