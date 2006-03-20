/***************************************************************************
                          rkpluginbrowser  -  description
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

#ifndef RKPLUGINBROWSER_H
#define RKPLUGINBROWSER_H

#include "rkcomponent.h"

#include "rkcomponentproperties.h"

class GetFileNameWidget;
class QDomElement;

/** RKComponent to select one or more file(s) or directories

TODO: Rename to somehting like RKComponentFileSelect or a similarilly ugly name
TODO: I ripped out multiple file selection for now. GetFileNameWidget should be extended to handle that internally (tfry)

@author Adrien d'Hardemare
*/

class RKPluginBrowser : public RKComponent {
	Q_OBJECT
public:
	RKPluginBrowser (const QDomElement &element, RKComponent *parent_component, QWidget *parent_widget);
	~RKPluginBrowser ();

	RKComponentPropertyBase *selection;
	QString value (const QString &modifier) { return (selection->value (modifier)); };
	int type () { return ComponentBrowser; };
public slots:
	void textChanged ();
	void textChanged (RKComponentPropertyBase *);
private:
	GetFileNameWidget *selector;
	bool updating;
};

#endif
