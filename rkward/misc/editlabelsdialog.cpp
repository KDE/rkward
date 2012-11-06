/***************************************************************************
                          editlabelsdialog  -  description
                             -------------------
    begin                : Tue Sep 21 2004
    copyright            : (C) 2004, 2006, 2007 by Thomas Friedrichsmeier
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
#include <kdialog.h>
#include <kaction.h>
#include <kactioncollection.h>
#include <kvbox.h>

#include <qlabel.h>
#include <qlayout.h>
#include <QHeaderView>
#include <QTimer>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include "../core/rkvariable.h"
#include "../dataeditor/rktextmatrix.h"
#include "celleditor.h"

#include "../debug.h"

RKVarLevelsTable::RKVarLevelsTable (QWidget *parent, const RObject::ValueLabels& labels) : QTableView (parent) {
	RK_TRACE (EDITOR);

	setHorizontalScrollBarPolicy (Qt::ScrollBarAlwaysOff);
	setSelectionMode (QAbstractItemView::ContiguousSelection);
	horizontalHeader ()->setStretchLastSection (true);
	verticalHeader ()->setFixedWidth (40);
	setMinimumWidth (80);

	KActionCollection *ac = new KActionCollection (this);
	ac->addAction (KStandardAction::Cut, this, SLOT (cut ()));
	ac->addAction (KStandardAction::Copy, this, SLOT (copy ()));
	ac->addAction (KStandardAction::Paste, this, SLOT (paste ()));

	setModel (lmodel = new RKVarLevelsTableModel (labels, this));
}

RKVarLevelsTable::~RKVarLevelsTable () {
	RK_TRACE (EDITOR);
}

bool RKVarLevelsTable::getSelectionBoundaries (int* top, int* bottom) const {
	RK_TRACE (EDITOR);

	RK_ASSERT (selectionModel ());
	QItemSelection sel = selectionModel ()->selection ();
	if (sel.isEmpty ()){
		QModelIndex current = currentIndex ();
		if (!current.isValid ()) return false;

		*top = current.row ();
		*bottom = current.row ();
	} else {
		RK_ASSERT (sel.size () == 1);
		*top = sel[0].top ();
		*bottom = sel[0].bottom ();
	}
	return true;
}

void RKVarLevelsTable::cut () {
	RK_TRACE (EDITOR);

	int top;
	int bottom;
	if (!getSelectionBoundaries (&top, &bottom)) return;

	copy ();

	for (int i = top; i <= bottom; ++i) lmodel->setData (lmodel->index (i, 0), QString ());
}

void RKVarLevelsTable::copy () {
	RK_TRACE (EDITOR);

	int top;
	int bottom;
	if (!getSelectionBoundaries (&top, &bottom)) return;

	RKTextMatrix mat;
	int trow = 0;
	for (int i = top; i <= bottom; ++i) {
		mat.setText (trow++, 0, lmodel->data (lmodel->index (i, 0)).toString ());
	}
	mat.copyToClipboard ();
}

void RKVarLevelsTable::paste () {
	RK_TRACE (EDITOR);

// Unfortunately, we need to duplicate some of TwinTable::paste () and RKEditorDataFramPart::doPaste. Those are not easy to reconcile.
	QModelIndex current = currentIndex ();
	if (!current.isValid ()) return;
	int row = current.row ();
	RK_ASSERT (current.column () == 0);

	RKTextMatrix pasted = RKTextMatrix::matrixFromClipboard ();
	if (pasted.isEmpty ()) return;

	if (pasted.numColumns () > 1) {		// there were tabs in the pasted text. Let's transpose the first row
		for (int i = 0; i < pasted.numColumns (); ++i) {
			lmodel->setData (lmodel->index (row++, 0), pasted.getText (0, i));
		}
	} else {		// else paste the first column
		for (int i = 0; i < pasted.numRows (); ++i) {
			lmodel->setData (lmodel->index (row++, 0), pasted.getText (i, 0));
		}
	}
}

/////////////// RKVarLevelsTableModel /////////////////

RKVarLevelsTableModel::RKVarLevelsTableModel (const RObject::ValueLabels& labels, QObject* parent) : QAbstractTableModel (parent) {
	RK_TRACE (EDITOR);

	RKVarLevelsTableModel::labels = labels;
}

RKVarLevelsTableModel::~RKVarLevelsTableModel () {
	RK_TRACE (EDITOR);
}

int RKVarLevelsTableModel::rowCount (const QModelIndex& parent) const {
	RK_TRACE (EDITOR);

	if (parent.isValid ()) return 0;
	return labels.count () + 1;
}

int RKVarLevelsTableModel::columnCount (const QModelIndex& parent) const {
	RK_TRACE (EDITOR);

	if (parent.isValid ()) return 0;
	return 1;
}

QVariant RKVarLevelsTableModel::data (const QModelIndex& index, int role) const {
	RK_TRACE (EDITOR);

	if (!index.isValid ()) return QVariant ();
	if (index.column () != 0) return QVariant ();
	if ((role == Qt::BackgroundRole) && (index.row () == labels.count ())) return QBrush (Qt::gray);
	if (index.row () >= labels.count ()) return QVariant ();

	if ((role == Qt::DisplayRole) || (role == Qt::EditRole)) return labels.value (QString::number (index.row ()+1));

	return QVariant ();
}

Qt::ItemFlags RKVarLevelsTableModel::flags (const QModelIndex& index) const {
	RK_TRACE (EDITOR);

	if (!index.isValid ()) return 0;
	if (index.column () != 0) return 0;
	if (index.row () >= labels.count ()) return (Qt::ItemIsEditable | Qt::ItemIsEnabled);
	return (Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
}

bool RKVarLevelsTableModel::setData (const QModelIndex& index, const QVariant& value, int role) {
	RK_TRACE (EDITOR);

	if (role != Qt::EditRole) return false;
	if (!index.isValid ()) return false;
	if (index.column () != 0) return false;
	if (!value.isValid ()) return false;
	if (index.row () > labels.count ()) return false;

	QString text = value.toString ();
	if (index.row () == labels.count ()) {
		beginInsertRows (QModelIndex (), index.row (), index.row ());
		labels.insert (QString::number (index.row () + 1), text);
		endInsertRows ();
	} else {
		labels.insert (QString::number (index.row () + 1), text);
		emit (dataChanged (index, index));
	}

	if (text.isEmpty ()) {	// remove trailing empty rows
		while ((!labels.isEmpty ()) && labels.value (QString::number (labels.count ())).isEmpty ()) {
			int row = labels.count () - 1;
			beginRemoveRows (QModelIndex (), row, row);
			labels.remove (QString::number (row + 1));
			endRemoveRows ();
		}
	}

	return true;
}

QVariant RKVarLevelsTableModel::headerData (int section, Qt::Orientation orientation, int role) const {
	RK_TRACE (EDITOR);

	if (role != Qt::DisplayRole) return QVariant ();
	if (orientation == Qt::Vertical) return QString::number (section + 1);
	if (section != 0) return QVariant ();
	return i18n ("Label");
}

//////////////// EditLabelsDialog ///////////////////////

EditLabelsDialog::EditLabelsDialog (QWidget *parent, const RObject::ValueLabels& labels, const QString& varname) : KDialog (parent) {
	RK_TRACE (EDITOR);

	KVBox *mainvbox = new KVBox ();
	setMainWidget (mainvbox);
	QLabel *label = new QLabel (i18n ("Levels can be assigned only to consecutive integers starting with 1 (the index column is read only). To remove levels at the end of the list, just set them to empty."), mainvbox);
	label->setWordWrap (true);

	table = new RKVarLevelsTable (mainvbox, labels);

	setButtons (KDialog::Ok | KDialog::Cancel);
	setCaption (i18n ("Levels / Value labels for '%1'", varname));
}

EditLabelsDialog::~EditLabelsDialog () {
	RK_TRACE (EDITOR);
}

void EditLabelsDialog::accept () {
	RK_TRACE (EDITOR);

	table->setCurrentIndex (QModelIndex ());	// should flush editing
	KDialog::accept ();
}

////////////////// EditLabelsDialogProxy /////////////////////////

EditLabelsDialogProxy::EditLabelsDialogProxy (QWidget* parent) : QWidget (parent) {
	RK_TRACE (EDITOR);
	dialog = 0;
}

EditLabelsDialogProxy::~EditLabelsDialogProxy () {
	RK_TRACE (EDITOR);
}

void EditLabelsDialogProxy::initialize (const RObject::ValueLabels& labels, const QString& varname) {
	RK_TRACE (EDITOR);

	if (dialog) return;	// one dialog at a time, please!

	EditLabelsDialogProxy::labels = labels;		// we need to take a copy in case the dialog is rejected

	dialog = new EditLabelsDialog (this, labels, varname);
	connect (dialog, SIGNAL (finished(int)), this, SLOT (dialogDone(int)));
	QTimer::singleShot (0, dialog, SLOT (exec()));
}

void EditLabelsDialogProxy::dialogDone (int result) {
	RK_TRACE (EDITOR);
	RK_ASSERT (dialog);

	if (result == QDialog::Accepted) {
		labels = dialog->table->lmodel->labels;
		emit (done (this, RKItemDelegate::EditorExit));
	} else {
		emit (done (this, RKItemDelegate::EditorReject));
	}
	dialog->deleteLater ();
	dialog = 0;
}


#include "editlabelsdialog.moc"
