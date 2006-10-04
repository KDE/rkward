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

#include <qwidget.h>

class RKObjectListView;
class RKListViewItem;
class QListViewItem;
class QPushButton;
class QPopupMenu;
class RObject;
class RKCommandEditorWindow;


/**
This widget provides a browsable list of all objects in the R workspace

@author Thomas Friedrichsmeier
*/
class RObjectBrowser : public QWidget {
Q_OBJECT
public:
    RObjectBrowser ();

    ~RObjectBrowser ();
	
	enum PopupItems { Edit=1, View=2, Rename=3, Copy=4, CopyToGlobalEnv=5, Delete=6 };
	
public slots:
	void updateButtonClicked ();
	void contextMenuCallback (RKListViewItem *item, bool *suppress);
	
	void popupEdit ();
	void popupCopy ();
/** essentially like popupCopy, but does not ask for a name */
	void popupCopyToGlobalEnv ();
	void popupView ();
	void popupDelete ();
	void popupRename ();
/** when an object in the list is double clicked, insert its name in the current RKCommandEditor window */
	void slotListDoubleClicked (QListViewItem *item, const QPoint &pos, int);
private:
	friend class RKwardApp;
	void initialize ();

	QPushButton *update_button;
	RKObjectListView *list_view;	
};

#endif
