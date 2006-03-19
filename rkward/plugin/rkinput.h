/***************************************************************************
                          rkinput  -  description
                             -------------------
    begin                : Sat Mar 10 2005
    copyright            : (C) 2005, 2006 by Thomas Friedrichsmeier
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

#ifndef RKINPUT_H
#define RKINPUT_H

#include "rkcomponent.h"

#include "rkcomponentproperties.h"

class QTextEdit;
class QDomElement;

/** A component to enter plain text

TODO: Use separate internal widgets for single line and multi line input

@author Adrien d'Hardemare
*/
class RKInput : public RKComponent {
	Q_OBJECT
public:
	RKInput (const QDomElement &element, RKComponent *parent_component, QWidget *parent_widget);
	~RKInput ();

	RKComponentPropertyBase *text;
	QString value (const QString &modifier) { return (text->value (modifier)); };
public slots:
	void textChanged ();
	void textChanged (RKComponentPropertyBase *);
private:
	bool updating;
	QTextEdit *textedit;
};

#endif
