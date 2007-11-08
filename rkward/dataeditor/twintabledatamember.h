/***************************************************************************
                          twintabledatamember  -  description
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
#ifndef TWINTABLEDATAMEMBER_H
#define TWINTABLEDATAMEMBER_H

#include "twintablemember.h"
#if 0
/**
The TwinTableMember responsible for storing the data (i.e. the bottom half of the TwinTable). See TwinTable and TwinTableMember.

@author Thomas Friedrichsmeier
*/
class TwinTableDataMember : public TwinTableMember {
Q_OBJECT
public:
	TwinTableDataMember (QWidget *parent, TwinTable *table);

	~TwinTableDataMember ();
	
	// TODO: obsolete?
	TwinTableMember *varTable () { return twin; };

/** reimplemented form QTable not to work on TableColumns instead of QTableItems */
	void removeRow (int row);
/** reimplemented form QTable not to work on TableColumns instead of QTableItems */
	void insertRows (int row, int count=1);
/** reimplemented form QTable not to work on TableColumns instead of QTableItems */
	void setText (int row, int col, const QString &text);
/** reimplemented form QTable not to work on TableColumns instead of QTableItems */
	void paintCell (QPainter *p, int row, int col, const QRect &cr, bool selected, const QColorGroup &cg);
/** reimplemented form QTable not to work on TableColumns instead of QTableItems */
	QWidget *beginEdit (int row, int col, bool replace);
/** reimplemented form QTable not to work on TableColumns instead of QTableItems */
	QString text (int row, int col) const;
/** reimplemented form TwinTableDataMember to use information from RKVariable for proper treatment of values */
	QString rText (int row, int col) const;
};
#endif
#endif
