/***************************************************************************
                          twintablemember.cpp  -  description
                             -------------------
    begin                : Tue Oct 29 2002
    copyright            : (C) 2002, 2006 by Thomas Friedrichsmeier
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

#include "twintablemember.h"

#include <qevent.h>
#include <qpainter.h>
#include <qstyle.h>
//Added by qt3to4:
#include <Q3CString>
#include <QMouseEvent>
#include <QKeyEvent>
#include <Q3MemArray>

#include "celleditor.h"
#include "twintable.h"
#include "../debug.h"

TwinTableMember::TwinTableMember (QWidget *parent, TwinTable *table, int trailing_rows, int trailing_cols) : Q3Table (parent){
	twin = 0;
	TwinTableMember::table = table;
	setRowMovingEnabled (false);
	setVScrollBarMode (Q3ScrollView::AlwaysOn);
	horizontalHeader()->installEventFilter (this);
	verticalHeader()->installEventFilter (this);
	setSelectionMode (Q3Table::Single);
	
	TwinTableMember::trailing_cols = trailing_cols;
	TwinTableMember::trailing_rows = trailing_rows;
	
	tted = 0;
	changing_width = false;

	connect (this, SIGNAL (currentChanged (int, int)), this, SLOT (currentCellChanged (int, int)));
}

TwinTableMember::~TwinTableMember(){
}

int TwinTableMember::numTrueCols () const {
	return numCols () - trailing_cols;
}

int TwinTableMember::numTrueRows () const {
	return Q3Table::numRows () - trailing_rows;
}

void TwinTableMember::setTwin (TwinTableMember * new_twin) {
	twin = new_twin;
}

void TwinTableMember::columnWidthChanged (int col) {
	// does all repainting and stuff ...
	Q3Table::columnWidthChanged (col);

	// syncs the twin
	if (twin) {
		if (!changing_width) {
			changing_width = true;
			twin->setColumnWidth (col, columnWidth (col));
		}
		changing_width = false;
	}
}

bool TwinTableMember::eventFilter (QObject *object, QEvent *event) {
	// filter out right mouse button events of the varview-header
	if (event && (event->type () == QEvent::MouseButtonPress)) {
		QMouseEvent  *mouseEvent = (QMouseEvent *) event;
		if (mouseEvent && (mouseEvent->button () == Qt::RightButton)) {
			mouse_at = mouseEvent->globalPos ();
			if (object == horizontalHeader ()) {
				emit headerRightClick (-1, horizontalHeader ()->sectionAt (contentsX () + mouseEvent->x ()));
				return (true); // got it
			}
			if (object == verticalHeader ()) {
				emit headerRightClick (verticalHeader ()->sectionAt (contentsY () + mouseEvent->y ()), -1);
				return (true); // got it
			}
		}
		setFocus ();
	}

    // default processing
    return (Q3Table::eventFilter (object, event));
}

// virtual
QString TwinTableMember::rText (int row, int col) const {
	return (RObject::rQuote (text (row, col)));
}

void TwinTableMember::removeRows (const Q3MemArray<int> &) {
	RK_ASSERT (false);
}

void TwinTableMember::swapRows (int, int, bool) {
}

void TwinTableMember::swapCells (int, int, int, int) {
	RK_ASSERT (false);
}

void TwinTableMember::swapColumns (int, int, bool) {
}

void TwinTableMember::editorLostFocus () {
	RK_TRACE (EDITOR);
	stopEditing ();
}

void TwinTableMember::stopEditing () {
	RK_TRACE (EDITOR);
	if (tted) endEdit (currEditRow (), currEditCol (), true, false);
	RK_ASSERT (!tted);
}

QWidget *TwinTableMember::cellWidget (int row, int col) const {
	if (tted && (currEditRow () == row) && (currEditCol () == col)) return tted;
	return 0;
}

void TwinTableMember::currentCellChanged (int row, int col) {
	RK_TRACE (EDITOR);
	if ((row == currEditRow ()) && (col == currEditCol ())) return;
	if (tted) stopEditing ();

/*	if (numSelections ()) {
		QTableSelection sel = selection (currentSelection ());
		if (sel.bottomRow () != sel.topRow ()) return;
		if (sel.leftCol () != sel.rightCol ()) return;
	}

	editCell (row, col); */
}

void TwinTableMember::endEdit (int row, int col, bool, bool) {
	RK_TRACE (EDITOR);
	if (tted) setCellContentFromEditor (row, col);
	setEditMode (NotEditing, -1, -1);
}

