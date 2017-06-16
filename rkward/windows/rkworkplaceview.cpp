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

#include "rkworkplaceview.h"

#include <KLocalizedString>
#include <kactioncollection.h>
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

RKWorkplaceViewPane::RKWorkplaceViewPane (RKWorkplaceView* parent) : QTabWidget (parent) {
	RK_TRACE (APP);

	workplace_view = parent;

	// close button(s)
	QToolButton* close_button = new QToolButton (this);
	close_button->setIcon (QIcon::fromTheme("tab-close"));
	connect (close_button, &QToolButton::clicked, this, &RKWorkplaceViewPane::closeCurrentPage);
	close_button->adjustSize ();
	setCornerWidget (close_button, Qt::TopRightCorner);

	setTabsClosable (true);
	connect (this, &QTabWidget::tabCloseRequested, this, static_cast<void (RKWorkplaceViewPane::*)(int)>(&RKWorkplaceViewPane::closePage));

	setMovable (true);

	tabBar ()->setContextMenuPolicy (Qt::CustomContextMenu);
	connect (tabBar (), &QWidget::customContextMenuRequested, this, &RKWorkplaceViewPane::showContextMenu);

	KAcceleratorManager::setNoAccel (tabBar ());	// TODO: This is a WORKAROUND for a bug in kdelibs where tabs named "a0.txt", "a1.txt", etc. will steal the Alt+0/1... shortcuts
	connect (this, &QTabWidget::currentChanged, workplace_view, &RKWorkplaceView::currentPageChanged);
}

RKWorkplaceViewPane::~RKWorkplaceViewPane () {
	RK_TRACE (APP);
}

bool RKWorkplaceViewPane::isActive () {
	RK_TRACE (APP);

	return (currentWidget () && static_cast<RKMDIWindow*> (currentWidget ())->isActiveInsideToplevelWindow ());
}

