/***************************************************************************
                          typeselectcell.h  -  description
                             -------------------
    begin                : Sun Nov 3 2002
    copyright            : (C) 2002 by Thomas Friedrichsmeier
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

#ifndef TYPESELECTCELL_H
#define TYPESELECTCELL_H

#include <qtable.h>
#include <qstring.h>

/**
  *@author Thomas Friedrichsmeier
  */

class QTable;
class QPainter;
class QColorGroup;
class QRect;

class TypeSelectCell : public QTableItem  {
public: 
	TypeSelectCell (QTable *table);
	~TypeSelectCell ();
	enum BaseType {Number=0, String=1, Date=2, Invalid=3};
private:
	BaseType type;
protected:
	QWidget *createEditor () const;
	void setContentFromEditor (QWidget * w);
	QString text () const;
	void setText (const QString &str);
	void TypeSelectCell::paint (QPainter *p, const QColorGroup &cg, const QRect &cr, bool selected);
};

#endif
