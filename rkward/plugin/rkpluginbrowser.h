/*
rkpluginbrowser - This file is part of the RKWard project. Created: Sat Mar 10 2005
SPDX-FileCopyrightText: 2005-2018 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef RKPLUGINBROWSER_H
#define RKPLUGINBROWSER_H

#include "rkcomponent.h"

#include "rkcomponentproperties.h"

#include <QTimer>

class GetFileNameWidget;
class QDomElement;
class QCheckBox;

/** RKComponent to select one or more file(s) or directories

TODO: Rename to something like RKComponentFileSelect or a similarly ugly name
TODO: I ripped out multiple file selection for now. GetFileNameWidget should be extended to handle that internally (tfry)

@author Adrien d'Hardemare
*/

class RKPluginBrowser : public RKComponent {
	Q_OBJECT
public:
	RKPluginBrowser (const QDomElement &element, RKComponent *parent_component, QWidget *parent_widget);
	~RKPluginBrowser ();

	RKComponentPropertyBase *selection;
	QVariant value (const QString &modifier=QString ()) override { return (selection->value (modifier)); };
	QStringList getUiLabelPair () const override;
	int type () override { return ComponentBrowser; };
	ComponentStatus recursiveStatus () override;
public Q_SLOTS:
	void textChangedFromUi ();
	void textChanged (RKComponentPropertyBase *);
	void validateInput ();
private:
	void updateColor ();
	QTimer validation_timer;
	ComponentStatus status;
	GetFileNameWidget *selector;
	bool updating;
	bool only_local;
	QString label_string;
	QCheckBox* overwrite_confirm;
};

#endif
