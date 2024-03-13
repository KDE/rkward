/*
rkpluginspinbox - This file is part of the RKWard project. Created: Wed Aug 11 2004
SPDX-FileCopyrightText: 2004-2014 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef RKPLUGINSPINBOX_H
#define RKPLUGINSPINBOX_H

#include "rkcomponent.h"

#include "rkcomponentproperties.h"

class RKSpinBox;
class QDomElement;
class QLabel;

/** RKComponent for numeric input represented as a spinbox
TODO: rename file and class to RKComponentSpinBox
@author Thomas Friedrichsmeier
*/
class RKPluginSpinBox : public RKComponent {
	Q_OBJECT
public:
	RKPluginSpinBox (const QDomElement &element, RKComponent *parent_component, QWidget *parent_widget);

	~RKPluginSpinBox ();
	QVariant value (const QString &modifier=QString ()) override;
	QStringList getUiLabelPair () const override;
	int type () override { return ComponentSpinBox; };

	RKComponentPropertyInt *intvalue;
	RKComponentPropertyDouble *realvalue;
public Q_SLOTS:
	void valueChangedFromUi ();
	void valueChanged (RKComponentPropertyBase *property);
private:
	RKSpinBox *spinbox;
	QLabel *label;
	bool intmode;
	bool updating;
};

#endif
