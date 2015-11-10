/***************************************************************************
                          rkobjectlistview  -  description
                             -------------------
    begin                : Wed Sep 1 2004
    copyright            : (C) 2004-2015 by Thomas Friedrichsmeier
    email                : thomas.friedrichsmeier@kdemail.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "rkobjectlistview.h"

#include <klocale.h>
#include <kfilterproxysearchline.h>

#include <QContextMenuEvent>
#include <QMenu>
#include <QTimer>
#include <QStyledItemDelegate>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QGroupBox>
#include <QCheckBox>

#include "../rkglobals.h"
#include "../core/robjectlist.h"
#include "../core/renvironmentobject.h"
#include "../core/rkmodificationtracker.h"
#include "rkstandardicons.h"
#include "rkcommonfunctions.h"
#include "../settings/rksettingsmoduleobjectbrowser.h"

#include "../debug.h"

/** Responsible for drawing the root level items "My Objects" and "Other Environments" */
class RKObjectListViewRootDelegate : public QStyledItemDelegate {
public:
	RKObjectListViewRootDelegate (RKObjectListView* parent) : QStyledItemDelegate (parent) {
		tree = parent;
		expanded = RKStandardIcons::getIcon (RKStandardIcons::ActionCollapseUp);
		collapsed = RKStandardIcons::getIcon (RKStandardIcons::ActionExpandDown);
	}
	void initStyleOption (QStyleOptionViewItem* option, const QModelIndex& index) const {
		QStyledItemDelegate::initStyleOption (option, index);
		if (!index.parent ().isValid ()) {
			QStyleOptionViewItemV4 *v4 = qstyleoption_cast<QStyleOptionViewItemV4 *> (option);
			if (!v4) {
				RK_ASSERT (false);
				return;
			}
			v4->icon = tree->isExpanded (index) ? expanded : collapsed;
			//v4->decorationPosition = QStyleOptionViewItemV4::Right;  // easily gets out of the picture, thus using left-align
			v4->features |= QStyleOptionViewItemV2::HasDecoration;

			RObject* object = static_cast<RObject*> (tree->settings->mapToSource (index).internalPointer ());
			v4->font.setBold (true);
			v4->backgroundBrush = tree->palette ().mid ();
			if (object == RObjectList::getObjectList ()) {
				v4->text = i18n ("Other Environments");
			} else {
				if (tree->model ()->hasChildren (index)) v4->text = i18n ("My Workspace");
				else v4->text = i18n ("My Workspace (no objects matching filter)");
			}
		}
	}
	RKObjectListView* tree;
	QIcon expanded;
	QIcon collapsed;
};

RKObjectListView::RKObjectListView (QWidget *parent) : QTreeView (parent) {
	RK_TRACE (APP);

	root_object = 0;
	rkdelegate = new RKObjectListViewRootDelegate (this);
	settings = new RKObjectListViewSettings (this);
	setSortingEnabled (true);
	sortByColumn (0, Qt::AscendingOrder);

	menu = new QMenu (this);
	menu->addMenu (settings->showObjectsMenu ());
	menu->addMenu (settings->showFieldsMenu ());
	menu->addAction (i18n ("Configure Defaults"), this, SLOT (popupConfigure()));

	connect (this, SIGNAL(clicked(QModelIndex)), this, SLOT(itemClicked(QModelIndex)));
}

RKObjectListView::~RKObjectListView () {
	RK_TRACE (APP);
}

void RKObjectListView::itemClicked (const QModelIndex& index) {
	RK_TRACE (APP);

	if (!index.parent ().isValid ()) {  // root level (pseudo) items expand on click
		QModelIndex fixed_index = model ()->index (index.row (), 0, index.parent ());;
		if (!isExpanded (fixed_index)) {
			expand (fixed_index);
			resizeColumnToContents (0);
		}
		else collapse (fixed_index);
	}
}

