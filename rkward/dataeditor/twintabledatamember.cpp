/***************************************************************************
                          twintabledatamember  -  description
                             -------------------
    begin                : Mon Sep 13 2004
    copyright            : (C) 2004, 2006 by Thomas Friedrichsmeier
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

#warning TODO remove
#if 0

TwinTableDataMember::TwinTableDataMember (QWidget *parent, TwinTable *table) : TwinTableMember (parent, table, 1, 1) {
}

TwinTableDataMember::~TwinTableDataMember () {
}

void TwinTableDataMember::removeRow (int row) {
	RK_TRACE (EDITOR);
	Q3Table::removeRow (row);
	for (int i=0; i < table->numTrueCols (); ++i) {
		table->getColObject (i)->removeRow (row);
	}
	for (int i=row; i < numTrueRows (); ++i) {
		for (int j=0; j < table->numTrueCols (); ++j) {
			updateCell (i, j);
		}
	}
}

void TwinTableDataMember::insertRows (int row, int count) {
	RK_TRACE (EDITOR);
	for (int i=0; i < table->numTrueCols (); ++i) {
		table->getColObject (i)->insertRows (row, count);
	}
	Q3Table::insertRows (row, count);
}

void TwinTableDataMember::setText (int row, int col, const QString &text) {
	RK_TRACE (EDITOR);

	RKVariable *var = table->getColObject (col);
	if (var) {
		var->setText (row, text);
	}
	// we may also be in a row that has no var (e.g. left header)
}

void TwinTableDataMember::paintCell (QPainter *p, int row, int col, const QRect &cr, bool selected, const QColorGroup &cg) {
	// no trace for paint operations

	RKVariable *var = table->getColObject (col);
	RKVariable::Status cell_state = RKVariable::ValueInvalid;
	if (var) cell_state = var->cellStatus (row);

	QBrush *brush_override = 0;
	QPen *pen_override = 0;
	if (var) {
		if (cell_state == RKVariable::ValueInvalid) {
			brush_override = new QBrush (Qt::red);
		} else if (cell_state == RKVariable::ValueUnknown) {
			pen_override = new QPen (cg.light ());
		}
	}

	QString text;
	int align = 0;
	if (var && (row < numTrueRows ())) {
		text = var->getText (row, true);
		align = var->getAlignment ();
	}
	paintCellInternal (p, row, col, cr, selected, cg, brush_override, pen_override, text, align);

	delete pen_override;
	delete brush_override;
}

QWidget *TwinTableDataMember::beginEdit (int row, int col, bool) {
	RK_TRACE (EDITOR);
	RK_ASSERT (!tted);
	RKVariable *var = table->getColObject (col);
	if (!var) {
		RK_ASSERT (col >= numTrueCols ());
		table->insertNewColumn (col+1);
		var = table->getColObject (col);
		if (!var) {
			RK_ASSERT (false);
			return 0;
		}
	}

	if (row >= numTrueRows ()) {
		table->insertNewRow ();
	}

	if (var->cellStatus (row) == RKVariable::ValueUnknown) return 0;

	tted = new CellEditor (this, var->getText (row), 0, var->getValueLabels ());
	//tted->installEventFilter (this);

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
		return QString ();
	}
	return var->getText (row);
}

QString TwinTableDataMember::rText (int row, int col) const {
	RKVariable *var = table->getColObject (col);
	if (!var) {
		RK_ASSERT (false);
		return QString ();
	}
	return var->getRText (row);
}

#include "twintabledatamember.moc"
#endif