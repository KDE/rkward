/***************************************************************************
                          rkformula  -  description
                             -------------------
    begin                : Thu Aug 12 2004
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
#include "rkformula.h"

#include <qlistview.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qbuttongroup.h>
#include <qspinbox.h>
#include <qwidget.h>
#include <qlayout.h>
#include <qdom.h>

#include <klocale.h>

#include "rkplugin.h"
#include "rkvarslot.h"
#include "rkvarselector.h"
#include "../core/rkvariable.h"

#include "../debug.h"

RKFormula::RKFormula (const QDomElement &element, QWidget *parent, RKPlugin *plugin, QLayout *layout) : RKPluginWidget (element, parent, plugin, layout) {
	type_selector = new QButtonGroup (parent);
	type_selector->setColumnLayout (0, Qt::Vertical);
	type_selector->layout()->setSpacing (6);
	type_selector->layout()->setMargin (11);
	QVBoxLayout *group_layout = new QVBoxLayout(type_selector->layout());
	group_layout->addWidget (new QRadioButton (i18n ("Full Model"), type_selector));
	group_layout->addWidget (new QRadioButton (i18n ("Main Effects only"), type_selector));
	group_layout->addWidget (new QRadioButton (i18n ("Custom Model:"), type_selector));
	connect (type_selector, SIGNAL (clicked (int)), this, SLOT (typeChange (int)));
	
	custom_model_widget = new QWidget (type_selector);
	QHBoxLayout *model_hbox = new QHBoxLayout (custom_model_widget);
	predictors_view = new QListView (custom_model_widget);
	predictors_view->addColumn ("Name");
	predictors_view->setSelectionMode (QListView::Extended);
	predictors_view->setSorting (100);
	model_hbox->addWidget (predictors_view);
	model_hbox->addSpacing (6);
	
	QVBoxLayout *model_vbox = new QVBoxLayout (custom_model_widget);
	add_button = new QPushButton ("->", custom_model_widget);
	connect (add_button, SIGNAL (clicked ()), this, SLOT (addButtonClicked ()));
	model_vbox->addWidget (add_button);
	remove_button = new QPushButton ("<-", custom_model_widget);
	connect (remove_button, SIGNAL (clicked ()), this, SLOT (removeButtonClicked ()));
	model_vbox->addWidget (remove_button);
	level_box = new QSpinBox (custom_model_widget);
	level_box->setSpecialValueText ("Main effects");
	model_vbox->addWidget (level_box);
	model_hbox->addLayout (model_vbox);
	model_hbox->addSpacing (6);

	model_view = new QListView (custom_model_widget);
	model_view->addColumn ("Level");
	model_view->addColumn ("Term");
	model_view->setSorting (0);
	model_view->setRootIsDecorated (true);
	model_hbox->addWidget (model_view);	

	group_layout->addWidget (custom_model_widget);
	
	addWidget (type_selector);
	
	fixed_factors_id = element.attribute ("fixed_factors");
	dependent_id = element.attribute ("dependent");
	type_selector->setCaption (element.attribute ("label", "Specify model"));
}


RKFormula::~RKFormula () {
}

void RKFormula::initialize () {
	fixed_factors = plugin ()->getVarSlot (fixed_factors_id);
	connect (fixed_factors, SIGNAL (changed ()), this, SLOT (factorsChanged ()));
	dependent = plugin ()->getVarSlot (dependent_id);
	connect (dependent, SIGNAL (changed ()), this, SLOT (factorsChanged ()));
	typeChange (0);
}

void RKFormula::factorsChanged () {
	// trigger update:
	typeChange ((int) model_type);
}

void RKFormula::typeChange (int id) {
	type_selector->setButton (id);
	
	if (id == (int) FullModel) {
		custom_model_widget->setEnabled (false);
	} else if (id == (int) MainEffects) {
		custom_model_widget->setEnabled (false);
	} else if (id == (int) Custom) {
		predictors_view->clear ();
		item_map.clear ();
		QValueList<RKVariable*> fixed_list = fixed_factors->getVariables ();
		for (QValueList<RKVariable*>::Iterator it = fixed_list.begin (); it != fixed_list.end (); ++it) {
			QListViewItem *new_item = new QListViewItem (predictors_view, (*it)->getShortName ());
			item_map.insert (new_item, (*it));
		}
		checkCustomModel ();
		custom_model_widget->setEnabled (true);
	}
	
	model_type = (ModelType) id;
	makeModelString ();
	emit (changed ());
}

void RKFormula::makeModelString () {
	// first find out, whether mulitple tables are involved and construct table string
	multitable = false;
	model_ok = false;
	mangled_names.clear ();
	RKVariable *dep_var = 0;
	QString table = "";
	if (dependent->getNumVars()) {
		dep_var = dependent->getVariables ().first ();
		model_ok = true;
	}
	QValueList<RKVariable*> vlist = fixed_factors->getVariables ();
	if (vlist.empty ()) {
		model_ok = false;
	}
	if (dep_var) {
		table = dep_var->getTable ();
	} else if (!vlist.empty ()) {
		table = vlist.first ()->getTable ();
	}
	for (QValueList<RKVariable*>::iterator it = vlist.begin (); it != vlist.end (); ++it) {
		if ((*it)->getTable () != table) {
			multitable = false;
			break;
		}
	}
	if (multitable) {
		table_string = "data.frame (";
		if (dep_var) table_string.append (mangleName (dep_var) + "=" + dep_var->getFullName ());
		for (QValueList<RKVariable*>::iterator it = vlist.begin (); it != vlist.end (); ++it) {
			table_string.append (", " + mangleName ((*it)) + "=" + (*it)->getFullName ());
		}
		table_string.append (")");
	} else {
		table_string = table;
	}
	
	// construct model string
	model_string = mangleName (dep_var) + " ~ ";
	if (model_type == FullModel) {
		for (QValueList<RKVariable*>::iterator it = vlist.begin (); it != vlist.end (); ++it) {
			if (it != vlist.begin ()) model_string.append (" * ");
			model_string.append (mangleName (*it));
		}
	} else if (model_type == MainEffects) {
		for (QValueList<RKVariable*>::iterator it = vlist.begin (); it != vlist.end (); ++it) {
			if (it != vlist.begin ()) model_string.append (" + ");
			model_string.append (mangleName (*it));
		}
	} else if (model_type == Custom) {	
		if (interaction_map.empty ()) model_ok = false;
		for (InteractionMap::Iterator it = interaction_map.begin (); it != interaction_map.end (); ++it) {
			if (it != interaction_map.begin ()) {
				model_string.append (" + ");
			}
			for (int i=0; i <= it.data ().level; ++i) {
				if (i) {
					model_string.append (":");
				}
				model_string.append (mangleName (it.data ().vars[i]));
			}
		}
	}
}

QString RKFormula::mangleName (RKVariable *var) {
	if (!var) return "";
		
	QString dummy = var->getShortName ();
	QString dummy2= dummy;
	MangledNames::iterator it;
	int i=-1;
	while (((it = mangled_names.find (dummy)) != mangled_names.end ()) && (it.data () != var)) {
		dummy = dummy2.append (QString ().setNum (++i));
	}
	mangled_names.insert (dummy, var);
	return dummy;
}

void RKFormula::addButtonClicked () {
	// create an array of selcted variables
	// we allocate more than we'll probably need, but it's only going to be a handful of vars anyway.
	RKVariable *varlist[predictors_view->childCount ()];
	int num_selected_vars = 0;
	for (ItemMap::iterator item = item_map.begin (); item != item_map.end (); ++item) {
		if (item.key ()->isSelected ()) {
			varlist[num_selected_vars++] = item.data ();
		}
	}
	if (!num_selected_vars) {
		return;
	}

	// TODO: allow looping from 0 to level (i.e. adding all interactions up to level)
	// construct interactions
	int level = level_box->value ();
	int num_interactions;
	Interaction *interactions = makeInteractions (level, varlist, num_selected_vars, &num_interactions);
	
	if (!num_interactions) return;
	
	// find an appropriate parent item
	if (level_map.find (level) == level_map.end ()) {
		QListViewItem *item = new QListViewItem (model_view, QString().setNum (level));
		level_map.insert (level, item);
	}
	QListViewItem *parent = level_map[level];
	parent->setOpen (true);
	
	// check for duplicates (remove from old list - new terms might have a different order of naming)
	for (int inter = 0; inter < num_interactions; ++inter) {
		Interaction *new_inter = &(interactions[inter]);
		QListViewItem *dupe = 0;
		for (InteractionMap::Iterator it = interaction_map.begin (); it != interaction_map.end (); ++it) {
			Interaction *existing_inter = &(it.data ());
			// BEGIN: actual comparison
			if (new_inter->level == existing_inter->level) {
				int num_matches = 0;
				for (int a=0; a <= new_inter->level; ++a) {
					for (int b=0; b <= existing_inter->level; ++b) {
						if (new_inter->vars[a] == existing_inter->vars[b]) ++num_matches;
					}
				}
				if (num_matches == (new_inter->level + 1)) {
					dupe = it.key ();
					break;
				}
			}
			// END: actual comparison
		}
		if (dupe) {
			delete [] interaction_map[dupe].vars;
			interaction_map.remove (dupe);
			delete dupe;
		}
	}
	
	// add new interactions
	for (int i = 0; i < num_interactions; ++i) {
		QString dummy;
		for (int j=0; j <= interactions[i].level; ++j) {
			RK_DO (qDebug ("inserting interaction %d, level %d", i, j), PLUGIN, DL_DEBUG);
			if (j) {
				dummy.append (" X ");
			}
			dummy.append (interactions[i].vars[j]->getShortName ());
		}
		QListViewItem *item = new QListViewItem (parent, "", dummy);
		interaction_map.insert (item, interactions[i]);
	}
	
	makeModelString ();
	emit (changed ());
}

RKFormula::Interaction* RKFormula::makeInteractions (int level, const RKVarPtr *source_vars, int source_count, int *count) {
	RK_DO (qDebug ("makeInteractions: level %d, source_count %d", level, source_count), PLUGIN, DL_DEBUG);
	
	int start_var;
	
	// enough vars available?
	if (source_count < (level + 1)) {
		*count = 0;
		return 0;
	}
	
	// reached bottom level?
	if (!level) {
		// return an array of level 0 interactions
		*count = source_count;
		Interaction *ret = new Interaction[source_count];
		for (start_var = 0; start_var < source_count; ++start_var) {
			RK_DO (qDebug ("start_var %d, source_count %d", start_var, source_count), PLUGIN, DL_DEBUG);
			ret[start_var].level = 0;
			ret[start_var].vars = new RKVarPtr[1];
			ret[start_var].vars[0] = source_vars[start_var];
		}
		return ret;
	}

	// first get all sub-interactions on the lower levels
	Interaction **sub_interactions = new Interaction* [source_count];
	int sub_counts[source_count];
	int sub_total = 0;
	for (start_var = 0; start_var < source_count; ++start_var) {
		sub_interactions[start_var] = makeInteractions (level - 1, &(source_vars[start_var+1]), source_count - start_var - 1, &sub_counts[start_var]);
		sub_total += sub_counts[start_var];
	}
	
	// now cross the lower level interactions with the current level stuff
	int current_interaction = 0;
	Interaction *ret = new Interaction[(source_count-level) * sub_total];
	for (start_var = 0; start_var < (source_count - level); ++start_var) {
		for (int sub = 0; sub < sub_counts[start_var]; ++sub) {
			// copy values
			ret[current_interaction].vars = new RKVarPtr [sub_interactions[start_var][sub].level + 2];
			ret[current_interaction].vars[0] = source_vars[start_var];
			for (int i=1; i <= (sub_interactions[start_var][sub].level + 1); ++i) {
				ret[current_interaction].vars[i] = sub_interactions[start_var][sub].vars[i-1];
			}
			ret[current_interaction].level = sub_interactions[start_var][sub].level + 1;
			current_interaction++;
			// delete sub-interaction
			delete sub_interactions[start_var][sub].vars;
		}
		// delete interaction arrays;
		delete sub_interactions[start_var];
	}
	delete [] sub_interactions;
	
	*count = current_interaction;
	return ret;
}

void RKFormula::removeButtonClicked () {
	QListViewItem *current = model_view->firstChild ();
	while (current) {
		if (current->isSelected ()) {
			QListViewItem *next = current->nextSibling ();
			if (current->parent ()) {	// single item
				InteractionMap::iterator it = interaction_map.find (current);
				delete it.data ().vars;
				delete it.key ();
				interaction_map.remove (it);
				next = current->parent ();
			} else {	// level item: remove all children
				for (QListViewItem *child = current->firstChild (); child; ) {
					QListViewItem *next_child = child->nextSibling ();
					InteractionMap::iterator it = interaction_map.find (child);
					delete it.data ().vars;
					delete it.key ();
					interaction_map.remove (it);
					child = next_child;
				}
			}
			current = next;
		// else: walk tree
		} else if (current->firstChild ()) {
			current = current->firstChild ();
		} else if (current->nextSibling ()) {
			current = current->nextSibling ();
		} else if (current->parent ()) {
			current = current->parent ()->nextSibling ();
		} else {
			current = 0;
		}
	}
	
	// check whether some levels can be cleaned up
	for (int i=0; i < predictors_view->childCount (); ++i) {
		LevelMap::iterator lit;
		if ((lit = level_map.find (i)) != level_map.end ()) {
			if (!(lit.data ()->firstChild ())) {
				delete lit.data ();
				level_map.remove (lit);
			}
		}
	}
	
	makeModelString ();
	emit (changed ());
}

void RKFormula::checkCustomModel () {
	level_box->setMaxValue (predictors_view->childCount () - 1);
	
	// clear terms which are no longer valid
	for (InteractionMap::iterator in = interaction_map.begin (); in != interaction_map.end (); ++in) {
		Interaction inter = in.data ();
		int found_vars = 0;
		for (int i=0; i <= inter.level; ++i) {
			for (ItemMap::iterator item = item_map.begin (); item != item_map.end (); ++item) {
				RK_DO (qDebug ("level %d", i), PLUGIN, DL_DEBUG);
				if (item.data () == inter.vars[i]) {
					++found_vars;
					break;
				}
			}
		}
		if (found_vars < (inter.level + 1)) {
			delete [] (in.data ().vars);
			QListViewItem *parent = in.key ()->parent ();
			delete in.key ();
			if (!parent->firstChild ()) {
				delete parent;
				level_map.remove (inter.level);
			}
			interaction_map.remove (in);
		}
	}
}

bool RKFormula::isSatisfied () {
	return (model_ok);
}

QString RKFormula::value (const QString &modifier) {
	if (modifier == "data") {
		return table_string;
	} else if (modifier == "labels") {
		QString ret = "list (";
		MangledNames::iterator it;
		for (it = mangled_names.begin (); it != mangled_names.end (); ++it) {
			if (it != mangled_names.begin ()) {
				ret.append (", ");
			}
			ret.append (it.key () + "=\"" + it.data ()->getDescription () + "\"");
		}
		ret.append (")");
		return ret;
	} else {
		return model_string;
	}
}

#include "rkformula.moc"
