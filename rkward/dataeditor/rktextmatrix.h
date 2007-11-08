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

#ifndef RKTEXTMATRIX_H
#define RKTEXTMATRIX_H

#include <QVector>
#include <QString>

/** This class is meant to assist paste operations in tables. Most importantly, it provides methods to map
to and from text/tab-separated-values format.

@author Thomas Friedrichsmeier */
class RKTextMatrix {
public:
	RKTextMatrix ();
/** copy constructor. Since we're mostly just copying a QVector (which is implicitely shared) and two ints, this is pretty fast. */
	RKTextMatrix (const RKTextMatrix& copy);
	~RKTextMatrix ();

	static RKTextMatrix matrixFromClipboard ();
	static RKTextMatrix matrixFromTabSeparatedValues (const QString& tsv);

	QString toTabSeparatedValues () const;
	void copyToClipboard () const;

	void setText (int row, int col, const QString& text);
	void setColumn (int column, const QString* textarray, int length);

	QString getText (int row, int col) const;
	QString* getColumn (int col, int* col_length) const;

	void clear ();
	bool isEmpty () const;
private:
	typedef QVector<QString> TextColumn;
	QVector<TextColumn> columns;

	inline void upsize (int newrowcount, int newcolcount);

	int colcount;
	int rowcount;
};

#endif
