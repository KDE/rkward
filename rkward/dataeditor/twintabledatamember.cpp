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
#include "tablecolumn.h"
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
	for (int i=0; i < table->numCols (); ++i) {
		table->getColumn (i)->removeRow (row);
	}
	QTable::removeRow (row);
}

void TwinTableDataMember::insertRows (int row, int count) {
	RK_TRACE (EDITOR);
	for (int i=0; i < table->numCols (); ++i) {
		table->getColumn (i)->insertRows (row, count);
	}
	QTable::insertRows (row, count);
}

void TwinTableDataMember::setText (int row, int col, const QString &text) {
	RK_TRACE (EDITOR);
	table->getColumn (col)->setText (row, text);
}

void TwinTableDataMember::paintCell (QPainter *p, int row, int col, const QRect &cr, bool selected, const QColorGroup &cg) {
	// no trace for paint operations
	TableColumn *column = table->getColumn (col);
	TableColumn::Status cell_state = TableColumn::Invalid;
	if (column) cell_state = column->cellStatus (row);
	
	// draw background
	QBrush brush = QBrush (Qt::red);
	if (selected) {
		brush = cg.brush(QColorGroup::Highlight);
	}
	if ((row >= numRows ()) || (!column)) {
		brush = QBrush (Qt::gray);
	} else if (column && (cell_state != TableColumn::Invalid)) {
		if (!selected) {
			brush = cg.brush (QColorGroup::Base);
		}
	}
	p->fillRect(0, 0, cr.width(), cr.height(), brush);

	// draw grid
	QPen pen( p->pen() );
	int gridColor = style ().styleHint (QStyle::SH_Table_GridLineColor, this);
	if (gridColor != -1) {
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
	} else if (cell_state == TableColumn::Unknown) {
		p->setPen (cg.light ());
	} else {
		p->setPen (cg.text ());
	}

	if (column && (row < numRows ())) {
		p->drawText (2, 0, cr.width () - 4, cr.height (), Qt::AlignRight, column->getFormatted (row));
	}
}

QWidget *TwinTableDataMember::beginEdit (int row, int col, bool) {
	RK_TRACE (EDITOR);
	RK_ASSERT (!tted);
	TableColumn *column = table->getColumn (col);
	if (!column) {
		RK_ASSERT (false);
		return 0;
	}

	if (column->cellStatus (row) == TableColumn::Unknown) return 0;

	tted = new CellEditor (viewport (), column->getText (row), 0);
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

void TwinTableDataMember::endEdit (int row, int col, bool, bool) {
	RK_TRACE (EDITOR);
	if (tted) setCellContentFromEditor (row, col);
}

void TwinTableDataMember::setCellContentFromEditor (int row, int col) {
	RK_TRACE (EDITOR);
	RK_ASSERT (tted);
	TableColumn *column = table->getColumn (col);
	if (!column) {
		RK_ASSERT (false);
	}

	QString text = tted->text ();
	
	tted->removeEventFilter (this);
	delete tted;
	tted = 0;
	
	if (column->getText (row) != text) {
		column->setText (row, text);
		emit (valueChanged (row, col));
	}
	
	viewport ()->setFocus ();
}

void TwinTableDataMember::setCurrentCell (int row, int col) {
	RK_TRACE (EDITOR);
	if (tted) stopEditing ();
	QTable::setCurrentCell (row, col);
}

QString TwinTableDataMember::text (int row, int col) const {
	RK_TRACE (EDITOR);
	// called very often. do not trace
	TableColumn *column = table->getColumn (col);
	if (!column) {
		RK_ASSERT (false);
		return "";
	}
	return column->getText (row);
}


