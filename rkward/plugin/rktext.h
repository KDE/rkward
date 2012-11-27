/***************************************************************************
                          rktext.h  -  description
                             -------------------
    begin                : Sun Nov 10 2002
    copyright            : (C) 2002, 2006, 2012 by Thomas Friedrichsmeier
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
	QVariant value (const QString &modifier=QString ()) { return (text->value (modifier)); };
	int type () { return ComponentText; };
public slots:
	void textChanged (RKComponentPropertyBase *);
private:
	QLabel *label;
};

#endif
