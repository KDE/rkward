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
#include <qpainter.h>
#include <qregexp.h>

TypeSelectCell::TypeSelectCell (QTable *table) : QTableItem (table, QTableItem::WhenCurrent, "") {
	// default to numeric
	type = Number;
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

	edit_cb->setCurrentItem(static_cast<int> (type));
	return edit_cb;
}

void TypeSelectCell::setContentFromEditor (QWidget *w) {
	type = (BaseType) ((QComboBox*) w)->currentItem ();
}

QString TypeSelectCell::text () const {
	if (type == Number) return ("\"Number\"");
	if (type == String) return ("\"String\"");
	if (type == Date) return ("\"Date\"");
	return ("\"invalid\"");
}

void TypeSelectCell::setText (const QString &str) {
	QString text = str;
	text.replace(QRegExp("\""), "");

	if (text == "Number") {
		type = Number;
	} else if (text == "String") {
		type = String;
	} else if (text == "Date") {
		type = Date;
	} else {
		type = Invalid;
	}
}

void TypeSelectCell::paint (QPainter *p, const QColorGroup &cg, const QRect &cr, bool selected) {
    p->fillRect( 0, 0, cr.width(), cr.height(),
                 selected ? cg.brush( QColorGroup::Highlight )
                          : cg.brush( QColorGroup::Base ) );

    int w = cr.width();
    int h = cr.height();

    int x = 0;

    if ( selected )
        p->setPen(cg.highlightedText());
    else
        p->setPen(cg.text());

	QString txt;
	if (type == Number) txt = "Number";
	else if (type == String) txt = "String";
	else if (type == Date) txt = "Date";
	else txt = "invalid";

    p->drawText(x + 2, 0, w - x - 4, h, alignment (), txt);
}