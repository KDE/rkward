/***************************************************************************
                          rkformula  -  description
                             -------------------
    begin                : Thu Aug 12 2004
    copyright            : (C) 2004, 2006 by Thomas Friedrichsmeier
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

#include <q3listview.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <q3buttongroup.h>
#include <qspinbox.h>
#include <qwidget.h>
#include <qlayout.h>
#include <qdom.h>
//Added by qt3to4:
#include <Q3HBoxLayout>
#include <Q3VBoxLayout>

#include <klocale.h>
#include <kiconloader.h>

#include "rkcomponent.h"
#include "../core/rcontainerobject.h"
#include "../misc/xmlhelper.h"
#include "../rkglobals.h"

#include "../debug.h"

RKFormula::RKFormula (const QDomElement &element, RKComponent *parent_component, QWidget *parent_widget) : RKComponent (parent_component, parent_widget) {
	RK_TRACE (PLUGIN);

	// create and register properties
	fixed_factors = new RKComponentPropertyRObjects (this, true);
	connect (fixed_factors, SIGNAL (valueChanged (RKComponentPropertyBase *)), this, SLOT (factorsChanged (RKComponentPropertyBase *)));
	addChild ("fixed_factors", fixed_factors);
	dependent = new RKComponentPropertyRObjects (this, true);
	connect (dependent, SIGNAL (valueChanged (RKComponentPropertyBase *)), this, SLOT (factorsChanged (RKComponentPropertyBase *)));
	addChild ("dependent", dependent);
	model = new RKComponentPropertyBase (this, true);
	addChild ("model", model);
	table = new RKComponentPropertyBase (this, true);
	addChild ("table", table);
	labels = new RKComponentPropertyBase (this, true);
	addChild ("labels", labels);

	// get xmlHelper
	XMLHelper *xml = XMLHelper::getStaticHelper ();

	// create layout
	Q3VBoxLayout *vbox = new Q3VBoxLayout (this, RKGlobals::spacingHint ());

	type_selector = new Q3ButtonGroup (this);
	type_selector->setColumnLayout (0, Qt::Vertical);
	type_selector->layout ()->setSpacing (RKGlobals::spacingHint ());
	type_selector->layout ()->setMargin (RKGlobals::marginHint ());
	Q3VBoxLayout *group_layout = new Q3VBoxLayout (type_selector->layout());
	group_layout->addWidget (new QRadioButton (i18n ("Full Model"), type_selector));
	group_layout->addWidget (new QRadioButton (i18n ("Main Effects only"), type_selector));
	group_layout->addWidget (new QRadioButton (i18n ("Custom Model:"), type_selector));
	connect (type_selector, SIGNAL (clicked (int)), this, SLOT (typeChange (int)));
	
	custom_model_widget = new QWidget (type_selector);
	Q3HBoxLayout *model_hbox = new Q3HBoxLayout (custom_model_widget, RKGlobals::spacingHint ());
	predictors_view = new Q3ListView (custom_model_widget);
	predictors_view->addColumn (i18n ("Name"));
	predictors_view->setSelectionMode (Q3ListView::Extended);
	predictors_view->setSorting (100);
	model_hbox->addWidget (predictors_view);
	model_hbox->addSpacing (6);
	
	Q3VBoxLayout *model_vbox = new Q3VBoxLayout (model_hbox, RKGlobals::spacingHint ());
	add_button = new QPushButton (QString::null, custom_model_widget);
	add_button->setPixmap (SmallIcon ("arrow-right"));
	connect (add_button, SIGNAL (clicked ()), this, SLOT (addButtonClicked ()));
	model_vbox->addWidget (add_button);
	remove_button = new QPushButton (QString::null, custom_model_widget);
	remove_button->setPixmap (SmallIcon ("arrow-left"));
	connect (remove_button, SIGNAL (clicked ()), this, SLOT (removeButtonClicked ()));
	model_vbox->addWidget (remove_button);
	level_box = new QSpinBox (0, 0, 1, custom_model_widget);
	level_box->setSpecialValueText (i18n ("Main effects"));
	model_vbox->addWidget (level_box);
	model_hbox->addSpacing (6);

	model_view = new Q3ListView (custom_model_widget);
	model_view->addColumn (i18n ("Level"));
	model_view->addColumn (i18n ("Term"));
	model_view->setSorting (0);
	model_view->setRootIsDecorated (true);
	model_hbox->addWidget (model_view);	

	group_layout->addWidget (custom_model_widget);

	type_selector->setCaption (xml->getStringAttribute (element, "label", i18n ("Specify model"), DL_INFO));

	vbox->addWidget (type_selector);
}

RKFormula::~RKFormula () {
	RK_TRACE (PLUGIN);
}

void RKFormula::factorsChanged (RKComponentPropertyBase *) {
	RK_TRACE (PLUGIN);
	// trigger update:
	typeChange ((int) model_type);
}

void RKFormula::typeChange (int id) {
	RK_TRACE (PLUGIN);
	type_selector->setButton (id);
	
	if (id == (int) FullModel) {
		custom_model_widget->setEnabled (false);
	} else if (id == (int) MainEffects) {
		custom_model_widget->setEnabled (false);
	} else if (id == (int) Custom) {
		predictors_view->clear ();
		item_map.clear ();
		RObject::ObjectList fixed_list = fixed_factors->objectList ();
		for (RObject::ObjectList::const_iterator it = fixed_list.begin (); it != fixed_list.end (); ++it) {
			Q3ListViewItem *new_item = new Q3ListViewItem (predictors_view, (*it)->getShortName ());
			item_map.insert (new_item, (*it));
		}
		checkCustomModel ();
		custom_model_widget->setEnabled (true);
	}
	
	model_type = (ModelType) id;
	makeModelString ();
}

void RKFormula::makeModelString () {
	RK_TRACE (PLUGIN);
	// first find out, whether multiple containers are involved and construct table string
	multitable = false;
	model_ok = false;
	QString table_string, model_string, labels_string;
	mangled_names.clear ();
	RObject *dep_var = dependent->objectValue ();
	RObject *container = 0;
	if (dep_var) {
		model_ok = true;
	}
	RObject::ObjectList vlist = fixed_factors->objectList ();
	if (vlist.empty ()) {
		model_ok = false;
	}
	if (dep_var) {
		container = dep_var->getContainer ();
	} else if (!vlist.empty ()) {
		container = vlist.first ()->getContainer ();
	}
	for (RObject::ObjectList::const_iterator it = vlist.begin (); it != vlist.end (); ++it) {
		if ((*it)->getContainer () != container) {
			multitable = true;
			break;
		}
	}
	if (multitable) {
		table_string = "data.frame (";
		if (dep_var) table_string.append (mangleName (dep_var) + '=' + dep_var->getFullName ());
		for (RObject::ObjectList::const_iterator it = vlist.begin (); it != vlist.end (); ++it) {
			table_string.append (", " + mangleName ((*it)) + '=' + (*it)->getFullName ());
		}
		table_string.append (")");
	} else {
		if (container) table_string = container->getFullName ();;
	}
	
	// construct model string
	model_string = mangleName (dep_var) + " ~ ";
	if (model_type == FullModel) {
		for (RObject::ObjectList::const_iterator it = vlist.begin (); it != vlist.end (); ++it) {
			if (it != vlist.begin ()) model_string.append (" * ");
			model_string.append (mangleName (*it));
		}
	} else if (model_type == MainEffects) {
		for (RObject::ObjectList::const_iterator it = vlist.begin (); it != vlist.end (); ++it) {
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

	// labels
	labels_string = "list (";
	MangledNames::const_iterator it;
	for (it = mangled_names.begin (); it != mangled_names.end (); ++it) {
		if (it != mangled_names.begin ()) {
			labels_string.append (", ");
		}
		labels_string.append (it.key () + "=\"" + it.data ()->getDescription () + "\"");
	}
	labels_string.append (")");

	table->setValue (table_string);
	model->setValue (model_string);
	labels->setValue (labels_string);
	changed ();
}

QString RKFormula::mangleName (RObject *var) {
	RK_TRACE (PLUGIN);
	if (!var) return QString ();
		
	QString dummy = var->getShortName ();
	QString dummy2 = dummy;
	MangledNames::iterator it;
	int i=-1;
	while (((it = mangled_names.find (dummy)) != mangled_names.end ()) && (it.data () != var)) {
		dummy = dummy2.append (QString ().setNum (++i));
	}
	mangled_names.insert (dummy, var);
	return dummy;
}

void RKFormula::addButtonClicked () {
	RK_TRACE (PLUGIN);
	// create an array of selected variables
	// we allocate more than we'll probably need, but it's only going to be a handful of vars anyway.
	RObject *varlist[predictors_view->childCount ()];
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
		Q3ListViewItem *item = new Q3ListViewItem (model_view, QString().setNum (level));
		level_map.insert (level, item);
	}
	Q3ListViewItem *parent = level_map[level];
	parent->setOpen (true);
	
	// check for duplicates (remove from old list - new terms might have a different order of naming)
	for (int inter = 0; inter < num_interactions; ++inter) {
		Interaction *new_inter = &(interactions[inter]);
		Q3ListViewItem *dupe = 0;
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
		Q3ListViewItem *item = new Q3ListViewItem (parent, QString::null, dummy);
		interaction_map.insert (item, interactions[i]);
	}
	
	makeModelString ();
}

RKFormula::Interaction* RKFormula::makeInteractions (int level, const RObjectPtr *source_vars, int source_count, int *count) {
	RK_TRACE (PLUGIN);
	RK_DO (qDebug ("makeInteractions: level %d, source_count %d", level, source_count), PLUGIN, DL_DEBUG);
	RK_ASSERT (level >= 0);

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
			ret[start_var].vars = new RObjectPtr[1];
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
			ret[current_interaction].vars = new RObjectPtr [sub_interactions[start_var][sub].level + 2];
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
	RK_TRACE (PLUGIN);
	Q3ListViewItem *current = model_view->firstChild ();
	while (current) {
		if (current->isSelected ()) {
			Q3ListViewItem *next = current->nextSibling ();
			if (current->parent ()) {	// single item
				InteractionMap::iterator it = interaction_map.find (current);
				delete it.data ().vars;
				delete it.key ();
				interaction_map.remove (it);
				next = current->parent ();
			} else {	// level item: remove all children
				for (Q3ListViewItem *child = current->firstChild (); child; ) {
					Q3ListViewItem *next_child = child->nextSibling ();
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
}

void RKFormula::checkCustomModel () {
	RK_TRACE (PLUGIN);
	int max_level = predictors_view->childCount () - 1;
	if (max_level >= 0) {
		level_box->setMaxValue (max_level);
	} else {
		level_box->setMaxValue (0);
	}

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
			Q3ListViewItem *parent = in.key ()->parent ();
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
	RK_TRACE (PLUGIN);
	return (model_ok);
}

#include "rkformula.moc"
