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

#include "rkworkplaceview.h"

#include <ktabbar.h>
#include <klocale.h>
#include <kshortcut.h>
#include <kactioncollection.h>
#include <kaction.h>

#include <qapplication.h>
#include <qevent.h>
#include <qlayout.h>
#include <qicon.h>

#include "rkmdiwindow.h"

#include "../debug.h"


RKWorkplaceView::RKWorkplaceView (QWidget *parent) : KTabWidget (parent) {
	RK_TRACE (APP);

	setHoverCloseButtonDelayed (true);
	setTabBarHidden (true);		// initially
	connect (this, SIGNAL (currentChanged(int)), this, SLOT (currentPageChanged(int)));
}

RKWorkplaceView::~RKWorkplaceView () {
	RK_TRACE (APP);
}

void RKWorkplaceView::initActions (KActionCollection *ac, const char *id_left, const char *id_right) {
	RK_TRACE (APP);

	action_page_left = (KAction*) ac->addAction (id_left, this, SLOT (pageLeft()));
	action_page_left->setText (i18n ("Window Left"));
	action_page_left->setShortcut (KShortcut (Qt::ControlModifier + Qt::Key_Less, Qt::ControlModifier + Qt::Key_Comma));

	action_page_right = (KAction*) ac->addAction (id_right, this, SLOT (pageRight()));
	action_page_right->setText (i18n ("Window Right"));
	action_page_right->setShortcut (KShortcut (Qt::ControlModifier + Qt::Key_Greater, Qt::ControlModifier + Qt::Key_Period));

	updateActions ();
}

void RKWorkplaceView::updateActions () {
	RK_TRACE (APP);

	int index = currentIndex ();
	action_page_left->setEnabled (index > 0);
	action_page_right->setEnabled (index < (count () - 1));
}

void RKWorkplaceView::pageLeft () {
	RK_TRACE (APP);

	int index = currentIndex ();
	RK_ASSERT (index > 0);
	setCurrentIndex (index - 1);
}

void RKWorkplaceView::pageRight () {
	RK_TRACE (APP);

	int index = currentIndex ();
	RK_ASSERT (index < (count () - 1));
	setCurrentIndex (index + 1);
}

void RKWorkplaceView::addPage (RKMDIWindow *widget) {
	RK_TRACE (APP);

	int id = -1;
	setUpdatesEnabled (false);
	if (widget->icon ()) {
		id = addTab (widget, *(widget->icon ()), widget->shortCaption ());
	} else if (widget->topLevelWidget ()->icon ()) {
		id = addTab (widget, *(widget->topLevelWidget ()->icon ()), widget->shortCaption ());
	} else {
		RK_ASSERT (false);
		id = addTab (widget, widget->shortCaption ());
	}
	connect (widget, SIGNAL (captionChanged (RKMDIWindow *)), this, SLOT (childCaptionChanged (RKMDIWindow *)));
	widget->show ();

	if (count () > 1) setTabBarHidden (false);

	setCurrentIndex (id);		// activate the new tab

	setUpdatesEnabled (true);
}

bool RKWorkplaceView::hasPage (RKMDIWindow *widget) {
	return (indexOf (widget) != -1);
}

void RKWorkplaceView::removePage (RKMDIWindow *widget, bool destroyed) {
	RK_TRACE (APP);

	setUpdatesEnabled (false);

	int id = indexOf (widget);		// which page is it?
	RK_DO (if (id == -1) qDebug ("did not find page in RKWorkplaceView::removePage"), APP, DL_WARNING);
	if (!destroyed) disconnect (widget, SIGNAL (captionChanged (RKMDIWindow *)), this, SLOT (childCaptionChanged (RKMDIWindow *)));

	removeTab (id);
	int new_count = count ();
	if (new_count <= 1) {
		setTabBarHidden (true);
		if (new_count < 1) {
			// KDE4: is this still needed?
			setCaption (QString ());
			emit (currentChanged (-1));
		}
	}

	setUpdatesEnabled (true);
}

// KDE4 TODO: we can use setCurrentWidget, instead.
void RKWorkplaceView::setActivePage (RKMDIWindow *widget) {
	RK_TRACE (APP);

	int id = indexOf (widget);
	RK_DO (if (id == -1) qDebug ("did not find page in RKWorkplaceView::setActivePage"), APP, DL_WARNING);

	setCurrentIndex (id);
}

RKMDIWindow *RKWorkplaceView::activePage () {
	RK_TRACE (APP);

	QWidget *w = currentWidget ();
	return (dynamic_cast<RKMDIWindow *> (w));
}

// KDE4: Where is this called from? do we still need it?
void RKWorkplaceView::closePage (int index) {
	RK_TRACE (APP);

	QWidget *w = widget (index);
	RK_ASSERT (w);

	w->close (true);
}

void RKWorkplaceView::childCaptionChanged (RKMDIWindow *widget) {
	RK_TRACE (APP);

	int id = indexOf (widget);
	RK_ASSERT (id >= 0);
	setTabText (id, widget->shortCaption ());
	if (id == currentIndex ()) setCaption (widget->shortCaption ());
}

void RKWorkplaceView::setCaption (const QString &caption) {
	RK_TRACE (APP);

	QWidget::setCaption (caption);
	emit (captionChanged (caption));
}

void RKWorkplaceView::currentPageChanged (int) {
	RK_TRACE (APP);

	RKMDIWindow *w = activePage ();
	if (w) {
		setCaption (w->shortCaption ());
	} else {
		setCaption (QString ());
	}
}


#include "rkworkplaceview.moc"
