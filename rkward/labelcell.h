/***************************************************************************
                          labelcell.h  -  description
                             -------------------
    begin                : Wed Nov 13 2002
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

#ifndef LABELCELL_H
#define LABELCELL_H

#include <rtableitem.h>

/**The type of RTableItem use in the "Label"-row of the meta(varview)-table. Not much interesing happens in the class, since there is no such things as an invalid label.
  *@author Thomas Friedrichsmeier
  */

class LabelCell : public RTableItem  {
public: 
	LabelCell(TwinTableMember *table);
	~LabelCell();
protected:
	QString rText ();
	void checkValid ();
};

#endif
