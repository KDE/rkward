/***************************************************************************
                          rkradio.h  -  description
                             -------------------
    begin                : Thu Nov 7 2002
    copyright            : (C) 2002, 2006, 2007, 2014 by Thomas Friedrichsmeier
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
