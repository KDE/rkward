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

TypeSelectCell::TypeSelectCell (TwinTableMember *table) : RTableItem (table, QTableItem::WhenCurrent) {
	// default to numeric
	_type = RObject::Number;
	valid = true;
}
TypeSelectCell::~TypeSelectCell(){
}

QWidget *TypeSelectCell::createEditor () const {
	QComboBox *edit_cb;
	edit_cb = new QComboBox(table()->viewport());
	QObject::connect(edit_cb, SIGNAL(activated (int)), table(), SLOT(doValueChanged ()));
	edit_cb->insertItem(i18n ("Number"));
	edit_cb->insertItem(i18n ("String"));
	edit_cb->insertItem(i18n ("Date"));
	edit_cb->setEditable (true);
	edit_cb->setAutoCompletion (true);

	edit_cb->setCurrentItem(static_cast<int> (_type) - 1);
	return edit_cb;
}

void TypeSelectCell::setContentFromEditor (QWidget *w) {
	RObject::VarType old_type = _type;
	_type = RObject::textToType (static_cast<QComboBox*> (w)->currentText ());
	if (_type == RObject::Invalid) _type = old_type;
	if (_type != RObject::Invalid) valid = true;
	if (old_type != _type) {
		ttm ()->getTwin ()->checkColValid (col ());
	}
}

QString TypeSelectCell::text () const {
	return RObject::typeToText (_type);
}

QString TypeSelectCell::rText () {
	if (text () != "") {
		return ("\"" + text () + "\"");
	} else {
		return "NA";
	}
}

void TypeSelectCell::setText (const QString &str) {
	RObject::VarType old_type = _type;
	valid = true;
	_type = RObject::textToType (str);
	if (_type == RObject::Invalid) {
		valid = false;
	} else if (_type == RObject::Unknown) {
		_type = RObject::Number;
	}
	if (old_type != _type) {
		ttm ()->getTwin ()->checkColValid (col ());
	}
}
