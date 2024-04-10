/*
rktoolwindowbar - This file is part of RKWard (https://rkward.kde.org). Created: Fri Oct 12 2007
SPDX-FileCopyrightText: 2007-2024 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

/* This code is based substantially on kate's katemdi! */

#include "rktoolwindowbar.h"

#include <QMenu>
#include <KLocalizedString>
#include <kparts/partmanager.h>
#include <kselectaction.h>
#include <kwidgetsaddons_version.h>

#include <QSplitter>
#include <QContextMenuEvent>
#include <QHBoxLayout>

#include "rkworkplace.h"
#include "rkworkplaceview.h"
#include "rkmdiwindow.h"
#include "../rkward.h"
#include "../misc/rkstandardicons.h"

#include "../debug.h"

#define DEFAULT_SPLITTER_SIZE 200
#define SPLITTER_MIN_SIZE 30

RKToolWindowBar::RKToolWindowBar (KMultiTabBarPosition position, QWidget *parent) : KMultiTabBar (position, parent),
	container(nullptr) {
	RK_TRACE (APP);

	setStyle (KMultiTabBar::KDEV3ICON);
	last_known_size = SPLITTER_MIN_SIZE;
}

RKToolWindowBar::~RKToolWindowBar () {
	RK_TRACE (APP);
}

void RKToolWindowBar::captionChanged (RKMDIWindow* window) {
	RK_TRACE (APP);

	int id = widget_to_id.value (window);
	tab (id)->setText (window->shortCaption ());
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
	return (splitter->sizes().at(pos));
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
	container = new QWidget (splitter);
	new QHBoxLayout (container);
	splitter->setContentsMargins (0, 0, 0, 0);
	container->layout ()->setContentsMargins (0, 0, 0, 0);
	container->layout ()->setSpacing (0);
	container->hide ();

	connect (splitter, &QSplitter::splitterMoved, this, &RKToolWindowBar::splitterMoved);
}

void RKToolWindowBar::addWidget (RKMDIWindow *window) {
	RK_TRACE (APP);
	RK_ASSERT (window);
	if (window->tool_window_bar == this) return;	// may happen while restoring windows
	RK_ASSERT (container);

	static int id_count = 0;
	int id = ++id_count;

	if (window->tool_window_bar) {
		window->tool_window_bar->removeWidget (window);
	}

	appendTab (window->windowIcon (), id, window->shortCaption ());

	window->tool_window_bar = this;
	widget_to_id.insert (window, id);
	connect (window, &QObject::destroyed, this, &RKToolWindowBar::windowDestroyed);

	connect (tab (id), &KMultiTabBarTab::clicked, this, &RKToolWindowBar::tabClicked);
	tab (id)->installEventFilter (this);

	if (window->isAttached ()) {
		reclaimDetached (window);
	}

	// This really belongs to katepart integration, but cannot be placed there, without major pains
	// Needs to be a queued connect, as the "real" widget (listening for the call) may be constructed after it's container
	// is being added, here.
	QMetaObject::invokeMethod( RKWardMainWindow::getMain(),
	[w = RKWardMainWindow::getMain(), window, tab = tab(id)] {
		Q_EMIT w->tabForToolViewAdded(window, tab);
	}, Qt::QueuedConnection);

	show ();
}

void RKToolWindowBar::reclaimDetached (RKMDIWindow *window) {
	RK_TRACE (APP);

	if (window->parent () == container) return;

	window->hide();
	window->setParent (container);
	container->layout ()->addWidget (window);
}

void RKToolWindowBar::windowDestroyed(QObject* window) {
	RK_TRACE (APP);

	int id = widget_to_id.take (static_cast<RKMDIWindow *> (window));
	removeTab (id);
}

void RKToolWindowBar::removeWidget (RKMDIWindow *widget) {
	RK_TRACE (APP);
	RK_ASSERT (widget_to_id.contains (widget));

	int id = widget_to_id[widget];
	bool was_active_in_bar = isTabRaised (id);

	removeTab (id);
	widget_to_id.remove (widget);
	disconnect (widget, &QObject::destroyed, this, &RKToolWindowBar::windowDestroyed);
	widget->tool_window_bar = nullptr;

	if (widget->isAttached ()) {
		widget->setParent(nullptr);
		widget->hide ();
	}

	if (was_active_in_bar) {
		RK_ASSERT (widget->isAttached ());
		container->hide ();
		widget->active = false;
	}
	if (widget_to_id.isEmpty ()) hide ();
}

