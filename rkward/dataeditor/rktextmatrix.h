/*
rktextmatrix - This file is part of the RKWard project. Created: Thu Nov 08 2007
SPDX-FileCopyrightText: 2007-2012 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef RKTEXTMATRIX_H
#define RKTEXTMATRIX_H

#include <QStringList>
#include <QRegularExpression>

/** This class is meant to assist paste operations in tables. Most importantly, it provides methods to map
to and from text/tab-separated-values format, and it does not hickup in case of ragged length data.

@author Thomas Friedrichsmeier */
class RKTextMatrix {
public:
	RKTextMatrix ();
	~RKTextMatrix ();

	static RKTextMatrix matrixFromClipboard ();
	static RKTextMatrix matrixFromSeparatedValues (const QString& text, const QRegularExpression &tab = QRegularExpression(QStringLiteral("\t")), const QChar& brk = QLatin1Char('\n'));

	QString toTabSeparatedValues () const;
	void copyToClipboard () const;

	void setText (int row, int col, const QString& text);
	/** set an entire column at once. This takes a copy of the data, so you will still have to delete it when done.
	TODO: convert to using QStringList */
	void setColumn (int column, const QString* textarray, int length);
	/** set an entire row at once. */
	void appendRow (const QStringList& row);

	QString getText (int row, int col) const;
	/** get the contents of an entire column at once. It's your responsibility to delete the data when done. The returned list has length numRows() */
	QStringList getColumn (int col) const;
	/** get the contents of an entire row at once */
	QStringList getRow (int row) const;

	/** Return a transformed matrix. Not optimized for performance!
	@param reverse_h Reverse order of columns
	@param reverse_v Reverse order of rows
	@param transpose Switch rows against columns */
	RKTextMatrix transformed (bool reverse_h, bool reverse_v, bool transpose) const;

	void clear ();
	bool isEmpty () const;

	int numColumns () const { return colcount; }
	int numRows () const { return rows.size (); }
private:
	QList<QStringList> rows;

	inline void upsize (int newmaxrow, int newmaxcol);

	int colcount;
};

#endif
