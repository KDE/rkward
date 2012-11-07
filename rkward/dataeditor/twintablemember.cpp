/***************************************************************************
                          twintablemember.cpp  -  description
                             -------------------
    begin                : Tue Oct 29 2002
    copyright            : (C) 2002, 2006, 2007, 2009, 2010, 2012 by Thomas Friedrichsmeier
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

#include <QKeyEvent>
#include <QScrollBar>
#include <QHeaderView>

#include "twintable.h"
#include "rktextmatrix.h"
#include "rkvareditmodel.h"

#include "../debug.h"

TwinTableMember::TwinTableMember (QWidget *parent) : RKTableView (parent){
	RK_TRACE (EDITOR);

	rw = true;

	setVerticalScrollBarPolicy (Qt::ScrollBarAlwaysOn);
	setSelectionMode (QAbstractItemView::ContiguousSelection);

	verticalHeader ()->setContextMenuPolicy (Qt::CustomContextMenu);
	connect (verticalHeader (), SIGNAL (customContextMenuRequested(const QPoint&)), this, SLOT (handleContextMenuRequest(const QPoint&)));
	horizontalHeader ()->setContextMenuPolicy (Qt::CustomContextMenu);
	connect (horizontalHeader (), SIGNAL (customContextMenuRequested(const QPoint&)), this, SLOT (handleContextMenuRequest(const QPoint&)));
	setContextMenuPolicy (Qt::CustomContextMenu);
	connect (this, SIGNAL (customContextMenuRequested(const QPoint&)), this, SLOT (handleContextMenuRequest(const QPoint&)));

	updating_twin = false;
	connect (this, SIGNAL (blankSelectionRequest()), this, SLOT (blankSelected()));
}

TwinTableMember::~TwinTableMember(){
	RK_TRACE (EDITOR);
}

void TwinTableMember::setRKModel (RKVarEditModelBase* model) {
	RK_TRACE (EDITOR);

	mymodel = model;
	setModel (model);

	// now we should also have a selectionModel() (but not before)
	connect (selectionModel (), SIGNAL (selectionChanged(const QItemSelection&,const QItemSelection&)), this, SLOT (tableSelectionChanged(const QItemSelection&,const QItemSelection&)));
}

void TwinTableMember::setTwin (TwinTableMember * new_twin) {
	RK_TRACE (EDITOR);
	twin = new_twin;

	// probably we only need this one way (metaview->dataview), but why not be safe, when it's so easy
	connect (twin->horizontalHeader (), SIGNAL (sectionResized(int,int,int)), this, SLOT (updateColWidth(int,int,int)));
}

void TwinTableMember::tableSelectionChanged (const QItemSelection& selected, const QItemSelection&) {
	RK_TRACE (EDITOR);
	RK_ASSERT (twin);

	if (!selected.isEmpty ()) twin->clearSelection ();
}

int TwinTableMember::trueColumns () const {
    return mymodel->trueCols ();
}

int TwinTableMember::trueRows () const {
    return mymodel->trueRows();
}

void TwinTableMember::stopEditing () {
	RK_TRACE (EDITOR);

	// I wonder why Qt 4.3 doe not provide a function like this...
	if (state () != QAbstractItemView::EditingState) return;

	QModelIndex current = currentIndex ();
	setCurrentIndex (QModelIndex ());
	setCurrentIndex (current);
}

void TwinTableMember::copy () {
	RK_TRACE (EDITOR);

	QItemSelectionRange range = getSelectionBoundaries ();
	if (range.isValid ()) {
		RKTextMatrix mat = mymodel->getTextMatrix (range);
		mat.copyToClipboard ();
	}
}

void TwinTableMember::blankSelected () {
	RK_TRACE (EDITOR);

	if (!rw) return;

	QItemSelectionRange range = getSelectionBoundaries ();
	if (range.isValid ()) mymodel->blankRange (range);
}

void TwinTableMember::paste (RKEditor::PasteMode mode) {
	RK_TRACE (EDITOR);

	if (!rw) return;

	RKTextMatrix pasted = RKTextMatrix::matrixFromClipboard ();
	QItemSelectionRange selrange = getSelectionBoundaries ();
	QItemSelectionRange limrange;
	if (mode == RKEditor::PasteToSelection) {
		limrange = selrange;
	} else if (mode == RKEditor::PasteToTable) {
		limrange = QItemSelectionRange (mymodel->index (0, 0), mymodel->index (mymodel->trueRows () - 1, mymodel->trueCols () - 1));
	} // else: range not set means not confined = PasteAnywhere
	mymodel->setTextMatrix (selrange.topLeft (), pasted, limrange);
}

void TwinTableMember::scrollContentsBy (int dx, int dy) {
	RK_TRACE (EDITOR);

	if (updating_twin) return;
	updating_twin = true;
	RK_ASSERT (twin);
	QTableView::scrollContentsBy (dx, dy);
	twin->horizontalScrollBar ()->setValue (horizontalScrollBar ()->value ());
	updating_twin = false;
}

void TwinTableMember::updateColWidth (int section, int, int new_w) {
	RK_TRACE (EDITOR);

	if (updating_twin) return;
	updating_twin = true;
	setColumnWidth (section, new_w);
	twin->setColumnWidth (section, new_w);
	updating_twin = false;
}

void TwinTableMember::handleContextMenuRequest (const QPoint& pos) {
	RK_TRACE (EDITOR);

	if (sender () == horizontalHeader ()) {
		int col = horizontalHeader ()->logicalIndexAt (pos);
		if (col >= 0) emit (contextMenuRequest (-1, col, horizontalHeader ()->mapToGlobal (pos)));
	} else if (sender () == verticalHeader ()) {
		int row = verticalHeader ()->logicalIndexAt (pos);
		if (row >= 0) emit (contextMenuRequest (row, -1, verticalHeader ()->mapToGlobal (pos)));
	} else {
		RK_ASSERT (sender () == this);

		int col = columnAt (pos.x ());
		int row = rowAt (pos.y ());
		if ((row < 0) || (col < 0)) {
			row = col = -2;	// to differentiate from the headers, above
		}
		emit (contextMenuRequest (row, col, mapToGlobal (pos)));
	}
}

#include "twintablemember.moc"
