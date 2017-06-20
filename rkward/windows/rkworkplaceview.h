/***************************************************************************
                          rkworkplaceview  -  description
                             -------------------
    begin                : Tue Sep 26 2006
    copyright            : (C) 2006 - 2017 by Thomas Friedrichsmeier
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

#ifndef RKWORKPLACEVIEW_H
#define RKWORKPLACEVIEW_H

#include <QTabWidget>
#include <QSplitter>

class RKMDIWindow;
class QAction;
class KActionCollection;
class RKWorkplaceView;

class RKWorkplaceViewPane : public QTabWidget {
	Q_OBJECT
friend class RKWorkplaceView;
private:
	explicit RKWorkplaceViewPane (RKWorkplaceView *parent);
	~RKWorkplaceViewPane ();
	RKWorkplaceView* workplace_view;
/** Close a page given the correspoding widget */
	void closePage (QWidget* page);
/** Close a page given its index */
	void closePage (int page);
/** (Attempts to) close all tabs in this pane (and thus the pane itself) */
	void closeAll ();
	bool isActive ();
	void initActions ();
protected:
	void tabRemoved (int index) override;
	void tabInserted (int index) override;
	bool eventFilter (QObject* obj, QEvent* event) override;
signals:
	void becameEmpty (RKWorkplaceViewPane* pane);
private slots:
/** handle context menu requests */
	void showContextMenu (const QPoint &pos);
/** handle close request from context menu */
	void contextMenuClosePage ();
/** handle detach request from context menu */
	void contextMenuDetachWindow ();
/** Internal function to ensure proper focus and update caption, when the current page has changed. */
	void currentPageChanged (int page);
};

/** This is mostly a QTabWidget with some extras such as updating the caption, a context menu, etc.
 */

class RKWorkplaceView : public QSplitter {
	Q_OBJECT
friend class RKWorkplaceViewPane;
public:
/** constructor
@param parent parent QWidget */
	explicit RKWorkplaceView (QWidget *parent);
	~RKWorkplaceView ();

/** add the given window to the view */
	void addWindow (RKMDIWindow *widget);
/** remove the given window to the view
@param destroyed if the window is already destroyed, set this to true */
	void removeWindow (RKMDIWindow *widget, bool destroyed=false);
/** does this window exist in the view? */
	bool hasWindow (RKMDIWindow *widget) const;
/** show the given page (does not set focus) */
	void showWindow (RKMDIWindow *widget);
/** Returns true if the given window is in the active pane of this view. */
	bool windowInActivePane (RKMDIWindow *widget) const;

/** @returns the currently active window */
	RKMDIWindow *activePage () const;
/** reimplemented form QWidget::setCaption () to emit captionChanged () when the caption changes */
	void setCaption (const QString &caption);
/** initialize the window left/right actions */
	void initActions (KActionCollection *ac);
signals:
/** a new page / window was activated
@param widget the newly activated window */
	void pageChanged (RKMDIWindow *widget);
/** caption has changed
@param new_caption the new caption */
	void captionChanged (const QString &new_caption);
private slots:
/** called when the caption of a window changes. Updates the tab-label, and - if appropriate - the caption of this widget */
	void childCaptionChanged (RKMDIWindow *widget);
/** Active the page left of the current tab */
	void pageLeft ();
/** Active the page right of the current tab */
	void pageRight ();
/** Purge the given pane (if it is empty) */
	void purgePane (RKWorkplaceViewPane *pane);
/** Split current view vertically */
	void splitViewVert ();
/** Split current view horizontally */
	void splitViewHoriz ();
private:
	void updateActions ();
	void splitView (Qt::Orientation horiz, RKWorkplaceViewPane *pane);
	RKWorkplaceViewPane *createPane ();
	RKWorkplaceViewPane *findWindow (RKMDIWindow *window) const;

	QAction *action_page_left;
	QAction *action_page_right;
	QAction *action_split_horiz;
	QAction *action_split_vert;

	QList<RKWorkplaceViewPane*> panes;
	RKWorkplaceViewPane *activePane () const;
/** Newly added pane. Pointer needed so the first "new" window will go here. */
	RKWorkplaceViewPane *newpane;
};

#endif
