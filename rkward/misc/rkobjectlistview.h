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
#ifndef RKOBJECTLISTVIEW_H
#define RKOBJECTLISTVIEW_H

#include <qlistview.h>
#include <qmap.h>

class RObject;
class QPixmap;
class QPopupMenu;
class RKListViewItem;
class RKObjectListViewSettings;

/**
This class provides the common functionality for the list-views in the RObjectBrowser and RKVarselector(s). The caps it (will) provide are: keeping the list up to date and emitting change-signals when appropriate, filtering for certain types of objects, sorting, mapping items to objects. Maybe some GUI-stuff like popup-menus should also be added to this class?

@author Thomas Friedrichsmeier
*/
class RKObjectListView : public QListView {
	Q_OBJECT
public:
	RKObjectListView (QWidget *parent);
	
	~RKObjectListView ();

/** Takes care initializing the RKObjectListView (delayed, as the RObjectList may not have been created, yet) and of getting the current list of objects from the RObjectList if fetch_list is set to true*/
	void initialize (bool fetch_list);
/** @returns the RObject corresponding to the given RKListViewItem or 0 if no such item is known. */
	RObject *findItemObject (RKListViewItem *item);

/** This function returns a pointer to the context menu of the RKObjectListView. It is provided so you can add your own items.
@returns a pointer to the context menu
@see aboutToShowContextMenu */
	QPopupMenu *contextMenu () { return menu; };
/** This function returns the RObject the context menu has last been invoked on (or 0 if not invoked on an RObject). You can use this in slots called
from your custom menu items, to figure out, which object you should operate on. */
	RObject *menuObject () { return menu_object; };

	RKObjectListViewSettings *getSettings () { return settings; };
signals:
	void listChanged ();
/** This signal is emitted just before the context-menu is shown. If you connect to this signal, you can make some adjustments to the context-menu.
If you set *suppress to true, showing the context menu will be suppressed. */
	void aboutToShowContextMenu (RKListViewItem *item, bool *suppress);
public slots:
	void updateComplete ();
	void updateStarted ();

	void objectAdded (RObject *object);
	void objectRemoved (RObject *object);
	void objectPropertiesChanged (RObject *object);

	void objectBrowserSettingsChanged ();

	void requestedContextMenu (QListViewItem *item, const QPoint &pos, int col);
	
	virtual void popupConfigure ();
private:
// TODO: keep an additional map from RObject to RKListViewItem, in order to make this (often called) more efficient
	RKListViewItem *findObjectItem (RObject *object);
	void updateItem (RKListViewItem *item, RObject *object);

	void addObject (RKListViewItem *parent, RObject *object, bool recursive);

	typedef QMap<RKListViewItem *, RObject *> ObjectMap;
	ObjectMap object_map;

	bool update_in_progress;
	bool changes;

	QPopupMenu *menu;
	RObject *menu_object;

	RKObjectListViewSettings *settings;

	static QPixmap *icon_function;
	static QPixmap *icon_list;
	static QPixmap *package_environment;
	static QPixmap *environment;
};

/** This subclass of RKListViewItem reimplements the width ()-function to return 0 if the item is not currently visible. This is needed to get a sane column width in the listview. Also limit maximum default width to 200 px (TODO: make this configurable)

@author Thomas Friedrichsmeier
*/
class RKListViewItem : public QListViewItem {
public:
	RKListViewItem (QListView *parent) : QListViewItem (parent) {};
	RKListViewItem (QListViewItem *parent) : QListViewItem (parent) {};
	~RKListViewItem () {};

	int width (const QFontMetrics &fm, const QListView * lv, int c) const;
};

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

	bool shouldShowObject (RObject *object);

	QPopupMenu *showObjectsMenu () { return show_objects_menu; };
	QPopupMenu *showFieldsMenu () { return show_fields_menu; };
signals:
	void settingsChanged ();
public slots:
	void globalSettingsChanged ();
	void toggleSetting (int which);
private:
	State *settings;
	bool *settings_default;
	bool optionConfigurable (Settings setting);
	void insertPopupItem (QPopupMenu *menu, Settings setting, const QString &text);
	void createContextMenus ();
	void updateSelf ();

	QPopupMenu *show_objects_menu;
	QPopupMenu *show_fields_menu;
};

class RKObjectListViewSettingsWidget : public QWidget {
	Q_OBJECT
public:
	RKObjectListViewSettingsWidget (RKObjectListViewSettings *settings, QWidget *parent);
	~RKObjectListViewSettingsWidget ();
public slots:
	void settingsChanged ();
private:
	RKObjectListViewSettings *settings;
};

#endif
