/***************************************************************************
                          robjectbrowser  -  description
                             -------------------
    begin                : Thu Aug 19 2004
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
#ifndef ROBJECTBROWSER_H
#define ROBJECTBROWSER_H

#include "misc/rktogglewidget.h"

#include <qmap.h>

class QListView;
class QListViewItem;
class QPushButton;
class QPopupMenu;
class RObject;

/**
This widget provides a browsable list of all objects in the R workspace

@author Thomas Friedrichsmeier
*/
class RObjectBrowser : public RKToggleWidget
{
Q_OBJECT
public:
    RObjectBrowser ();

    ~RObjectBrowser ();
	
	enum PopupItems { Edit=1, View=2, Rename=3, Delete=4 };
	
public slots:
	void updateButtonClicked ();
	void updateComplete ();
	void requestedContextMenu (QListViewItem *item, const QPoint &pos, int col);
	
	void popupEdit ();
	void popupView ();
	void popupDelete ();
	void popupRename ();
	
	void objectAdded (RObject *object);
	void objectRemoved (RObject *object);
	void objectPropertiesChanged (RObject *object);
private:
	friend class RKwardApp;
	void initialize ();
// TODO: keep an additional map from RObject to QListViewItem, in order to make this (often called) more efficient
	QListViewItem *findObjectItem (RObject *object);
	void updateItem (QListViewItem *item, RObject *object);

	QPushButton *update_button;
	QListView *list_view;
	
	typedef QMap<QListViewItem *, RObject *> ObjectMap;
	ObjectMap object_map;
	
	void addObject (QListViewItem *parent, RObject *object);
	
	QPopupMenu *menu;
	/// the object the menu was invoked on
	RObject *menu_object;
};

#endif
