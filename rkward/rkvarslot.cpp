/***************************************************************************
                          rkvarslot.cpp  -  description
                             -------------------
    begin                : Thu Nov 7 2002
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

#include "rkvarslot.h"

int RKVarSlot::i = 0;

RKVarSlot::RKVarSlot(QWidget *parent) : QLineEdit (parent) {
	QString dummy;
	setText (dummy.setNum (i));
}
RKVarSlot::~RKVarSlot(){
}

QWidget *RKVarSlot::widget () const {
	return (QWidget *) this;
}