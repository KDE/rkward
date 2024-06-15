/*
rkobjectlistview - This file is part of RKWard (https://rkward.kde.org). Created: Wed Sep 1 2004
SPDX-FileCopyrightText: 2004-2018 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "rkobjectlistview.h"

#include <KLocalizedString>

#include <QContextMenuEvent>
#include <QMenu>
#include <QTimer>
#include <QStyledItemDelegate>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QGroupBox>
#include <QCheckBox>
#include <QComboBox>

#include "../core/robjectlist.h"
#include "../core/renvironmentobject.h"
#include "../core/rkmodificationtracker.h"
#include "rkstandardicons.h"
#include "rkcommonfunctions.h"
#include "rkdynamicsearchline.h"
#include "../settings/rksettingsmoduleobjectbrowser.h"

#include "../debug.h"

/** Responsible for drawing the root level items "My Objects" and "Other Environments" */
class RKObjectListViewRootDelegate : public QStyledItemDelegate {
public:
	explicit RKObjectListViewRootDelegate(RKObjectListView* parent) : QStyledItemDelegate(parent) {
		tree = parent;
		expanded = RKStandardIcons::getIcon (RKStandardIcons::ActionCollapseUp);
		collapsed = RKStandardIcons::getIcon (RKStandardIcons::ActionExpandDown);
	}
	void initStyleOption (QStyleOptionViewItem* option, const QModelIndex& index) const override {
		QStyledItemDelegate::initStyleOption (option, index);
		if (!index.parent ().isValid ()) {
			option->icon = tree->isExpanded (index) ? expanded : collapsed;
			//v4->decorationPosition = QStyleOptionViewItemV4::Right;  // easily gets out of the picture, thus using left-align
			option->features |= QStyleOptionViewItem::HasDecoration;

			RObject* object = static_cast<RObject*> (tree->settings->mapToSource (index).internalPointer ());
			option->font.setBold (true);
			option->backgroundBrush = tree->palette ().mid ();
			if (object == RObjectList::getObjectList ()) {
				option->text = i18n ("Other Environments");
			} else {
				if (tree->model ()->hasChildren (index)) option->text = i18n ("My Workspace");
				else option->text = i18n ("My Workspace (no objects matching filter)");
			}
		}
	}
	RKObjectListView* tree;
	QIcon expanded;
	QIcon collapsed;
};

RKObjectListView::RKObjectListView (bool toolwindow, QWidget *parent) : QTreeView (parent) {
	RK_TRACE (APP);

	root_object = nullptr;
	rkdelegate = new RKObjectListViewRootDelegate (this);
	settings = new RKObjectListViewSettings (toolwindow, this);
	setSortingEnabled (true);
	sortByColumn (0, Qt::AscendingOrder);

	menu = new QMenu (this);
	settings->addSettingsToMenu(menu, nullptr);

	connect (this, &QAbstractItemView::clicked, this, &RKObjectListView::itemClicked);
}

RKObjectListView::~RKObjectListView () {
	RK_TRACE (APP);
}

