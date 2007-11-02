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
#ifndef RKOBJECTLISTVIEW_H
#define RKOBJECTLISTVIEW_H

#include <QTreeView>
#include <qtooltip.h>
#include <qmap.h>
#include <Q3PopupMenu>

#include "../settings/rksettings.h"
#include "../core/robject.h"

class QPixmap;
class Q3PopupMenu;
class RKListViewItem;
class RKObjectListViewSettings;

/**
This class provides the common functionality for the tree views in the RObjectBrowser and RKVarselector(s). The caps it (will) provide are: keeping the list up to date and emitting change-signals when appropriate, filtering for certain types of objects, sorting, mapping items to objects. Maybe some GUI-stuff like popup-menus should also be added to this class?

@author Thomas Friedrichsmeier
*/
class RKObjectListView : public QTreeView {
	Q_OBJECT
public:
	explicit RKObjectListView (QWidget *parent);
	
	~RKObjectListView ();

	void initializeLater ();

/** This function returns a pointer to the context menu of the RKObjectListView. It is provided so you can add your own items.
@returns a pointer to the context menu
@see aboutToShowContextMenu */
	Q3PopupMenu *contextMenu () { return menu; };
/** This function returns the RObject the context menu has last been invoked on (or 0 if not invoked on an RObject). You can use this in slots called
from your custom menu items, to figure out, which object you should operate on. */
	RObject *menuObject () const { return menu_object; };

	RKObjectListViewSettings *getSettings () { return settings; };

/** Scrolls so that the item representing object becomes visible, and makes it current */
	void setObjectCurrent (RObject *object, bool only_if_none_current=false);

	RObject::ObjectList selectedObjects () const;
signals:
	void listChanged ();
/** This signal is emitted just before the context-menu is shown. If you connect to this signal, you can make some adjustments to the context-menu.
If you set *suppress to true, showing the context menu will be suppressed. */
	void aboutToShowContextMenu (RObject *object, bool *suppress);
public slots:
/** Takes care initializing the RKObjectListView (delayed, as the RObjectList may not have been created, yet) and of getting the current list of objects from the RObjectList */
	void initialize ();

	void updateComplete ();
	void updateStarted ();

	void objectBrowserSettingsChanged ();

	virtual void popupConfigure ();
protected:
	void contextMenuEvent (QContextMenuEvent* event);
private:
	bool update_in_progress;
	bool changes;

	Q3PopupMenu *menu;
	RObject *menu_object;

	RKObjectListViewSettings *settings;
};

/** This subclass of RKListViewItem reimplements the width ()-function to return 0 if the item is not currently visible. This is needed to get a sane column width in the listview. Also limit maximum default width to 200 px (TODO: make this configurable)

@author Thomas Friedrichsmeier
*/
/*
class RKListViewItem : public Q3ListViewItem {
public:
	RKListViewItem (Q3ListView *parent) : Q3ListViewItem (parent) {};
	RKListViewItem (Q3ListViewItem *parent) : Q3ListViewItem (parent) {};
	~RKListViewItem () {};

	int width (const QFontMetrics &fm, const Q3ListView * lv, int c) const;
}; */

/** Represents the filter/view settings possible for an RKListView. */
class RKObjectListViewSettings : public QObject {
	Q_OBJECT
public:
/** ctor. copies the default settings from RKSettingsModuleObjectBrowser */ 
	RKObjectListViewSettings ();
	~RKObjectListViewSettings ();

	enum Settings {
		ShowObjectsVariable=0,
		ShowObjectsAllEnvironments=1,
		ShowObjectsFunction=2,
		ShowObjectsContainer=3,
		ShowObjectsHidden=4,
		ShowFieldsType=5,
		ShowFieldsClass=6,
		ShowFieldsLabel=7,
		SettingsCount=8
	};

	enum State {
		Never,
		No,
		Yes,
		Always
	};

	void setSetting (Settings setting, State to);
	State getSetting (Settings setting);
	bool settingActive (Settings setting);
	bool optionConfigurable (Settings setting);

	bool shouldShowObject (RObject *object);

	Q3PopupMenu *showObjectsMenu () { return show_objects_menu; };
	Q3PopupMenu *showFieldsMenu () { return show_fields_menu; };
signals:
	void settingsChanged ();
public slots:
	void globalSettingsChanged (RKSettings::SettingsPage);
	void toggleSetting (int which);
private:
	State *settings;
	bool *settings_default;
	void insertPopupItem (Q3PopupMenu *menu, Settings setting, const QString &text);
	void createContextMenus ();
	void updateSelf ();

	Q3PopupMenu *show_objects_menu;
	Q3PopupMenu *show_fields_menu;
};

#endif
