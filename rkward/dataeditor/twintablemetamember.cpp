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
#include "twintablemetamember.h"

#include "../debug.h"

TwinTableMetaMember::TwinTableMetaMember (QWidget *parent, TwinTable *table) : TwinTableMember (parent, table, 0, 1) {
}

TwinTableMetaMember::~TwinTableMetaMember () {
}

void TwinTableMetaMember::removeRow (int) {
	RK_ASSERT (false);
}

void TwinTableMetaMember::insertRows (int, int) {
	RK_ASSERT (false);
}
