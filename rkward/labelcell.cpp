/***************************************************************************
                          labelcell.cpp  -  description
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

#include "labelcell.h"

#include <qregexp.h>


LabelCell::LabelCell(TwinTableMember *table) : RTableItem (table) {
	valid = true;
}

LabelCell::~LabelCell(){
}

QString LabelCell::rText () {
	return ("\"" + text ().replace (QRegExp ("\""), "\\\"") + "\"");
}

void LabelCell::checkValid () {
	// must stay, even if empty, in order to override RTableItem::checkValid ()
}
