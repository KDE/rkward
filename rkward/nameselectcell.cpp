/***************************************************************************
                          nameselectcell.cpp  -  description
                             -------------------
    begin                : Mon Nov 4 2002
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

#include "nameselectcell.h"

#include "twintablemember.h"

int NameSelectCell::num = 0;

NameSelectCell::NameSelectCell(TwinTableMember *table) : RTableItem (table) {
}

NameSelectCell::~NameSelectCell(){
}

void NameSelectCell::init () {
    QString dummy;
	dummy.setNum (num++);
	while (dummy.length () < 5) {
		dummy.prepend ("0");
	}
	setText ("var" + dummy);
}

void NameSelectCell::setText (const QString &str) {
	// TODO: check for illegal characters in name and duplicates!
	((TwinTableMember *) table ())->varTable ()->horizontalHeader ()->setLabel (col (), str);
	QTableItem::setText (str);
}

QString NameSelectCell::rText () {
	if (text () != "") {
		return ("\"" + text () + "\"");
	} else {
		return "unnamed";
	}
}
