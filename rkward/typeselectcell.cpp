/***************************************************************************
                          typeselectcell.cpp  -  description
                             -------------------
    begin                : Sun Nov 3 2002
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

#include "typeselectcell.h"

#include <klocale.h>

#include <qcombobox.h>

#include "rtableitem.h"

TypeSelectCell::TypeSelectCell (TwinTableMember *table) : RTableItem (table) {
	// default to numeric
	_type = Number;
}
TypeSelectCell::~TypeSelectCell(){
}

QWidget *TypeSelectCell::createEditor () const {
	QComboBox *edit_cb;
	edit_cb = new QComboBox(table()->viewport());
	QObject::connect(edit_cb, SIGNAL(activated(int)), table(), SLOT(doValueChanged()));
	edit_cb->insertItem(i18n ("Number"));
	edit_cb->insertItem(i18n ("String"));
	edit_cb->insertItem(i18n ("Date"));

	edit_cb->setCurrentItem(static_cast<int> (_type));
	return edit_cb;
}

void TypeSelectCell::setContentFromEditor (QWidget *w) {
	_type = (BaseType) ((QComboBox*) w)->currentItem ();
}

QString TypeSelectCell::text () const {
	if (_type == Number) return ("Number");
	if (_type == String) return ("String");
	if (_type == Date) return ("Date");
	return ("invalid");
}

QString TypeSelectCell::rText () {
	if (text () != "") {
		return ("\"" + text () + "\"");
	} else {
		return "NA";
	}
}

void TypeSelectCell::setText (const QString &str) {
	if (str == "Number") {
		_type = Number;
	} else if (str == "String") {
		_type = String;
	} else if (str == "Date") {
		_type = Date;
	} else {
		_type = Invalid;
	}
}
