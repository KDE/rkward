/***************************************************************************
                          twintablemetamember  -  description
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
#include "twintablemetamember.h"

#include "../core/rkvariable.h"
#include "../core/rcontainerobject.h"
#include "../core/rkmodificationtracker.h"
#include "twintable.h"
#include "celleditor.h"
#include "editlabelsdialog.h"
#include "editformatdialog.h"
#include "../rkglobals.h"

#include <qpainter.h>
#include <qrect.h>
#include <qpalette.h>
#include <qstyle.h>

#include "../debug.h"

TwinTableMetaMember::TwinTableMetaMember (QWidget *parent, TwinTable *table) : TwinTableMember (parent, table, 0, 1) {
	type_values.insert (QString::number (RObject::Number), RObject::typeToText (RObject::Number));
	type_values.insert (QString::number (RObject::Factor), RObject::typeToText (RObject::Factor));
	type_values.insert (QString::number (RObject::String), RObject::typeToText (RObject::String));
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
	RK_ASSERT (var || (col < 0));
	if (text == TwinTableMetaMember::text (row, col)) return;
	if (row == NAME_ROW) {
		RKGlobals::tracker ()->renameObject (var, var->getContainer ()->validizeName (text));
	} else if (row == LABEL_ROW) {
		var->setLabel (text, true);
	} else if (row == TYPE_ROW) {
		var->setVarType ((RObject::VarType) text.toInt ());
	} else if (row == FORMAT_ROW) {
		return var->setFormattingOptionsString (text);
	} else if (row == LEVELS_ROW) {
		var->setValueLabelString (text);
	}
}

void TwinTableMetaMember::paintCell (QPainter *p, int row, int col, const QRect &cr, bool selected, const QColorGroup &cg) {
	// no trace for paint operations
	RKVariable *var = table->getColObject (col);

	QString text;
	if (var && (row < numTrueRows ())) {
		text = formattedText (row, col);
	}
	paintCellInternal (p, row, col, cr, selected, cg, 0, 0, text, 0);
}

QWidget *TwinTableMetaMember::beginEdit (int row, int col, bool) {
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

	if (row == TYPE_ROW) {
		tted = new CellEditor (this, text (row, col), 0, &type_values);
	} else if (row == LEVELS_ROW) {
		EditLabelsDialog *dialog = new EditLabelsDialog (0, var, 0);
		dialog->exec ();
		delete (dialog);
		return 0;
	} else if (row == FORMAT_ROW) {
		EditFormatDialog *dialog = new EditFormatDialog (0, var, 0);
		dialog->exec ();
		delete (dialog);
		return 0;
	} else {
		tted = new CellEditor (this, text (row, col), 0, 0);
	}
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

QString TwinTableMetaMember::text (int row, int col) const {
	RK_TRACE (EDITOR);
	// called very often. do not trace
	RKVariable *var = table->getColObject (col);
	if (!var) {
		RK_ASSERT (false);
		return QString::null;
	}
	if (row == NAME_ROW) {
		return var->getShortName ();
	} else if (row == LABEL_ROW) {
		return var->getLabel ();
	} else if (row == TYPE_ROW) {
		return QString::number (var->getVarType ());
	} else if (row == FORMAT_ROW) {
		return var->getFormattingOptionsString ();
	} else if (row == LEVELS_ROW) {
		return var->getValueLabelString ();
	}
	return QString::null;
}

QString TwinTableMetaMember::formattedText (int row, int col) const {
	// called very often. do not trace
	RKVariable *var = table->getColObject (col);
	if (!var) {
		RK_ASSERT (false);
		return QString::null;
	}
	if (row == NAME_ROW) {
		return var->getShortName ();
	} else if (row == LABEL_ROW) {
		return var->getLabel ();
	} else if (row == TYPE_ROW) {
		return var->getVarTypeString ();
	} else if (row == FORMAT_ROW) {
		return var->getFormattingOptionsString ();
	} else if (row == LEVELS_ROW) {
		return var->getValueLabelString ();
	}
	return QString::null;
}

#include "twintablemetamember.moc"
