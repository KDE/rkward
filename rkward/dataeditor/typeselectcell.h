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

#include "rtableitem.h"

/**
  *@author Thomas Friedrichsmeier
  */

class TwinTableMember;

class TypeSelectCell : public RTableItem  {
public: 
	TypeSelectCell (TwinTableMember *table);
	~TypeSelectCell ();
	RObject::VarType type () { return _type; };
private:
	RObject::VarType _type;
protected:
	QWidget *createEditor () const;
	void setContentFromEditor (QWidget * w);
	QString text () const;
	QString rText ();
	void setText (const QString &str);
};

#endif
