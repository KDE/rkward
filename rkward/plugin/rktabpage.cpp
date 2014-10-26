/***************************************************************************
                          rktabpage.cpp  -  description
                             -------------------
    begin                : Wed Apr 5 2006
    copyright            : (C) 2006, 2010 by Thomas Friedrichsmeier
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

#include "rktabpage.h"

#include <qstring.h>
#include <qtabwidget.h>
#include <QVBoxLayout>

#include <kvbox.h>

#include "../rkglobals.h"
#include "../misc/xmlhelper.h"
#include "../debug.h"

RKTabPage::RKTabPage (const QDomElement &element, RKComponent *parent_component, QTabWidget *parent_widget) : RKComponent (parent_component, parent_widget) {
	RK_TRACE (PLUGIN);

	XMLHelper* xml = parent_component->xmlHelper ();
	label = xml->i18nStringAttribute (element, "label", QString (), DL_WARNING);

	QVBoxLayout *layout = new QVBoxLayout (this);
	page = new KVBox (this);
	layout->addWidget (page);

	tabbook = parent_widget;
	tabbook->addTab (this, label);
	index = tabbook->indexOf (this);
	// for whatever reason, this needs to be set *after* the page was added to the tabbook
	page->setSizePolicy (QSizePolicy (QSizePolicy::Expanding, QSizePolicy::Expanding));

	inserted = true;
	connect (visibility_property, SIGNAL (valueChanged (RKComponentPropertyBase *)), this, SLOT (visibleEnabledChanged (RKComponentPropertyBase *)));
	connect (enabledness_property, SIGNAL (valueChanged (RKComponentPropertyBase *)), this, SLOT (visibleEnabledChanged (RKComponentPropertyBase *)));
}

RKTabPage::~RKTabPage () {
	RK_TRACE (PLUGIN);
}

void RKTabPage::visibleEnabledChanged (RKComponentPropertyBase *property) {
	RK_TRACE (PLUGIN);

	if (property == visibility_property) {
		if (visibility_property->boolValue ()) {
			if (!inserted) {
#warning this may not be reliable, if an earlier page is invisible as well
				tabbook->insertTab (index, this, label);
				inserted = true;
			}
		} else {
			if (inserted) {
				tabbook->removeTab (tabbook->indexOf (this));
				inserted = false;
			}
		}
	} else if (property == enabledness_property) {
		tabbook->setTabEnabled (tabbook->indexOf (this), enabledness_property->boolValue ());
	}
}

#include "rktabpage.moc"
