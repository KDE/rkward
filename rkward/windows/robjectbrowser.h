/*
robjectbrowser - This file is part of the RKWard project. Created: Thu Aug 19 2004
SPDX-FileCopyrightText: 2004-2022 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef ROBJECTBROWSER_H
#define ROBJECTBROWSER_H

#include "rkmdiwindow.h"

#include <QModelIndex>
#include <QFocusEvent>

#include "rkcommandeditorwindow.h"

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
	RObjectBrowser(QWidget *parent, bool tool_window, const char *name=nullptr);
	~RObjectBrowser();
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
class RObjectBrowserInternal : public QWidget, public RKScriptContextProvider {
Q_OBJECT
public:
	explicit RObjectBrowserInternal (QWidget *parent, RObjectBrowser *browser);
	~RObjectBrowserInternal ();
private Q_SLOTS:
	void updateButtonClicked ();
	void contextMenuCallback (RObject *object, bool *suppress);

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
	void currentHelpContext (QString *symbol, QString *package) override;
private:
	enum PopupActions {
		Help=0,
		SearchOnline,
		Edit,
		View,
		Rename,
		Copy,
		CopyToGlobalEnv,
		Delete,
		NewFromClipboard,
		Unload,
		LoadUnloadPackages,
		ActionCount
	};
	QList<QAction*> actions;

	QPushButton *update_button;
	RKObjectListView *list_view;
};

#endif
