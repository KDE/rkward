/*
rktext.h - This file is part of the RKWard project. Created: Sun Nov 10 2002
SPDX-FileCopyrightText: 2002-2012 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef RKTEXT_H
#define RKTEXT_H

#include "rkcomponent.h"

#include "rkcomponentproperties.h"

class QLabel;
class QDomElement;

/**
  *@author Thomas Friedrichsmeier
  */
class RKText : public RKComponent {
	Q_OBJECT
public:
	RKText (const QDomElement &element, RKComponent *parent_component, QWidget *parent_widget);

	~RKText ();

	RKComponentPropertyBase *text;
	QVariant value (const QString &modifier=QString ()) override { return (text->value (modifier)); };
	int type () override { return ComponentText; };
public Q_SLOTS:
	void textChanged (RKComponentPropertyBase *);
private:
	QLabel *label;
};

#endif
