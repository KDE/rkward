/***************************************************************************
                          rtableitem.cpp  -  description
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

#include "rtableitem.h"
#include "typeselectcell.h"

#include <qpainter.h>

RTableItem::RTableItem(TwinTableMember *table) : QTableItem (table, QTableItem::WhenCurrent, "") {
	valid = false;	// for now
}

RTableItem::~RTableItem(){
}

QString RTableItem::rText () {
	BaseType type = ((TypeSelectCell *) ((TwinTableMember *) table ())->varTable ()->item (TYPE_ROW, col ()))->type ();
	if ((type == Number) && isValid ()) {
		return text ();	
	} else if (type == String) {
		return ("\"" + text () + "\"");
	} else {
		if (text () != "") {
			return ("\"" + text () + "\"");
		} else {
			return "NA";
		}
	}
}

void RTableItem::paint (QPainter *p, const QColorGroup &cg, const QRect &cr, bool selected) {
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

	QString txt = text ();
	if (txt == "") {
		txt = "NA";
	}

    p->drawText(x + 2, 0, w - x - 4, h, alignment (), txt);
}