void RKObjectListView::setObjectCurrent (RObject *object, bool only_if_none_current) {
	RK_TRACE (APP);

	if (!object) return;
	if (only_if_none_current && currentIndex ().isValid ()) return;

	QModelIndex index = settings->mapFromSource (RKGlobals::tracker ()->indexFor (object));
	if (index.isValid ()) {
		scrollTo (index);
		setCurrentIndex (index);
		resizeColumnToContents (0);
	} else {
		RK_ASSERT (false);
	}
}

void RKObjectListView::setRootObject (RObject *root) {
	RK_TRACE (APP);

	root_object = root;
	if (!root && !settings->getSetting (RKObjectListViewSettings::ShowObjectsAllEnvironments)) root = RObjectList::getGlobalEnv ();
	QModelIndex index = settings->mapFromSource (RKGlobals::tracker ()->indexFor (root));
	if (index != rootIndex ()) {
		setRootIndex (index);
		settingsChanged ();    // Updates column sizes. Note: Recurses into this function, but with index == rootIndex()
	}
	setRootIsDecorated (index.isValid ());
}

RObject* RKObjectListView::objectAtIndex (const QModelIndex& index) const {
	RK_TRACE (APP);

	return static_cast<RObject*> (settings->mapToSource (index).internalPointer ());
}

void RKObjectListView::selectionChanged (const QItemSelection&, const QItemSelection&) {
	RK_TRACE (APP);

	emit (selectionChanged ());
}

RObject::ObjectList RKObjectListView::selectedObjects () const {
	RK_TRACE (APP);

	RObject::ObjectList list;
	QModelIndexList selected = selectedIndexes ();
	for (int i = 0; i < selected.size (); ++i) {
		QModelIndex index = settings->mapToSource (selected[i]);
		if (index.column () != 0) continue;
		if (!index.isValid ()) continue;
		list.append (static_cast<RObject*> (index.internalPointer ()));
	}
	return list;
}

void RKObjectListView::popupConfigure () {
	RK_TRACE (APP);
	RKSettings::configureSettings (RKSettings::PageObjectBrowser, this);
}

void RKObjectListView::contextMenuEvent (QContextMenuEvent* event) {
	RK_TRACE (APP);

	menu_object = objectAtIndex (indexAt (event->pos ()));
	bool suppress = false;
	emit (aboutToShowContextMenu (menu_object, &suppress));

	if (!suppress) menu->popup (event->globalPos ());
}

void RKObjectListView::initialize () {
	RK_TRACE (APP);

	setUniformRowHeights (true);

	settings->setSourceModel (RKGlobals::tracker ());
	setModel (settings);

	QModelIndex genv = settings->mapFromSource (RKGlobals::tracker ()->indexFor (RObjectList::getGlobalEnv ()));
	setExpanded (genv, true);
	setMinimumHeight (rowHeight (genv) * 5);
	settingsChanged ();

	connect (RObjectList::getObjectList (), SIGNAL (updateComplete()), this, SLOT (updateComplete()));
	connect (RObjectList::getObjectList (), SIGNAL (updateStarted()), this, SLOT (updateStarted()));
	connect (selectionModel (), SIGNAL (selectionChanged(QItemSelection,QItemSelection)), this, SLOT (selectionChanged(QItemSelection,QItemSelection)));
	connect (settings, SIGNAL (settingsChanged()), this, SLOT (settingsChanged()));

	updateComplete ();
}

void RKObjectListView::updateComplete () {
	RK_TRACE (APP);

	setEnabled (true);
}

void RKObjectListView::updateStarted () {
	RK_TRACE (APP);

	setEnabled (false);
}

void RKObjectListView::settingsChanged () {
	RK_TRACE (APP);

	setRootObject (root_object);
	if (!root_object) {
		setFirstColumnSpanned (0, QModelIndex (), true);
		setItemDelegateForRow (0, rkdelegate);
		setFirstColumnSpanned (1, QModelIndex (), true);
		setItemDelegateForRow (1, rkdelegate);
	}
	resizeColumnToContents (0);
}

//////////////////// RKObjectListViewSettings //////////////////////////

