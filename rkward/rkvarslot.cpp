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

#include <qlineedit.h>
#include <qdom.h>
#include <qlabel.h>
#include <qpushbutton.h>


RKVarSlot::RKVarSlot(const QDomElement &element, QWidget *parent) : RKPluginWidget (element, parent) {
	qDebug ("creating varselector");

	QGridLayout *g_layout = new QGridLayout (3, 3, 6);

	select = new QPushButton ("-->", parent);
	select->setFixedWidth (select->fontMetrics ().width (" --> "));
	g_layout->addWidget (select, 1, 0);

	g_layout->addColSpacing (1, 5);

	label = new QLabel (element.attribute ("label", "Variable:"), parent);
	g_layout->addWidget (label, 0, 2);

	line_edit = new QLineEdit (parent);
	g_layout->addWidget (line_edit, 1, 2);

	addLayout (g_layout);
}

RKVarSlot::~RKVarSlot(){
}
