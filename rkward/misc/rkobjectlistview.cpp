/***************************************************************************
                          rkobjectlistview  -  description
                             -------------------
    begin                : Wed Sep 1 2004
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
#include "rkobjectlistview.h"

#include <klocale.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kiconloader.h>

#include <qpopupmenu.h>
#include <qpixmap.h>
#include <qimage.h>

#include "../rkglobals.h"
#include "../core/robjectlist.h"
#include "../core/renvironmentobject.h"
#include "../core/rfunctionobject.h"
#include "../core/rkvariable.h"
#include "../core/rkmodificationtracker.h"
#include "../settings/rksettings.h"
#include "../settings/rksettingsmoduleobjectbrowser.h"
#include "../debug.h"

// static
QPixmap *RKObjectListView::icon_function = 0;
QPixmap *RKObjectListView::icon_list = 0;
QPixmap *RKObjectListView::package_environment = 0;
QPixmap *RKObjectListView::environment = 0;

RKObjectListView::RKObjectListView (QWidget *parent) : QListView (parent) {
	RK_TRACE (APP);
	if (icon_function == 0) {
		icon_function = new QPixmap (QImage (KGlobal::dirs ()->findResourceDir ("data", "rkward/phpfiles/common.php") + "rkward/icons/function.png"));
		icon_list = new QPixmap (QImage (KGlobal::dirs ()->findResourceDir ("data", "rkward/phpfiles/common.php") + "rkward/icons/list.png"));
		package_environment = new QPixmap (SmallIcon ("ark", 12));
		environment = new QPixmap (SmallIcon ("konqueror", 12));
	}

	setSorting (100);
	addColumn (i18n("Name"));
	addColumn (i18n("Label"));
	addColumn (i18n("Type"));
	addColumn (i18n("Class"));
	settings = new RKObjectListViewSettings ();
	connect (settings, SIGNAL (settingsChanged ()), this, SLOT (objectBrowserSettingsChanged ()));

	menu = new QPopupMenu (this);
	menu->insertItem (i18n ("Show Objects"), settings->showObjectsMenu ());
	menu->insertItem (i18n ("Show Fields"), settings->showFieldsMenu ());
	menu->insertItem (i18n ("Configure Defaults"), this, SLOT (popupConfigure ()));
	connect (this, SIGNAL (contextMenuRequested (QListViewItem *, const QPoint &, int)), this, SLOT (requestedContextMenu (QListViewItem*, const QPoint&, int)));

	setShowToolTips (false);
	tip = new RKObjectListViewTip (this);

	objectBrowserSettingsChanged ();
}

RKObjectListView::~RKObjectListView () {
	RK_TRACE (APP);

	delete tip;
}

void RKObjectListView::objectBrowserSettingsChanged () {
	setColumnWidthMode (0, QListView::Maximum);
	if (settings->settingActive (RKObjectListViewSettings::ShowFieldsLabel)) {
		if (columnWidth (1) == 0) setColumnWidth (1, 50);
		setColumnWidthMode (1, QListView::Maximum);
	} else {
		setColumnWidthMode (1, QListView::Manual);
		hideColumn (1);
	}

	if (settings->settingActive (RKObjectListViewSettings::ShowFieldsType)) {
		if (columnWidth (2) == 0) setColumnWidth (2, 50);
		setColumnWidthMode (2, QListView::Maximum);
	} else {
		setColumnWidthMode (2, QListView::Manual);
		hideColumn (2);
	}

	if (settings->settingActive (RKObjectListViewSettings::ShowFieldsClass)) {
		if (columnWidth (3) == 0) setColumnWidth (3, 50);
		setColumnWidthMode (3, QListView::Maximum);
	} else {
		setColumnWidthMode (3, QListView::Manual);
		hideColumn (3);
	}

	triggerUpdate ();

	for (QListViewItemIterator it (this); it.current (); ++it) {
		RObject *object = findItemObject (static_cast<RKListViewItem*> (it.current ()));
		RK_ASSERT (object);

		it.current ()->setVisible (settings->shouldShowObject (object));
	}
}

//virtual 
void RKObjectListView::popupConfigure () {
	RKSettings::configureSettings (RKSettings::ObjectBrowser, this);
}

void RKObjectListView::requestedContextMenu (QListViewItem *item, const QPoint &pos, int) {
	RObject *object = findItemObject (static_cast<RKListViewItem *> (item));

	menu_object = object;

	bool suppress = false;
	emit (aboutToShowContextMenu (static_cast<RKListViewItem *> (item), &suppress));

	if (!suppress) menu->popup (pos);
}

void RKObjectListView::initialize () {
	RK_TRACE (APP);

	setUpdatesEnabled (false);
	addObject (0, RObjectList::getObjectList (), true);
	setUpdatesEnabled (true);
	RKListViewItem *item = findObjectItem (RObjectList::getGlobalEnv ());
	RK_ASSERT (item);
	item->setOpen (true);

	connect (RKGlobals::tracker (), SIGNAL (objectRemoved (RObject *)), this, SLOT (objectRemoved (RObject*)));
	connect (RKGlobals::tracker (), SIGNAL (objectPropertiesChanged (RObject *)), this, SLOT (objectPropertiesChanged (RObject*)));
	connect (RKGlobals::tracker (), SIGNAL (objectAdded (RObject *)), this, SLOT (objectAdded (RObject*)));

	connect (RObjectList::getObjectList (), SIGNAL (updateComplete ()), this, SLOT (updateComplete ()));
	disconnect (RObjectList::getObjectList (), SIGNAL (updateComplete ()), this, SLOT (initialize ()));
	connect (RObjectList::getObjectList (), SIGNAL (updateStarted ()), this, SLOT (updateStarted ()));

	emit (listChanged ());
	changes = false;
	updateComplete ();
}

void RKObjectListView::initializeLater () {
	RK_TRACE (APP);

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

void RKObjectListView::objectAdded (RObject *object) {
	RK_TRACE (APP);

	RKListViewItem *parent = 0;
	if (!object->isType (RObject::Workspace)) {
		parent = findObjectItem (object->getContainer ());
		RK_ASSERT (parent);
	}
	addObject (parent, object, true);
	
	if (update_in_progress) {
		changes = true;
	} else {
		emit (listChanged ());
	}
}

void RKObjectListView::objectRemoved (RObject *object) {
	RK_TRACE (APP);

	RKListViewItem *item = findObjectItem (object);
	RK_ASSERT (item);
	// also take care of removing the children of the removed item!
	// this can NOT be done by using QListViewItemIterator (item), as that is NOT constrained to the children!
	RObject **children = object->children ();
	for (int i = 0; i < object->numChildren (); ++i) {
		objectRemoved (children[i]);
	}

	object_map.remove (item);
	delete item;
	
	if (update_in_progress) {
		changes = true;
	} else {
		emit (listChanged ());
	}
}

void RKObjectListView::objectPropertiesChanged (RObject *object) {
	RK_TRACE (APP);

	RKListViewItem *item = findObjectItem (object);
	RK_ASSERT (item);
	updateItem (item, object);

	if (update_in_progress) {
		changes = true;
	} else {
		emit (listChanged ());
	}
}

RKListViewItem *RKObjectListView::findObjectItem (RObject *object) {
	RK_TRACE (APP);
	for (ObjectMap::const_iterator it = object_map.constBegin (); it != object_map.constEnd (); ++it) {
		if (it.data () == object) return it.key ();
	}
	return 0;
}

RObject *RKObjectListView::findItemObject (RKListViewItem *item) {
	RK_TRACE (APP);
	if (!item) return 0;
	if (object_map.find (item) == object_map.end ()) {
		return 0;
	} else {
		return object_map[item];
	}
}

void RKObjectListView::updateItem (RKListViewItem *item, RObject *object) {
	RK_TRACE (APP);

	item->setText (0, object->getShortName ());
	item->setText (1, object->getLabel ());
	if (object->isVariable ()) {
		item->setText (2, RObject::typeToText (object->getDataType ()));
	}
	item->setText (3, object->makeClassString ("; "));

	if (object->isDataFrame ()) {
		item->setPixmap (0, SmallIcon("spreadsheet"));
	} else if (object->isVariable()) {
		switch(object->getDataType ()) {
			case RObject::DataNumeric:
				item->setPixmap (0, SmallIcon("math_paren",12));
				break;
			case RObject::DataFactor:
				item->setPixmap (0, SmallIcon("math_onetwomatrix",12));
				break;
			case RObject::DataCharacter:
				item->setPixmap (0, SmallIcon("text",12));
				break;
			case RObject::DataLogical:
				#warning TODO icon for logical
			case RObject::DataUnknown:
				item->setPixmap (0, SmallIcon("help",12));
				break;
			default:
				item->setPixmap (0, SmallIcon("no",12));
				break;
		}
	} else if (object->isType (RObject::List)) {
		item->setPixmap (0, *icon_list);
	} else if (object->isType (RObject::Function)) {
		item->setPixmap (0, *icon_function);
	} else if (object->isType (RObject::PackageEnv)) {
		item->setPixmap (0, *package_environment);
	} else if (object->isType (RObject::Environment)) {
		item->setPixmap (0, *environment);
	}

	if (!settings->shouldShowObject (object)) item->setVisible (false);
}

void RKObjectListView::addObject (RKListViewItem *parent, RObject *object, bool recursive) {
	RK_TRACE (APP);
	
	RKListViewItem *item;

	if (parent) {
		item = new RKListViewItem (parent);
	} else {
		item = new RKListViewItem (this);
	}

	updateItem (item, object);
	object_map.insert (item, object);

	if (recursive) {
		RObject **children = object->children ();
		for (int i=0; i < object->numChildren (); ++i) {
			addObject (item, children[i], true);
		}
	}

// special treatment for the workspace object
	if (!parent) {
		item->setPixmap (0, SmallIcon("view_tree"));
		item->setText (0, i18n ("[Objects]"));
		item->setOpen (true);
	}

// code below won't work, as objects get added before editor is opened. Need to call from RKEditor(Manager)
/*	if (object->numChildren () && RKGlobals::editorManager ()->objectOpened (object)) {
		item->setOpen (true);
		while (item->parent ()) {
			item = item->parent ();
			item->setOpen (true);
		}
	} */
}