RKObjectListViewSettings::RKObjectListViewSettings (QObject* parent) : KRecursiveFilterProxyModel (parent) {
	RK_TRACE (APP);

	update_timer = new QTimer (this);
	update_timer->setSingleShot (true);
	connect (update_timer, SIGNAL(timeout()), this, SLOT(updateSelfNow()));

	filter_widget = 0;
	show_containers = show_functions = show_variables = true;
	filter_on_class = filter_on_label = filter_on_name = true;
	show_hidden_objects = RKSettingsModuleObjectBrowser::isSettingActive (ShowObjectsHidden);

	connect (RKSettings::tracker (), SIGNAL (settingsChanged(RKSettings::SettingsPage)), this, SLOT (globalSettingsChanged(RKSettings::SettingsPage)));

	action_group = new QActionGroup (this);
	action_group->setExclusive (false);
	settings[ShowObjectsAllEnvironments].action = new QAction (i18n ("All Environments"), action_group);
	settings[ShowObjectsHidden].action = new QAction (i18n ("Hidden Objects"), action_group);
	settings[ShowFieldsType].action = new QAction (i18n ("Type"), action_group);
	settings[ShowFieldsLabel].action = new QAction (i18n ("Label"), action_group);
	settings[ShowFieldsClass].action = new QAction (i18n ("Class"), action_group);

	for (int i = 0; i < SettingsCount; ++i) {
		settings[i].action->setCheckable (true);
	}

	createContextMenus ();

	// initialize the settings states
	globalSettingsChanged (RKSettings::PageObjectBrowser);
}

RKObjectListViewSettings::~RKObjectListViewSettings () {
	RK_TRACE (APP);

	delete show_fields_menu;
	delete show_objects_menu;
}

QWidget* RKObjectListViewSettings::filterWidget (QWidget *parent) {
	RK_TRACE (APP);

	if (filter_widget) return filter_widget;

	filter_widget = new QWidget (parent);

	QVBoxLayout *layout = new QVBoxLayout (filter_widget);
	layout->setContentsMargins (0, 0, 0, 0);
	QHBoxLayout* hlayout = new QHBoxLayout ();
	hlayout->setContentsMargins (0, 0, 0, 0);
	layout->addLayout (hlayout);

	KFilterProxySearchLine* sline = new KFilterProxySearchLine (filter_widget);
	sline->setProxy (this);
	hlayout->addWidget (sline);
	QPushButton* expander = new QPushButton (filter_widget);
	expander->setIcon (RKStandardIcons::getIcon (RKStandardIcons::ActionExpandDown));
	expander->setCheckable (true);
	hlayout->addWidget (expander);

	filter_widget_expansion = new QWidget (filter_widget);
	layout->addWidget (filter_widget_expansion);
	connect (expander, SIGNAL (toggled(bool)), filter_widget_expansion, SLOT (setShown(bool)));
	filter_widget_expansion->hide ();
	QVBoxLayout *elayout = new QVBoxLayout (filter_widget_expansion);
	elayout->setContentsMargins (0, 0, 0, 0);

	QGroupBox *box = new QGroupBox (i18nc ("Fields==columns in tree view", "Fields to search in"), filter_widget_expansion);
	elayout->addWidget (box);
	QHBoxLayout *boxlayout = new QHBoxLayout (box);
	filter_on_name_box = new QCheckBox (i18n ("Name"));
	boxlayout->addWidget (filter_on_name_box);
	filter_on_label_box = new QCheckBox (i18n ("Label"));
	boxlayout->addWidget (filter_on_label_box);
	filter_on_class_box = new QCheckBox (i18n ("Class"));
	boxlayout->addWidget (filter_on_class_box);

	connect (filter_on_name_box, SIGNAL(clicked(bool)), this, SLOT (filterSettingsChanged()));
	connect (filter_on_label_box, SIGNAL(clicked(bool)), this, SLOT (filterSettingsChanged()));
	connect (filter_on_class_box, SIGNAL(clicked(bool)), this, SLOT (filterSettingsChanged()));

	show_functions_box = new QCheckBox (i18n ("Show Functions"));
	RKCommonFunctions::setTips (i18n ("Uncheck this to exclude functions from the list"), show_functions_box);
	elayout->addWidget (show_functions_box);
	show_variables_box = new QCheckBox (i18n ("Show Vectors"));
	RKCommonFunctions::setTips (i18n ("Uncheck this to exclude vectors (i.e. most types of data objects) from the list"), show_variables_box);
	elayout->addWidget (show_variables_box);
	show_containers_box = new QCheckBox (i18n ("Show Containers"));
	RKCommonFunctions::setTips (i18n ("Uncheck this to exclude \"container\"-objects such as <i>list</i>s or <i>environment</i>s from the list. Note that containers will continue to be show, if they contain child objects matching the filter settings."), show_containers_box);
	elayout->addWidget (show_containers_box);

	connect (show_functions_box, SIGNAL(clicked(bool)), this, SLOT (filterSettingsChanged()));
	connect (show_variables_box, SIGNAL(clicked(bool)), this, SLOT (filterSettingsChanged()));
	connect (show_containers_box, SIGNAL(clicked(bool)), this, SLOT (filterSettingsChanged()));

	hidden_objects_box = new QCheckBox (i18n ("Show Hidden Objects"));
	connect (hidden_objects_box, SIGNAL (clicked(bool)), this, SLOT (filterSettingsChanged()));
	layout->addWidget (hidden_objects_box);

	filter_on_name_box->setChecked (filter_on_name);
	filter_on_label_box->setChecked (filter_on_label);
	filter_on_class_box->setChecked (filter_on_class);
	show_functions_box->setChecked (show_functions);
	show_variables_box->setChecked (show_variables);
	show_containers_box->setChecked (show_containers);
	hidden_objects_box->setChecked (show_hidden_objects);

	return filter_widget;
}

