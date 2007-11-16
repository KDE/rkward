/***************************************************************************
                          rktext.cpp  -  description
                             -------------------
    begin                : Sun Nov 10 2002
    copyright            : (C) 2002, 2006, 2007 by Thomas Friedrichsmeier
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

#include "rktext.h"

#include <qstring.h>
#include <qstringlist.h>
#include <qlabel.h>
#include <qdom.h>
#include <QVBoxLayout>

#include "../rkglobals.h"
#include "../misc/xmlhelper.h"
#include "../debug.h"

RKText::RKText (const QDomElement &element, RKComponent *parent_component, QWidget *parent_widget) : RKComponent (parent_component, parent_widget) {
	RK_TRACE (PLUGIN);

	// get xml-helper
	XMLHelper *xml = XMLHelper::getStaticHelper ();

	// create layout and label
	QVBoxLayout *vbox = new QVBoxLayout (this);
	vbox->setContentsMargins (0, 0, 0, 0);

	label = new QLabel (QString (), this);
	label->setWordWrap (true);
	vbox->addWidget (label);

	QString initial_text;
	QStringList lines = element.text ().split ("\n");
	for (int i=0; i < lines.count (); i++) {
		QString line = lines[i].trimmed ();
		if (!line.isEmpty ()) {
			initial_text.append (line + '\n');
		}
	}

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

	// strip final newline
	initial_text.truncate (initial_text.length () -1);

	// create and add property
	addChild ("text", text = new RKComponentPropertyBase (this, true));
	connect (text, SIGNAL (valueChanged (RKComponentPropertyBase *)), this, SLOT (textChanged (RKComponentPropertyBase *)));

	// initialize
	text->setValue (initial_text);
}

RKText::~RKText(){
	RK_TRACE (PLUGIN);
}

void RKText::textChanged (RKComponentPropertyBase *) {
	RK_TRACE (PLUGIN);

	label->setText (text->value ());
	changed ();
}

#include "rktext.moc"
