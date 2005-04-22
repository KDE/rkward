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

class RKObjectListView;
class QListViewItem;
class QPushButton;
class QPopupMenu;
class RObject;
class RKCommandEditorWindow;


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
	void contextMenuCallback (QListViewItem *item, bool *suppress);
	
	void popupEdit ();
	void popupView ();
	void popupDelete ();
	void popupRename ();
/** when an object in the list is double clicked, insert its name in the current RKCommandEditor windo */
    void slotListDoubleClicked(QListViewItem *item, const QPoint &pos, int);
private:
	friend class RKwardApp;
	void initialize ();

	QPushButton *update_button;
	RKObjectListView *list_view;	
};

#endif