void RKObjectListViewSettings::filterSettingsChanged () {
	RK_TRACE (APP);

	if (!filter_widget) {
		RK_ASSERT (filter_widget);
		return;
	}

	filter_on_name = filter_on_name_box->isChecked ();
	filter_on_label = filter_on_label_box->isChecked ();
	filter_on_class = filter_on_class_box->isChecked ();
	show_functions = show_functions_box->isChecked ();
	show_variables = show_variables_box->isChecked ();
	show_containers = show_containers_box->isChecked ();
	show_hidden_objects = hidden_objects_box->isChecked ();
#warning TODO: It is also a QAction, currently.
	updateSelf ();
}

void RKObjectListViewSettings::setSetting (Settings setting, bool to) {
	RK_TRACE (APP);

	settings[setting].state = to;
	settings[setting].is_at_default = false;

	updateSelf ();
}

bool RKObjectListViewSettings::filterAcceptsColumn (int source_column, const QModelIndex&) const {
	RK_TRACE (APP);

	if (source_column == RKObjectListModel::NameColumn) return true;
	if (source_column == RKObjectListModel::LabelColumn) return (settings[ShowFieldsLabel].state);
	if (source_column == RKObjectListModel::TypeColumn) return (settings[ShowFieldsType].state);
	if (source_column == RKObjectListModel::ClassColumn) return (settings[ShowFieldsClass].state);

	RK_ASSERT (false);
	return false;
}

bool RKObjectListViewSettings::acceptRow (int source_row, const QModelIndex& source_parent) const {
	RK_TRACE (APP);

	// always show the root item
	if (!source_parent.isValid ()) return true;

	RObject* object = static_cast<RObject*> (source_parent.internalPointer ());
	object = object->findChildByObjectModelIndex (source_row);
	RK_ASSERT (object);

	// always show global env and search path
	if (!object->parentObject ()) return true;

	if (!show_hidden_objects) {
		if (object->getShortName ().startsWith ('.')) return false;
		if (object == reinterpret_cast<RObject*> (RObjectList::getObjectList ()->orphanNamespacesObject ())) return false;
	}

	if (!show_functions && object->isType (RObject::Function)) return false;
	if (!show_containers && object->isType (RObject::Container | RObject::Environment)) return false;
	if (!show_variables && object->isVariable ()) return false;

	if (filter_on_name && object->getShortName ().contains (filterRegExp ())) return true;
	if (filter_on_label && object->getLabel ().contains (filterRegExp ())) return true;
	if (filter_on_class) {
		QStringList cnames = object->classNames ();
		for (int i = cnames.length () - 1; i >= 0; --i) {
			if (cnames[i].contains (filterRegExp ())) return true;
		}
	}

	return false;
}

