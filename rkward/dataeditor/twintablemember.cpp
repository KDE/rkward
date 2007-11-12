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
#include "editlabelsdialog.h"
#include "twintable.h"
#include "rktextmatrix.h"
#include "rkvareditmodel.h"

#include "../debug.h"

TwinTableMember::TwinTableMember (QWidget *parent, TwinTable *table) : QTableView (parent){
	RK_TRACE (EDITOR);

	twin = 0;
#warning member "table" seems to be unused
	TwinTableMember::table = table;
	setVerticalScrollBarPolicy (Qt::ScrollBarAlwaysOn);
	setSelectionMode (QAbstractItemView::ContiguousSelection);

	verticalHeader ()->setContextMenuPolicy (Qt::CustomContextMenu);
	connect (verticalHeader (), SIGNAL (customContextMenuRequested(const QPoint&)), this, SLOT (headerContextMenuRequested(const QPoint&)));
	horizontalHeader ()->setContextMenuPolicy (Qt::CustomContextMenu);
	connect (horizontalHeader (), SIGNAL (customContextMenuRequested(const QPoint&)), this, SLOT (headerContextMenuRequested(const QPoint&)));

	updating_twin = false;
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

	QItemSelectionRange range = getSelectionBoundaries ();
	if (range.isValid ()) mymodel->blankRange (range);
}

void TwinTableMember::paste (RKEditor::PasteMode mode) {
	RK_TRACE (EDITOR);

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

void TwinTableMember::updateColWidth (int section, int, int new_w) {
	RK_TRACE (EDITOR);

	if (updating_twin) return;
	updating_twin = true;
	setColumnWidth (section, new_w);
	twin->setColumnWidth (section, new_w);
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
		} else if (row == RKVarEditMetaModel::LevelsRow) {
			ed = new EditLabelsDialogProxy (parent);
		} else {
			ed = new CellEditor (parent);
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
		CellEditor* ced = static_cast<CellEditor*> (editor);
		ced->setText (datamodel->data (index, Qt::EditRole).toString ());

		if (index.column () < datamodel->trueCols ()) {
			ced->setValueLabels (datamodel->getObject (index.column ())->getValueLabels ());
		}
	} else if (metamodel) {
		int row = index.row ();
		if (row == RKVarEditMetaModel::FormatRow) {
			EditFormatDialogProxy* fed = static_cast<EditFormatDialogProxy*> (editor);
			fed->initialize (RKVariable::parseFormattingOptionsString (metamodel->data (index, Qt::EditRole).toString ()), metamodel->headerData (index.column (), Qt::Horizontal).toString ());
		} else if (row == RKVarEditMetaModel::LevelsRow) {
			EditLabelsDialogProxy* led = static_cast<EditLabelsDialogProxy*> (editor);
			led->initialize (metamodel->getValueLabels (index.column ()), metamodel->headerData (index.column (), Qt::Horizontal).toString ());
		} else {
			CellEditor* ced = static_cast<CellEditor*> (editor);
			ced->setText (metamodel->data (index, Qt::EditRole).toString ());

			if (row == RKVarEditMetaModel::TypeRow) {
				RObject::ValueLabels labels;
				for (int i = RObject::MinKnownDataType; i <= RObject::MaxKnownDataType; ++i) {
					labels.insert (QString::number (i), RObject::typeToText ((RObject::RDataType) i));
				}
				ced->setValueLabels (labels);
			}
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
		} else if (row == RKVarEditMetaModel::LevelsRow) {
			EditLabelsDialogProxy* led = static_cast<EditLabelsDialogProxy*> (editor);
			metamodel->setValueLabels (index.column (), led->getLabels ());
			return;
		} // else all others use the regular CellEditor
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
