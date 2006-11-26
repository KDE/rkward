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
class RKObjectListViewSettings;
class RKListViewItem;
class QListViewItem;
class QPushButton;
class QRadioButton;
class QButtonGroup;
class QCheckBox;
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
	
	enum PopupItems { Help=1, Edit=2, View=3, Rename=4, Copy=5, CopyToGlobalEnv=6, Delete=7 };
	
public slots:
	void updateButtonClicked ();
	void contextMenuCallback (RKListViewItem *item, bool *suppress);
	
	void popupHelp ();
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
	friend class RKWardMainWindow;
	void initialize ();

	QPushButton *update_button;
	RKObjectListView *list_view;	
};

/** This class provides a widget to switch quickly between the most important RKObjectListViewSettings */
class RKObjectListViewSettingsWidget : public QWidget {
	Q_OBJECT
public:
	RKObjectListViewSettingsWidget (RKObjectListViewSettings *settings, QWidget *parent);
	~RKObjectListViewSettingsWidget ();

	enum Modes {
		All = 0,
		NonFunctions = 1,
		Functions = 2
	};
public slots:
	void settingsChanged ();
	void modeActivated (int mode);
	void boxChanged (int);
private:
	QButtonGroup *group;
	QRadioButton *all;
	QRadioButton *nonfunctions;
	QRadioButton *functions;
	QCheckBox *all_envirs;
	QCheckBox *hidden_objects;
	RKObjectListViewSettings *settings;
};

#endif
