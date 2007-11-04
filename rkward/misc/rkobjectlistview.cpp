/***************************************************************************
                          rkobjectlistview  -  description
                             -------------------
    begin                : Wed Sep 1 2004
    copyright            : (C) 2004, 2006, 2007 by Thomas Friedrichsmeier
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
#include "rkobjectlistview.h"

#include <klocale.h>

#include <QHelpEvent>
#include <QMenu>

#include "../rkglobals.h"
#include "../core/robjectlist.h"
#include "../core/renvironmentobject.h"
#include "../core/rkmodificationtracker.h"
#include "../settings/rksettingsmoduleobjectbrowser.h"

#include "../debug.h"

RKObjectListView::RKObjectListView (QWidget *parent) : QTreeView (parent) {
	RK_TRACE (APP);

	settings = new RKObjectListViewSettings (this);
	setSortingEnabled (true);

	menu = new QMenu (this);
	menu->addMenu (settings->showObjectsMenu ());
	menu->addMenu (settings->showFieldsMenu ());
	menu->addAction (i18n ("Configure Defaults"), this, SLOT (popupConfigure ()));
}

RKObjectListView::~RKObjectListView () {
	RK_TRACE (APP);
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

// KDE4 TODO: does this really need to be virtual?
//virtual 
void RKObjectListView::popupConfigure () {
	RK_TRACE (APP);
	RKSettings::configureSettings (RKSettings::PageObjectBrowser, this);
}

void RKObjectListView::contextMenuEvent (QContextMenuEvent* event) {
	RK_TRACE (APP);

	QModelIndex index = settings->mapToSource (indexAt (event->pos ()));
	menu_object = static_cast<RObject*> (index.internalPointer ());

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
	QModelIndex olist = settings->mapFromSource (RKGlobals::tracker ()->indexFor (RObjectList::getObjectList ()));
	setExpanded (olist, true);
	setExpanded (genv, true);
	setMinimumHeight (rowHeight (genv) * 5);
	resizeColumnToContents (0);

	connect (RObjectList::getObjectList (), SIGNAL (updateComplete ()), this, SLOT (updateComplete ()));
	disconnect (RObjectList::getObjectList (), SIGNAL (updateComplete ()), this, SLOT (initialize ()));
	connect (RObjectList::getObjectList (), SIGNAL (updateStarted ()), this, SLOT (updateStarted ()));
	connect (selectionModel (), SIGNAL (selectionChanged(const QItemSelection&, const QItemSelection&)), this, SLOT (selectionChanged(const QItemSelection&, const QItemSelection&)));

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

//////////////////// RKObjectListViewSettings //////////////////////////

RKObjectListViewSettings::RKObjectListViewSettings (QObject* parent) : QSortFilterProxyModel (parent) {
	RK_TRACE (APP);

	for (int i = 0; i < SettingsCount; ++i) settings_default[i] = true;
	connect (RKSettings::tracker (), SIGNAL (settingsChanged (RKSettings::SettingsPage)), this, SLOT (globalSettingsChanged (RKSettings::SettingsPage)));

	createContextMenus ();
	globalSettingsChanged (RKSettings::PageObjectBrowser);
}

RKObjectListViewSettings::~RKObjectListViewSettings () {
	RK_TRACE (APP);

	delete show_fields_menu;
	delete show_objects_menu;
}

void RKObjectListViewSettings::setSetting (Settings setting, State to) {
	RK_TRACE (APP);

	settings[setting] = to;
	settings_default[setting] = false;

	updateSelf ();
}

RKObjectListViewSettings::State RKObjectListViewSettings::getSetting (Settings setting) {
	RK_TRACE (APP);

	return (settings[setting]);
}

bool RKObjectListViewSettings::settingActive (Settings setting) {
	RK_TRACE (APP);

	return (settings[setting] >= Yes);
}

bool RKObjectListViewSettings::filterAcceptsColumn (int source_column, const QModelIndex&) const {
	RK_TRACE (APP);

	if (source_column == RKObjectListModel::NameColumn) return true;
	if (source_column == RKObjectListModel::LabelColumn) return (settings[ShowFieldsLabel] >= Yes);
	if (source_column == RKObjectListModel::TypeColumn) return (settings[ShowFieldsType] >= Yes);
	if (source_column == RKObjectListModel::ClassColumn) return (settings[ShowFieldsClass] >= Yes);

	RK_ASSERT (false);
	return false;
}

bool RKObjectListViewSettings::filterAcceptsRow (int source_row, const QModelIndex& source_parent) const {
	RK_TRACE (APP);

	// always show the root item
	if (!source_parent.isValid ()) return true;

	RObject* object = static_cast<RObject*> (source_parent.internalPointer ());
	RK_ASSERT (object->isContainer ());
	object = static_cast<RContainerObject*> (object)->findChildByIndex (source_row);
	RK_ASSERT (object);

	// always show the global env
	if (object->isType (RObject::GlobalEnv)) return true;

	if (settings[ShowObjectsHidden] <= No) {
		if (object->getShortName ().startsWith ('.')) return false;
	}

	bool base_filter = QSortFilterProxyModel::filterAcceptsRow (source_row, source_parent);

	if (object->isType (RObject::ToplevelEnv)) {
		return (base_filter && (settings[ShowObjectsAllEnvironments] >= Yes));
	} else if (object->isType (RObject::Function)) {
		return (base_filter && (settings[ShowObjectsFunction] >= Yes));
	} else if (object->isType (RObject::Container)) {
		return (base_filter && (settings[ShowObjectsContainer] >= Yes));
	} else if (object->isVariable ()) {
		return (base_filter && (settings[ShowObjectsVariable] >= Yes));
	}

	return base_filter;
}

bool RKObjectListViewSettings::lessThan (const QModelIndex& left, const QModelIndex& right) const {
	// don't trace this. Used in sorting

	if (!(left.isValid () && right.isValid ())) return false;

	RObject* left_object = static_cast<RObject*> (left.internalPointer ());
	RObject* right_object = static_cast<RObject*> (right.internalPointer ());

	// for top-level environments, always use the search order
	if (left_object->isType (RObject::ToplevelEnv) && right_object->isType (RObject::ToplevelEnv)) {
		RContainerObject* left_parent = left_object->getContainer ();
		RContainerObject* right_parent = right_object->getContainer ();
		if (!(left_parent && right_parent)) return false;

		return (left_parent->getIndexOf (left_object) < right_parent->getIndexOf (right_object));
	}

	return (QSortFilterProxyModel::lessThan (left, right));
}

void RKObjectListViewSettings::createContextMenus () {
	RK_TRACE (APP);

	action_group = new QActionGroup (this);
	action_group->setExclusive (false);
	actions[ShowObjectsAllEnvironments] = new QAction (i18n ("All Environments"), action_group);
	actions[ShowObjectsContainer] = new QAction (i18n ("Objects with children"), action_group);
	actions[ShowObjectsVariable] = new QAction (i18n ("Variables"), action_group);
	actions[ShowObjectsFunction] = new QAction (i18n ("Functions"), action_group);
	actions[ShowObjectsHidden] = new QAction (i18n ("Hidden Objects"), action_group);
	actions[ShowFieldsType] = new QAction (i18n ("Type"), action_group);
	actions[ShowFieldsLabel] = new QAction (i18n ("Label"), action_group);
	actions[ShowFieldsClass] = new QAction (i18n ("Class"), action_group);
	for (int i = 0; i < SettingsCount; ++i) actions[i]->setCheckable (true);

	show_objects_menu = new QMenu (i18n ("Show Objects"), 0);
	show_objects_menu->addAction (actions[ShowObjectsAllEnvironments]);
	show_objects_menu->addAction (actions[ShowObjectsContainer]);
	show_objects_menu->addAction (actions[ShowObjectsVariable]);
	show_objects_menu->addAction (actions[ShowObjectsFunction]);
	show_objects_menu->addSeparator ();
	show_objects_menu->addAction (actions[ShowObjectsHidden]);

	show_fields_menu = new QMenu (i18n ("Show Fields"), 0);
	show_fields_menu->addAction (actions[ShowFieldsType]);
	show_fields_menu->addAction (actions[ShowFieldsLabel]);
	show_fields_menu->addAction (actions[ShowFieldsClass]);

	connect (action_group, SIGNAL (triggered(QAction*)), this, SLOT(settingToggled(QAction*)));
	updateSelf ();
}

void RKObjectListViewSettings::updateSelf () {
	RK_TRACE (APP);

	for (int i = 0; i < SettingsCount; ++i) {
		actions[i]->setChecked (settings[(Settings) i] >= Yes);
		actions[i]->setEnabled (optionConfigurable ((Settings) i));
	}

	invalidateFilter ();
	emit (settingsChanged ());
}

void RKObjectListViewSettings::insertPopupItem (QMenu *menu, Settings setting, const QString &text) {
	RK_TRACE (APP);

	menu->insertItem (text, setting);
	menu->setItemParameter (setting, setting);
	menu->connectItem (setting, this, SLOT (toggleSetting (int)));
}

void RKObjectListViewSettings::globalSettingsChanged (RKSettings::SettingsPage page) {
	if (page != RKSettings::PageObjectBrowser) return;

	RK_TRACE (APP);

	for (int i = 0; i < SettingsCount; ++i) {
		if (settings_default[i]) {
			if (RKSettingsModuleObjectBrowser::isSettingActive ((Settings) i)) settings[i] = Yes;
			else settings[i] = No;
		}
	}

	updateSelf ();
}

void RKObjectListViewSettings::settingToggled (QAction* which) {
	RK_TRACE (APP);

	int setting = -1;
	for (int i = 0; i < SettingsCount; ++i) {
		if (actions[i] == which) {
			setting = i;
			break;
		}
	}
	if (setting < 0) {
		RK_ASSERT (false);
		return;
	}

	setSetting (static_cast<Settings> (setting), which->isChecked () ? Yes : No);
}

bool RKObjectListViewSettings::optionConfigurable (Settings setting) {
	RK_TRACE (APP);

	if (settings[setting] == Always) return false;
	if (settings[setting] == Never) return false;
	return true;
}

#include "rkobjectlistview.moc"
