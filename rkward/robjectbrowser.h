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
	
	enum PopupItems { Edit=1, View=2, Delete=3 };
	
public slots:
	void updateButtonClicked ();
	void updateComplete (bool changed);
	void requestedContextMenu (QListViewItem *item, const QPoint &pos, int col);
	
	void popupEdit ();
	void popupView ();
	void popupDelete ();
private:
	friend class RKwardApp;
	void initialize ();

	QPushButton *update_button;
	QListView *list_view;
	
	QMap<QListViewItem *, RObject *> object_map;
	
	void addObject (QListViewItem *parent, RObject *object);
	
	QPopupMenu *menu;
	/// the object the menu was invoked on
	RObject *menu_object;
};

#endif
