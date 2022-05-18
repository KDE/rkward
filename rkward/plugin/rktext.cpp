/*
rktext.cpp - This file is part of RKWard (https://rkward.kde.org). Created: Sun Nov 10 2002
SPDX-FileCopyrightText: 2002-2014 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "rktext.h"

#include <qstring.h>
#include <qstringlist.h>
#include <qlabel.h>
#include <qdom.h>
#include <QVBoxLayout>

#include "../misc/xmlhelper.h"
#include "../misc/rkcommonfunctions.h"
#include "../debug.h"

RKText::RKText (const QDomElement &element, RKComponent *parent_component, QWidget *parent_widget) : RKComponent (parent_component, parent_widget) {
	RK_TRACE (PLUGIN);

	// get xml-helper
	XMLHelper *xml = parent_component->xmlHelper ();

	// create layout and label
	QVBoxLayout *vbox = new QVBoxLayout (this);
	vbox->setContentsMargins (0, 0, 0, 0);

	label = RKCommonFunctions::linkedWrappedLabel (QString ());
	vbox->addWidget (label);

	int type = xml->getMultiChoiceAttribute (element, "type", "normal;warning;error", 0, DL_INFO);
	if (type != 0) {
		QFont font = label->font ();
		QPalette palette = label->palette ();
		if (type == 1) {		// warning
			palette.setColor (label->foregroundRole (), QColor (255, 100, 0));
			font.setWeight (QFont::DemiBold);
		} else if (type == 2) {		// error
			palette.setColor (label->foregroundRole (), QColor (255, 0, 0));
			font.setWeight (QFont::Bold);
		}
		label->setPalette (palette);
		label->setFont (font);
	}

	QString initial_text = xml->i18nElementText (element, true, DL_ERROR);

	// create and add property
	addChild ("text", text = new RKComponentPropertyBase (this, true));
	text->setInternal (true);
	connect (text, &RKComponentPropertyBase::valueChanged, this, &RKText::textChanged);

	// initialize
	text->setValue (initial_text);
}

RKText::~RKText(){
	RK_TRACE (PLUGIN);
}

void RKText::textChanged (RKComponentPropertyBase *) {
	RK_TRACE (PLUGIN);

	label->setText (fetchStringValue (text));
	changed ();
}