void RKObjectListView::itemClicked (const QModelIndex& index) {
	RK_TRACE (APP);

	if (!index.parent ().isValid ()) {  // root level (pseudo) items expand on click
		QModelIndex fixed_index = model ()->index (index.row (), 0, index.parent ());
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

	QModelIndex index = settings->mapFromSource (RKModificationTracker::instance()->indexFor (object));
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
	QModelIndex index = settings->mapFromSource (RKModificationTracker::instance()->indexFor (root));
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

void RKObjectListView::contextMenuEvent (QContextMenuEvent* event) {
	RK_TRACE (APP);

	menu_object = objectAtIndex (indexAt (event->pos ()));
	bool suppress = false;
	Q_EMIT aboutToShowContextMenu(menu_object, &suppress);

	if (!suppress) menu->popup (event->globalPos ());
}

void RKObjectListView::initialize () {
	RK_TRACE (APP);

	setUniformRowHeights (true);

	settings->setSourceModel (RKModificationTracker::instance());
	setModel (settings);

	QModelIndex genv = settings->mapFromSource (RKModificationTracker::instance()->indexFor (RObjectList::getGlobalEnv ()));
	setExpanded (genv, true);
	setMinimumHeight (rowHeight (genv) * 5);
	settingsChanged ();

	connect (RObjectList::getObjectList (), &RObjectList::updateComplete, this, &RKObjectListView::updateComplete);
	connect (RObjectList::getObjectList (), &RObjectList::updateStarted, this, &RKObjectListView::updateStarted);
	connect (settings, &RKObjectListViewSettings::settingsChanged, this, &RKObjectListView::settingsChanged);

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
	setFirstColumnSpanned (0, QModelIndex (), true);
	setItemDelegateForRow (0, rkdelegate);
	setFirstColumnSpanned (1, QModelIndex (), true);
	setItemDelegateForRow (1, rkdelegate);
	resizeColumnToContents (0);
}

//////////////////// RKObjectListViewSettings //////////////////////////

RKObjectListViewSettings::RKObjectListViewSettings (bool tool_window, QObject* parent) : QSortFilterProxyModel (parent) {
	RK_TRACE (APP);

	is_tool_window = tool_window;

	update_timer = new QTimer (this);
	update_timer->setSingleShot (true);
	connect (update_timer, &QTimer::timeout, this, &RKObjectListViewSettings::updateSelfNow);

	filter_widget = nullptr;
	in_reset_filters = false;

	persistent_settings_actions[ShowObjectsHidden] = new QAction (i18n ("Show Hidden Objects"), this);
	persistent_settings_actions[ShowFieldsType] = new QAction (i18n ("Type"), this);
	persistent_settings_actions[ShowFieldsLabel] = new QAction (i18n ("Label"), this);
	persistent_settings_actions[ShowFieldsClass] = new QAction (i18n ("Class"), this);

	for (int i = 0; i < SettingsCount; ++i) {
		if (is_tool_window) persistent_settings[i] = RKSettingsModuleObjectBrowser::isDefaultForWorkspace ((PersistentSettings) i);
		else persistent_settings[i] = RKSettingsModuleObjectBrowser::isDefaultForVarselector ((PersistentSettings) i);
		persistent_settings_actions[i]->setCheckable (true);
		persistent_settings_actions[i]->setChecked (persistent_settings[i]);
		connect (persistent_settings_actions[i], &QAction::toggled, this, &RKObjectListViewSettings::filterSettingsChanged);
	}

	resetFilters (); // inits defaults
}

RKObjectListViewSettings::~RKObjectListViewSettings () {
	RK_TRACE (APP);
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

	sline = new RKDynamicSearchLine (filter_widget);
	filter_widget->setFocusProxy (sline);
	sline->setModelToFilter (this);
	RKCommonFunctions::setTips (sline->regexpTip (), sline);
	connect (sline, &RKDynamicSearchLine::searchChanged, this, &RKObjectListViewSettings::filterSettingsChanged);
	hlayout->addWidget (sline);
	QPushButton* expander = new QPushButton (filter_widget);
	expander->setIcon (RKStandardIcons::getIcon (RKStandardIcons::ActionConfigureGeneric));
	expander->setCheckable (true);
	hlayout->addWidget (expander);

	filter_widget_expansion = new QWidget (filter_widget);
	layout->addWidget (filter_widget_expansion);
	connect (expander, &QPushButton::toggled, filter_widget_expansion, &QWidget::setVisible);
	filter_widget_expansion->hide ();
	QVBoxLayout *elayout = new QVBoxLayout (filter_widget_expansion);
	elayout->setContentsMargins (0, 0, 0, 0);

	QGroupBox *box = new QGroupBox (i18nc ("Fields==columns in tree view", "Fields to search in"), filter_widget_expansion);
	elayout->addWidget (box);
	QVBoxLayout *boxvlayout = new QVBoxLayout (box);
	QHBoxLayout *boxhlayout = new QHBoxLayout ();
	boxvlayout->addLayout (boxhlayout);
	boxhlayout->setContentsMargins (0, 0, 0, 0);
	filter_on_name_box = new QCheckBox (i18n ("Name"));
	boxhlayout->addWidget (filter_on_name_box);
	filter_on_label_box = new QCheckBox (i18n ("Label"));
	boxhlayout->addWidget (filter_on_label_box);
	filter_on_class_box = new QCheckBox (i18n ("Class"));
	boxhlayout->addWidget (filter_on_class_box);

	filter_on_name_box->setChecked (filter_on_name);
	filter_on_label_box->setChecked (filter_on_label);
	filter_on_class_box->setChecked (filter_on_class);
	connect (filter_on_name_box, &QCheckBox::clicked, this, &RKObjectListViewSettings::filterSettingsChanged);
	connect (filter_on_label_box, &QCheckBox::clicked, this, &RKObjectListViewSettings::filterSettingsChanged);
	connect (filter_on_class_box, &QCheckBox::clicked, this, &RKObjectListViewSettings::filterSettingsChanged);

	depth_box = new QComboBox ();
	depth_box->addItem (i18n ("Top level objects, only"));
	depth_box->addItem (i18n ("Top level objects, and direct children"));
	RKCommonFunctions::setTips (i18n ("Depth of search in the object tree.<ul>"
	                                      "<li><i>%1</i> means looking for matches in objects that are on the search path, only (in <i>.GlobalEnv</i> or a loaded package)</li>"
	                                      "<li><i>%2</i> includes direct child objects. In this case, the list will show matching objects on the search path, <i>and</i> objects on the search path that hold matching child objects.</li>", depth_box->itemText (0), depth_box->itemText (1)), depth_box);
	boxvlayout->addWidget (depth_box);

	depth_box->setCurrentIndex (1);
	connect (depth_box, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &RKObjectListViewSettings::filterSettingsChanged);

	type_box = new QComboBox ();
	type_box->addItem (i18n ("Show all objects"));
	type_box->addItem (i18n ("Show functions, only"));
	type_box->addItem (i18n ("Show objects excluding functions"));
	RKCommonFunctions::setTips (i18n ("When looking for a particular function, you may want to exclude 'data' objects, and vice versa. This control allows you to limit the list to objects that are not (or do not contain) functions, or to those that are (or contain) functions."), type_box);
	boxvlayout->addWidget (type_box);

	if (hide_functions) type_box->setCurrentIndex (2);
	else if (hide_non_functions) type_box->setCurrentIndex (1);
	else type_box->setCurrentIndex (0);
	connect (type_box, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &RKObjectListViewSettings::filterSettingsChanged);

	QHBoxLayout *bottom_layout = new QHBoxLayout();
	layout->addLayout(bottom_layout);
	QCheckBox* hidden_objects_box = new QCheckBox (i18n ("Show Hidden Objects"));
	hidden_objects_box->setChecked (persistent_settings[ShowObjectsHidden]);
	connect (hidden_objects_box, &QCheckBox::clicked, persistent_settings_actions[ShowObjectsHidden], &QAction::setChecked);
	connect (persistent_settings_actions[ShowObjectsHidden], &QAction::triggered, hidden_objects_box, &QCheckBox::setChecked);
	bottom_layout->addWidget (hidden_objects_box);

	// KF5 TODO: In frameworks, there is a function KIconUtils::kIconAddOverlay(). We could use this to overlay "view-filter" and discard, then use that
	// in a tool button (with tooltip), in order to save space.
	reset_filters_button = new QPushButton (i18nc ("Width is limited, please opt for something that is not much longer than the English string. Simply 'Clear'/'Reset' should be good enough to understand the function.", "Reset filters"), filter_widget);
	connect (reset_filters_button, &QPushButton::clicked, this, &RKObjectListViewSettings::resetFilters);
	RKCommonFunctions::setTips (i18n ("Discards the current object search filters"), reset_filters_button);
	reset_filters_button->hide ();
	bottom_layout->addWidget (reset_filters_button);

	return filter_widget;
}

void RKObjectListViewSettings::resetFilters () {
	RK_TRACE (APP);

	in_reset_filters = true;
	if (filter_widget) {
		type_box->setCurrentIndex (0);
		filter_on_name_box->setChecked (true);
		filter_on_label_box->setChecked (true);
		filter_on_class_box->setChecked (true);
		depth_box->setCurrentIndex (1);
		sline->setText (QString ());
	} else {
		hide_functions = hide_non_functions = false;
		filter_on_class = filter_on_label = filter_on_name = true;
		depth_limit = 1;
		setFilterRegularExpression (QRegularExpression{});
	}
	in_reset_filters = false;
	updateSelf ();
}

void RKObjectListViewSettings::filterSettingsChanged () {
	RK_TRACE (APP);

	if (in_reset_filters) return;  // Avoid updating for each setting, individually, when many are changed at once.

	if (filter_widget) {
		filter_on_name = filter_on_name_box->isChecked ();
		filter_on_label = filter_on_label_box->isChecked ();
		filter_on_class = filter_on_class_box->isChecked ();
		depth_limit = depth_box->currentIndex ();
		hide_functions = type_box->currentIndex () == 2;
		hide_non_functions = type_box->currentIndex () == 1;

		reset_filters_button->setVisible (!filter_on_name || !filter_on_class || !filter_on_label || (depth_limit != 1) || hide_functions || hide_non_functions || !sline->text ().isEmpty ());
	}

	for (int i = 0; i < SettingsCount; ++i) {
		persistent_settings[i] = persistent_settings_actions[i]->isChecked ();
		if (is_tool_window) RKSettingsModuleObjectBrowser::setDefaultForWorkspace ((PersistentSettings) i, persistent_settings[i]);
		else RKSettingsModuleObjectBrowser::setDefaultForVarselector ((PersistentSettings) i, persistent_settings[i]);
	}

	updateSelf ();
}

bool RKObjectListViewSettings::filterAcceptsColumn (int source_column, const QModelIndex&) const {
	RK_TRACE (APP);

	if (source_column == RKObjectListModel::NameColumn) return true;
	if (source_column == RKObjectListModel::LabelColumn) return (persistent_settings[ShowFieldsLabel]);
	if (source_column == RKObjectListModel::TypeColumn) return (persistent_settings[ShowFieldsType]);
	if (source_column == RKObjectListModel::ClassColumn) return (persistent_settings[ShowFieldsClass]);

	RK_ASSERT (false);
	return false;
}

bool RKObjectListViewSettings::filterAcceptsRow (int source_row, const QModelIndex& source_parent) const {
//	RK_TRACE (APP);

	// So I tried to use a KRecursiveFilterProxyModel, but
	// a) we don't really want recursion to the full depth. Thus limiting it, here.
	// b) While we don't handle insertions / removals of source indices in the presence of a filter, correctly, with KRecursiveFilterProxyModel
	//    I got crashes on this (arguably with the depth-limit in place)
	if (acceptRow (source_row, source_parent)) return true;

	RObject *parent = static_cast<RObject*> (source_parent.internalPointer ());
	if (!parent) {
		RK_ASSERT (parent);    // should have been accepted, above
		return true;
	}
	RObject *object = parent->findChildByObjectModelIndex (source_row);
	if (!object) {
		RK_ASSERT (object);    // should have been accepted, above
		RK_DEBUG (APP, DL_ERROR, "row %d of %d in %s", source_row, sourceModel ()->rowCount (source_parent), qPrintable (parent->getShortName ()));
		return false;
	}

	if (object->isType (RObject::ToplevelEnv | RObject::Workspace) || ((depth_limit > 0) && parent->isType (RObject::ToplevelEnv | RObject::Workspace))) {
		QModelIndex source_index = sourceModel ()->index (source_row, 0, source_parent);
		for (int row = 0, rows = sourceModel()->rowCount (source_index); row < rows; ++row) {
			if (filterAcceptsRow (row, source_index)) return true;
		}
	}

	return false;
}

bool RKObjectListViewSettings::acceptRow (int source_row, const QModelIndex& source_parent) const {
//	RK_TRACE (APP);

	// always show the root item
	if (!source_parent.isValid ()) return true;

	RObject* object = static_cast<RObject*> (source_parent.internalPointer ());
	// always show global env and search path
	if (!object) return true;
	if (!object->findChildByObjectModelIndex (source_row)) {
		return true;
	}
	object = object->findChildByObjectModelIndex (source_row);
	RK_ASSERT (object);

	if (!persistent_settings[ShowObjectsHidden]) {
		if (object->getShortName ().startsWith ('.')) return false;
		if (object == reinterpret_cast<RObject*> (RObjectList::getObjectList ()->orphanNamespacesObject ())) return false;
	}

	if (hide_functions && object->isType (RObject::Function)) return false;
	if (hide_non_functions && !object->isType (RObject::Function)) return false;

	if (filterRegularExpression().pattern().isEmpty()) return true;
	if (filter_on_name && object->getShortName ().contains (filterRegularExpression ())) return true;
	if (filter_on_label && object->getLabel ().contains (filterRegularExpression ())) return true;
	if (filter_on_class) {
		QStringList cnames = object->classNames ();
		for (int i = cnames.length () - 1; i >= 0; --i) {
			if (cnames[i].contains (filterRegularExpression ())) return true;
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

void RKObjectListViewSettings::addSettingsToMenu (QMenu* menu, QAction* before) {
	RK_TRACE (APP);

	menu->insertAction (before, persistent_settings_actions[ShowObjectsHidden]);

	QMenu *show_fields_menu = new QMenu (i18n ("Show Fields"), menu);
	show_fields_menu->addAction (persistent_settings_actions[ShowFieldsType]);
	show_fields_menu->addAction (persistent_settings_actions[ShowFieldsLabel]);
	show_fields_menu->addAction (persistent_settings_actions[ShowFieldsClass]);
	menu->insertMenu (before, show_fields_menu);

	updateSelf ();
}

void RKObjectListViewSettings::updateSelf () {
	RK_TRACE (APP);

	update_timer->start (0);
}

void RKObjectListViewSettings::updateSelfNow () {
	RK_TRACE (APP);

	invalidateFilter ();

	Q_EMIT settingsChanged();
}
