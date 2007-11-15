/***************************************************************************
                          rktabpage.cpp  -  description
                             -------------------
    begin                : Wed Apr 5 2006
    copyright            : (C) 2006 by Thomas Friedrichsmeier
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

#ifndef RKTABPAGE_H
#define RKTABPAGE_H

#include "rkcomponent.h"

#include <qstring.h>

class QDomElement;
class QTabWidget;
class KVBox;

/** A passive component acting as a page in a tabbook. The only function is, that if the component is hidden, the corresponding tab in the tabbook is also hidden.

@author Thomas Friedrichsmeier
*/
class RKTabPage : public RKComponent {
	Q_OBJECT
public:
	RKTabPage (const QDomElement &element, RKComponent *parent_component, QTabWidget *parent_widget);

	~RKTabPage ();

	// returns the page child elements should be drawn in
	KVBox *getPage () { return page; };

	int type () { return ComponentTab; };

public slots:
/** called when visibile or enabled properties change */
	void visibleEnabledChanged (RKComponentPropertyBase *property);
private:
	KVBox *page;
	QTabWidget *tabbook;
	int index;
	QString label;
	bool inserted;
};

#endif
