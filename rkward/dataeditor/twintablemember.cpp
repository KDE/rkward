/***************************************************************************
                          twintablemember.cpp  -  description
                             -------------------
    begin                : Tue Oct 29 2002
    copyright            : (C) 2002, 2006, 2007 by Thomas Friedrichsmeier
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

#include "celleditor.h"
#include "twintable.h"
#include "rktextmatrix.h"
#include "rkvareditmodel.h"

#include "../debug.h"

TwinTableMember::TwinTableMember (QWidget *parent, TwinTable *table) : QTableView (parent){
	RK_TRACE (EDITOR);

	twin = 0;
	TwinTableMember::table = table;
	setVerticalScrollBarPolicy (Qt::ScrollBarAlwaysOn);
	setSelectionMode (QAbstractItemView::ContiguousSelection);
	setContextMenuPolicy (Qt::CustomContextMenu);
	connect (this, SIGNAL (customContextMenuRequested (const QPoint&)), this, SLOT (headerContextMenuRequested (const QPoint&)));
	
	tted = 0;
#warning tted and changing width currently unused, but likey will be used.
	changing_width = false;
	changing_scroll = false;

	connect (this, SIGNAL (currentChanged (int, int)), this, SLOT (currentCellChanged (int, int)));
}

TwinTableMember::~TwinTableMember(){
	RK_TRACE (EDITOR);
}

void TwinTableMember::setRKModel (RKVarEditModelBase* model) {
	RK_TRACE (EDITOR);
	mymodel = model; setModel (model);
};

void TwinTableMember::setTwin (TwinTableMember * new_twin) {
	RK_TRACE (EDITOR);
	twin = new_twin;
}

void TwinTableMember::editorLostFocus () {
	RK_TRACE (EDITOR);
	stopEditing ();
}

void TwinTableMember::stopEditing () {
	RK_TRACE (EDITOR);
#warning todo
//	if (tted) endEdit (currEditRow (), currEditCol (), true, false);
	RK_ASSERT (!tted);
}

#if 0
void TwinTableMember::endEdit (int row, int col, bool, bool) {
	RK_TRACE (EDITOR);
	if (tted) setCellContentFromEditor (row, col);
	setEditMode (NotEditing, -1, -1);
}
#endif

#if 0
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
#endif

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

	QItemSelectionRange range = getSelectionBoundaries ();
	if (range.isValid ()) mymodel->blankRange (range);
}

void TwinTableMember::paste (RKEditor::PasteMode mode) {
	RK_TRACE (EDITOR);

	RKTextMatrix pasted = RKTextMatrix::matrixFromClipboard ();
	QItemSelectionRange range;
	if (mode == RKEditor::PasteToSelection) {
		range = getSelectionBoundaries ();
	} else if (mode == RKEditor::PasteToTable) {
		range = QItemSelectionRange (mymodel->index (0, 0), mymodel->index (mymodel->trueRows (), mymodel->trueCols ()));
	} // else: range not set means not confined = PasteAnywhere
	mymodel->setTextMatrix (currentIndex (), pasted, range);
}

QItemSelectionRange TwinTableMember::getSelectionBoundaries () {
	RK_TRACE (EDITOR);

	RK_ASSERT (selectionModel ());
	QItemSelection sel = selectionModel ()->selection ();
	if (sel.isEmpty ()){
		QModelIndex current = currentIndex ();
		if (!current.isValid ()) return (QItemSelectionRange ());

		return (QItemSelectionRange (currentIndex (), currentIndex ()));
	} else {
		RK_ASSERT (sel.size () == 1);
		return (sel[0]);
	}
}

void TwinTableMember::keyPressEvent (QKeyEvent *e) {
	RK_TRACE (EDITOR);

	if ((e->key () == Qt::Key_Delete) || (e->key () == Qt::Key_Backspace)) {
		blankSelected ();
		e->accept ();
	} else {
		QTableView::keyPressEvent (e);
	}
}

void TwinTableMember::scrollContentsBy (int dx, int dy) {
	RK_TRACE (EDITOR);

	if (changing_scroll) return;
	changing_scroll = true;
	RK_ASSERT (twin);
	QTableView::scrollContentsBy (dx, dy);
	twin->horizontalScrollBar ()->setValue (horizontalScrollBar ()->value ());
	changing_scroll = false;
}

void TwinTableMember::headerContextMenuRequested (const QPoint& pos) {
	RK_TRACE (EDITOR);

	mouse_at = pos;
#warning TODO
}

#include "twintablemember.moc"
