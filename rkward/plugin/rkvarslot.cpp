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
#include <qlayout.h>

#include <klocale.h>

#include "rkvarselector.h"
#include "rkplugin.h"
#include "../rkglobals.h"
#include "../core/rkvariable.h"

RKVarSlot::RKVarSlot(const QDomElement &element, QWidget *parent, RKPlugin *plugin) : RKPluginWidget (element, parent, plugin) {
	qDebug ("creating varselector");
	
	// layout
	QGridLayout *g_layout = new QGridLayout (this, 3, 3, RKGlobals::spacingHint ());

	select = new QPushButton ("-->", this);
	select->setFixedWidth (select->fontMetrics ().width (" --> "));
	connect (select, SIGNAL (clicked ()), this, SLOT (selectPressed ()));
	g_layout->addWidget (select, 1, 0);

	g_layout->addColSpacing (1, 5);

	label = new QLabel (element.attribute ("label", "Variable:"), this);
	g_layout->addWidget (label, 0, 2);

	multi = (element.attribute ("multi") == "true");
	
	if (!multi) {
		line_edit = new QLineEdit (this);
		line_edit->setReadOnly (true);
		g_layout->addWidget (line_edit, 1, 2);
		min_vars = 1;
	} else {
		list = new QListView (this);
		list->setSorting (100);
		list->setSelectionMode (QListView::Extended);
		list->addColumn ("Name");
		g_layout->addWidget (list, 1, 2);
		QString dummy = element.attribute ("min_vars", "1");
		min_vars = dummy.toInt ();
		connect (list, SIGNAL (selectionChanged ()), this, SLOT (listSelectionChanged ()));
	}

	// further infos
	source_id = element.attribute ("source");
	depend = element.attribute ("depend", "");
	required = (element.attribute ("required") == "true");
	num_vars = 0;
	selection = false;
	updateState ();
}

RKVarSlot::~RKVarSlot(){
}

void RKVarSlot::initialize () {
	source = plugin ()->getVarSelector (source_id);
	if (!source) return;
	connect (source, SIGNAL (changed ()), this, SLOT (objectListChanged ()));
}

void RKVarSlot::objectListChanged () {
	if (!source) return;
	if (!multi) {
		if (num_vars) {
			if (!source->containsObject (item_map[0])) {
				line_edit->setText ("");
				item_map.remove (0);
				num_vars = 0;
				select->setText ("-->");
			} else {
				line_edit->setText (item_map[0]->getShortName ());
			}
		}
	} else {

		QListViewItem *item = list->firstChild ();
		while (item) {
			if (source->containsObject (item_map[item])) {
				item->setText (0, item_map[item]->getShortName ());
				item = item->nextSibling ();
			} else {
				QListViewItem *dummy = item->nextSibling ();
				list->takeItem (item);
				item_map.remove (item);
				delete item;
				item = dummy;
				num_vars--;
			}
		}
	}

	updateState ();
// only after that signal the change to the plugin
	emit (changed ());
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
	if (!multi) {
		if (!num_vars) {
			if (!source) return;
			if (source->numSelectedVars() != 1) return;
			RKVariable *sel = source->selectedVars ().first ();
			line_edit->setText (sel->getShortName ());
			item_map.insert (0, sel);
			num_vars = 1;
			select->setText ("<--");
   	 } else {
			line_edit->setText ("");
			item_map.remove (0);
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
					item_map.remove (item);
					delete item;
					item = dummy;
				} else {
					item = item->nextSibling ();
				}
			}
			listSelectionChanged ();
		} else {
			if (!source) return;
			QValueList<RKVariable*> itemlist = source->selectedVars();
			for (QValueList<RKVariable*>::Iterator it = itemlist.begin (); it != itemlist.end (); ++it) {
				RKVariable* sel = *it;
				// don't allow duplicates
				bool duplicate = false;
				for (ItemMap::const_iterator iit = item_map.begin (); iit != item_map.end (); ++iit) {
					if (iit.data () == sel) {
						duplicate = true;
						break;
					}
				}
				if (!duplicate) {
					QListViewItem *new_item = new QListViewItem (list, sel->getShortName ());
					list->insertItem (new_item);
					item_map.insert (new_item, sel);
				}
			}
		}
		num_vars = list->childCount ();
	}
	
	updateState ();
	emit (changed ());
}

QValueList<RKVariable*> RKVarSlot::getVariables () {
	QValueList<RKVariable*> ret;
	for (ItemMap::iterator it = item_map.begin (); it != item_map.end (); ++it) {
		ret.append (it.data ());
	}
	return ret;
}

void RKVarSlot::updateState () {
	if (!isSatisfied ()) {
		if (multi) {
			list->setPaletteBackgroundColor (QColor (255, 0, 0));
		} else {
			line_edit->setPaletteBackgroundColor (QColor (255, 0, 0));
		}
	} else {
		if (multi) {
			list->setPaletteBackgroundColor (QColor (255, 255, 255));
		} else {
			line_edit->setPaletteBackgroundColor (QColor (255, 255, 255));
		}
	}
}

bool RKVarSlot::isSatisfied () {
	if (!required) return ((!num_vars) || (num_vars >= min_vars));
	return (num_vars >= min_vars);
}

QString RKVarSlot::value (const QString &modifier) {
	if (!multi) {
		if (num_vars) {
			if (modifier == "label") {
				return item_map[0]->getDescription ();
			} else if (modifier == "shortname") {
				return item_map[0]->getShortName ();
			} else {
				return (item_map[0]->getFullName ());
			}
		} else {
			return "";
		}
	} else {
		QString ret;

		QListViewItem *item = list->firstChild ();
		while (item) {
			if (modifier == "label") {
				ret.append (item_map[item]->getDescription () + "\n");
			} else if (modifier == "shortname") {
				ret.append (item_map[item]->getShortName () + "\n");
			} else {
				ret.append (item_map[item]->getFullName () + "\n");
			}
			item = item->nextSibling ();
		}
	
		return ret;
	}
}

QString RKVarSlot::complaints () {
	if (isSatisfied ()) return "";
	return i18n (" - You have to select a variable for the \"" + label->text () + "\"-field\n");
}

void RKVarSlot::setEnabled(bool checked){
  line_edit->setEnabled(checked);   
  select->setEnabled(checked);
    }

void RKVarSlot::slotActive(){
  bool isOk = line_edit->isEnabled();
  line_edit->setEnabled(! isOk) ;
  select->setEnabled(! isOk) ;
}

void RKVarSlot::slotActive(bool isOk){
  line_edit->setEnabled(isOk) ;
  select->setEnabled(isOk) ;
}

