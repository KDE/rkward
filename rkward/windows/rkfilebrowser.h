/***************************************************************************
                          rkfilebrowser  -  description
                             -------------------
    begin                : Thu Apr 26 2007
    copyright            : (C) 2007-2016 by Thomas Friedrichsmeier
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

#ifndef RKFILEBROWSER_H
#define RKFILEBROWSER_H

#include "rkmdiwindow.h"

#include <QList>
#include <QUrl>

class KDirOperator;
class RKFileBrowserWidget;
class KUrlComboBox;
class KFileItem;
class KFileItemActions;
class QMenu;
class QAction;

/** The file browser (tool) window. In order to save some startup time, the file browser is not really created until it is first shown. Hence, this is mostly just a wrapper around RKFileBrowserWidget */
class RKFileBrowser : public RKMDIWindow {
	Q_OBJECT
public:
	RKFileBrowser (QWidget *parent, bool tool_window, const char *name=0);
	~RKFileBrowser ();

/** reimplemented to create the real file browser widget only when the file browser is shown for the first time */
	void showEvent (QShowEvent *e) override;
	static RKFileBrowser *getMainBrowser() { return main_browser; };
public slots:
	void currentWDChanged ();
private:
	RKFileBrowserWidget *real_widget;
	QWidget *layout_widget;
friend class RKWardMainWindow;
	static RKFileBrowser *main_browser;
};

/** The internal widget used in RKFileBrowser 
TODO: KDE4: check whether there is a ready widget for this. Much of the implementation is a modified copy from Kate / kdevelop.
*/
class RKFileBrowserWidget : public QWidget {
	Q_OBJECT
public:
	explicit RKFileBrowserWidget (QWidget *widget);
	~RKFileBrowserWidget ();

	bool eventFilter (QObject* o, QEvent* e) override;
public slots:
	void setURL (const QUrl &url);
	void urlChangedInView (const QUrl &url);
	void stringChangedInCombo (const QString &url);
	void urlChangedInCombo (const QUrl &url);
	void fileActivated (const KFileItem& item);
	void saveConfig ();
	void contextMenuHook (const KFileItem &item, QMenu *menu);
	void rename ();
private:
	QList<QAction*> added_service_actions;
	KDirOperator *dir;
	KUrlComboBox *urlbox;
	KFileItemActions *fi_actions;
	QAction *rename_action;
	QUrl context_menu_url;
};

#endif
