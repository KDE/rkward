/*
rktextmatrix - This file is part of RKWard (https://rkward.kde.org). Created: Thu Nov 08 2007
SPDX-FileCopyrightText: 2007-2012 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "rktextmatrix.h"

#include <QApplication>
#include <QClipboard>
#include <QMimeData>

#include "../debug.h"

RKTextMatrix::RKTextMatrix() : colcount(0) {
	RK_TRACE(EDITOR);
}

RKTextMatrix::~RKTextMatrix() {
	RK_TRACE(EDITOR);
}

// static
RKTextMatrix RKTextMatrix::matrixFromClipboard() {
	RK_TRACE(EDITOR);

	const QMimeData *data = QApplication::clipboard()->mimeData();
	// actually, we don't care, whether tsv or plain gets pasted - it's both
	// treated the same. We should however encourage external senders to
	// provided the two in order.
	if (data->hasFormat(QStringLiteral("text/tab-separated-values"))) {
		RK_DEBUG(EDITOR, DL_DEBUG, "paste tsv");
		return (matrixFromSeparatedValues(QString::fromLocal8Bit(data->data(QStringLiteral("text/tab-separated-values")))));
	} else if (data->hasText()) {
		RK_DEBUG(EDITOR, DL_DEBUG, "paste plain text");
		return (matrixFromSeparatedValues(data->text()));
	}

	return RKTextMatrix();
}

// static
RKTextMatrix RKTextMatrix::matrixFromSeparatedValues(const QString &text, const QRegularExpression &tab, const QChar &brk) {
	RK_TRACE(EDITOR);

	RKTextMatrix ret;
	if (text.isEmpty()) return ret;

	QStringList textrows = text.split(brk);
	if (textrows.last().isEmpty()) textrows.removeLast(); // some apps append a trailing line break
	for (int i = 0; i < textrows.size(); ++i) {
		QStringList split = textrows[i].split(tab);
		ret.appendRow(split);
	}

	return ret;
}

QString RKTextMatrix::toTabSeparatedValues() const {
	RK_TRACE(EDITOR);

	QString ret;
	for (int row = 0; row < rows.size(); ++row) {
		if (row) ret.append(u'\n');
		ret.append(rows[row].join(u'\t'));
	}
	return ret;
}

void RKTextMatrix::copyToClipboard() const {
	RK_TRACE(EDITOR);

	QString text = toTabSeparatedValues();
	QMimeData *data = new QMimeData();
	data->setText(text);
	data->setData(QStringLiteral("text/tab-separated-values"), text.toLocal8Bit());
	QApplication::clipboard()->setMimeData(data);
}

RKTextMatrix RKTextMatrix::transformed(bool reverse_h, bool reverse_v, bool transpose) const {
	RK_TRACE(EDITOR);

	RKTextMatrix ret;
	if (isEmpty()) return ret; // empty matrices would violate some assumptions of the following code

	const int maxrow = rows.size() - 1; // for easier typing
	const int maxcol = rows[0].size() - 1;

	if (transpose) ret.upsize(maxcol, maxrow); // set dimensions from the start to save a few cycles
	else ret.upsize(maxrow, maxcol);

	for (int row = 0; row <= maxrow; ++row) {
		for (int col = 0; col <= maxcol; ++col) {
			int dest_row = row;
			if (reverse_v) dest_row = maxrow - row;
			int dest_col = col;
			if (reverse_h) dest_col = maxcol - col;

			if (transpose) {
				int dummy = dest_row;
				dest_row = dest_col;
				dest_col = dummy;
			}

			ret.setText(dest_row, dest_col, rows[row][col]);
		}
	}

	return ret;
}

void RKTextMatrix::setText(int row, int col, const QString &text) {
	//	RK_TRACE (EDITOR);

	upsize(row, col);
	rows[row][col] = text;
}

void RKTextMatrix::setColumn(int column, const QString *textarray, int length) {
	RK_TRACE(EDITOR);

	upsize(length - 1, column);
	for (int i = 0; i < length; ++i) {
		rows[i][column] = textarray[i];
	}
}

void RKTextMatrix::appendRow(const QStringList &row) {
	RK_TRACE(EDITOR);

	QStringList _row = row;
	while (colcount > _row.size())
		_row.append(QString());
	rows.append(_row);
	upsize(rows.size() - 1, row.size() - 1);
}

QString RKTextMatrix::getText(int row, int col) const {
	//	RK_TRACE (EDITOR);

	if (row > rows.size()) return QString();
	if (col > colcount) return QString();
	return (rows[row][col]);
}

QStringList RKTextMatrix::getColumn(int col) const {
	RK_TRACE(EDITOR);

	if (col > colcount) {
		return QStringList();
	}

	QStringList ret;
	ret.reserve(rows.size());
	for (int i = 0; i < rows.size(); ++i) {
		ret.append(rows[i][col]);
	}
	return ret;
}

QStringList RKTextMatrix::getRow(int row) const {
	RK_TRACE(EDITOR);

	if (row >= rows.size()) return (QStringList());
	RK_ASSERT(rows[row].size() == colcount);
	return (rows[row]);
}

void RKTextMatrix::clear() {
	RK_TRACE(EDITOR);

	rows.clear();
	colcount = 0;
}

bool RKTextMatrix::isEmpty() const {
	RK_TRACE(EDITOR);

	if (rows.isEmpty() || (colcount == 0)) return true;
	return false;
}

void RKTextMatrix::upsize(int newmaxrow, int newmaxcol) {
	//	RK_TRACE (EDITOR);

	while (newmaxrow >= rows.size()) {
		QStringList list;
		for (int i = 0; i < colcount; ++i)
			list.append(QString());
		rows.append(list);
	}

	if (newmaxcol >= colcount) {
		for (int i = 0; i < rows.size(); ++i) {
			while (newmaxcol >= rows[i].size())
				rows[i].append(QString());
		}
		colcount = newmaxcol + 1;
	}
}
