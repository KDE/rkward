/***************************************************************************
                          rkradio.h  -  description
                             -------------------
    begin                : Thu Nov 7 2002
    copyright            : (C) 2002, 2006 by Thomas Friedrichsmeier
    email                : tfry@users.sourceforge.net
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

class Q3ButtonGroup;

/** This RKPluginWidget provides a group of radio-buttons for use in plugins.
@author Thomas Friedrichsmeier
*/
class RKRadio : public RKAbstractOptionSelector {
	Q_OBJECT
public: 
	RKRadio (const QDomElement &element, RKComponent *parent_component, QWidget *parent_widget);
	~RKRadio ();
	int type () { return ComponentRadio; };
protected:
	void setItemInGUI (int id);
	void addOptionToGUI (const QString &label, int id);
	void setItemEnabledInGUI (int id, bool enabled);
private:
	Q3ButtonGroup *group;
};

#endif