void RKWorkplaceViewPane::showContextMenu (const QPoint &pos) {
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

void RKWorkplaceViewPane::closeCurrentPage () {
	RK_TRACE (APP);

	closePage (currentWidget ());
}

void RKWorkplaceViewPane::closePage (int index) {
	RK_TRACE (APP);

	closePage (widget (index));
}

void RKWorkplaceViewPane::closePage (QWidget* page) {
	RK_TRACE (APP);

	if (!page) {
		RK_ASSERT (false);
		return;
	}
	static_cast<RKMDIWindow*>(page)->close (true);
}

void RKWorkplaceViewPane::tabRemoved (int index) {
	RK_TRACE (APP);
	QTabWidget::tabRemoved (index);
	if (count () < 1) emit (becameEmpty (this));
}

void RKWorkplaceViewPane::contextMenuClosePage () {
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

void RKWorkplaceViewPane::contextMenuDetachWindow () {
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



RKWorkplaceView::RKWorkplaceView (QWidget *parent) : QSplitter (parent) {
	RK_TRACE (APP);
	newPane (0);
}

RKWorkplaceView::~RKWorkplaceView () {
	RK_TRACE (APP);
}

RKWorkplaceViewPane* RKWorkplaceView::activePane () const {
	RK_TRACE (APP);

	for (int i = 0; i < panes.size (); ++i) {
		if (panes[i]->isActive ()) return panes[i];
	}
	if (!panes.isEmpty ()) return panes.first ();
	return 0;
}

void RKWorkplaceView::initActions (KActionCollection *ac) {
	RK_TRACE (APP);

	action_page_left = (QAction *) ac->addAction ("left_window", this, SLOT (pageLeft()));
	action_page_left->setText (i18n ("Window Left"));
	ac->setDefaultShortcuts (action_page_left, QList<QKeySequence>() << Qt::ControlModifier + Qt::Key_Less << Qt::ControlModifier + Qt::Key_Comma);

	action_page_right = (QAction *) ac->addAction ("right_window", this, SLOT (pageRight()));
	action_page_right->setText (i18n ("Window Right"));
	ac->setDefaultShortcuts (action_page_right, QList<QKeySequence>() << Qt::ControlModifier + Qt::Key_Greater << Qt::ControlModifier + Qt::Key_Period);

	updateActions ();
}

void RKWorkplaceView::updateActions () {
	RK_TRACE (APP);

	bool several_pages = panes.count () > 1 || (panes.count () > 0 && panes.first()->count () > 1);
	action_page_left->setEnabled (several_pages);
	action_page_right->setEnabled (several_pages);
}

void RKWorkplaceView::pageLeft () {
	RK_TRACE (APP);

	RKWorkplaceViewPane *current = activePane ();
	if (!current) {
		RK_ASSERT (current);  // can this happen? It should not, as long as the action does not get called on an empty workplace view
		return;
	}

	int index = current->currentIndex ();
	if (index > 0) {
		current->setCurrentIndex (index - 1);
	} else {
		int pindex = panes.indexOf (current);
		if (pindex > 0) --pindex;
		else pindex = panes.size () - 1;
		if (panes[pindex]->count () < 1) {
			RK_ASSERT (false); // it should have been purged.
			return;
		}
		panes[pindex]->setCurrentIndex (panes[pindex]->count () - 1);
	}
}

void RKWorkplaceView::pageRight () {
	RK_TRACE (APP);

	RKWorkplaceViewPane *current = activePane ();
	if (!current) {
		RK_ASSERT (current);  // can this happen? It should not, as long as the action does not get called on an empty workplace view
		return;
	}

	int index = current->currentIndex ();
	if (index < current->count () - 1) {
		current->setCurrentIndex (index + 1);
	} else {
		int pindex = panes.indexOf (current);
		if (pindex < panes.count () - 1) ++pindex;
		else pindex = 0;
		if (panes[pindex]->count () < 1) {
			RK_ASSERT (false); // it should have been purged.
			return;
		}
		panes[pindex]->setCurrentIndex (0);
	}
}

RKWorkplaceViewPane* RKWorkplaceView::newPane (int index) {
	RK_TRACE (APP);

	if (index < 0) index = count ();
	RKWorkplaceViewPane *pane = new RKWorkplaceViewPane (this);
	addWidget (pane);
	connect (pane, &RKWorkplaceViewPane::becameEmpty, this, &RKWorkplaceView::purgePane);
	panes.append (pane);
	return pane;
}

void RKWorkplaceView::addWindow (RKMDIWindow *widget) {
	RK_TRACE (APP);

	int id = -1;

	QIcon icon = widget->windowIcon ();
	if (icon.isNull ()) icon = widget->topLevelWidget ()->windowIcon ();
	if (icon.isNull ()) RK_ASSERT (false);

	RKWorkplaceViewPane *pane = activePane ();
	if (!pane) {
		RK_ASSERT (count () == 0);
		pane = newPane (0);
	}
	id = pane->addTab (widget, icon, widget->shortCaption ());

	connect (widget, &RKMDIWindow::captionChanged, this, &RKWorkplaceView::childCaptionChanged);
	widget->show ();

	pane->setCurrentIndex (id);		// activate the new tab
}

void RKWorkplaceView::showWindow (RKMDIWindow *widget) {
	RK_TRACE (APP);

	RKWorkplaceViewPane *pane = findWindow (widget);
	pane->setCurrentIndex (pane->indexOf (widget));
}

RKWorkplaceViewPane* RKWorkplaceView::findWindow (RKMDIWindow *widget) const {
	for (int i = 0; i < panes.size (); ++i) {
		if (panes[i]->indexOf (widget) > -1) return panes[i];
	}
	return 0;
}

bool RKWorkplaceView::hasWindow (RKMDIWindow *widget) const {
	return (findWindow (widget) != 0);
}

void RKWorkplaceView::removeWindow (RKMDIWindow *widget, bool destroyed) {
	RK_TRACE (APP);

	RKWorkplaceViewPane *pane = findWindow (widget);
	int id = pane ? pane->indexOf (widget) : -1;		// which page is it?
	if (id == -1) RK_DEBUG (APP, DL_WARNING, "did not find page in RKWorkplaceView::removeWindow");
	if (!destroyed) disconnect (widget, &RKMDIWindow::captionChanged, this, &RKWorkplaceView::childCaptionChanged);

	if (pane) pane->removeTab (id);
}

void RKWorkplaceView::purgePane (RKWorkplaceViewPane* pane) {
	RK_TRACE (APP);
	RK_ASSERT (pane);
	if (pane->count () > 0) return;
	if (count () == 1 && pane->parentWidget () == this) return;  // keep at least one pane around for layout purposes

	QSplitter* split = static_cast<QSplitter*> (pane->parentWidget ());
	pane->hide ();
	pane->setParent (0); // TODO: needed?
	pane->deleteLater ();
	while (split != this && split->count () < 1) {
		QSplitter* p = static_cast<QSplitter*> (split->parentWidget ());
		delete (split);
		split = p;
	}
	bool removed = panes.removeAll (pane) > 0;
	RK_ASSERT (removed);
}

RKMDIWindow *RKWorkplaceView::activePage () const {
	RK_TRACE (APP);

	RKWorkplaceViewPane *pane = activePane ();
	if (pane) return (dynamic_cast<RKMDIWindow *> (pane->currentWidget ()));
	return 0;
}

void RKWorkplaceView::childCaptionChanged (RKMDIWindow *widget) {
	RK_TRACE (APP);

	RKWorkplaceViewPane *pane = findWindow (widget);
	if (!pane)  {
		RK_ASSERT (pane);
		return;
	}
	int id = pane->indexOf (widget);
	RK_ASSERT (id >= 0);
	pane->setTabText (id, widget->shortCaption ());
	if (id == pane->currentIndex ()) setCaption (widget->shortCaption ());
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


