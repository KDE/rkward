/***************************************************************************
                          rkvarselector.cpp  -  description
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

#include "rkvarselector.h"

#include <qwidget.h>

RKVarSelector::RKVarSelector(QWidget *parent) : QListView (parent) {
	qDebug ("created varselector");
}

RKVarSelector::~RKVarSelector(){
}

QWidget *RKVarSelector::widget () const {
	return (QWidget *) this;
}