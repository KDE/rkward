/*
rkradio.h - This file is part of the RKWard project. Created: Thu Nov 7 2002
SPDX-FileCopyrightText: 2002-2014 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef RKRADIO_H
#define RKRADIO_H

#include "rkabstractoptionselector.h"

class QButtonGroup;
class QGroupBox;

/** This RKPluginWidget provides a group of radio-buttons for use in plugins.
@author Thomas Friedrichsmeier
*/
class RKRadio : public RKAbstractOptionSelector {
	Q_OBJECT
public: 
	RKRadio (const QDomElement &element, RKComponent *parent_component, QWidget *parent_widget);
	~RKRadio ();
	int type () override { return ComponentRadio; };
protected:
	void setItemInGUI (int id) override;
	void addOptionToGUI (const QString &label, int id) override;
	void setItemEnabledInGUI (int id, bool enabled) override;
	QStringList getUiLabelPair () const override;
private:
	QButtonGroup* group;
	QGroupBox* group_box;
};

#endif
