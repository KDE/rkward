/***************************************************************************
                          twintabledatamember  -  description
                             -------------------
    begin                : Mon Sep 13 2004
    copyright            : (C) 2004 by Thomas Friedrichsmeier
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
#include "twintabledatamember.h"

#include "twintable.h"
#include "../core/rkvariable.h"
#include "celleditor.h"

#include <qpainter.h>
#include <qrect.h>
#include <qpalette.h>
#include <qstyle.h>

#include "../debug.h"

TwinTableDataMember::TwinTableDataMember (QWidget *parent, TwinTable *table) : TwinTableMember (parent, table, 1, 1) {
}

TwinTableDataMember::~TwinTableDataMember () {
}

void TwinTableDataMember::removeRow (int row) {
	RK_TRACE (EDITOR);
	QTable::removeRow (row);
	for (int i=0; i < table->numCols (); ++i) {
		table->getColObject (i)->removeRow (row);
	}
	for (int i=row; i < numRows (); ++i) {
		for (int j=0; j < table->numCols (); ++j) {
			updateCell (i, j);
		}
	}
}

void TwinTableDataMember::insertRows (int row, int count) {
	RK_TRACE (EDITOR);
	QTable::insertRows (row, count);
	for (int i=0; i < table->numCols (); ++i) {
		table->getColObject (i)->insertRows (row, count);
	}
}

void TwinTableDataMember::setText (int row, int col, const QString &text) {
	RK_TRACE (EDITOR);
	table->getColObject (col)->setText (row, text);
}

void TwinTableDataMember::paintCell (QPainter *p, int row, int col, const QRect &cr, bool selected, const QColorGroup &cg) {
	// no trace for paint operations
	RKVariable *var = table->getColObject (col);
	RKVariable::Status cell_state = RKVariable::ValueInvalid;
	if (var) cell_state = var->cellStatus (row);
	
	// draw background
	QBrush brush = QBrush (Qt::red);
	if (selected) {
		brush = cg.brush(QColorGroup::Highlight);
	}
	if ((row >= numRows ()) || (!var)) {
		brush = QBrush (Qt::gray);
	} else if (cell_state != RKVariable::ValueInvalid) {
		if (!selected) {
			brush = cg.brush (QColorGroup::Base);
		}
	}
	p->fillRect(0, 0, cr.width(), cr.height(), brush);

	// draw grid
	QPen pen (p->pen ());
	int gridColor = style ().styleHint (QStyle::SH_Table_GridLineColor, this);
	if (gridColor != -1) {
		const QPalette &pal = palette ();
		if (cg != colorGroup () && cg != pal.disabled () && cg != pal.inactive ()) p->setPen (cg.mid ());
		else p->setPen ((QRgb) gridColor);
	} else {
		p->setPen (cg.mid ());
	}
	int x2 = cr.width () - 1;
	int y2 = cr.height () - 1;
	p->drawLine (x2, 0, x2, y2);
	p->drawLine (0, y2, x2, y2);
	p->setPen (pen);

	if (tted && (currEditRow () == row) && (currEditCol () == col)) {
		tted->raise ();
		return;
	}
	
	// draw text
	if (selected) {
		p->setPen (cg.highlightedText());
	} else if (cell_state == RKVariable::ValueUnknown) {
		p->setPen (cg.light ());
	} else {
		p->setPen (cg.text ());
	}

	if (var && (row < numRows ())) {
		p->drawText (2, 0, cr.width () - 4, cr.height (), Qt::AlignRight, var->getFormatted (row));
	}
}

QWidget *TwinTableDataMember::beginEdit (int row, int col, bool) {
	RK_TRACE (EDITOR);
	RK_ASSERT (!tted);
	RKVariable *var = table->getColObject (col);
	if (!var) {
		table->insertNewColumn (col);
		var = table->getColObject (col);
		if (!var) {
			RK_ASSERT (false);
			return 0;
		}
	}

	if (var->cellStatus (row) == RKVariable::ValueUnknown) return 0;

	tted = new CellEditor (viewport (), var->getText (row), 0);
	tted->installEventFilter (this);

	QRect cr = cellGeometry (row, col);
	tted->resize (cr.size ());
	moveChild (tted, cr.x (), cr.y ());
	tted->show ();
	
	tted->setActiveWindow ();
	tted->setFocus ();
	connect (tted, SIGNAL (lostFocus ()), this, SLOT (editorLostFocus ()));

	updateCell (row, col);
	return (tted);
}

QString TwinTableDataMember::text (int row, int col) const {
	RK_TRACE (EDITOR);
	// called very often. do not trace
	RKVariable *var = table->getColObject (col);
	if (!var) {
		RK_ASSERT (false);
		return "";
	}
	return var->getText (row);
}

QString TwinTableDataMember::rText (int row, int col) const {
	RKVariable *var = table->getColObject (col);
	if (!var) {
		RK_ASSERT (false);
		return "";
	}
	return var->getRText (row);
}
