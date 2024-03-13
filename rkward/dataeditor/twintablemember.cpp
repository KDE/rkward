/*
twintablemember.cpp - This file is part of RKWard (https://rkward.kde.org). Created: Tue Oct 29 2002
SPDX-FileCopyrightText: 2002-2012 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

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
	connect (verticalHeader (), &QWidget::customContextMenuRequested, this, &TwinTableMember::handleContextMenuRequest);
	horizontalHeader ()->setContextMenuPolicy (Qt::CustomContextMenu);
	connect (horizontalHeader (), &QWidget::customContextMenuRequested, this, &TwinTableMember::handleContextMenuRequest);
	setContextMenuPolicy (Qt::CustomContextMenu);
	connect (this, &QWidget::customContextMenuRequested, this, &TwinTableMember::handleContextMenuRequest);

	updating_twin = false;
	connect (this, &TwinTableMember::blankSelectionRequest, this, &TwinTableMember::blankSelected);
}

TwinTableMember::~TwinTableMember(){
	RK_TRACE (EDITOR);
}

void TwinTableMember::setRKModel (RKVarEditModelBase* model) {
	RK_TRACE (EDITOR);

	mymodel = model;
	setModel (model);

	// now we should also have a selectionModel() (but not before)
	connect (selectionModel (), &QItemSelectionModel::selectionChanged, this, &TwinTableMember::tableSelectionChanged);
}

void TwinTableMember::setTwin (TwinTableMember * new_twin) {
	RK_TRACE (EDITOR);
	twin = new_twin;

	// probably we only need this one way (metaview->dataview), but why not be safe, when it's so easy
	connect (twin->horizontalHeader (), &QHeaderView::sectionResized, this, &TwinTableMember::updateColWidth);
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
		if (col >= 0) Q_EMIT contextMenuRequest(-1, col, horizontalHeader()->mapToGlobal(pos));
	} else if (sender () == verticalHeader ()) {
		int row = verticalHeader ()->logicalIndexAt (pos);
		if (row >= 0) Q_EMIT contextMenuRequest(row, -1, verticalHeader()->mapToGlobal(pos));
	} else {
		RK_ASSERT (sender () == this);

		int col = columnAt (pos.x ());
		int row = rowAt (pos.y ());
		if ((row < 0) || (col < 0)) {
			row = col = -2;	// to differentiate from the headers, above
		}
		Q_EMIT contextMenuRequest(row, col, mapToGlobal(pos));
	}
}

