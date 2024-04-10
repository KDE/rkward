/*
rkpluginframe.cpp - This file is part of RKWard (https://rkward.kde.org). Created: Sat Jun 4 2011
SPDX-FileCopyrightText: 2011-2018 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "rkpluginframe.h"

#include <QVBoxLayout>
#include <QGroupBox>

#include "../misc/rkstyle.h"
#include "../misc/xmlhelper.h"
#include "../debug.h"

RKPluginFrame::RKPluginFrame (const QDomElement &element, RKComponent *parent_component, QWidget *parent_widget) : RKComponent (parent_component, parent_widget) {
	RK_TRACE (PLUGIN);

	XMLHelper* xml = parent_component->xmlHelper ();

	QVBoxLayout *layout = new QVBoxLayout (this);
	layout->setContentsMargins (0, 0, 0, 0);
	frame = new QGroupBox (xml->i18nStringAttribute (element, "label", QString(), DL_INFO));
	layout->addWidget (frame);

	QVBoxLayout *inner = new QVBoxLayout (frame);
	inner->setSpacing(RKStyle::spacingHint());

	checked = nullptr;
	if (xml->getBoolAttribute (element, "checkable", false, DL_INFO)) {
		frame->setCheckable (true);
		frame->setChecked (xml->getBoolAttribute (element, "checked", true, DL_INFO));
		initCheckedProperty ();
		connect (frame, &QGroupBox::toggled, this, &RKPluginFrame::checkedChanged);
	}
}

RKPluginFrame::~RKPluginFrame () {
	RK_TRACE (PLUGIN);
}

void RKPluginFrame::initCheckedProperty () {
	RK_TRACE (PLUGIN);
	RK_ASSERT (!checked);
	if (!frame->isCheckable ()) {
		RK_DEBUG (PLUGIN, DL_DEBUG, "This frame does not have a property 'checked', as it is not checkable");
		return;
	}

	addChild ("checked", checked = new RKComponentPropertyBool (this, false, frame->isChecked (), "1", "0"));
	connect (checked, &RKComponentPropertyBase::valueChanged, this, &RKPluginFrame::propertyChanged);
}

QWidget* RKPluginFrame::getPage () {
	return frame;
}

RKComponentBase* RKPluginFrame::lookupComponent (const QString& identifier, QString* remainder) {
	if ((!checked) && (identifier == "checked")) initCheckedProperty ();
	return RKComponentBase::lookupComponent(identifier, remainder);
}

QVariant RKPluginFrame::value (const QString &modifier) {
	if (!frame->isCheckable ()) return RKComponent::value (modifier);
	if (!checked) initCheckedProperty();
	return checked->value (modifier);
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

