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
#include <qlistview.h>

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

	multi = (element.attribute ("multi") == "true");
	
	if (!multi) {
		line_edit = new QLineEdit (parent);
		line_edit->setReadOnly (true);
		g_layout->addWidget (line_edit, 1, 2);
		min_vars = 1;
	} else {
		list = new QListView (parent);
		list->addColumn ("Name");
		g_layout->addWidget (list, 1, 2);
		QString dummy = element.attribute ("min_vars", "1");
		min_vars = dummy.toInt ();
		connect (list, SIGNAL (selectionChanged ()), this, SLOT (listSelectionChanged ()));
	}

	addLayout (g_layout);

	// further infos
	source_id = element.attribute ("source");
	required = (element.attribute ("required") == "true");
	num_vars = 0;
	selection = false;
}

RKVarSlot::~RKVarSlot(){
}

void RKVarSlot::listSelectionChanged() {
	selection = false;

	QListViewItem *item = list->firstChild ();
	while (item) {
		if (item->isSelected ()) {
			selection = true;
			item = 0;	// end loop
		} else {
			item = item->nextSibling ();
		}
	}
	
	if (selection) {
		select->setText ("<--");
	} else {
		select->setText ("-->");
	}
}

void RKVarSlot::selectPressed () {
	RKVarSelector *source = plugin ()->getVarSelector (source_id);
	if (!multi) {
		if (!num_vars) {
			if (!source) return;
			if (source->numSelectedVars() != 1) return;
			line_edit->setText (source->selectedVars ().first ());
			num_vars = 1;
			select->setText ("<--");
   	 } else {
			line_edit->setText ("");
			num_vars = 0;
			select->setText ("-->");
		}
	} else {	// multi-slot
		if (selection) {
			QListViewItem *item = list->firstChild ();
			while (item) {
				if (item->isSelected ()) {
					selection = true;
					QListViewItem *dummy = item->nextSibling ();
					list->takeItem (item);
					delete item;
					item = dummy;
				} else {
					item = item->nextSibling ();
				}
			}
			selection = false;
		} else {
			if (!source) return;
			QStrList itemlist = source->selectedVars();
			for (QStrList::Iterator it = itemlist.begin (); it != itemlist.end (); ++it) {
				list->insertItem (new QListViewItem (list, *it));
			}
		}
		num_vars = list->childCount ();
	}
	
	plugin ()->changed ();
}

bool RKVarSlot::isSatisfied () {
	if (!required) return ((!num_vars) || (num_vars >= min_vars));
	return (num_vars >= min_vars);
}

QString RKVarSlot::value () {
	if (!multi) {
		return line_edit->text ();
	} else {
		QString ret;	
	
		QListViewItem *item = list->firstChild ();
		while (item) {
			ret.append (item->text (0) + "\n");
			item = item->nextSibling ();
		}
		
		return ret;
	}
}

QString RKVarSlot::complaints () {
	if (isSatisfied ()) return "";
	return i18n (" - You have to select a variable for the \"" + label->text () + "\"-field\n");
}