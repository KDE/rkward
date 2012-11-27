/***************************************************************************
                          rkpluginframe.cpp  -  description
                             -------------------
    begin                : Sat Jun 4 2011
    copyright            : (C) 2011 by Thomas Friedrichsmeier
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
	KVBox *getPage () { return page; };
	int type () { return ComponentFrame; };
/** reimplemented to return the value of the checked property by default */
	QVariant value (const QString &modifier=QString ());

/** re-implemented to create "checked" property on demand. */
	RKComponentBase* lookupComponent (const QString &identifier, QString *remainder);
private slots:
/** called when checked property changes */
	void propertyChanged (RKComponentPropertyBase *property);
	void checkedChanged (bool new_state);
private:
	void initCheckedProperty ();
	RKComponentPropertyBool *checked;
	KVBox *page;
	QGroupBox *frame;
};

#endif
