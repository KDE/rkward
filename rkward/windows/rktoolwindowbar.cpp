/***************************************************************************
                          rktoolwindowbar  -  description
                             -------------------
    begin                : Fri Oct 12 2007
    copyright            : (C) 2007 by Thomas Friedrichsmeier
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

/* This code is based substantially on kate's katemdi! */

#include "rktoolwindowbar.h"

#include <khbox.h>
#include <kmenu.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kicon.h>

#include <QSplitter>

#include "rkworkplace.h"
#include "rkworkplaceview.h"
#include "rkmdiwindow.h"

#include "../debug.h"

#define CHANGE_ATTACHMENT_ACTION_ID 10

RKToolWindowBar::RKToolWindowBar (KMultiTabBarPosition position, QWidget *parent) : KMultiTabBar (position, parent),
	container (0) {
	RK_TRACE (APP);
}

RKToolWindowBar::~RKToolWindowBar () {
	RK_TRACE (APP);
}

void RKToolWindowBar::setSplitter (QSplitter *splitter) {
	RK_TRACE (APP);
	RK_ASSERT (!container);

	container = new KHBox (splitter);
	container->layout ()->setSpacing (0);
	container->layout ()->setMargin (0);
	container->hide ();
}

void RKToolWindowBar::addWidget (RKMDIWindow *window) {
	RK_TRACE (APP);
	RK_ASSERT (window);
	RK_ASSERT (container);

	if (window->tool_window_bar) {
		RK_ASSERT (window->tool_window_bar != this);	// no problem, but would be useless code
		window->tool_window_bar->removeWidget (window);
	}

	int id = appendTab (window->windowIcon ().pixmap (QSize (16, 16)), -1, window->shortCaption ());

	window->tool_window_bar = this;
	widget_to_id.insert (window, id);

	connect (tab (id), SIGNAL (clicked(int)), this, SLOT (tabClicked(int)));
	tab (id)->installEventFilter (this);

	if (window->isAttached ()) {
		window->hide();
		window->setParent (container);
	}

	show ();
}

void RKToolWindowBar::removeWidget (RKMDIWindow *widget) {
	RK_TRACE (APP);
	RK_ASSERT (widget_to_id.contains (widget));

	int id = widget_to_id[widget];
	bool was_active_in_bar = isTabRaised (id);

	removeTab (id);
	widget_to_id.remove (widget);
	widget->tool_window_bar = 0;

	if (was_active_in_bar) {
		RK_ASSERT (widget->isAttached ());
		container->hide ();
		widget->active = false;
	}
	if (widget_to_id.isEmpty ()) hide ();
}

void RKToolWindowBar::showWidget (RKMDIWindow *widget) {
	RK_TRACE (APP);
	RK_ASSERT (widget_to_id.contains (widget));

	int id = widget_to_id[widget];

	// close any others
	QMap<RKMDIWindow*, int>::const_iterator it = widget_to_id.constBegin ();
	while (it != widget_to_id.constEnd()) {
		RKMDIWindow *cur = it.key ();
		if (cur != widget) {
			if (cur->isAttached ()) {
				cur->active = false;
				cur->hide ();
	
			}
			setTab (it.value (), false);
		}
	}

	if (widget->isAttached ()) {
		setTab (id, true);
		container->show ();
	} else {
		widget->topLevelWidget ()->show ();
		widget->topLevelWidget ()->raise ();
	}
	widget->active = true;
	widget->show ();
}

void RKToolWindowBar::hideWidget (RKMDIWindow *widget) {
	RK_TRACE (APP);
	RK_ASSERT (widget_to_id.contains (widget));

	// prevent recursion
	if (!widget->active) return;
	int id = widget_to_id[widget];

	bool was_active_in_bar = isTabRaised (id);
	if (was_active_in_bar) {
		RK_ASSERT (widget->isAttached ());
		container->hide ();
	}

	widget->active = false;
	widget->close (false);

	setTab (id, false);

	RKWorkplace::mainWorkplace ()->view ()->setFocus ();
}

void RKToolWindowBar::tabClicked (int id) {
	RK_TRACE (APP);

	RKMDIWindow *widget = idToWidget (id);
	RK_ASSERT (widget);

	if (widget->isActive ()) {
		widget->close (false);
	} else {
		widget->activate (true);
	}
}

RKMDIWindow* RKToolWindowBar::idToWidget (int id) {
	RK_TRACE (APP);

	QMap<RKMDIWindow*, int>::const_iterator it = widget_to_id.constBegin ();
	while (it != widget_to_id.constEnd()) {
		if (it.value () == id) {
			return (it.key ());
		}
	}

	return 0;
}

bool RKToolWindowBar::eventFilter (QObject *obj, QEvent *ev) {
	if (ev->type() == QEvent::ContextMenu) {
		RK_TRACE (APP);

		QContextMenuEvent *e = (QContextMenuEvent *) ev;
		KMultiTabBarTab *bt = dynamic_cast<KMultiTabBarTab*>(obj);

		if (bt) {
			id_of_popup = bt->id ();

			RKMDIWindow *widget = idToWidget (id_of_popup);
			RK_ASSERT (widget);
			if (widget) {
				KMenu *p = new KMenu (this);

				p->addTitle (SmallIcon("view_remove"), i18n("Attachment"));
				
				p->addAction (widget->isAttached () ? KIcon("view-restore") : KIcon("view-fullscreen"), widget->isAttached () ? i18n("Detach") : i18n("Attach"))->setData (CHANGE_ATTACHMENT_ACTION_ID);

				p->addTitle (SmallIcon("move"), i18n("Move To"));
	
				if (position () != KMultiTabBar::Left) p->addAction(KIcon("go-previous"), i18n("Left Sidebar"))->setData(KMultiTabBar::Left);
	
				if (position () != KMultiTabBar::Right) p->addAction(KIcon("go-next"), i18n("Right Sidebar"))->setData(KMultiTabBar::Right);
				if (position () != KMultiTabBar::Top) p->addAction(KIcon("go-up"), i18n("Top Sidebar"))->setData(KMultiTabBar::Top);
				if (position () != KMultiTabBar::Bottom) p->addAction(KIcon("go-down"), i18n("Bottom Sidebar"))->setData(KMultiTabBar::Bottom);
	
				connect (p, SIGNAL (triggered(QAction *)), this, SLOT (buttonPopupActivate(QAction *)));
				p->exec (e->globalPos());
				delete p;
	
				return true;
			}
		}
	}
	
	return false;
}

void RKToolWindowBar::buttonPopupActivate (QAction *a) {
	RK_TRACE (APP);

	int action = a->data().toInt();
	RKMDIWindow *window = idToWidget (id_of_popup);
	RK_ASSERT (window);

	// move to another bar
	if (action < 4) {
		// move + show ;)
		RKWorkplace::mainWorkplace ()->placeInToolWindowBar (window, (KMultiTabBar::KMultiTabBarPosition) action);
		window->activate ();
	}

	// toggle attachment
	if (action == CHANGE_ATTACHMENT_ACTION_ID) {
		if (window->isAttached ()) RKWorkplace::mainWorkplace ()->detachWindow (window);
		else RKWorkplace::mainWorkplace ()->attachWindow (window);
	}
}

#include "rktoolwindowbar.moc"
