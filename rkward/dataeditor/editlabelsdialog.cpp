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
#include <kdialogbase.h>
#include <kaction.h>

#include <qapplication.h>
#include <qclipboard.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qpainter.h>
#include <qrect.h>
#include <qpalette.h>
#include <qstyle.h>

#include "../core/rkvariable.h"
#include "rkdrag.h"
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
	setHScrollBarMode (QScrollView::AlwaysOff);
	setLeftMargin (40);
	setMinimumWidth (80);

	KActionCollection *ac = new KActionCollection (this);
	KStdAction::cut (this, SLOT (cut ()), ac);
	KStdAction::copy (this, SLOT (copy ()), ac);
	KStdAction::paste (this, SLOT (paste ()), ac);
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

	QApplication::clipboard()->setData (new RKDrag (this));
}

void LevelsTable::paste () {
	RK_TRACE (EDITOR);

// Unfortunately, we need to duplicate some of TwinTable::paste () and RKEditorDataFramPart::doPaste. Those are not easy to reconcile.

	// actually, we don't care, whether tsv or plain gets pasted - it's both
	// treated the same. We should however encourage external senders to
	// provided the two in order.
	QString pasted;
	if (QApplication::clipboard()->data()->provides ("text/tab-separated-values")) {
		pasted = QString (QApplication::clipboard ()->data ()->encodedData ("text/tab-separated-values"));
	} else if (QApplication::clipboard()->data()->provides ("text/plain")) {
		pasted = QString (QApplication::clipboard ()->data ()->encodedData ("text/plain"));
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

QString LevelsTable::text (int row, int col) const {
	RK_TRACE (EDITOR);
	RK_ASSERT (col == 0);
	RK_ASSERT (row < numTrueRows ());

	return ((*storage)[QString::number (row+1)]);
}

void LevelsTable::paintCell (QPainter *p, int row, int col, const QRect &cr, bool selected, const QColorGroup &cg) {
	// no trace for paint operations

	// draw background
	QBrush brush = QBrush (Qt::red);
	if (selected) {
		brush = cg.brush(QColorGroup::Highlight);
	} else {
			brush = cg.brush (QColorGroup::Base);
	}
	if (row >= numTrueRows ()) {
		brush = QBrush (Qt::gray);
	}

	p->fillRect(0, 0, cr.width(), cr.height(), brush);

	// draw grid
	QPen pen (p->pen ());
	int gridColor = style ().styleHint (QStyle::SH_Table_GridLineColor, this);
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
	
	// draw text
	if (selected) {
		p->setPen (cg.highlightedText());
	} else {
		p->setPen (cg.text ());
	}

	p->drawText (2, 0, cr.width () - 4, cr.height (), Qt::AlignLeft, (*storage)[QString::number (row+1)]);
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

	QTable::resizeEvent (e);
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

	QTable::columnWidthChanged (col);
}



EditLabelsDialog::EditLabelsDialog (QWidget *parent, RKVariable *var, int mode) : QDialog (parent) {
	RK_TRACE (EDITOR);
	RK_ASSERT (var);
	RK_ASSERT (var->objectOpened ());
	
	EditLabelsDialog::var = var;
	EditLabelsDialog::mode = mode;

	QVBoxLayout *mainvbox = new QVBoxLayout (this, KDialog::marginHint (), KDialog::spacingHint ());
	QLabel *label = new QLabel (i18n ("Levels can be assigned only to consecutive integers starting with 1 (the index column is read only). To remove levels at the end of the list, just set them to empty."), this);
	label->setAlignment (Qt::AlignAuto | Qt::AlignVCenter | Qt::ExpandTabs | Qt::WordBreak);
	mainvbox->addWidget (label);

	QHBoxLayout *hbox = new QHBoxLayout (mainvbox, KDialog::spacingHint ());

	RObject::ValueLabels *labels = var->getValueLabels ();
	if (!labels) {
		labels = new RObject::ValueLabels;
	}

	table = new LevelsTable (this, labels);
	hbox->addWidget (table);

	QHBoxLayout *buttonbox = new QHBoxLayout (mainvbox, KDialog::spacingHint ());

	QPushButton *ok_button = new QPushButton (i18n ("Ok"), this);
	connect (ok_button, SIGNAL (clicked ()), this, SLOT (accept ()));
	buttonbox->addWidget (ok_button);

	QPushButton *cancel_button = new QPushButton (i18n ("Cancel"), this);
	connect (cancel_button, SIGNAL (clicked ()), this, SLOT (reject ()));
	buttonbox->addWidget (cancel_button);
	
	setCaption (i18n ("Levels / Value labels for '%1'").arg (var->getShortName ()));
}

EditLabelsDialog::~EditLabelsDialog () {
	RK_TRACE (EDITOR);
}

void EditLabelsDialog::accept () {
	RK_TRACE (EDITOR);

	RObject::ValueLabels *labels = table->storage;
	if (labels->isEmpty ()) {
		var->setValueLabels (0);
	} else {
		var->setValueLabels (labels);
	}

	QDialog::accept ();
}

#include "editlabelsdialog.moc"
