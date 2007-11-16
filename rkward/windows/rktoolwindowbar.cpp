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
#include <QContextMenuEvent>

#include "rkworkplace.h"
#include "rkworkplaceview.h"
#include "rkmdiwindow.h"
#include "../misc/rkstandardicons.h"

#include "../debug.h"

#define CHANGE_ATTACHMENT_ACTION_ID 10
#define DEFAULT_SPLITTER_SIZE 200
#define SPLITTER_MIN_SIZE 30

RKToolWindowBar::RKToolWindowBar (KMultiTabBarPosition position, QWidget *parent) : KMultiTabBar (position, parent),
	container (0) {
	RK_TRACE (APP);

	setStyle (KMultiTabBar::KDEV3ICON);
	last_known_size = SPLITTER_MIN_SIZE;
}

RKToolWindowBar::~RKToolWindowBar () {
	RK_TRACE (APP);
}

void RKToolWindowBar::restoreSize (const KConfigGroup &cg) {
	RK_TRACE (APP);

	last_known_size = cg.readEntry (QString ("view_size_%1").arg (position ()), DEFAULT_SPLITTER_SIZE);
}

void RKToolWindowBar::saveSize (KConfigGroup &cg) const {
	RK_TRACE (APP);

	cg.writeEntry (QString ("view_size_%1").arg (position ()), last_known_size);
}

int RKToolWindowBar::getSplitterSize () const {
	RK_TRACE (APP);

	int pos = splitter->indexOf (container);
	if (pos < 0) {
		RK_ASSERT (false);
		return 0;
	}
	return (splitter->sizes ()[pos]);
}

void RKToolWindowBar::setSplitterSize (int new_size) {
	RK_TRACE (APP);

	// HACK / WORKAROUND: reset the collapsed state of the container (if collapsed). Else we will not be able to open it again
	int index = splitter->indexOf (container);
	QList<int> sizes = splitter->sizes ();
	if (sizes[index] == 0) {
		sizes[index] = last_known_size;
		splitter->setSizes (sizes);
	}

	if (splitter->orientation () == Qt::Horizontal) {
		container->resize (new_size, container->height ());
	} else {
		container->resize (container->width (), new_size);
	}
}

void RKToolWindowBar::splitterMoved (int, int) {
	RK_TRACE (APP);

	int pos = getSplitterSize ();
	if (pos >= SPLITTER_MIN_SIZE) last_known_size = pos;

	if (!pos) {		// collapsed. Hide it properly.
		for (QMap<RKMDIWindow*, int>::const_iterator it = widget_to_id.constBegin (); it != widget_to_id.constEnd (); ++it) {
			if (isTabRaised (it.value ())) {
				hideWidget (it.key ());
				break;
			}
		}
	}
}

void RKToolWindowBar::setSplitter (QSplitter *splitter) {
	RK_TRACE (APP);
	RK_ASSERT (!container);

	RKToolWindowBar::splitter = splitter;
	container = new KHBox (splitter);
	splitter->setContentsMargins (0, 0, 0, 0);
	container->layout ()->setContentsMargins (0, 0, 0, 0);
	container->layout ()->setSpacing (0);
	container->layout ()->setMargin (0);
	container->hide ();

	connect (splitter, SIGNAL(splitterMoved(int,int)), this, SLOT(splitterMoved(int,int)));
}

void RKToolWindowBar::addWidget (RKMDIWindow *window) {
	RK_TRACE (APP);
	RK_ASSERT (window);
	RK_ASSERT (container);
	static int id_count = 0;
	int id = ++id_count;

	if (window->tool_window_bar) {
		RK_ASSERT (window->tool_window_bar != this);	// no problem, but would be useless code
		window->tool_window_bar->removeWidget (window);
	}

	appendTab (window->windowIcon ().pixmap (QSize (16, 16)), id, window->shortCaption ());

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

void RKToolWindowBar::reclaimDetached (RKMDIWindow *window) {
	RK_TRACE (APP);

	window->hide();
	window->setParent (container);
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
	for (QMap<RKMDIWindow*, int>::const_iterator it = widget_to_id.constBegin (); it != widget_to_id.constEnd (); ++it) {
		RKMDIWindow *cur = it.key ();
		if (cur != widget) {
			if (cur->isAttached ()) {
				cur->active = false;
				cur->hide ();
			}
			setTab (it.value (), false);
		}
	}

	widget->show ();
	if (widget->isAttached ()) {
		setTab (id, true);
		container->show ();
		setSplitterSize (last_known_size);
	} else {
		widget->topLevelWidget ()->show ();
		widget->topLevelWidget ()->raise ();
	}
	widget->active = true;
}

void RKToolWindowBar::hideWidget (RKMDIWindow *widget) {
	RK_TRACE (APP);
	RK_ASSERT (widget_to_id.contains (widget));

	// prevent recursion
	if (!widget->active) return;
	int id = widget_to_id[widget];

	bool was_active_in_bar = ((widget->parent () == container) && widget->isVisible ());
	if (was_active_in_bar) {
		container->hide ();
	}

	widget->active = false;
	widget->hide ();

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

RKMDIWindow* RKToolWindowBar::idToWidget (int id) const {
	RK_TRACE (APP);

	for (QMap<RKMDIWindow*, int>::const_iterator it = widget_to_id.constBegin (); it != widget_to_id.constEnd (); ++it) {
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

				p->addTitle (i18n("Attachment"));
				
				p->addAction (RKStandardIcons::getIcon (widget->isAttached () ? RKStandardIcons::ActionDetachWindow : RKStandardIcons::ActionAttachWindow), widget->isAttached () ? i18n("Detach") : i18n("Attach"))->setData (CHANGE_ATTACHMENT_ACTION_ID);

				p->addTitle (i18n("Move To"));
	
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
