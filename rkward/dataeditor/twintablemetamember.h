/***************************************************************************
                          twintablemetamember  -  description
                             -------------------
    begin                : Mon Sep 13 2004
    copyright            : (C) 2004 by Thomas Friedrichsmeier
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
#ifndef TWINTABLEMETAMEMBER_H
#define TWINTABLEMETAMEMBER_H

#include "twintablemember.h"

#include "../core/robject.h"

class QWidget;
class TwinTable;
#if 0
/**
The TwinTableMember responsible for storing the meta-information (i.e. the top half of the TwinTable). See TwinTable and TwinTableMember.

@author Thomas Friedrichsmeier
*/
class TwinTableMetaMember : public TwinTableMember {
Q_OBJECT
public:
	TwinTableMetaMember (QWidget *parent, TwinTable *table);

	~TwinTableMetaMember ();

/** reimplemented to raise an assert (should never be called) */
	void removeRow (int row);
/** reimplemented to raise an assert (should never be called) */
	void insertRows (int row, int count=1);
/** reimplemented form QTable not to work on TableColumns instead of QTableItems */
	void setText (int row, int col, const QString &text);
/** reimplemented form QTable not to work on TableColumns instead of QTableItems */
	void paintCell (QPainter *p, int row, int col, const QRect &cr, bool selected, const QColorGroup &cg);
/** reimplemented form QTable not to work on TableColumns instead of QTableItems */
	QWidget *beginEdit (int row, int col, bool replace);
/** reimplemented form QTable not to work on TableColumns instead of QTableItems */
	QString text (int row, int col) const;
	
	QString formattedText (int row, int col) const;
private:
	RObject::ValueLabels type_values;
};
#endif
#endif
