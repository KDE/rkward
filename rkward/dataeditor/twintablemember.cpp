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
#include <QHeaderView>

#include "celleditor.h"
#include "editformatdialog.h"
#include "twintable.h"
#include "rktextmatrix.h"
#include "rkvareditmodel.h"

#include "../debug.h"

TwinTableMember::TwinTableMember (QWidget *parent, TwinTable *table) : QTableView (parent){
	RK_TRACE (EDITOR);

	twin = 0;
	TwinTableMember::table = table;		// TODO: seems unused
	setVerticalScrollBarPolicy (Qt::ScrollBarAlwaysOn);
	setSelectionMode (QAbstractItemView::ContiguousSelection);

	verticalHeader ()->setContextMenuPolicy (Qt::CustomContextMenu);
	connect (verticalHeader (), SIGNAL (customContextMenuRequested(const QPoint&)), this, SLOT (headerContextMenuRequested(const QPoint&)));
	horizontalHeader ()->setContextMenuPolicy (Qt::CustomContextMenu);
	connect (horizontalHeader (), SIGNAL (customContextMenuRequested(const QPoint&)), this, SLOT (headerContextMenuRequested(const QPoint&)));

	connect (this, SIGNAL (selectionChanged(const QItemSelection&,const QItemSelection&)), this, SLOT (tableSelectionChanged(const QItemSelection&,const QItemSelection&)));

	updating_twin = false;

#warning currently unused, but likey will be used.
	tted = 0;

	connect (this, SIGNAL (currentChanged (int, int)), this, SLOT (currentCellChanged (int, int)));
}

TwinTableMember::~TwinTableMember(){
	RK_TRACE (EDITOR);
}

void TwinTableMember::setRKModel (RKVarEditModelBase* model) {
	RK_TRACE (EDITOR);
	mymodel = model;
	setModel (model);
}

