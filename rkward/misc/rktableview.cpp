/***************************************************************************
                          rktableview  -  description
                             -------------------
    begin                : Tue Nov 06
    copyright            : (C) 2012 by Thomas Friedrichsmeier
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

#include "rktableview.h"

#include "../debug.h"

RKTableView::RKTableView (QWidget *parent) : QTableView (parent) {
	RK_TRACE (MISC);

	trailing_columns = 0;
	trailing_rows = 0;
}

RKTableView::~RKTableView () {
	RK_TRACE (MISC);
}

int RKTableView::apparentColumns () const {
	return model ()->columnCount ();
}

int RKTableView::apparentRows () const {
	return model ()->rowCount ();
}

QItemSelectionRange RKTableView::getSelectionBoundaries () const {
	RK_TRACE (MISC);

	RK_ASSERT (selectionModel ());
	QItemSelection sel = selectionModel ()->selection ();
	if (sel.isEmpty ()){
		QModelIndex current = currentIndex ();
		if ((!current.isValid ()) || isIndexHidden (current)) return (QItemSelectionRange ());

		return (QItemSelectionRange (currentIndex (), currentIndex ()));
	} else {
		RK_ASSERT (sel.size () == 1);

		QItemSelectionRange range = sel[0];
		while (isColumnHidden (range.left ())) {
			// purge hidden leading columns from the range
			range = QItemSelectionRange (model ()->index (range.top (), range.left () + 1, rootIndex ()), range.bottomRight ());
		}
		return (range);
	}
}

void RKTableView::editorDone (QWidget* editor, RKItemDelegate::EditorDoneReason reason) {
	RK_TRACE (EDITOR);

	int row = currentIndex ().row ();
	int col = currentIndex ().column ();

	closeEditor (editor, QAbstractItemDelegate::NoHint);

	if (reason == RKItemDelegate::EditorExitRight) ++col;
	else if (reason == RKItemDelegate::EditorExitLeft) --col;
	else if (reason == RKItemDelegate::EditorExitUp) --row;
	else if (reason == RKItemDelegate::EditorExitDown) ++row;

	if (row >= trueRows ()) {
		// if we have edited the trailing row, a new row may have been inserted, apparently *above* the
		// current index. We need to fix this up. Basically, we can only ever be in the last row after
		// a reject, or an exit to the next row
		if ((reason != RKItemDelegate::EditorExitDown) && (reason != RKItemDelegate::EditorReject)) --row;
	}
	if (col >= trueColumns ()) {
		// see above
		if ((reason != RKItemDelegate::EditorExitRight) && (reason != RKItemDelegate::EditorReject)) --col;
	}

	if ((row < apparentRows ()) && (col < apparentColumns ())) {
		setCurrentIndex (model ()->index (row, col));
	}
}

void RKTableView::setRKItemDelegate (RKItemDelegate* delegate) {
	RK_TRACE (EDITOR);

	setItemDelegate (delegate);
	connect (delegate, SIGNAL (doCloseEditor(QWidget*,RKItemDelegate::EditorDoneReason)), this, SLOT (editorDone(QWidget*,RKItemDelegate::EditorDoneReason)));
}


/////////////////// RKItemDelegate /////////////////////

#include "../dataeditor/rkvareditmodel.h"
#include "celleditor.h"
#include "editformatdialog.h"
#include "editlabelsdialog.h"

#include <QKeyEvent>

RKItemDelegate::RKItemDelegate (QObject *parent, RKVarEditModel* datamodel) : QItemDelegate (parent) {
	RK_TRACE (EDITOR);

	RKItemDelegate::datamodel = datamodel;
	metamodel = 0;
	genericmodel = 0;
	locked_for_modal_editor = false;
}

RKItemDelegate::RKItemDelegate (QObject *parent, RKVarEditMetaModel* metamodel) : QItemDelegate (parent) {
	RK_TRACE (EDITOR);

	RKItemDelegate::metamodel = metamodel;
	datamodel = 0;
	genericmodel = 0;
	locked_for_modal_editor = false;
}

RKItemDelegate::RKItemDelegate (QObject *parent, QAbstractItemModel* model, bool dummy) : QItemDelegate (parent) {
	RK_TRACE (EDITOR);
	Q_UNUSED (dummy);

	genericmodel = model;
	metamodel = 0;
	datamodel = 0;
	locked_for_modal_editor = false;
}

RKItemDelegate::~RKItemDelegate () {
	RK_TRACE (EDITOR);
}

QWidget* RKItemDelegate::createEditor (QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const {
	RK_TRACE (EDITOR);

	QWidget* ed;
	if (metamodel) {
		int row = index.row ();
		if (row == RKVarEditMetaModel::FormatRow) {
			ed = new EditFormatDialogProxy (parent);
			const_cast<RKItemDelegate*> (this)->locked_for_modal_editor = true;
		} else if (row == RKVarEditMetaModel::LevelsRow) {
			ed = new EditLabelsDialogProxy (parent);
			const_cast<RKItemDelegate*> (this)->locked_for_modal_editor = true;
		} else {
			ed = new CellEditor (parent);
		}
	} else {
		RK_ASSERT (datamodel || genericmodel);
		ed = new CellEditor (parent);
	}

	ed->setFont (option.font);
	connect (ed, SIGNAL (done(QWidget*,RKItemDelegate::EditorDoneReason)), this, SLOT (editorDone(QWidget*,RKItemDelegate::EditorDoneReason)));
	return ed;
}

void RKItemDelegate::setEditorData (QWidget* editor, const QModelIndex& index) const {
	RK_TRACE (EDITOR);

	if (!index.isValid ()) return;

	if (metamodel) {
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
		CellEditor* ced = static_cast<CellEditor*> (editor);
		ced->setText (index.data (Qt::EditRole).toString ());
		if (datamodel) {
			if (index.column () < datamodel->trueCols ()) {
				ced->setValueLabels (datamodel->getObject (index.column ())->getValueLabels ());
			}
		} else {
			RK_ASSERT (genericmodel);
		}
	}
}

void RKItemDelegate::setModelData (QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const {
	RK_TRACE (EDITOR);

	if (!index.isValid ()) return;

	if (metamodel) {
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
	} else if (datamodel) {
		RK_ASSERT (model == datamodel);
	} else {
		RK_ASSERT (genericmodel && (model == genericmodel));
	}

	CellEditor* ced = static_cast<CellEditor*> (editor);
	model->setData (index, ced->text (), Qt::EditRole);
}

bool RKItemDelegate::eventFilter (QObject* object, QEvent* event) {
	RK_TRACE (EDITOR);

	if (locked_for_modal_editor) return false;	// Needed on MacOSX: Pressing Ok in one of the modal editors seems to
							// generate a Return-like event.
							// This would be handled *before* the editor had a chance to update its data,
							// thus committing the old, not new state.

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

	if (reason != EditorReject) commitData (editor);
	emit (doCloseEditor (editor, reason));
	locked_for_modal_editor = false;
}

#include "rktableview.moc"
