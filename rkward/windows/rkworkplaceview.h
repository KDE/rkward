/***************************************************************************
                          rkworkplaceview  -  description
                             -------------------
    begin                : Tue Sep 26 2006
    copyright            : (C) 2006, 2007 by Thomas Friedrichsmeier
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

#ifndef RKWORKPLACEVIEW_H
#define RKWORKPLACEVIEW_H

#include <ktabwidget.h>

class RKMDIWindow;
class KAction;
class KActionCollection;

/** The widget containing all the MDI document windows. Right now it mostly acts as a QTabWidget (is not one, as unfortunately, QTabWidget always has a sunken frame), but might be extended to allow switching between different view modes

KDE4 TODO: We now use KTabWidget natively, hoping, we can avoid the sunken frame problem somehow. We may need to revisit this decision, later. TODO cleanup
 */

class RKWorkplaceView : public KTabWidget {
	Q_OBJECT
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
	bool hasWindow (RKMDIWindow *widget);

/** activate the given window */
	void setActivePage (RKMDIWindow *widget);
/** @returns the currently active window */
	RKMDIWindow *activePage ();
/** reimplemented form QWidget::setCaption () to emit captionChanged () when the caption changes */
	void setCaption (const QString &caption);
/** initialize the window left/right actions */
	void initActions (KActionCollection *ac, const char *id_left, const char *id_right);
signals:
/** a new page / window was activated
@param widget the newly activated window */
	void pageChanged (RKMDIWindow *widget);
/** caption has changed
@param new_caption the new caption */
	void captionChanged (const QString &new_caption);
public slots:
	void currentPageChanged (int page);
/** called when the caption of a window changes. Updates the tab-label, and - if appropriate - the caption of this widget */
	void childCaptionChanged (RKMDIWindow *widget);
/** Active the page left of the current tab */
	void pageLeft ();
/** Active the page right of the current tab */
	void pageRight ();
private slots:
/** (Attempts to) close the current tab */
	void closeCurrentPage ();
private:
	void updateActions ();

	KAction *action_page_left;
	KAction *action_page_right;
};

#endif
