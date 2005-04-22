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
	RObject *findItemObject (QListViewItem *item);
signals:
	void listChanged ();
public slots:
	void updateComplete ();
	void updateStarted ();
	
	void objectAdded (RObject *object);
	void objectRemoved (RObject *object);
	void objectPropertiesChanged (RObject *object);

	void objectBrowserSettingsChanged ();
private:
// TODO: keep an additional map from RObject to QListViewItem, in order to make this (often called) more efficient
	QListViewItem *findObjectItem (RObject *object);
	void updateItem (QListViewItem *item, RObject *object);

	void addObject (QListViewItem *parent, RObject *object, bool recursive);

	typedef QMap<QListViewItem *, RObject *> ObjectMap;
	ObjectMap object_map;

	bool update_in_progress;
	bool changes;
};

#endif