//////////////////// RKListViewItem //////////////////////////
int RKListViewItem::width (const QFontMetrics &fm, const QListView * lv, int c) const {
	if (parent ()) {
		if (!parent ()->isOpen ()) {
			return 0;
		}
	}

	int ret = QListViewItem::width (fm, lv, c);
	if (ret > 200) return 200;
	return ret;
}

//////////////////// RKObjectListViewSettings //////////////////////////

RKObjectListViewSettings::RKObjectListViewSettings () {
	RK_TRACE (APP);

	settings = new State[SettingsCount];
	settings_default = new bool[SettingsCount];
	for (int i = 0; i < SettingsCount; ++i) settings_default[i] = true;
	connect (RKSettings::tracker (), SIGNAL (objectBrowserSettingsChanged ()), this, SLOT (globalSettingsChanged ()));

	createContextMenus ();
	globalSettingsChanged ();
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

	show_objects_menu = new QPopupMenu (0);
	insertPopupItem (show_objects_menu, ShowObjectsAllEnvironments, i18n ("All Environments"));
	insertPopupItem (show_objects_menu, ShowObjectsContainer, i18n ("Objects with children"));
	insertPopupItem (show_objects_menu, ShowObjectsVariable, i18n ("Variables"));
	insertPopupItem (show_objects_menu, ShowObjectsFunction, i18n ("Functions"));
	show_objects_menu->insertSeparator ();
	insertPopupItem (show_objects_menu, ShowObjectsHidden, i18n ("Hidden Objects"));

	show_fields_menu = new QPopupMenu (0);
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

void RKObjectListViewSettings::insertPopupItem (QPopupMenu *menu, Settings setting, const QString &text) {
	RK_TRACE (APP);

	menu->insertItem (text, setting);
	menu->setItemParameter (setting, setting);
	menu->connectItem (setting, this, SLOT (toggleSetting (int)));
}

void RKObjectListViewSettings::globalSettingsChanged () {
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


///////// RKObjectListViewTip ////////////
RKObjectListViewTip::RKObjectListViewTip (RKObjectListView *parent) : QToolTip (parent->viewport ()) {
	RK_TRACE (APP);

	view = parent;
}

RKObjectListViewTip::~RKObjectListViewTip () {
	RK_TRACE (APP);
}

void RKObjectListViewTip::maybeTip (const QPoint &pos) {
	RK_TRACE (APP);

	RKListViewItem *item = static_cast<RKListViewItem *> (view->itemAt (pos));
	if (!item) return;
	RObject *object = view->findItemObject (item);
	if (!object) return;

	// TODO: move all this to RObject::getDescription () or something similar (and complete it)
	// merge with age-old code in RObjectViewer

	tip (view->itemRect (item), object->getObjectDescription ());
}

#include "rkobjectlistview.moc"
