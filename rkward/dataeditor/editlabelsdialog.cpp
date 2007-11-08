/***************************************************************************
                          editlabelsdialog  -  description
                             -------------------
    begin                : Tue Sep 21 2004
    copyright            : (C) 2004, 2006 by Thomas Friedrichsmeier
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

#include <qapplication.h>
#include <qclipboard.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qpainter.h>
#include <qrect.h>
#include <qpalette.h>
#include <qstyle.h>
//Added by qt3to4:
#include <Q3HBoxLayout>
#include <QResizeEvent>
#include <Q3VBoxLayout>

#include "../core/rkvariable.h"
#include "celleditor.h"

#include "../debug.h"

LevelsTable::LevelsTable (QWidget *parent, RObject::ValueLabels *labels) : TwinTableMember (parent, 0, 1, 0) {
	RK_TRACE (EDITOR);

	RK_ASSERT (labels);
	storage = labels;

	updating_size = false;

	setNumCols (1);
	setNumRows (storage->count () + 1);
	horizontalHeader ()->setLabel (0, i18n ("Label"));
	setHScrollBarMode (Q3ScrollView::AlwaysOff);
	setLeftMargin (40);
	setMinimumWidth (80);

	KActionCollection *ac = new KActionCollection (this);
	ac->addAction (KStandardAction::Cut, this, SLOT (cut ()));
	ac->addAction (KStandardAction::Copy, this, SLOT (copy ()));
	ac->addAction (KStandardAction::Paste, this, SLOT (paste ()));
}

LevelsTable::~LevelsTable () {
	RK_TRACE (EDITOR);
}

void LevelsTable::cut () {
	RK_TRACE (EDITOR);

	copy ();
	blankSelected ();
}

void LevelsTable::copy () {
	RK_TRACE (EDITOR);

#warning copy functionality should be move to TwinTableMember directly. See also RKEditorDataFramePart
	QString text = getSelectionText ();
	QMimeData* data = new QMimeData ();
	data->setText (text);
	data->setData ("text/tab-separated-values", text.toLocal8Bit ());
	QApplication::clipboard()->setMimeData (data);
}

void LevelsTable::paste () {
	RK_TRACE (EDITOR);

// Unfortunately, we need to duplicate some of TwinTable::paste () and RKEditorDataFramPart::doPaste. Those are not easy to reconcile.

	// actually, we don't care, whether tsv or plain gets pasted - it's both
	// treated the same. We should however encourage external senders to
	// provided the two in order.
	QString pasted;
	const QMimeData* data = QApplication::clipboard ()->mimeData ();
	if (data->hasFormat ("text/tab-separated-values")) {
		pasted = QString::fromLocal8Bit (data->data ("text/tab-separated-values"));
	} else if (data->hasText ()) {
		pasted = data->text ();
	} else {
		RK_DO (qDebug ("no suitable format for pasting"), EDITOR, DL_INFO);
		return;
	}

	int content_offset = 0;
	int content_length = pasted.length ();
	bool look_for_tabs;			// break on tabs or on lines?
	int next_delim;

	int first_tab = pasted.find ('\t', 0);
	if (first_tab < 0) first_tab = content_length;
	int first_line = pasted.find ('\n', 0);
	if (first_line < 0) first_line = content_length;
	if (first_tab < first_line) {
		look_for_tabs = true;
		next_delim = first_tab;
	} else {
		look_for_tabs = false;
		next_delim = first_line;
	}

	int row = currentRow ();
	do {
		if (row >= numTrueRows ()) insertRows (row);
		setText (row, 0, pasted.mid (content_offset, next_delim - content_offset));

		++row;
		content_offset = next_delim + 1;
		if (look_for_tabs) {
			next_delim = pasted.find ('\t', content_offset);
		} else {
			next_delim = pasted.find ('\n', content_offset);
		}
		if (next_delim < 0) next_delim = content_length;
	} while (content_offset < content_length);
}

void LevelsTable::setText (int row, int col, const QString &text) {
	RK_TRACE (EDITOR);
	RK_ASSERT (col == 0);

	storage->insert (QString::number (row+1), text);
	if (text.isEmpty ()) {
		int maxrow = numTrueRows ()-1;
		while ((maxrow >= 0) && LevelsTable::text (maxrow, 1).isEmpty ()) {
			storage->remove (QString::number (maxrow + 1));
			--maxrow;
		}
		setNumRows (maxrow + 2);
	}

	updateCell (row, col);
}

QString LevelsTable::text (int row, int) const {
	RK_TRACE (EDITOR);

	if (row < numTrueRows ()) {
		return ((*storage)[QString::number (row+1)]);
	}
	return QString ();
}

void LevelsTable::paintCell (QPainter *p, int row, int col, const QRect &cr, bool selected, const QColorGroup &cg) {
	// no trace for paint operations

	paintCellInternal (p, row, col, cr, selected, cg, 0, 0, text (row, col), 0);
}

QWidget *LevelsTable::beginEdit (int row, int col, bool) {
	RK_TRACE (EDITOR);
	RK_ASSERT (!tted);

	if (col != 0) return 0;

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

void LevelsTable::resizeEvent (QResizeEvent *e) {
	RK_TRACE (EDITOR);

	updating_size = true;
	int nwidth = e->size ().width () - leftMargin ();
	if (nwidth < 40) {
		setLeftMargin (e->size ().width () - 40);
		nwidth = 40;
	}
	setColumnWidth (0, nwidth);
	updating_size = false;

	Q3Table::resizeEvent (e);
}

void LevelsTable::columnWidthChanged (int col) {
	RK_TRACE (EDITOR);

	if (updating_size) return;

	updating_size = true;

	if (columnWidth (0) < 40) {
		setColumnWidth (0, 40);
	}
	setLeftMargin (width () - columnWidth (0));

	updating_size = false;

	Q3Table::columnWidthChanged (col);
}



EditLabelsDialog::EditLabelsDialog (QWidget *parent, RKVariable *var, int mode) : KDialog (parent) {
	RK_TRACE (EDITOR);
	RK_ASSERT (var);
//	RK_ASSERT (var->objectOpened ());

	EditLabelsDialog::var = var;
	EditLabelsDialog::mode = mode;

	KVBox *mainvbox = new KVBox ();
	setMainWidget (mainvbox);
	QLabel *label = new QLabel (i18n ("Levels can be assigned only to consecutive integers starting with 1 (the index column is read only). To remove levels at the end of the list, just set them to empty."), mainvbox);
	label->setWordWrap (true);

	Q3HBoxLayout *hbox = new Q3HBoxLayout (mainvbox, KDialog::spacingHint ());

	RObject::ValueLabels *labels = var->getValueLabels ();
	if (!labels) {
		labels = new RObject::ValueLabels;
	}

	table = new LevelsTable (this, labels);
	hbox->addWidget (table);

	Q3HBoxLayout *buttonbox = new Q3HBoxLayout (mainvbox, KDialog::spacingHint ());

	QPushButton *ok_button = new QPushButton (i18n ("Ok"), this);
	connect (ok_button, SIGNAL (clicked ()), this, SLOT (accept ()));
	buttonbox->addWidget (ok_button);

	QPushButton *cancel_button = new QPushButton (i18n ("Cancel"), this);
	connect (cancel_button, SIGNAL (clicked ()), this, SLOT (reject ()));
	buttonbox->addWidget (cancel_button);
	
	setCaption (i18n ("Levels / Value labels for '%1'", var->getShortName ()));
}

EditLabelsDialog::~EditLabelsDialog () {
	RK_TRACE (EDITOR);
}

void EditLabelsDialog::accept () {
	RK_TRACE (EDITOR);

	table->stopEditing ();
	RObject::ValueLabels *labels = table->storage;
	if (labels->isEmpty ()) {
		var->setValueLabels (0);
	} else {
		var->setValueLabels (labels);
	}

	QDialog::accept ();
}

#include "editlabelsdialog.moc"
