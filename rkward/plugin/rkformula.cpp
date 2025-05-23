/*
rkformula - This file is part of RKWard (https://rkward.kde.org). Created: Thu Aug 12 2004
SPDX-FileCopyrightText: 2004-2014 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "rkformula.h"

#include <QButtonGroup>
#include <QLabel>
#include <QTreeWidget>
#include <qdom.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qspinbox.h>
#include <qwidget.h>

#include <KLocalizedString>

#include "../core/rcontainerobject.h"
#include "../misc/rkstandardicons.h"
#include "../misc/xmlhelper.h"
#include "rkcomponent.h"

#include "../debug.h"

RKFormula::RKFormula(const QDomElement &element, RKComponent *parent_component, QWidget *parent_widget) : RKComponent(parent_component, parent_widget) {
	RK_TRACE(PLUGIN);

	// create and register properties
	fixed_factors = new RKComponentPropertyRObjects(this, false);
	connect(fixed_factors, &RKComponentPropertyBase::valueChanged, this, &RKFormula::factorsChanged);
	addChild(QStringLiteral("fixed_factors"), fixed_factors);
	fixed_factors->setInternal(true);
	dependent = new RKComponentPropertyRObjects(this, false);
	connect(dependent, &RKComponentPropertyBase::valueChanged, this, &RKFormula::factorsChanged);
	addChild(QStringLiteral("dependent"), dependent);
	dependent->setInternal(true);
	model = new RKComponentPropertyBase(this, true);
	addChild(QStringLiteral("model"), model);
	model->setInternal(true);
	table = new RKComponentPropertyBase(this, true);
	addChild(QStringLiteral("table"), table);
	table->setInternal(true);
	labels = new RKComponentPropertyBase(this, true);
	addChild(QStringLiteral("labels"), labels);
	labels->setInternal(true);

	// get xmlHelper
	XMLHelper *xml = parent_component->xmlHelper();

	// create layout
	QVBoxLayout *vbox = new QVBoxLayout(this);
	vbox->setContentsMargins(0, 0, 0, 0);

	label_string = xml->i18nStringAttribute(element, QStringLiteral("label"), i18n("Specify model"), DL_INFO);
	vbox->addWidget(new QLabel(label_string, this));

	type_selector = new QButtonGroup(this);
	QRadioButton *button;
	vbox->addWidget(button = new QRadioButton(i18n("Full Model"), this));
	type_selector->addButton(button, (int)FullModel);
	vbox->addWidget(button = new QRadioButton(i18n("Main Effects only"), this));
	type_selector->addButton(button, (int)MainEffects);
	vbox->addWidget(button = new QRadioButton(i18n("Custom Model:"), this));
	type_selector->addButton(button, (int)Custom);
	connect(type_selector, &QButtonGroup::idClicked, this, &RKFormula::typeChange);

	custom_model_widget = new QWidget(this);
	QHBoxLayout *model_hbox = new QHBoxLayout(custom_model_widget);
	predictors_view = new QTreeWidget(custom_model_widget);
	predictors_view->setHeaderLabel(i18n("Name"));
	predictors_view->setSelectionMode(QAbstractItemView::ExtendedSelection);
	predictors_view->setSortingEnabled(false);
	predictors_view->setRootIsDecorated(false);
	model_hbox->addWidget(predictors_view);
	model_hbox->addSpacing(6);

	QVBoxLayout *model_vbox = new QVBoxLayout();
	model_hbox->addLayout(model_vbox);
	add_button = new QPushButton(QString(), custom_model_widget);
	add_button->setIcon(RKStandardIcons::getIcon(RKStandardIcons::ActionAddRight));
	connect(add_button, &QPushButton::clicked, this, &RKFormula::addButtonClicked);
	model_vbox->addWidget(add_button);
	remove_button = new QPushButton(QString(), custom_model_widget);
	remove_button->setIcon(RKStandardIcons::getIcon(RKStandardIcons::ActionRemoveLeft));
	connect(remove_button, &QPushButton::clicked, this, &RKFormula::removeButtonClicked);
	model_vbox->addWidget(remove_button);
	level_box = new QSpinBox(custom_model_widget);
	level_box->setRange(0, 0);
	level_box->setSpecialValueText(i18n("Main effects"));
	model_vbox->addWidget(level_box);
	model_hbox->addSpacing(6);

	model_view = new QTreeWidget(custom_model_widget);
	model_view->setHeaderLabels(QStringList() << i18n("Level") << i18n("Term"));
	model_view->setRootIsDecorated(true);
	model_hbox->addWidget(model_view);

	vbox->addWidget(custom_model_widget);

	typeChange(FullModel); // initialize
}

RKFormula::~RKFormula() {
	RK_TRACE(PLUGIN);
}

void RKFormula::factorsChanged(RKComponentPropertyBase *) {
	RK_TRACE(PLUGIN);
	// trigger update:
	typeChange((int)model_type);
}

void RKFormula::typeChange(int id) {
	RK_TRACE(PLUGIN);

	QAbstractButton *b = type_selector->button(id);
	RK_ASSERT(b);
	b->setChecked(true);

	if (id == (int)FullModel) {
		custom_model_widget->setEnabled(false);
	} else if (id == (int)MainEffects) {
		custom_model_widget->setEnabled(false);
	} else if (id == (int)Custom) {
		predictors_view->clear();
		predictors_map.clear();
		RObject::ObjectList fixed_list = fixed_factors->objectList();
		for (int i = 0; i < fixed_list.count(); ++i) {
			QTreeWidgetItem *new_item = new QTreeWidgetItem(predictors_view);
			new_item->setText(0, fixed_list[i]->getShortName());
			predictors_map.insert(new_item, (fixed_list[i]));
		}
		checkCustomModel();
		custom_model_widget->setEnabled(true);
	}

	model_type = (ModelType)id;
	makeModelString();
}

void RKFormula::makeModelString() {
	RK_TRACE(PLUGIN);
	// first find out, whether multiple containers are involved and construct table string
	multitable = false;
	model_ok = false;
	QString table_string, model_string, labels_string;
	mangled_names.clear();
	RObject *dep_var = dependent->objectValue();
	RObject *container = nullptr;
	if (dep_var) {
		model_ok = true;
	}
	RObject::ObjectList vlist = fixed_factors->objectList();
	if (vlist.empty()) {
		model_ok = false;
	}
	if (dep_var) {
		container = dep_var->parentObject();
	} else if (!vlist.empty()) {
		container = vlist.first()->parentObject();
	}
	for (RObject::ObjectList::const_iterator it = vlist.cbegin(); it != vlist.cend(); ++it) {
		if ((*it)->parentObject() != container) {
			multitable = true;
			break;
		}
	}
	if (multitable) {
		table_string = u"data.frame ("_s;
		if (dep_var) table_string.append(mangleName(dep_var) + u'=' + dep_var->getFullName());
		for (RObject::ObjectList::const_iterator it = vlist.cbegin(); it != vlist.cend(); ++it) {
			table_string.append(u", "_s + mangleName((*it)) + u'=' + (*it)->getFullName());
		}
		table_string.append(u")"_s);
	} else {
		if (container) table_string = container->getFullName();
	}

	// construct model string
	model_string = mangleName(dep_var) + u" ~ "_s;
	if (model_type == FullModel) {
		for (RObject::ObjectList::const_iterator it = vlist.cbegin(); it != vlist.cend(); ++it) {
			if (it != vlist.cbegin()) model_string.append(u" * "_s);
			model_string.append(mangleName(*it));
		}
	} else if (model_type == MainEffects) {
		for (RObject::ObjectList::const_iterator it = vlist.cbegin(); it != vlist.cend(); ++it) {
			if (it != vlist.cbegin()) model_string.append(u" + "_s);
			model_string.append(mangleName(*it));
		}
	} else if (model_type == Custom) {
		if (interaction_map.empty()) model_ok = false;
		for (InteractionMap::Iterator it = interaction_map.begin(); it != interaction_map.end(); ++it) {
			if (it != interaction_map.begin()) {
				model_string.append(u" + "_s);
			}
			for (int i = 0; i <= it.value().level; ++i) {
				if (i) {
					model_string.append(u":"_s);
				}
				model_string.append(mangleName(it.value().vars[i]));
			}
		}
	}

	// labels
	labels_string = u"list ("_s;
	MangledNames::const_iterator it;
	for (it = mangled_names.cbegin(); it != mangled_names.cend(); ++it) {
		if (it != mangled_names.cbegin()) {
			labels_string.append(u", "_s);
		}
		labels_string.append(it.key() + u"=\""_s + it.value()->getDescription() + u'"');
	}
	labels_string.append(u')');

	table->setValue(table_string);
	model->setValue(model_string);
	labels->setValue(labels_string);
	changed();
}

QString RKFormula::mangleName(RObject *var) {
	RK_TRACE(PLUGIN);
	if (!var) return QString();

	QString dummy = var->getShortName();
	QString dummy2 = dummy;
	MangledNames::iterator it;
	int i = -1;
	while (((it = mangled_names.find(dummy)) != mangled_names.end()) && (it.value() != var)) {
		dummy = dummy2.append(QString().setNum(++i));
	}
	mangled_names.insert(dummy, var);
	return dummy;
}

void RKFormula::addButtonClicked() {
	RK_TRACE(PLUGIN);

	// create an array of selected variables
	RObject::ObjectList varlist;
	QList<QTreeWidgetItem *> selected_predictors = predictors_view->selectedItems();
	for (int i = 0; i < selected_predictors.count(); ++i) {
		varlist.append(predictors_map.value(selected_predictors[i]));
	}
	if (varlist.isEmpty()) return;

	// TODO: allow looping from 0 to level (i.e. adding all interactions up to level)
	// construct interactions
	int level = level_box->value();
	QList<Interaction> interactions = makeInteractions(level, varlist);

	if (interactions.isEmpty()) return;

	// check for duplicates (remove from old list - new terms might have a different order of naming)
	for (int inter = 0; inter < interactions.count(); ++inter) {
		Interaction new_inter = interactions[inter];
		QTreeWidgetItem *dupe = nullptr;
		for (InteractionMap::Iterator it = interaction_map.begin(); it != interaction_map.end(); ++it) {
			Interaction existing_inter = it.value();
			// BEGIN: actual comparison
			if (new_inter.level == existing_inter.level) {
				int num_matches = 0;
				for (int a = 0; a <= new_inter.level; ++a) {
					if (existing_inter.vars.contains(new_inter.vars[a])) ++num_matches;
				}
				if (num_matches == (new_inter.level + 1)) {
					dupe = it.key();
					break;
				}
			}
			// END: actual comparison
		}
		if (dupe) {
			interaction_map.remove(dupe);
			delete dupe;
		}
	}

	// add new interactions
	for (int i = 0; i < interactions.count(); ++i) {
		QString dummy;
		for (int j = 0; j <= interactions[i].level; ++j) {
			RK_DEBUG(PLUGIN, DL_DEBUG, "inserting interaction %d, level %d", i, j);
			if (j) {
				dummy.append(u" X "_s);
			}
			dummy.append(interactions[i].vars[j]->getShortName());
		}
		QTreeWidgetItem *item = new QTreeWidgetItem(model_view);
		item->setText(0, QString::number(level));
		item->setText(1, dummy);
		interaction_map.insert(item, interactions[i]);
	}

	model_view->sortItems(0, Qt::AscendingOrder);

	makeModelString();
}

QList<RKFormula::Interaction> RKFormula::makeInteractions(int level, RObject::ObjectList source_vars) {
	RK_TRACE(PLUGIN);
	RK_DEBUG(PLUGIN, DL_DEBUG, "makeInteractions: level %d, source_count %d", level, source_vars.count());
	RK_ASSERT(level >= 0);

	QList<Interaction> ret;

	int start_var;

	// enough vars available for this level of crossing?
	if (source_vars.count() < (level + 1)) return ret;

	// reached bottom level?
	if (!level) {
		// return an list of level 0 interactions (i.e. each var in a single "interaction")
		for (start_var = 0; start_var < source_vars.count(); ++start_var) {
			Interaction inter;
			inter.level = 0;
			inter.vars.append(source_vars[start_var]);
			ret.append(inter);
		}
		return ret;
	}

	// cross each input variable with all interactions of the further variables at the next lower level
	RObject::ObjectList sub_vars = source_vars;
	for (int i = 0; i < source_vars.count(); ++i) {
		// get the next lower level interactions
		sub_vars.removeFirst(); // not to be included in the sub-interactions
		QList<Interaction> sub_interactions = makeInteractions(level - 1, sub_vars);

		// now cross each with the current var
		for (int sub = 0; sub < sub_interactions.count(); ++sub) {
			Interaction inter;
			inter.level = level; // well, actually level should always be count of vars - 1, so redundant...
			inter.vars = sub_interactions[sub].vars;
			inter.vars.insert(0, source_vars[i]);
			ret.append(inter);
		}
	}

	return ret;
}

void RKFormula::removeButtonClicked() {
	RK_TRACE(PLUGIN);

	QList<QTreeWidgetItem *> selected = model_view->selectedItems();
	if (selected.isEmpty()) return;

	for (int i = 0; i < selected.count(); ++i) {
		QTreeWidgetItem *current = selected[i];

		interaction_map.remove(current);
		delete current;
	}

	makeModelString();
}

void RKFormula::checkCustomModel() {
	RK_TRACE(PLUGIN);
	int max_level = predictors_view->topLevelItemCount() - 1;
	if (max_level >= 0) {
		level_box->setMaximum(max_level);
	} else {
		level_box->setMaximum(0);
	}

	InteractionMap new_map = interaction_map;
	// clear terms which are no longer valid
	for (InteractionMap::iterator in = interaction_map.begin(); in != interaction_map.end(); ++in) {
		Interaction inter = in.value();
		int found_vars = 0;
		for (int i = 0; i <= inter.level; ++i) {
			for (ItemMap::const_iterator item = predictors_map.constBegin(); item != predictors_map.constEnd(); ++item) {
				RK_DEBUG(PLUGIN, DL_DEBUG, "level %d", i);
				if (item.value() == inter.vars[i]) {
					++found_vars;
					break;
				}
			}
		}
		if (found_vars < (inter.level + 1)) {
			delete (in.key());
			new_map.remove(in.key());
		}
	}
	interaction_map = new_map;
}

bool RKFormula::isValid() {
	RK_TRACE(PLUGIN);
	return (model_ok);
}

QStringList RKFormula::getUiLabelPair() const {
	RK_TRACE(PLUGIN);

	QStringList ret(label_string);
	QString m = stripAccelerators(type_selector->checkedButton()->text());
	if (model_type == Custom) m.append(u' ' + model->value().toString());
	ret.append(m);
	return ret;
}