bool RKObjectListViewSettings::lessThan (const QModelIndex& left, const QModelIndex& right) const {
	// don't trace this. Used in sorting

	if (!(left.isValid () && right.isValid ())) return false;

	RObject* left_object = static_cast<RObject*> (left.internalPointer ());
	RObject* right_object = static_cast<RObject*> (right.internalPointer ());

	// for top-level environments, always use the search order
	if (left_object->isType (RObject::ToplevelEnv) && right_object->isType (RObject::ToplevelEnv)) {
		RObject* left_parent = left_object->parentObject ();
		RObject* right_parent = right_object->parentObject ();
		if (!(left_parent && right_parent)) return false;

		return (left_parent->getObjectModelIndexOf (left_object) < right_parent->getObjectModelIndexOf (right_object));
	} else if (left_object->isPseudoObject ()) return false;
	else if (right_object->isPseudoObject ()) return true;
	else if (!left.parent ().isValid ()) {
		// For the two root items, _always_ sort .GlobalEnv on top, irrespective of sort order
		return ((static_cast<RObject*> (left.internalPointer ()) == RObjectList::getGlobalEnv ()) == (sortOrder () == Qt::AscendingOrder));
	}

	return (QSortFilterProxyModel::lessThan (left, right));
}

void RKObjectListViewSettings::createContextMenus () {
	RK_TRACE (APP);

	show_objects_menu = new QMenu (i18n ("Show Objects"), 0);
	show_objects_menu->addAction (settings[ShowObjectsAllEnvironments].action);
	show_objects_menu->addSeparator ();
	show_objects_menu->addAction (settings[ShowObjectsHidden].action);

	show_fields_menu = new QMenu (i18n ("Show Fields"), 0);
	show_fields_menu->addAction (settings[ShowFieldsType].action);
	show_fields_menu->addAction (settings[ShowFieldsLabel].action);
	show_fields_menu->addAction (settings[ShowFieldsClass].action);

	connect (action_group, SIGNAL (triggered(QAction*)), this, SLOT(settingToggled(QAction*)));
	updateSelf ();
}

void RKObjectListViewSettings::updateSelf () {
	RK_TRACE (APP);

	update_timer->start (0);
}

void RKObjectListViewSettings::updateSelfNow () {
	RK_TRACE (APP);

	for (int i = 0; i < SettingsCount; ++i) settings[i].action->setChecked (settings[i].state);

	invalidateFilter ();

	emit (settingsChanged ());
}

void RKObjectListViewSettings::globalSettingsChanged (RKSettings::SettingsPage page) {
	if (page != RKSettings::PageObjectBrowser) return;

	RK_TRACE (APP);

	for (int i = 0; i < SettingsCount; ++i) {
		if (settings[i].is_at_default) {     // Only settings that have been left at their default are copied
			settings[i].state = RKSettingsModuleObjectBrowser::isSettingActive ((Settings) i);
		}
	}

	updateSelf ();
}

void RKObjectListViewSettings::settingToggled (QAction* which) {
	RK_TRACE (APP);

	int setting = -1;
	for (int i = 0; i < SettingsCount; ++i) {
		if (settings[i].action == which) {
			setting = i;
			break;
		}
	}
	if (setting < 0) {
		RK_ASSERT (false);
		return;
	}

	setSetting (static_cast<Settings> (setting), which->isChecked ());
}

#include "rkobjectlistview.moc"
