/*
rkdropdown.h - This file is part of the RKWard project. Created: Fri Jan 12 2007
SPDX-FileCopyrightText: 2007-2014 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef RKDROPDOWN_H
#define RKDROPDOWN_H

#include "rkabstractoptionselector.h"

class QComboBox;
class QListWidget;
class QLabel;

/** This RKPluginWidget provides a drop down list of options for use in plugins
@author Thomas Friedrichsmeier
*/
class RKDropDown : public RKAbstractOptionSelector {
	Q_OBJECT
public: 
	RKDropDown (const QDomElement &element, RKComponent *parent_component, QWidget *parent_widget);
	~RKDropDown ();
	int type () override { return ComponentDropDown; };
public Q_SLOTS:
	// workaround for a qt bug (3.3.7)
	void comboItemActivated (int id);
protected:
	void setItemInGUI (int id) override;
	void addOptionToGUI (const QString &label, int id) override;
	void setItemEnabledInGUI (int id, bool enabled) override;
	QStringList getUiLabelPair () const override;
private:
	QComboBox *box;
	QListWidget *listwidget;
	QLabel *label;
};

#endif
