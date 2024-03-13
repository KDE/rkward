/*
rktabpage.cpp - This file is part of the RKWard project. Created: Wed Apr 5 2006
SPDX-FileCopyrightText: 2006 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef RKTABPAGE_H
#define RKTABPAGE_H

#include "rkcomponent.h"

#include <qstring.h>

class QDomElement;
class QTabWidget;

/** A passive component acting as a page in a tabbook. The only function is, that if the component is hidden, the corresponding tab in the tabbook is also hidden.

@author Thomas Friedrichsmeier
*/
class RKTabPage : public RKComponent {
	Q_OBJECT
public:
	RKTabPage (const QDomElement &element, RKComponent *parent_component, QTabWidget *parent_widget);

	~RKTabPage ();

	/** @returns the page child elements should be drawn in */
	QWidget *getPage () { return this; };

	int type () override { return ComponentTab; };

public Q_SLOTS:
/** called when visibile or enabled properties change */
	void visibleEnabledChanged (RKComponentPropertyBase *property);
private:
	QTabWidget *tabbook;
	int index;
	QString label;
	bool inserted;
};

#endif
