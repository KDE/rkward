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

#include "../debug.h"

LabelCell::LabelCell(TwinTableMember *table) : RTableItem (table) {
	RK_TRACE (EDITOR);
	valid = true;
}

LabelCell::~LabelCell(){
	RK_TRACE (EDITOR);
}

QString LabelCell::rText () {
	RK_TRACE (EDITOR);
	return (RObject::rQuote (text ()));
}

void LabelCell::checkValid () {
	RK_TRACE (EDITOR);
	// must stay, even if empty, in order to override RTableItem::checkValid ()
}
