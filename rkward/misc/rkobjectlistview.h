/***************************************************************************
                          rkobjectlistview  -  description
                             -------------------
    begin                : Wed Sep 1 2004
    copyright            : (C) 2004 by Thomas Friedrichsmeier
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
class QPopupMenu;

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
/** @returns the RObject corresponding to the given QListViewItem or 0 if no such item is known. */
	RObject *findItemObject (QListViewItem *item);

/** This function returns a pointer to the context menu of the RKObjectListView. It is provided so you can add your own items.
@returns a pointer to the context menu
@see aboutToShowContextMenu */
	QPopupMenu *contextMenu () { return menu; };
/** This function returns the RObject the context menu has last been invoked on (or 0 if not invoked on an RObject). You can use this in slots called
from your custom menu items, to figure out, which object you should operate on. */
	RObject *menuObject () { return menu_object; };
signals:
	void listChanged ();
/** This signal is emitted just before the context-menu is shown. If you connect to this signal, you can make some adjustments to the context-menu.
If you set *suppress to true, showing the context menu will be suppressed. */
	void aboutToShowContextMenu (QListViewItem *item, bool *suppress);
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
// TODO: keep an additional map from RObject to QListViewItem, in order to make this (often called) more efficient
	QListViewItem *findObjectItem (RObject *object);
	void updateItem (QListViewItem *item, RObject *object);

	void addObject (QListViewItem *parent, RObject *object, bool recursive);

	typedef QMap<QListViewItem *, RObject *> ObjectMap;
	ObjectMap object_map;

	bool update_in_progress;
	bool changes;

	QPopupMenu *menu;
	RObject *menu_object;
};

#endif
