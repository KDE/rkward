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

#include "rkpluginframe.h"

#include <QVBoxLayout>
#include <QGroupBox>

#include <kvbox.h>

#include "../rkglobals.h"
#include "../misc/xmlhelper.h"
#include "../debug.h"

RKPluginFrame::RKPluginFrame (const QDomElement &element, RKComponent *parent_component, QWidget *parent_widget) : RKComponent (parent_component, parent_widget) {
	RK_TRACE (PLUGIN);

	XMLHelper* xml = XMLHelper::getStaticHelper ();

	QVBoxLayout *layout = new QVBoxLayout (this);
	layout->setContentsMargins (0, 0, 0, 0);
	frame = new QGroupBox (xml->getStringAttribute (element, "label", QString(), DL_INFO), this);
	layout->addWidget (frame);
	layout = new QVBoxLayout (frame);
	page = new KVBox (this);
	page->setSpacing (RKGlobals::spacingHint ());
	layout->addWidget (page);

	checked = 0;
	if (xml->getBoolAttribute (element, "checkable", false, DL_INFO)) {
		frame->setCheckable (true);
		frame->setChecked (xml->getBoolAttribute (element, "checked", false, DL_INFO));
		initCheckedProperty ();
		connect (frame, SIGNAL (toggled(bool)), this, SLOT (checkedChanged(bool)));
	}
}

RKPluginFrame::~RKPluginFrame () {
	RK_TRACE (PLUGIN);
}

void RKPluginFrame::initCheckedProperty () {
	RK_TRACE (PLUGIN);
	RK_ASSERT (!checked);
	if (!frame->isCheckable ()) {
		RK_DO (qDebug ("This frame does not have a property 'checked', as it is not checkable"), PLUGIN, DL_DEBUG);
		return;
	}

	addChild ("checked", checked = new RKComponentPropertyBool (this, false, frame->isChecked (), "1", "0"));
	connect (checked, SIGNAL (valueChanged (RKComponentPropertyBase *)), this, SLOT (propertyChanged (RKComponentPropertyBase *)));
	checked->setInternal (true);
}

RKComponentBase* RKPluginFrame::lookupComponent (const QString& identifier, QString* remainder) {
	if ((!checked) && (identifier == "checked")) initCheckedProperty ();
	return RKComponentBase::lookupComponent(identifier, remainder);
}

void RKPluginFrame::propertyChanged (RKComponentPropertyBase* property) {
	RK_TRACE (PLUGIN);

	RK_ASSERT (checked && (property == checked));
	if (frame->isChecked () == checked->boolValue ()) return;
	frame->setChecked (checked->boolValue ());
	changed ();
}

void RKPluginFrame::checkedChanged (bool new_state) {
	RK_TRACE (PLUGIN);
	RK_ASSERT (checked);

	if (new_state == checked->boolValue ()) return;
	checked->setBoolValue (new_state);
	changed ();
}

#include "rkpluginframe.moc"
