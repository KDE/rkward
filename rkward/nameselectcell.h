/***************************************************************************
                          nameselectcell.h  -  description
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

#ifndef NAMESELECTCELL_H
#define NAMESELECTCELL_H

#include <rtableitem.h>

#include <qstring.h>

class TwinTableMember;

/**
  *@author Thomas Friedrichsmeier
  */

class NameSelectCell : public RTableItem  {
public: 
	NameSelectCell(TwinTableMember *table);
	~NameSelectCell();
	void init ();
protected:
	void setText (const QString &str);
	QString rText ();
};

#endif