void RKToolWindowBar::closeOthers (RKMDIWindow* widget) {
	RK_TRACE (APP);

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
}

void RKToolWindowBar::showWidget (RKMDIWindow *widget) {
	RK_TRACE (APP);
	RK_ASSERT (widget_to_id.contains (widget));

	int id = widget_to_id[widget];

	// close any others
	closeOthers (widget);

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

	RKWardMainWindow::getMain()->partManager()->setActivePart(nullptr);
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
		if (!widget->isAttached ()) widget->close (RKMDIWindow::NoAskSaveModified);
		else hideWidget (widget);
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

	return nullptr;
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
				QMenu menu (this);

				QAction *a = menu.addAction (RKStandardIcons::getIcon (widget->isAttached () ? RKStandardIcons::ActionDetachWindow : RKStandardIcons::ActionAttachWindow), widget->isAttached () ? i18n("Detach") : i18n("Attach"));
				connect (a, &QAction::triggered, this, &RKToolWindowBar::changeAttachment);

				KSelectAction *sel = new KSelectAction (i18n ("Position"), &menu);
				sel->addAction (RKStandardIcons::getIcon (RKStandardIcons::ActionMoveLeft), i18n ("Left Sidebar"));
				sel->addAction (RKStandardIcons::getIcon (RKStandardIcons::ActionMoveRight), i18n ("Right Sidebar"));
				sel->addAction (RKStandardIcons::getIcon (RKStandardIcons::ActionMoveUp), i18n ("Top Sidebar"));
				sel->addAction (RKStandardIcons::getIcon (RKStandardIcons::ActionMoveDown), i18n ("Bottom Sidebar"));
				sel->addAction (RKStandardIcons::getIcon (RKStandardIcons::ActionDelete), i18n ("Not shown in sidebar"));
				connect(sel, &KSelectAction::indexTriggered, this, &RKToolWindowBar::moveToolWindow);
				menu.addAction (sel);
	
				menu.exec (e->globalPos());

				return true;
			}
		}
	}

	return false;
}

void RKToolWindowBar::contextMenuEvent (QContextMenuEvent* event) {
	RK_TRACE (APP);

	QMenu menu (this);
	const auto windows = RKToolWindowList::registeredToolWindows ();
	for (const RKToolWindowList::ToolWindowRepresentation& rep : windows) {
		QAction *a = menu.addAction (rep.window->windowIcon (), rep.window->shortCaption ());
		a->setCheckable (true);
		a->setChecked (rep.window->tool_window_bar == this);
		a->setData (rep.id);
	}
	connect (&menu, &QMenu::triggered, this, &RKToolWindowBar::addRemoveToolWindow);
	menu.exec (event->globalPos ());

	event->accept ();
}

void RKToolWindowBar::changeAttachment () {
	RK_TRACE (APP);

	RKMDIWindow *window = idToWidget (id_of_popup);
	RK_ASSERT (window);

	// toggle attachment
	if (window->isAttached ()) RKWorkplace::mainWorkplace ()->detachWindow (window);
	else RKWorkplace::mainWorkplace ()->attachWindow (window);
}

void RKToolWindowBar::moveToolWindow (int target) {
	RK_TRACE (APP);

	RK_ASSERT (target >= RKToolWindowList::Left);
	RK_ASSERT (target <= RKToolWindowList::Bottom);
	if (target == position ()) return;
	RKMDIWindow *window = idToWidget (id_of_popup);
	RK_ASSERT (window);

	RKWorkplace::mainWorkplace ()->placeInToolWindowBar (window, target);
}


void RKToolWindowBar::addRemoveToolWindow (QAction *action) {
	RK_TRACE (APP);
	RK_ASSERT (action);

	RKMDIWindow *win = RKToolWindowList::findToolWindowById (action->data ().toString ());
	if (action->isChecked ()) {
		RKWorkplace::mainWorkplace ()->placeInToolWindowBar (win, position ());
	} else {
		RK_ASSERT (win->tool_window_bar == this);
		removeWidget (win);
	}
}

