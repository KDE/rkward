/*
rktabpage.cpp - This file is part of RKWard (https://rkward.kde.org). Created: Wed Apr 5 2006
SPDX-FileCopyrightText: 2006-2010 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "rktabpage.h"

#include <qstring.h>
#include <qtabwidget.h>
#include <QVBoxLayout>

#include "../misc/xmlhelper.h"
#include "../debug.h"

RKTabPage::RKTabPage (const QDomElement &element, RKComponent *parent_component, QTabWidget *parent_widget) : RKComponent (parent_component, parent_widget) {
	RK_TRACE (PLUGIN);

	XMLHelper* xml = parent_component->xmlHelper ();
	label = xml->i18nStringAttribute (element, "label", QString (), DL_WARNING);

	QVBoxLayout *layout = new QVBoxLayout (this);
	layout->setContentsMargins (0, 0, 0, 0);

	tabbook = parent_widget;
	tabbook->addTab (this, label);
	index = tabbook->indexOf (this);

	inserted = true;
	connect (visibility_property, &RKComponentPropertyBase::valueChanged, this, &RKTabPage::visibleEnabledChanged);
	connect (enabledness_property, &RKComponentPropertyBase::valueChanged, this, &RKTabPage::visibleEnabledChanged);
}

RKTabPage::~RKTabPage () {
	RK_TRACE (PLUGIN);
}

void RKTabPage::visibleEnabledChanged (RKComponentPropertyBase *property) {
	RK_TRACE (PLUGIN);

	if (property == visibility_property) {
		if (visibility_property->boolValue ()) {
			if (!inserted) {
#ifdef __GNUC__
#	warning this may not be reliable, if an earlier page is invisible as well
#endif
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

