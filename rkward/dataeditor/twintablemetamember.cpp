/***************************************************************************
                          twintablemetamember  -  description
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
#include "twintablemetamember.h"

#include "tablecolumn.h"
#include "../core/rkvariable.h"
#include "../core/rcontainerobject.h"
#include "../core/rkmodificationtracker.h"
#include "twintable.h"
#include "celleditor.h"
#include "../rkglobals.h"

#include <qpainter.h>
#include <qrect.h>
#include <qpalette.h>
#include <qstyle.h>

#include "../debug.h"

TwinTableMetaMember::TwinTableMetaMember (QWidget *parent, TwinTable *table) : TwinTableMember (parent, table, 0, 1) {
	type_values.insert (QString::number (RObject::Number), RObject::typeToText (RObject::Number));
	type_values.insert (QString::number (RObject::String), RObject::typeToText (RObject::String));
	type_values.insert (QString::number (RObject::Date), RObject::typeToText (RObject::Date));
}

TwinTableMetaMember::~TwinTableMetaMember () {
}

void TwinTableMetaMember::removeRow (int) {
	RK_ASSERT (false);
}

void TwinTableMetaMember::insertRows (int, int) {
	RK_ASSERT (false);
}

void TwinTableMetaMember::setText (int row, int col, const QString &text) {
	RK_TRACE (EDITOR);
	RKVariable *var = table->getColObject (col);
	RK_ASSERT (var);
	if (text == var->getText (row)) return;
	if (row == NAME_ROW) {
		RKGlobals::tracker ()->renameObject (var, var->getContainer ()->validizeName (text));
	} else if (row == LABEL_ROW) {
		var->setLabel (text, true);
	} else if (row == TYPE_ROW) {
		var->setVarType ((RObject::VarType) text.toInt ());
	}
}

void TwinTableMetaMember::paintCell (QPainter *p, int row, int col, const QRect &cr, bool selected, const QColorGroup &cg) {
	// no trace for paint operations
	RKVariable *var = table->getColObject (col);
	
	// draw background
	QBrush brush = QBrush (Qt::red);
	if (selected) {
		brush = cg.brush(QColorGroup::Highlight);
	} else {
		brush = cg.brush (QColorGroup::Base);
	}	
	if ((row >= numRows ()) || (!var)) {
		brush = QBrush (Qt::gray);
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
	} else {
		p->setPen (cg.text ());
	}

	if (var && (row < numRows ())) {
		p->drawText (2, 0, cr.width () - 4, cr.height (), Qt::AlignLeft, formattedText (row, col));
	}
}

QWidget *TwinTableMetaMember::beginEdit (int row, int col, bool) {
	RK_TRACE (EDITOR);
	RK_ASSERT (!tted);
	RKVariable *var = table->getColObject (col);
	if (!var) {
		RK_ASSERT (false);
		return 0;
	}

	if (var->cellStatus (row) == RKVariable::ValueUnknown) return 0;

	if (row == TYPE_ROW) {
		tted = new CellEditor (viewport (), text (row, col), 0, &type_values);
	} else {
		tted = new CellEditor (viewport (), text (row, col), 0, 0);
	}
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

QString TwinTableMetaMember::text (int row, int col) const {
	RK_TRACE (EDITOR);
	// called very often. do not trace
	RKVariable *var = table->getColObject (col);
	if (!var) {
		RK_ASSERT (false);
		return "";
	}
	if (row == NAME_ROW) {
		return var->getShortName ();
	} else if (row == LABEL_ROW) {
		return var->getLabel ();
	} else if (row == TYPE_ROW) {
		return QString::number (var->getVarType ());
	}
	return "";
}

QString TwinTableMetaMember::formattedText (int row, int col) const {
	// called very often. do not trace
	RKVariable *var = table->getColObject (col);
	if (!var) {
		RK_ASSERT (false);
		return "";
	}
	if (row == NAME_ROW) {
		return var->getShortName ();
	} else if (row == LABEL_ROW) {
		return var->getLabel ();
	} else if (row == TYPE_ROW) {
		return var->getVarTypeString ();
	}
	return "";
}


