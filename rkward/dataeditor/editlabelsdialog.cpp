/***************************************************************************
                          editlabelsdialog  -  description
                             -------------------
    begin                : Tue Sep 21 2004
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
#include "editlabelsdialog.h"

#include <klocale.h>
#include <kdialogbase.h>

#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qpainter.h>
#include <qrect.h>
#include <qpalette.h>
#include <qstyle.h>

#include "../core/rkvariable.h"
#include "celleditor.h"

#include "../debug.h"

LevelsTable::LevelsTable (QWidget *parent, RObject::ValueLabels *labels) : TwinTableMember (parent, 0, 1, 0) {
	RK_TRACE (EDITOR);

	RK_ASSERT (labels);
	storage = labels;

	setNumCols (2);
	verticalHeader ()->setLabel (0, i18n ("Index"));
	verticalHeader ()->setLabel (1, i18n ("Label"));
	setColumnReadOnly (0, true);

	setNumRows (storage->count () + 1);
}

LevelsTable::~LevelsTable () {
	RK_TRACE (EDITOR);
}

void LevelsTable::setText (int row, int col, const QString &text) {
	RK_TRACE (EDITOR);
	RK_ASSERT (col == 1);

	storage->insert (QString::number (row+1), text);
	if (text.isEmpty ()) {
		int maxrow = numTrueRows ()-1;
		while ((maxrow >= 0) && LevelsTable::text (maxrow, 1).isEmpty ()) {
			storage->remove (QString::number (maxrow + 1));
			--maxrow;
		}
		setNumRows (maxrow + 2);
	}
}

QString LevelsTable::text (int row, int col) const {
	RK_TRACE (EDITOR);
	RK_ASSERT (col == 1);
	RK_ASSERT (row < numTrueRows ());

	return ((*storage)[QString::number (row+1)]);
}

void LevelsTable::paintCell (QPainter *p, int row, int col, const QRect &cr, bool selected, const QColorGroup &cg) {
	// no trace for paint operations

	// draw background
	QBrush brush = QBrush (Qt::red);
	if (selected) {
		brush = cg.brush(QColorGroup::Highlight);
	} else {
			brush = cg.brush (QColorGroup::Base);
	}
	if (row >= numTrueRows ()) {
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

	QString dummy;
	if (row < numTrueRows ()) {
		if (!col) {
			dummy.setNum (row +1);
		} else {
			dummy = (*storage)[QString::number (row+1)];
		}
	}
	p->drawText (2, 0, cr.width () - 4, cr.height (), Qt::AlignLeft, dummy);
}


QWidget *LevelsTable::beginEdit (int row, int col, bool) {
	RK_TRACE (EDITOR);
	RK_ASSERT (!tted);

	if (row >= numTrueRows ()) {
		insertRows (numRows (), 1);
	}
	
	tted = new CellEditor (this, text (row, col), 0, 0);

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


EditLabelsDialog::EditLabelsDialog (QWidget *parent, RKVariable *var, int mode) : QDialog (parent) {
	RK_TRACE (EDITOR);
	RK_ASSERT (var);
	RK_ASSERT (var->objectOpened ());
	
	EditLabelsDialog::var = var;
	EditLabelsDialog::mode = mode;

	QVBoxLayout *mainvbox = new QVBoxLayout (this, KDialog::marginHint (), KDialog::spacingHint ());
	QLabel *label = new QLabel (i18n ("Levels can be assigned only to consecutive integers starting with 1. To remove levels at the end of the list, just set them to empty."), this);
	label->setAlignment (Qt::AlignAuto | Qt::AlignVCenter | Qt::ExpandTabs | Qt::WordBreak);
	mainvbox->addWidget (label);

	QHBoxLayout *hbox = new QHBoxLayout (mainvbox, KDialog::spacingHint ());

	RObject::ValueLabels *labels = var->getValueLabels ();
	if (!labels) {
		labels = new RObject::ValueLabels;
	}

	table = new LevelsTable (this, labels);
	hbox->addWidget (table);

	QHBoxLayout *buttonbox = new QHBoxLayout (mainvbox, KDialog::spacingHint ());

	QPushButton *ok_button = new QPushButton (i18n ("Ok"), this);
	connect (ok_button, SIGNAL (clicked ()), this, SLOT (accept ()));
	buttonbox->addWidget (ok_button);

	QPushButton *cancel_button = new QPushButton (i18n ("Cancel"), this);
	connect (cancel_button, SIGNAL (clicked ()), this, SLOT (reject ()));
	buttonbox->addWidget (cancel_button);
	
	setCaption (i18n ("Levels / Value labels for '%1'").arg (var->getShortName ()));
}

EditLabelsDialog::~EditLabelsDialog () {
	RK_TRACE (EDITOR);
}

void EditLabelsDialog::accept () {
	RK_TRACE (EDITOR);

	RObject::ValueLabels *labels = table->storage;
	if (labels->isEmpty ()) {
		var->setValueLabels (0);
	} else {
		var->setValueLabels (labels);
	}

	QDialog::accept ();
}

#include "editlabelsdialog.moc"
