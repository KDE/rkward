/***************************************************************************
                          rtableitem.h  -  description
                             -------------------
    begin                : Mon Nov 4 2002
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

#ifndef RTABLEITEM_H
#define RTABLEITEM_H

#include <qtable.h>

#include "twintablemember.h"
#include "../core/robject.h"

class QPainter;
class QColorGroup;
class QRect;

/**
  *@author Thomas Friedrichsmeier
  */

class RTableItem : public QTableItem {
public: 
	RTableItem (TwinTableMember *table, QTableItem::EditType edit_type=OnTyping);
	~RTableItem();
/** Returns, whether this cell holds a value that is legal for it */
	bool isValid () { return valid; };
/** Returns the text in a format suitable for submission to R
	(Practically that means, to quote strings) */
	virtual QString rText ();
	virtual void checkValid ();
	RObject::VarType type ();
	TwinTableMember *ttm ();
	QWidget *createEditor () const;
protected:
friend class TwinTable;
	void paint (QPainter *p, const QColorGroup &cg, const QRect &cr, bool selected);
/** Stores, whether this cell holds a value that is legal for it */
	bool valid;
	void setText (const QString &str);
};

#endif
