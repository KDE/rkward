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

#include "rkpluginbrowser.h"

#include <qlayout.h>

#include <klocale.h>

#include "../misc/xmlhelper.h"
#include "../misc/getfilenamewidget.h"
#include "../rkglobals.h"
#include "../debug.h"

RKPluginBrowser::RKPluginBrowser (const QDomElement &element, RKComponent *parent_component, QWidget *parent_widget) : RKComponent (parent_component, parent_widget) {
	RK_TRACE (PLUGIN);

	// get xml-helper
	XMLHelper *xml = XMLHelper::getStaticHelper ();

	// create and add property
	addChild ("selection", selection = new RKComponentPropertyBase (this, true));
	connect (selection, SIGNAL (valueChanged (RKComponentPropertyBase *)), this, SLOT (textChanged (RKComponentPropertyBase *)));

	QVBoxLayout *vbox = new QVBoxLayout (this, RKGlobals::spacingHint ());

	int intmode = xml->getMultiChoiceAttribute (element, "type", "file;dir;savefile", 0, DL_INFO);
	GetFileNameWidget::FileType mode;
	if (intmode == 0) {
		mode = GetFileNameWidget::ExistingFile;
	} else if (intmode == 0) {
		mode = GetFileNameWidget::ExistingDirectory;
	} else {
		mode = GetFileNameWidget::SaveFile;
	}
	selector = new GetFileNameWidget (this, mode, xml->getStringAttribute (element, "label", i18n ("Enter filename"), DL_INFO), i18n ("Select"), xml->getStringAttribute (element, "initial", QString::null, DL_INFO));
	selector->setFilter (xml->getStringAttribute (element, "filter", QString::null, DL_INFO));
	connect (selector, SIGNAL (locationChanged ()), SLOT (textChanged ()));

	vbox->addWidget (selector);

	// initialize
	updating = true;
	textChanged ();
	updating = false;
}

RKPluginBrowser::~RKPluginBrowser () {
	RK_TRACE (PLUGIN);
}

void RKPluginBrowser::textChanged (RKComponentPropertyBase *) {
	RK_TRACE (PLUGIN);

	if (updating) return;
	updating = true;

	selector->setLocation (selection->value ());

	updating = false;
	changed ();
}

void RKPluginBrowser::textChanged () {
	RK_TRACE (PLUGIN);

	selection->setValue (selector->getLocation ());
}

#include "rkpluginbrowser.moc"
