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

#include <klocale.h>

#include "rkvarselector.h"
#include "rkplugin.h"

RKVarSlot::RKVarSlot(const QDomElement &element, QWidget *parent, RKPlugin *plugin, QLayout *layout) : RKPluginWidget (element, parent, plugin, layout) {
	qDebug ("creating varselector");

	// layout
	QGridLayout *g_layout = new QGridLayout (3, 3, 6);

	select = new QPushButton ("-->", parent);
	select->setFixedWidth (select->fontMetrics ().width (" --> "));
	connect (select, SIGNAL (clicked ()), this, SLOT (selectPressed ()));
	g_layout->addWidget (select, 1, 0);

	g_layout->addColSpacing (1, 5);

	label = new QLabel (element.attribute ("label", "Variable:"), parent);
	g_layout->addWidget (label, 0, 2);

	line_edit = new QLineEdit (parent);
	line_edit->setReadOnly (true);
	g_layout->addWidget (line_edit, 1, 2);

	addLayout (g_layout);

	// further infos
	source_id = element.attribute ("source");
	required = (element.attribute ("required") == "true");
	filled = false;
}

RKVarSlot::~RKVarSlot(){
}

void RKVarSlot::selectPressed () {
	RKVarSelector *source = plugin ()->getVarSelector (source_id);
	if (!filled) {
		if (!source) return;
		if (source->numSelectedVars() != 1) return;
		line_edit->setText (source->selectedVars ().first ());
		filled = true;
		select->setText ("<--");
    } else {
		line_edit->setText ("");
		filled = false;
		select->setText ("-->");
	}
	
	plugin ()->changed ();
}

bool RKVarSlot::isSatisfied () {
	if (!required) return true;
	return filled;
}

QString RKVarSlot::value () {
	return line_edit->text ();
}

QString RKVarSlot::complaints () {
	if (isSatisfied ()) return "";
	return i18n (" - You have to select a variable for the \"" + label->text () + "\"-field\n");
}