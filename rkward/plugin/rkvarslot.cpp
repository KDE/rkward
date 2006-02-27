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
#include <qstringlist.h>

#include <klocale.h>
#include <kiconloader.h>

#include "rkvarselector.h"
#include "rkplugin.h"
#include "../rkglobals.h"
#include "../debug.h"
#include "../core/rkvariable.h"
#include "../core/rcontainerobject.h"


RKVarSlot::RKVarSlot(const QDomElement &element, QWidget *parent, RKPlugin *plugin) : RKPluginWidget (element, parent, plugin) {
	RK_TRACE (PLUGIN);

	// layout
	QGridLayout *g_layout = new QGridLayout (this, 3, 3, RKGlobals::spacingHint ());

	
	select = new QPushButton (QString::null, this);
	select->setPixmap(SmallIcon("1rightarrow"));

	connect (select, SIGNAL (clicked ()), this, SLOT (selectPressed ()));
	g_layout->addWidget (select, 1, 0);

	g_layout->addColSpacing (1, 5);

	label = new QLabel (element.attribute ("label", "Variable:"), this);
	g_layout->addWidget (label, 0, 2);

	multi = (element.attribute ("multi") == "true");
	QString temp  = element.attribute ("duplicate","false") ; 
	if (temp == "true") {
		dupli = true;
	}else {
		dupli = false;
	}
	
	if (!multi) {
		line_edit = new QLineEdit (this);
		line_edit->setReadOnly (true);
		g_layout->addWidget (line_edit, 1, 2);
		min_vars = 1;
	} else {
		list = new QListView (this);
		list->setSelectionMode (QListView::Extended);
		list->addColumn (" ");
		list->addColumn ("Name");
		list->setSorting (2); 
		ordre = 1 ;
		g_layout->addWidget (list, 1, 2);
		QString dummy = element.attribute ("min_vars", "1");
		min_vars = dummy.toInt ();
		connect (list, SIGNAL (selectionChanged ()), this, SLOT (listSelectionChanged ()));
	}

	// further infos
	source_id = element.attribute ("source");
	depend = element.attribute ("depend", QString::null);
	classes = element.attribute ("classes", "all");
	if (classes=="frame") classes = "data.frame matrix list array";
	else if (classes=="number") classes = "numeric integer" ;
	else if (classes=="vector") classes ="numeric integer character factor" ; 
	required = (element.attribute ("required") == "true");
	num_vars = 0;
	selection = false;
	updateState ();
}

RKVarSlot::~RKVarSlot(){
	RK_TRACE (PLUGIN);
}

void RKVarSlot::initialize () {
	RK_TRACE (PLUGIN);
	source = plugin ()->getVarSelector (source_id);
	if (!source) return;
	connect (source, SIGNAL (changed ()), this, SLOT (objectListChanged ()));
}

