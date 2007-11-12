/***************************************************************************
                          rktextmatrix  -  description
                             -------------------
    begin                : Thu Nov 08 2007
    copyright            : (C) 2007 by Thomas Friedrichsmeier
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

#include "rktextmatrix.h"

#include <QApplication>
#include <QClipboard>
#include <QMimeData>

#include "../debug.h"

RKTextMatrix::RKTextMatrix () {
	RK_TRACE (EDITOR);

	clear ();
}

RKTextMatrix::RKTextMatrix (const RKTextMatrix& copy) {
	RK_TRACE (EDITOR);

	rowcount = copy.rowcount;
	colcount = copy.colcount;
	columns = copy.columns;
}

RKTextMatrix::~RKTextMatrix () {
	RK_TRACE (EDITOR);
}

// static
RKTextMatrix RKTextMatrix::matrixFromClipboard () {
	RK_TRACE (EDITOR);

	const QMimeData* data = QApplication::clipboard ()->mimeData ();
	// actually, we don't care, whether tsv or plain gets pasted - it's both
	// treated the same. We should however encourage external senders to
	// provided the two in order.
	if (data->hasFormat ("text/tab-separated-values")) {
		RK_DO (qDebug ("paste tsv"), EDITOR, DL_DEBUG);
		return (matrixFromTabSeparatedValues (QString::fromLocal8Bit (data->data ("text/tab-separated-values"))));
	} else if (data->hasText ()) {
		RK_DO (qDebug ("paste plain text"), EDITOR, DL_DEBUG);
		return (matrixFromTabSeparatedValues (data->text ()));
	}

	return RKTextMatrix ();
}

// static
RKTextMatrix RKTextMatrix::matrixFromTabSeparatedValues (const QString& tsv) {
	RK_TRACE (EDITOR);

	RKTextMatrix ret;
	if (tsv.isEmpty ()) return ret;

	QChar tab ('\t');
	QChar brk ('\n');

	int buffer_len = tsv.length ();
	int row = 0;
	int col = 0;

	QString current_word;
	for (int pos = 0; pos < buffer_len; ++pos) {
		QChar c = tsv.at (pos);
		if (c == tab) {
			ret.setText (row, col++, current_word);
			current_word.clear ();
		} else if (c == brk) {
			ret.setText (row++, col, current_word);
			col = 0;
			current_word.clear ();
		} else {
			current_word.append (c);
		}
	}
	ret.setText (row, col, current_word);

	return ret;
}

QString RKTextMatrix::toTabSeparatedValues () const {
	RK_TRACE (EDITOR);

	QString ret;
	RK_ASSERT (columns.size () == colcount);
	for (int row = 0; row < rowcount; ++row) {
		if (row) ret.append ('\n');

		for (int col = 0; col < colcount; ++col) {
			if (col) ret.append ('\t');
			if (!row) RK_ASSERT (columns[col].size () == rowcount);
			ret.append (columns[col][row]);
		}
	}
	return ret;
}

void RKTextMatrix::copyToClipboard () const {
	RK_TRACE (EDITOR);

	QString text = toTabSeparatedValues ();
	QMimeData* data = new QMimeData ();
	data->setText (text);
	data->setData ("text/tab-separated-values", text.toLocal8Bit ());
	QApplication::clipboard()->setMimeData (data);
}

void RKTextMatrix::setText (int row, int col, const QString& text) {
//	RK_TRACE (EDITOR);

	upsize (row, col);
	columns[col][row] = text;
}

void RKTextMatrix::setColumn (int column, const QString* textarray, int length) {
	RK_TRACE (EDITOR);

	upsize (length - 1, column);
	for (int i = 0; i < length; ++i) {
		columns[column][i] = textarray[i];
	}
}

QString RKTextMatrix::getText (int row, int col) const {
//	RK_TRACE (EDITOR);

	if ((row > rowcount) || (col > colcount)) return QString ();
	return (columns[col][row]);
}

QString* RKTextMatrix::getColumn (int col) const {
	RK_TRACE (EDITOR);

	if (col > colcount) {
		return 0;
	}

	TextColumn column = columns[col];
	QString* ret = new QString[column.size ()];
	for (int i = 0; i < column.size (); ++i) {
		ret[i] = column[i];
	}
	return ret;
}

void RKTextMatrix::clear () {
	RK_TRACE (EDITOR);

	columns.clear ();
	rowcount = colcount = 0;
}

bool RKTextMatrix::isEmpty () const {
	RK_TRACE (EDITOR);

	if ((rowcount == 0) || (colcount == 0)) return true;
	RK_ASSERT (!columns.isEmpty ());
	return false;
}

void RKTextMatrix::upsize (int newmaxrow, int newmaxcol) {
//	RK_TRACE (EDITOR);

	if (newmaxcol >= colcount) {
		for (; colcount <= newmaxcol; ++colcount) {
			TextColumn column;
			column.resize (rowcount);
			columns.append (column);
		}
		RK_ASSERT (colcount == columns.size ());
	}

	if (newmaxrow >= rowcount) {
		for (int i = 0; i < colcount; ++i) {
			columns[i].resize (newmaxrow + 1);
		}
		rowcount = newmaxrow + 1;
	}
}
