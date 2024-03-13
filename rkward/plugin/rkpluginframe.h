/*
rkpluginframe.cpp - This file is part of the RKWard project. Created: Sat Jun 4 2011
SPDX-FileCopyrightText: 2011-2018 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef RKPLUGINFRAME_H
#define RKPLUGINFRAME_H

#include "rkcomponent.h"

class QDomElement;
class QGroupBox;
class KVBox;

/** A passive component acting as a group box. Provides property "checked".

@author Thomas Friedrichsmeier
*/
class RKPluginFrame : public RKComponent {
	Q_OBJECT
public:
	RKPluginFrame (const QDomElement &element, RKComponent *parent_component, QWidget *parent_widget);
	~RKPluginFrame ();

/** returns the page child elements should be drawn in */
	QWidget *getPage ();
	int type () override { return ComponentFrame; };
/** reimplemented to return the value of the checked property by default */
	QVariant value (const QString &modifier=QString ()) override;

/** re-implemented to create "checked" property on demand. */
	RKComponentBase* lookupComponent (const QString &identifier, QString *remainder) override;
private Q_SLOTS:
/** called when checked property changes */
	void propertyChanged (RKComponentPropertyBase *property);
	void checkedChanged (bool new_state);
private:
	void initCheckedProperty ();
	RKComponentPropertyBool *checked;
	QGroupBox *frame;
};

#endif