void TwinTableMember::seRKItemDelegate (RKItemDelegate* delegate) {
	RK_TRACE (EDITOR);

	setItemDelegate (delegate);
	connect (delegate, SIGNAL (doCloseEditor(QWidget*,RKItemDelegate::EditorDoneReason)), this, SLOT (editorDone(QWidget*,RKItemDelegate::EditorDoneReason)));
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

void TwinTableMember::editorDone (QWidget* editor, RKItemDelegate::EditorDoneReason reason) {
	RK_TRACE (EDITOR);

	int row = currentIndex ().row ();
	int col = currentIndex ().column ();

	closeEditor (editor, QAbstractItemDelegate::NoHint);

	if (reason == RKItemDelegate::EditorExitRight) ++col;
	else if (reason == RKItemDelegate::EditorExitLeft) --col;
	else if (reason == RKItemDelegate::EditorExitUp) --row;
	else if (reason == RKItemDelegate::EditorExitDown) ++row;

	if ((row < mymodel->rowCount ()) && (col < mymodel->columnCount ())) {
		setCurrentIndex (mymodel->index (row, col));
	}
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

	if (updating_twin) return;
	updating_twin = true;
	RK_ASSERT (twin);
	QTableView::scrollContentsBy (dx, dy);
	twin->horizontalScrollBar ()->setValue (horizontalScrollBar ()->value ());
	updating_twin = false;
}

void TwinTableMember::updateColWidth (int section, int old_w, int new_w) {
	RK_TRACE (EDITOR);

	if (updating_twin) return;
	updating_twin = true;
	RK_ASSERT (columnWidth (section) == old_w);
	setColumnWidth (section, new_w);
	updating_twin = false;
}

void TwinTableMember::headerContextMenuRequested (const QPoint& pos) {
	RK_TRACE (EDITOR);

	if (sender () == horizontalHeader ()) {
		int col = horizontalHeader ()->logicalIndexAt (pos);
		if (col >= 0) emit (contextMenuRequest (-1, col, horizontalHeader ()->mapToGlobal (pos)));
	} else if (sender () == verticalHeader ()) {
		int row = verticalHeader ()->logicalIndexAt (pos);
		if (row >= 0) emit (contextMenuRequest (row, -1, verticalHeader ()->mapToGlobal (pos)));
	} else {
		RK_ASSERT (false);
	}
}

/////////////////// RKItemDelegate /////////////////////

RKItemDelegate::RKItemDelegate (QObject *parent, RKVarEditModel* datamodel) : QItemDelegate (parent) {
	RK_TRACE (EDITOR);

	RKItemDelegate::datamodel = datamodel;
	metamodel = 0;
}

RKItemDelegate::RKItemDelegate (QObject *parent, RKVarEditMetaModel* metamodel) : QItemDelegate (parent) {
	RK_TRACE (EDITOR);

	RKItemDelegate::metamodel = metamodel;
	datamodel = 0;
}

RKItemDelegate::~RKItemDelegate () {
	RK_TRACE (EDITOR);
}

QWidget* RKItemDelegate::createEditor (QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const {
	RK_TRACE (EDITOR);

	QWidget* ed = 0;
	if (datamodel) {
		ed = new CellEditor (parent);
	} else if (metamodel) {
		int row = index.row ();
		if (row == RKVarEditMetaModel::FormatRow) {
			ed = new EditFormatDialogProxy (parent);
		} else {
			ed = new CellEditor (parent);
#warning implement
		}
	}

	if (ed) {
		ed->setFont (option.font);
		connect (ed, SIGNAL (done(QWidget*,RKItemDelegate::EditorDoneReason)), this, SLOT (editorDone(QWidget*,RKItemDelegate::EditorDoneReason)));
		return ed;
	}

	RK_ASSERT (false);
	return 0;
}

void RKItemDelegate::setEditorData (QWidget* editor, const QModelIndex& index) const {
	RK_TRACE (EDITOR);

	if (!index.isValid ()) return;

	if (datamodel) {
		// do nothing. CellEditor will be intialized below
		CellEditor* ced = static_cast<CellEditor*> (editor);
		ced->setText (datamodel->data (index, Qt::EditRole).toString ());

		RObject::ValueLabels* labels = 0;
		if (index.column () < datamodel->trueCols ()) {
			labels = datamodel->getObject (index.column ())->getValueLabels ();
		}
		if (labels) ced->setValueLabels (labels);
	} else if (metamodel) {
		int row = index.row ();
		if (row == RKVarEditMetaModel::FormatRow) {
			EditFormatDialogProxy* fed = static_cast<EditFormatDialogProxy*> (editor);
			fed->initialize (RKVariable::parseFormattingOptionsString (metamodel->data (index, Qt::EditRole).toString ()), metamodel->data (metamodel->index (RKVarEditMetaModel::FormatRow, index.column ())).toString ());
		} else {
#warning implement
			CellEditor* ced = static_cast<CellEditor*> (editor);
			ced->setText (metamodel->data (index, Qt::EditRole).toString ());
		}
	} else {
		RK_ASSERT (false);
	}


}

void RKItemDelegate::setModelData (QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const {
	RK_TRACE (EDITOR);

	if (!index.isValid ()) return;

	if (datamodel) {
		RK_ASSERT (model == datamodel);
		// real work is done down below
	} else if (metamodel) {
		RK_ASSERT (model == metamodel);

		int row = index.row ();
		if (row == RKVarEditMetaModel::FormatRow) {
			EditFormatDialogProxy* fed = static_cast<EditFormatDialogProxy*> (editor);
			model->setData (index, RKVariable::formattingOptionsToString (fed->getOptions ()), Qt::EditRole);
			return;
		} else {
#warning implement
		}
	} else {
		RK_ASSERT (false);
	}

	CellEditor* ced = static_cast<CellEditor*> (editor);
	model->setData (index, ced->text (), Qt::EditRole);
}

bool RKItemDelegate::eventFilter (QObject* object, QEvent* event) {
	RK_TRACE (EDITOR);

	QWidget *editor = qobject_cast<QWidget*> (object);
	if (!editor) return false;

	if (event->type() == QEvent::KeyPress) {
		QKeyEvent* ke = static_cast<QKeyEvent *> (event);
		if (ke->key () == Qt::Key_Tab) editorDone (editor, EditorExitRight);
		else if (ke->key () == Qt::Key_Tab) editorDone (editor, EditorExitRight);
		else if (ke->key () == Qt::Key_Enter) editorDone (editor, EditorExitDown);
		else if (ke->key () == Qt::Key_Return) editorDone (editor, EditorExitDown);
		else return QItemDelegate::eventFilter (editor, event);
		return true;
	}
	return QItemDelegate::eventFilter (editor, event);
}

void RKItemDelegate::editorDone (QWidget* editor, RKItemDelegate::EditorDoneReason reason) {
	RK_TRACE (EDITOR);

	if (reason != EditorReject) emit (commitData (editor));
	emit (doCloseEditor (editor, reason));
}

#include "twintablemember.moc"
