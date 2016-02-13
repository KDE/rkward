/***************************************************************************
                          robjectbrowser  -  description
                             -------------------
    begin                : Thu Aug 19 2004
    copyright            : (C) 2004 - 2015 by Thomas Friedrichsmeier
    email                : thomas.friedrichsmeier@kdemail.net
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

#include "rkmdiwindow.h"

#include <QModelIndex>
#include <QFocusEvent>

class RKObjectListView;
class RKObjectListViewSettings;
class QPushButton;
class RObject;
class RObjectBrowserInternal;

/**
This widget provides a browsable list of all objects in the R workspace

Note: Most actual functionality is realized in RObjectBrowserInternal, which is created as soon as the RObjectBrowser is shown for the first time.

@author Thomas Friedrichsmeier
*/
class RObjectBrowser : public RKMDIWindow {
public:
	RObjectBrowser (QWidget *parent, bool tool_window, const char *name=0);
	~RObjectBrowser ();
	void unlock ();
	static RObjectBrowser *mainBrowser () { return object_browser; };
/** reimplemented to create the real file browser widget only when the file browser is shown for the first time */
	void showEvent (QShowEvent *e) override;
private:
	RObjectBrowserInternal *internal;
	QWidget *layout_widget;

	bool locked;
	friend class RKWardMainWindow;
	static RObjectBrowser *object_browser;
	void initialize ();
};

/**
Provides most of the functionality of RObjectBrowser

@author Thomas Friedrichsmeier
*/
class RObjectBrowserInternal : public QWidget {
Q_OBJECT
public:
	explicit RObjectBrowserInternal (QWidget *parent);
	~RObjectBrowserInternal ();
private slots:
	void updateButtonClicked ();
	void contextMenuCallback (RObject *object, bool *suppress);
	
	void popupHelp ();
	void popupEdit ();
	void popupCopy ();
/** essentially like popupCopy, but does not ask for a name */
	void popupCopyToGlobalEnv ();
	void popupView ();
	void popupDelete ();
	void popupUnload ();
	void popupRename ();
/** when an object in the list is double clicked, insert its name in the current RKCommandEditor window */
	void doubleClicked (const QModelIndex &index);
protected:
/** reimplemnented from QWidget to make show the globalenv object when activated (other than by mouse click) */
	void focusInEvent (QFocusEvent *e) override;
private:
	enum PopupActions {
		Help=0,
		Edit,
		View,
		Rename,
		Copy,
		CopyToGlobalEnv,
		Delete,
		Unload,
		LoadUnloadPackages,
		ActionCount
	};
	QList<QAction*> actions;

	QPushButton *update_button;
	RKObjectListView *list_view;
};

#endif
