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

// KDE4: TODO: sorting
//	setSorting (100);
	settings = new RKObjectListViewSettings ();
	connect (settings, SIGNAL (settingsChanged ()), this, SLOT (objectBrowserSettingsChanged ()));

	menu = new Q3PopupMenu (this);
	menu->insertItem (i18n ("Show Objects"), settings->showObjectsMenu ());
	menu->insertItem (i18n ("Show Fields"), settings->showFieldsMenu ());
	menu->insertItem (i18n ("Configure Defaults"), this, SLOT (popupConfigure ()));

// KDE4: do we need this?
//	setShowToolTips (false);

	objectBrowserSettingsChanged ();
}

RKObjectListView::~RKObjectListView () {
	RK_TRACE (APP);
}

void RKObjectListView::setObjectCurrent (RObject *object, bool only_if_none_current) {
	RK_TRACE (APP);

	if (!object) return;
	if (only_if_none_current && currentIndex ().isValid ()) return;

	QModelIndex index = RKGlobals::tracker ()->indexFor (object);
	if (index.isValid ()) {
		scrollTo (index);
		setCurrentIndex (index);
		resizeColumnToContents (0);
	}
}

RObject::ObjectList RKObjectListView::selectedObjects () const {
	RK_TRACE (APP);

	RObject::ObjectList list;
	QModelIndexList selected = selectedIndexes ();
	for (int i = 0; i < selected.size (); ++i) {
		QModelIndex index = selected[i];
		if (index.column () != 0) continue;
		if (!index.isValid ()) continue;
		list.append (static_cast<RObject*> (index.internalPointer ()));
	}
	return list;
}

void RKObjectListView::objectBrowserSettingsChanged () {
/*	setColumnWidthMode (0, Q3ListView::Maximum);
	if (settings->settingActive (RKObjectListViewSettings::ShowFieldsLabel)) {
		if (columnWidth (1) == 0) setColumnWidth (1, 50);
		setColumnWidthMode (1, Q3ListView::Maximum);
	} else {
		setColumnWidthMode (1, Q3ListView::Manual);
		hideColumn (1);
	}

	if (settings->settingActive (RKObjectListViewSettings::ShowFieldsType)) {
		if (columnWidth (2) == 0) setColumnWidth (2, 50);
		setColumnWidthMode (2, Q3ListView::Maximum);
	} else {
		setColumnWidthMode (2, Q3ListView::Manual);
		hideColumn (2);
	}

	if (settings->settingActive (RKObjectListViewSettings::ShowFieldsClass)) {
		if (columnWidth (3) == 0) setColumnWidth (3, 50);
		setColumnWidthMode (3, Q3ListView::Maximum);
	} else {
		setColumnWidthMode (3, Q3ListView::Manual);
		hideColumn (3);
	}

	triggerUpdate ();

	for (Q3ListViewItemIterator it (this); it.current (); ++it) {
		RObject *object = findItemObject (static_cast<RKListViewItem*> (it.current ()));
		RK_ASSERT (object);

		it.current ()->setVisible (settings->shouldShowObject (object));
	} */
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

	// KDE4: initialization logic is likely wrong, now.
	setModel (RKGlobals::tracker ());

	setExpanded (RKGlobals::tracker ()->indexFor (RObjectList::getObjectList ()), true);
	setExpanded (RKGlobals::tracker ()->indexFor (RObjectList::getGlobalEnv ()), true);
	setMinimumHeight (rowHeight (RKGlobals::tracker ()->indexFor (RObjectList::getGlobalEnv ())) * 5);
	resizeColumnToContents (0);

	connect (RObjectList::getObjectList (), SIGNAL (updateComplete ()), this, SLOT (updateComplete ()));
	disconnect (RObjectList::getObjectList (), SIGNAL (updateComplete ()), this, SLOT (initialize ()));
	connect (RObjectList::getObjectList (), SIGNAL (updateStarted ()), this, SLOT (updateStarted ()));

// KDE4 TODO: this signal needed?
	emit (listChanged ());
	changes = false;
	updateComplete ();
}

void RKObjectListView::initializeLater () {
	RK_TRACE (APP);

// KDE4: TODO huh?
	connect (RObjectList::getObjectList (), SIGNAL (updateComplete ()), this, SLOT (initialize ()));
	updateStarted ();
}

void RKObjectListView::updateComplete () {
	RK_TRACE (APP);

	setEnabled (true);
	update_in_progress = false;
	if (changes) {
		emit (listChanged ());
		changes = false;
	}
}

void RKObjectListView::updateStarted () {
	RK_TRACE (APP);

	setEnabled (false);
	update_in_progress = true;
}

/*
//////////////////// RKListViewItem //////////////////////////
int RKListViewItem::width (const QFontMetrics &fm, const Q3ListView * lv, int c) const {
	if (parent ()) {
		if (!parent ()->isOpen ()) {
			return 0;
		}
	}

	int ret = Q3ListViewItem::width (fm, lv, c);
	if (ret > 200) return 200;
	return ret;
} */

//////////////////// RKObjectListViewSettings //////////////////////////

RKObjectListViewSettings::RKObjectListViewSettings () {
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

bool RKObjectListViewSettings::shouldShowObject (RObject *object) {
	RK_TRACE (APP);

	if (object->getShortName ().startsWith (".")) {
		if ((!object->isType (RObject::GlobalEnv)) && (settings[ShowObjectsHidden] <= No)) return false;
	}
	if (object->isType (RObject::ToplevelEnv)) {
		if (object->isType (RObject::GlobalEnv)) return true;
		return (settings[ShowObjectsAllEnvironments] >= Yes);
	} else if (object->isType (RObject::Function)) {
		return (settings[ShowObjectsFunction] >= Yes);
	} else if (object->isType (RObject::Container)) {
		return (settings[ShowObjectsContainer] >= Yes);
	} else if (object->isVariable ()) {
		return (settings[ShowObjectsVariable] >= Yes);
	}
	return true;
}

void RKObjectListViewSettings::createContextMenus () {
	RK_TRACE (APP);

	show_objects_menu = new Q3PopupMenu (0);
	insertPopupItem (show_objects_menu, ShowObjectsAllEnvironments, i18n ("All Environments"));
	insertPopupItem (show_objects_menu, ShowObjectsContainer, i18n ("Objects with children"));
	insertPopupItem (show_objects_menu, ShowObjectsVariable, i18n ("Variables"));
	insertPopupItem (show_objects_menu, ShowObjectsFunction, i18n ("Functions"));
	show_objects_menu->insertSeparator ();
	insertPopupItem (show_objects_menu, ShowObjectsHidden, i18n ("Hidden Objects"));

	show_fields_menu = new Q3PopupMenu (0);
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

	emit (settingsChanged ());
}

void RKObjectListViewSettings::insertPopupItem (Q3PopupMenu *menu, Settings setting, const QString &text) {
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
