/***************************************************************************
                          rkworkplaceview  -  description
                             -------------------
    begin                : Tue Sep 26 2006
    copyright            : (C) 2006, 2007, 2009, 2010 by Thomas Friedrichsmeier
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

#include "rkworkplaceview.h"

#include <klocale.h>
#include <kactioncollection.h>
#include <kdeversion.h>
#include <kacceleratormanager.h>

#include <qapplication.h>
#include <qevent.h>
#include <qlayout.h>
#include <QToolButton>
#include <QTabBar>
#include <QAction>
#include <QIcon>
#include <QMenu>
#include <QKeySequence>

#include "rkmdiwindow.h"
#include "rkworkplace.h"
#include "../misc/rkstandardicons.h"

#include "../debug.h"


RKWorkplaceView::RKWorkplaceView (QWidget *parent) : QTabWidget (parent) {
	RK_TRACE (APP);

	// close button(s)
	QToolButton* close_button = new QToolButton (this);
	close_button->setIcon (QIcon::fromTheme("tab-close"));
	connect (close_button, &QToolButton::clicked, this, &RKWorkplaceView::closeCurrentPage);
	close_button->adjustSize ();
	setCornerWidget (close_button, Qt::TopRightCorner);

	setTabsClosable (true);
	connect (this, &QTabWidget::tabCloseRequested, this, static_cast<void (RKWorkplaceView::*)(int)>(&RKWorkplaceView::closePage));

	setMovable (true);

	tabBar ()->setContextMenuPolicy (Qt::CustomContextMenu);
	connect (tabBar (), &QWidget::customContextMenuRequested, this, &RKWorkplaceView::showContextMenu);

	KAcceleratorManager::setNoAccel (tabBar ());	// TODO: This is a WORKAROUND for a bug in kdelibs where tabs named "a0.txt", "a1.txt", etc. will steal the Alt+0/1... shortcuts
	tabBar ()->hide ();		// initially
	connect (this, &QTabWidget::currentChanged, this, &RKWorkplaceView::currentPageChanged);
}

RKWorkplaceView::~RKWorkplaceView () {
	RK_TRACE (APP);
}

void RKWorkplaceView::initActions (KActionCollection *ac, const char *id_left, const char *id_right) {
	RK_TRACE (APP);

	action_page_left = (QAction *) ac->addAction (id_left, this, SLOT (pageLeft()));
	action_page_left->setText (i18n ("Window Left"));
	ac->setDefaultShortcuts (action_page_left, QList<QKeySequence>() << Qt::ControlModifier + Qt::Key_Less << Qt::ControlModifier + Qt::Key_Comma);

	action_page_right = (QAction *) ac->addAction (id_right, this, SLOT (pageRight()));
	action_page_right->setText (i18n ("Window Right"));
	ac->setDefaultShortcuts (action_page_right, QList<QKeySequence>() << Qt::ControlModifier + Qt::Key_Greater << Qt::ControlModifier + Qt::Key_Period);

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

void RKWorkplaceView::addWindow (RKMDIWindow *widget) {
	RK_TRACE (APP);

	int id = -1;

	QIcon icon = widget->windowIcon ();
	if (icon.isNull ()) icon = widget->topLevelWidget ()->windowIcon ();
	if (icon.isNull ()) RK_ASSERT (false);

	id = addTab (widget, icon, widget->shortCaption ());

	connect (widget, &RKMDIWindow::captionChanged, this, &RKWorkplaceView::childCaptionChanged);
	widget->show ();

	if (count () > 1) tabBar ()->show ();

	setCurrentIndex (id);		// activate the new tab
}

bool RKWorkplaceView::hasWindow (RKMDIWindow *widget) {
	return (indexOf (widget) != -1);
}

void RKWorkplaceView::removeWindow (RKMDIWindow *widget, bool destroyed) {
	RK_TRACE (APP);

	int id = indexOf (widget);		// which page is it?
	if (id == -1) RK_DEBUG (APP, DL_WARNING, "did not find page in RKWorkplaceView::removeWindow");
	if (!destroyed) disconnect (widget, &RKMDIWindow::captionChanged, this, &RKWorkplaceView::childCaptionChanged);

	removeTab (id);
	if (count () <= 1) tabBar ()->hide ();
}

RKMDIWindow *RKWorkplaceView::activePage () {
	RK_TRACE (APP);

	QWidget *w = currentWidget ();
	return (dynamic_cast<RKMDIWindow *> (w));
}

void RKWorkplaceView::closeCurrentPage () {
	RK_TRACE (APP);

	RKMDIWindow* w = activePage ();

	if (!w) {
		RK_ASSERT (false);	// the close button should not be visible, if there are no pages
		return;
	}

	w->close (true);
}

void RKWorkplaceView::closePage (QWidget* page) {
	RK_TRACE (APP);

	if (!page) {
		RK_ASSERT (false);
		return;
	}
	static_cast<RKMDIWindow*>(page)->close (true);
}

void RKWorkplaceView::closePage (int page) {
	RK_TRACE (APP);

	closePage (widget (page));
}

void RKWorkplaceView::showContextMenu (const QPoint &pos) {
	RK_TRACE (APP);

	int tab = tabBar ()->tabAt (pos);
	if (tab < 0) return;	// no context menu for the empty area

	QMenu* m = new QMenu (this);
	QAction *action = KStandardAction::close (this, SLOT (contextMenuClosePage()), this);
	action->setData (tab);
	m->addAction (action);
	action = m->addAction (RKStandardIcons::getIcon (RKStandardIcons::ActionDetachWindow), i18n("Detach"), this, SLOT (contextMenuDetachWindow()));
	action->setData (tab);
	m->exec (mapToGlobal (pos));
	delete m;
}

void RKWorkplaceView::contextMenuClosePage () {
	RK_TRACE (APP);

	QAction* action = dynamic_cast<QAction*> (sender ());
	if (!action) {
		RK_ASSERT (false);
		return;
	}

	int tab = action->data ().toInt ();
	RK_ASSERT (tab >= 0);
	closePage (tab);
}

void RKWorkplaceView::contextMenuDetachWindow () {
	RK_TRACE (APP);

	QAction* action = dynamic_cast<QAction*> (sender ());
	if (!action) {
		RK_ASSERT (false);
		return;
	}

	int tab = action->data ().toInt ();
	RK_ASSERT (tab >= 0);
	RKWorkplace::mainWorkplace ()->detachWindow (static_cast<RKMDIWindow*> (widget (tab)));
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

	QWidget::setWindowTitle (caption);
	emit (captionChanged (caption));
}

void RKWorkplaceView::currentPageChanged (int) {
	RK_TRACE (APP);

	RKMDIWindow *w = activePage ();
	if (w) {
		setCaption (w->shortCaption ());
		w->activate ();		// not always automatically active
	} else {
		setCaption (QString ());
	}
	updateActions ();
}


