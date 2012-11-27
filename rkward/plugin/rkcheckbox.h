/***************************************************************************
                          rkcheckbox  -  description
                             -------------------
    begin                : Fri Jul 30 2004
    copyright            : (C) 2004, 2006 by Thomas Friedrichsmeier
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
	int type () { return ComponentCheckBox; };
	RKComponentPropertyBool *state;
	QVariant value (const QString &modifier=QString ()) {
		if (modifier.isEmpty ()) return state->value ("labeled");
		return (state->value (modifier));
	};
public slots:
	void changedState (int);
	void changedState (RKComponentPropertyBase *);
private:
	bool updating;		// prevent recursion
	QCheckBox *checkbox;
};

#endif
