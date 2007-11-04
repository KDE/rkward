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
#include <kiconloader.h>

#include <q3popupmenu.h>
#include <qpixmap.h>
#include <qimage.h>
#include <QHelpEvent>

#include "../rkglobals.h"
#include "../core/robjectlist.h"
#include "../core/renvironmentobject.h"
#include "../core/rfunctionobject.h"
#include "../core/rkvariable.h"
#include "../core/rkmodificationtracker.h"
#include "../settings/rksettingsmoduleobjectbrowser.h"
#include "../misc/rkcommonfunctions.h"
#include "../debug.h"

RKObjectListView::RKObjectListView (QWidget *parent) : QTreeView (parent) {
	RK_TRACE (APP);

	settings = new RKObjectListViewSettings (this);
	setSortingEnabled (true);

	menu = new Q3PopupMenu (this);
	menu->insertItem (i18n ("Show Objects"), settings->showObjectsMenu ());
	menu->insertItem (i18n ("Show Fields"), settings->showFieldsMenu ());
	menu->insertItem (i18n ("Configure Defaults"), this, SLOT (popupConfigure ()));
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

	QModelIndex index = indexAt (event->pos ());
	menu_object = static_cast<RObject*> (index.internalPointer ());

	bool suppress = false;
	emit (aboutToShowContextMenu (menu_object, &suppress));

	if (!suppress) menu->popup (event->globalPos ());
}

void RKObjectListView::initialize () {
	RK_TRACE (APP);

	setUniformRowHeights (true);		// KDE4: can we do this?

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

	settings = new State[SettingsCount];
	settings_default = new bool[SettingsCount];
	for (int i = 0; i < SettingsCount; ++i) settings_default[i] = true;
	connect (RKSettings::tracker (), SIGNAL (settingsChanged (RKSettings::SettingsPage)), this, SLOT (globalSettingsChanged (RKSettings::SettingsPage)));

	createContextMenus ();
	globalSettingsChanged (RKSettings::PageObjectBrowser);
}

RKObjectListViewSettings::~RKObjectListViewSettings () {
	RK_TRACE (APP);

	delete settings;
	delete settings_default;
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

	// always show the global evnt
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

	show_objects_menu = new QMenu (0);
	insertPopupItem (show_objects_menu, ShowObjectsAllEnvironments, i18n ("All Environments"));
	insertPopupItem (show_objects_menu, ShowObjectsContainer, i18n ("Objects with children"));
	insertPopupItem (show_objects_menu, ShowObjectsVariable, i18n ("Variables"));
	insertPopupItem (show_objects_menu, ShowObjectsFunction, i18n ("Functions"));
	show_objects_menu->insertSeparator ();
	insertPopupItem (show_objects_menu, ShowObjectsHidden, i18n ("Hidden Objects"));

	show_fields_menu = new QMenu (0);
	insertPopupItem (show_fields_menu, ShowFieldsType, i18n ("Type"));
	insertPopupItem (show_fields_menu, ShowFieldsLabel, i18n ("Label"));
	insertPopupItem (show_fields_menu, ShowFieldsClass, i18n ("Class"));
}

void RKObjectListViewSettings::updateSelf () {
	RK_TRACE (APP);

	for (int i = 0; i <= ShowObjectsHidden; ++i) {
		show_objects_menu->setItemChecked (i, settings[(Settings) i] >= Yes);
		show_objects_menu->setItemEnabled (i, optionConfigurable ((Settings) i));
	}

	for (int i = ShowFieldsType; i <= ShowFieldsLabel; ++i) {
		show_fields_menu->setItemChecked (i, settings[(Settings) i] >= Yes);
		show_fields_menu->setItemEnabled (i, optionConfigurable ((Settings) i));
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

void RKObjectListViewSettings::toggleSetting (int which) {
	RK_TRACE (APP);
	RK_ASSERT (which < SettingsCount);

	if (settings[which] == Yes) {
		settings[which] = No;
	} else if (settings[which] == No) {
		settings[which] = Yes;
	} else {
		RK_ASSERT (false);
	}
	settings_default[which] = false;

	updateSelf ();
}

bool RKObjectListViewSettings::optionConfigurable (Settings setting) {
	RK_TRACE (APP);

	if (settings[setting] == Always) return false;
	if (settings[setting] == Never) return false;
	return true;
}

#include "rkobjectlistview.moc"
