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

RTableItem::RTableItem(TwinTableMember *table, QTableItem::EditType edit_type) : QTableItem (table, edit_type, "") {
	valid = false;	// for now
}

RTableItem::~RTableItem(){
}

TwinTableMember *RTableItem::ttm () {
	return (TwinTableMember *) table ();
}

RObject::VarType RTableItem::type () {
	return ((TypeSelectCell *) ttm ()->varTable ()->item (TYPE_ROW, col ()))->type ();
}

void RTableItem::checkValid () {
	bool prev_valid = valid;
	if (type () == RObject::String) {
		valid = true;		// there are no invalid strings!
	} else if (type () == RObject::Number) {
		if (text () != "") {
			bool ok;
			text ().toFloat (&ok);
			valid = ok;
		} else {
			// allow empty cells (will be translated into "NA")
			valid = true;
		}
	} else if (type () == RObject::Date) {
		// TODO
		valid = false;
	} else {		// invalid type
		valid = false;
	}
	if (valid != prev_valid) {
		table ()->updateCell (row (), col ());
	}
}

void RTableItem::setText (const QString &str) {
	if (str == "NA") {
		QTableItem::setText ("");
	} else {
		QTableItem::setText (str);
	}
	checkValid ();
}

QString RTableItem::rText () {
	checkValid ();

	if (text () == "") {
		return "NA";
	}
	if ((type () == RObject::Number) && isValid ()) {
		return text ();	
	}
	if (type () == RObject::String) {
		return ("\"" + text () + "\"");
	}

	// else	
	return ("\"" + text () + "\"");
}

void RTableItem::paint (QPainter *p, const QColorGroup &cg, const QRect &cr, bool selected) {
	QBrush brush = QBrush (Qt::red);
	if (selected) {
		brush = cg.brush(QColorGroup::Highlight);
	}
	if ((row () >= static_cast<TwinTableMember *> (table ())->numRows ()) || (col () >= static_cast<TwinTableMember *> (table ())->numCols ())) {
		brush = QBrush (Qt::gray);
	} else if (isValid ()) {
		if (!selected) {
			brush = cg.brush(QColorGroup::Base);
		}
	}

	p->fillRect(0, 0, cr.width(), cr.height(), brush);

	int w = cr.width();
	int h = cr.height();

	int x = 0;

	if (selected) {
		p->setPen(cg.highlightedText());
	} else {
		p->setPen(cg.text());
	}

	p->drawText(x+2, 0, w-x- 4, h, alignment (), text ());
}