// TODO make the same with cont_map 
void RKVarSlot::objectListChanged () {
	RK_TRACE (PLUGIN);

	if (!source) return;
	if (!multi) {
		if (num_vars) {
			if (!source->containsObject (item_map[0])) {
				line_edit->setText (QString::null);
				item_map.remove (0);
				num_vars = 0;
				select->setPixmap(SmallIcon("1rightarrow"));
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
	RK_TRACE (PLUGIN);

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
		select->setPixmap(SmallIcon("1leftarrow"));
	} else {
		select->setPixmap(SmallIcon("1rightarrow"));
	}
}

void RKVarSlot::selectPressed () {
	RK_TRACE (PLUGIN);

	if (!multi) {
		if (!num_vars) {
			if (!source) return;
			if (source->numSelectedVars() == 1) {
				RKVariable *sel = source->selectedVars ().first ();
				if (belongToClasses(sel->makeClassString (QString::null))){
					line_edit->setText (sel->getShortName ());
					item_map.insert (0, sel);
					num_vars = 1;
					select->setPixmap(SmallIcon("1leftarrow"));
					varOrCont = true ; 
				}
			}else if (source->numSelectedContainer() == 1 ){
				RContainerObject *sel = source->selectedContainer().first ();
				if (belongToClasses(sel->makeClassString(QString::null))){
					line_edit->setText (sel->getShortName ());
					cont_map.insert (0, sel);
					num_vars = 1;
					select->setPixmap(SmallIcon("1leftarrow"));
					varOrCont = false ; 
				}
			}
			else return ;
			
		} else {
			line_edit->setText (QString::null);
			item_map.remove (0);
			cont_map.remove (0);
			num_vars = 0;
			select->setPixmap(SmallIcon("1rightarrow"));
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
				if (dupli) duplicate = false ; 
				if (!duplicate && belongToClasses(sel->makeClassString(QString::null))) {
					QString string; 
        				string = string.setNum( ordre);  
					QListViewItem *new_item = new QListViewItem (list,string, sel->getShortName ());
					list->insertItem (new_item);
					item_map.insert (new_item, sel);
					ordre ++ ;
				}
			}
			QValueList<RContainerObject*> contlist = source->selectedContainer();
			for (QValueList<RContainerObject*>::Iterator et = contlist.begin (); et != contlist.end (); ++et) {
				RContainerObject* selcont = *et;
				// don't allow duplicates
				bool duplicate = false;
				for (ContMap::const_iterator eet = cont_map.begin (); eet != cont_map.end (); ++eet) {
					if (eet.data () == selcont) {
						duplicate = true;
						break;
					}
				}
				if (dupli) duplicate = false ; 
				if (!duplicate && belongToClasses(selcont->makeClassString(QString::null))) {
					QString string; 
        				string = string.setNum( ordre);  
					QListViewItem *new_item = new QListViewItem (list,string,selcont->getShortName ());
					list->insertItem (new_item);
					cont_map.insert (new_item, selcont);
					ordre ++ ;
				}
			}
		}
		num_vars = list->childCount ();
	}
	
	updateState ();
	emit (changed ());
}

QValueList<RKVariable*> RKVarSlot::getVariables () {
	RK_TRACE (PLUGIN);

	QValueList<RKVariable*> ret;
	for (ItemMap::iterator it = item_map.begin (); it != item_map.end (); ++it) {
		ret.append (it.data ());
	}
	return ret;
}

void RKVarSlot::updateState () {
	RK_TRACE (PLUGIN);

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
	RK_TRACE (PLUGIN);

	if (!required) return ((!num_vars) || (num_vars >= min_vars));
	return (num_vars >= min_vars);
}

QString RKVarSlot::value (const QString &modifier) {
	RK_TRACE (PLUGIN);

	QString ret;
	if (!multi) {
		if (num_vars) {
			if (modifier == "label") {
				if (varOrCont)	return item_map[0]->getDescription();
				else	return cont_map[0]->getDescription();
			} else if (modifier == "shortname") {
				if (varOrCont)	return item_map[0]->getShortName();
				else	return cont_map[0]->getShortName();
			} else {
				if (varOrCont)	return item_map[0]->getFullName();
				else	return cont_map[0]->getFullName();
			}
		} else {
			return QString::null;
		}
	} else {
		QListViewItem *item = list->firstChild ();
		while (item) {
			if (modifier == "label") {
				ItemMap::iterator it = item_map.find (item);
				if (it != item_map.end ())  ret.append (item_map[item]->getDescription() + "\n");
				ContMap::iterator et = cont_map.find (item);
				if (et != cont_map.end ())  ret.append (cont_map[item]->getDescription() + "\n");
			} else if (modifier == "shortname") {
				ItemMap::iterator it = item_map.find (item);
				if (it != item_map.end ())  ret.append (item_map[item]->getShortName () + "\n");
				ContMap::iterator et = cont_map.find (item);
				if (et != cont_map.end ())  ret.append (cont_map[item]->getShortName () + "\n");
			} else {
				ItemMap::iterator it = item_map.find (item);
				if (it != item_map.end ())  ret.append (item_map[item]->getFullName () + "\n");
				ContMap::iterator et = cont_map.find (item);
				if (et != cont_map.end ())  ret.append (cont_map[item]->getFullName () + "\n");
			}
			item = item->nextSibling ();
		}
	}

	//		qDebug ( "%s", ret.latin1() ) ;
	return ret;
}

QString RKVarSlot::complaints () {
	RK_TRACE (PLUGIN);

	if (isSatisfied ()) return QString::null;
	return (i18n (" - You have to select a variable for the \"%1\"-field\n").arg (label->text ()));
}

void RKVarSlot::setEnabled(bool checked){
	RK_TRACE (PLUGIN);
  line_edit->setEnabled(checked);   
  select->setEnabled(checked);
    }

void RKVarSlot::slotActive(){
	RK_TRACE (PLUGIN);
  bool isOk = line_edit->isEnabled();
  line_edit->setEnabled(! isOk) ;
  select->setEnabled(! isOk) ;
}

void RKVarSlot::slotActive(bool isOk){
	RK_TRACE (PLUGIN);
  line_edit->setEnabled(isOk) ;
  select->setEnabled(isOk) ;
}

bool RKVarSlot::belongToClasses(const QString &nom ) {
	RK_TRACE (PLUGIN);
	if (classes == "all") return true;
	if (classes.find (nom, 0) != -1) return true; 
	else return false;
}

#include "rkvarslot.moc"
