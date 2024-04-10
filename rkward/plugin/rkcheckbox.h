/*
rkcheckbox - This file is part of the RKWard project. Created: Fri Jul 30 2004
SPDX-FileCopyrightText: 2004-2014 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef RKCHECKBOX_H
#define RKCHECKBOX_H

#include "rkcomponent.h"

#include "rkcomponentproperties.h"
#include <qstring.h>

class QCheckBox;
class QDomElement;

/**
This RKComponent provides a checkbox

@author Thomas Friedrichsmeier
*/
class RKCheckBox : public RKComponent  {
	Q_OBJECT
public: 
	RKCheckBox (const QDomElement &element, RKComponent *parent_component, QWidget *parent_widget);
	~RKCheckBox ();
	int type () override { return ComponentCheckBox; };
	RKComponentPropertyBool *state;
	QVariant value (const QString &modifier=QString ()) override {
		if (modifier.isEmpty ()) return state->value ("labeled");
		return (state->value (modifier));
	};
	QStringList getUiLabelPair () const override;
public Q_SLOTS:
	void changedStateFromUi (int);
	void changedState (RKComponentPropertyBase *);
private:
	bool updating;		// prevent recursion
	QCheckBox *checkbox;
};

#endif