void TwinTableMember::setCellContentFromEditor (int row, int col) {
	RK_TRACE (EDITOR);
	RK_ASSERT (tted);

	QString text_save = tted->text ();
	
	//tted->removeEventFilter (this);
	tted->hide ();
	tted->deleteLater ();
	tted = 0;
	
	if (text (row, col) != text_save) {
		setText (row, col, text_save);
		emit (valueChanged (row, col));
	}
	
	viewport ()->setFocus ();
}

Q3CString TwinTableMember::encodeSelection () {
	RK_TRACE (EDITOR);

	int top_row, left_col, bottom_row, right_col;
	getSelectionBoundaries (&top_row, &left_col, &bottom_row, &right_col);
// QCString uses (explicit) sharing, so we're not being too wasteful, here
	return (encodeRange (top_row, left_col, bottom_row, right_col));
}

Q3CString TwinTableMember::encodeRange (int top_row, int left_col, int bottom_row, int right_col) {
	RK_TRACE (EDITOR);

	QString data;
	for (int row=top_row; row <= bottom_row; ++row) {
		for (int col=left_col; col <= right_col; ++col) {
			data.append (text (row, col));
			if (col != right_col) {
				data.append ("\t");
			}
		}
		if (row != bottom_row) {
			data.append ("\n");
		}
	}

	return data.local8Bit ();
}

void TwinTableMember::blankSelected () {
	RK_TRACE (EDITOR);

	int top_row, left_col, bottom_row, right_col;
	getSelectionBoundaries (&top_row, &left_col, &bottom_row, &right_col);

	for (int row=top_row; row <= bottom_row; ++row) {
		for (int col=left_col; col <= right_col; ++col) {
			setText (row, col, QString::null);
		}
	}
}

void TwinTableMember::getSelectionBoundaries (int *top_row, int *left_col, int *bottom_row, int *right_col) {
	RK_TRACE (EDITOR);

	RK_ASSERT (top_row);
	RK_ASSERT (bottom_row);
	RK_ASSERT (left_col);
	RK_ASSERT (right_col);

	int selnum = -1;
	if (currentSelection () >= 0) selnum = currentSelection ();
	else if (numSelections () >= 1) selnum = 0;		// this is the one and only selection, as we only allow one single selection. Unfortunately, QTable does not regard a selection as current, if it was added programatically, instead of user-selected.
	if (selnum >= 0) {
		Q3TableSelection sel = selection (selnum);
		*top_row = sel.topRow ();
		*left_col = sel.leftCol ();
		*bottom_row = sel.bottomRow ();
		*right_col = sel.rightCol ();
	} else {
		// Nothing selected. Set current cell coordinates
		*top_row = *bottom_row = currentRow ();
		*left_col = *right_col = currentColumn ();
	}
}

void TwinTableMember::paintCellInternal (QPainter *p, int row, int col, const QRect &cr, bool selected, const QColorGroup &cg, QBrush *brush_override, QPen *pen_override, const QString &text, int alignment) {
	// no trace for paint operations

	// draw background
	QBrush brush = cg.brush (QColorGroup::Base);
	if (!brush_override) {
		if (selected) {
			brush = cg.brush(QColorGroup::Highlight);
			if ((row >= numTrueRows ()) || (col >= numTrueCols ())) {
				brush = QBrush (QColor (127, 127, 255));
			}
		} else {
			if ((row >= numTrueRows ()) || (col >= numTrueCols ())) {
				brush = QBrush (Qt::gray);
			}
		}
	} else {
		brush = *brush_override;
	}
	p->fillRect(0, 0, cr.width(), cr.height(), brush);

	// draw grid
	QPen pen (p->pen ());
	int gridColor = style ()->styleHint (QStyle::SH_Table_GridLineColor, 0, this);
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

	if (text.isNull ()) return;

	if (!pen_override) {
		if (selected) {
			p->setPen (cg.highlightedText());
		} else {
			p->setPen (cg.text ());
		}
	} else {
		p->setPen (*pen_override);
	}

	if (alignment == 1) {
		p->drawText (2, 0, cr.width () - 4, cr.height (), Qt::AlignRight, text);
	} else {
		p->drawText (2, 0, cr.width () - 4, cr.height (), Qt::AlignLeft, text);
	}
}

void TwinTableMember::keyPressEvent (QKeyEvent *e) {
	RK_TRACE (EDITOR);

	if ((e->key () == Qt::Key_Delete) || (e->key () == Qt::Key_Backspace)) {
		blankSelected ();
		e->accept ();
	} else {
		Q3Table::keyPressEvent (e);
	}
}

#include "twintablemember.moc"
