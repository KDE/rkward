/***************************************************************************
                          twintablemember.cpp  -  description
                             -------------------
    begin                : Tue Oct 29 2002
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

#include "twintablemember.h"

#include <qevent.h>

#include "celleditor.h"
#include "twintable.h"
#include "../debug.h"

bool TwinTableMember::changing_width = false;

TwinTableMember::TwinTableMember (QWidget *parent, TwinTable *table, int trailing_rows, int trailing_cols) : QTable (parent){
	twin = 0;
	TwinTableMember::table = table;
	setRowMovingEnabled (false);
	setVScrollBarMode (QScrollView::AlwaysOn);
	horizontalHeader()->installEventFilter (this);
	verticalHeader()->installEventFilter (this);
	
	TwinTableMember::trailing_cols = trailing_cols;
	TwinTableMember::trailing_rows = trailing_rows;
	
	tted = 0;

	connect (this, SIGNAL (currentChanged (int, int)), this, SLOT (currentCellChanged (int, int)));
}

TwinTableMember::~TwinTableMember(){
}

int TwinTableMember::numAllRows () {
	return QTable::numRows ();
}

int TwinTableMember::numAllCols () {
	return QTable::numCols ();
}

int TwinTableMember::numRows () {
	return QTable::numRows () - trailing_rows;
}

int TwinTableMember::numCols () {
	return QTable::numCols () - trailing_cols;
}

void TwinTableMember::setTwin (TwinTableMember * new_twin) {
	twin = new_twin;
}

void TwinTableMember::columnWidthChanged (int col) {
	// does all repainting and stuff ...
	QTable::columnWidthChanged (col);

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
	if (event && (event->type() == QEvent::MouseButtonPress)) {
		QMouseEvent  *mouseEvent = (QMouseEvent *) event;
		if (mouseEvent && (mouseEvent->button() == Qt::RightButton)) {
			mouse_at = mouseEvent->globalPos ();
			if (object == horizontalHeader()) {
				emit headerRightClick (-1, horizontalHeader ()->sectionAt (mouseEvent->x ()));
                return(true); // got it
            }
			if (object == verticalHeader()) {
				emit headerRightClick (verticalHeader ()->sectionAt (mouseEvent->y ()), -1);
                return(true); // got it
            }
        }
		setFocus ();
    }

    // default processing
    return(QTable::eventFilter (object, event));
}

// virtual
QString TwinTableMember::rText (int row, int col) const {
	return (RObject::rQuote (text (row, col)));
}

void TwinTableMember::removeRows (const QMemArray<int> &) {
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
}

void TwinTableMember::setCellContentFromEditor (int row, int col) {
	RK_TRACE (EDITOR);
	RK_ASSERT (tted);

	QString text_save = tted->text ();
	
	//tted->removeEventFilter (this);
	delete tted;
	tted = 0;
	
	if (text (row, col) != text_save) {
		setText (row, col, text_save);
		emit (valueChanged (row, col));
	}
	
	viewport ()->setFocus ();
}
