/***************************************************************************
                          rktext.cpp  -  description
                             -------------------
    begin                : Sun Nov 10 2002
    copyright            : (C) 2002 by Thomas Friedrichsmeier
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
#include <qdom.h>
#include <qlabel.h>

RKText::RKText(const QDomElement &element, QWidget *parent, RKPlugin *plugin, QLayout *layout) : RKPluginWidget (element, parent, plugin, layout) {
	QString text;
	QStringList lines = lines.split ("\n", element.text (), false);
	for (unsigned int i=0; i < lines.count (); i++) {
		QString line = lines[i].stripWhiteSpace ();
		if (line != "") {
			text.append (line + "\n");
		}
	}

	// strip final newline
	text.truncate (text.length () -1);

	label = new QLabel (text, parent);
	label->setAlignment (Qt::AlignAuto | Qt::AlignVCenter | Qt::ExpandTabs | Qt::WordBreak);
	addWidget (label);
}

RKText::~RKText(){
}
