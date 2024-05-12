/*
rkworkplaceview - This file is part of RKWard (https://rkward.kde.org). Created: Tue Sep 26 2006
SPDX-FileCopyrightText: 2006-2017 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

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

RKWorkplaceViewPane::RKWorkplaceViewPane (RKWorkplaceView* parent) : QTabWidget () {
	RK_TRACE (APP);

	setDocumentMode(true);
	workplace_view = parent;

	setTabsClosable (true);
	connect (this, &QTabWidget::tabCloseRequested, this, static_cast<void (RKWorkplaceViewPane::*)(int)>(&RKWorkplaceViewPane::closePage));

	setMovable (true);

	tabBar ()->setContextMenuPolicy (Qt::CustomContextMenu);
	connect (tabBar (), &QWidget::customContextMenuRequested, this, &RKWorkplaceViewPane::showContextMenu);

	KAcceleratorManager::setNoAccel (tabBar ());	// TODO: This is a WORKAROUND for a bug in kdelibs where tabs named "a0.txt", "a1.txt", etc. will steal the Alt+0/1... shortcuts
//	tabBar ()->hide ();  // initially
	connect (this, &QTabWidget::currentChanged, this, &RKWorkplaceViewPane::currentPageChanged);
}

RKWorkplaceViewPane::~RKWorkplaceViewPane () {
	RK_TRACE (APP);
}

void RKWorkplaceViewPane::initActions () {
	RK_TRACE (APP);

	QToolButton *split_button = new QToolButton (this);
	split_button->setAutoRaise (true);
	split_button->setPopupMode (QToolButton::InstantPopup);
	split_button->setIcon (QIcon::fromTheme (QStringLiteral("view-split-left-right")));
	split_button->addAction (workplace_view->action_split_vert);
	split_button->addAction (workplace_view->action_split_horiz);
	QAction *close_all = new QAction (QIcon::fromTheme("tab-close"), i18n ("Close all"), this);
	connect (close_all, &QAction::triggered, this, &RKWorkplaceViewPane::closeAll);
	split_button->addAction (close_all);
	split_button->installEventFilter (this); // on click, active this pane
	split_button->adjustSize ();
	setCornerWidget (split_button, Qt::TopRightCorner);
}

bool RKWorkplaceViewPane::eventFilter (QObject* obj, QEvent* event) {
	if (event->type () == QEvent::MouseButtonPress) {
		RKMDIWindow *current = static_cast<RKMDIWindow*> (currentWidget ());
		if (current && !current->isActiveInsideToplevelWindow()) {
			current->activate ();  // make sure this pane is active
		}
	}
	return QObject::eventFilter (obj, event);
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
	QAction *action = KStandardAction::close(this, &RKWorkplaceViewPane::contextMenuClosePage, this);
	action->setData (tab);
	m->addAction (action);
	action = m->addAction(RKStandardIcons::getIcon(RKStandardIcons::ActionDetachWindow), i18n("Detach"), this, &RKWorkplaceViewPane::contextMenuDetachWindow);
	action->setData (tab);
	m->exec (mapToGlobal (pos));
	delete m;
}

void RKWorkplaceViewPane::closeAll () {
	RK_TRACE (APP);

	QList<RKMDIWindow*> windows;
	for (int i = count () - 1; i >= 0; --i) {
		windows.append (static_cast<RKMDIWindow*> (widget (i)));
	}
	RKWorkplace::mainWorkplace ()->closeWindows (windows);
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
	static_cast<RKMDIWindow*>(page)->close (RKMDIWindow::AutoAskSaveModified);
}

void RKWorkplaceViewPane::tabRemoved (int index) {
	RK_TRACE (APP);
	QTabWidget::tabRemoved (index);
//	if (count () < 2) tabBar ()->hide ();
	if (count () < 1) Q_EMIT becameEmpty(this);
	workplace_view->updateActions ();
}

void RKWorkplaceViewPane::tabInserted (int index) {
	RK_TRACE (APP);
	QTabWidget::tabInserted (index);
//	if (count () > 1) tabBar ()->show ();
	workplace_view->updateActions ();
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

void RKWorkplaceViewPane::currentPageChanged (int) {
	RK_TRACE (APP);

	RKMDIWindow *w = static_cast<RKMDIWindow*> (currentWidget ());
	if (w) {
		workplace_view->setCaption (w->shortCaption ());
		w->activate ();		// not always automatically active
	} else {
		// happens when empty
		workplace_view->setCaption (QString ());
	}
}


// Create new splitter with default setup
QSplitter *createSplitter (Qt::Orientation orientation) {
	RK_TRACE (APP);

	QSplitter* ret = new QSplitter (orientation);
	ret->setChildrenCollapsible (false);
	return ret;
}

RKWorkplaceViewPane* RKWorkplaceView::createPane (bool init_actions) {
	RK_TRACE (APP);
	newpane = new RKWorkplaceViewPane (this);
	QObject::connect (newpane, &RKWorkplaceViewPane::becameEmpty, this, &RKWorkplaceView::purgePane);
	if (init_actions) newpane->initActions ();
	return newpane;
}

RKWorkplaceView::RKWorkplaceView (QWidget *parent) : QSplitter (parent) {
	RK_TRACE (APP);

	newpane = nullptr;
	RKWorkplaceViewPane *pane = createPane (false);
	addWidget (pane);
	panes.append (pane);
	setChildrenCollapsible (false);
}

RKWorkplaceView::~RKWorkplaceView () {
	RK_TRACE (APP);
}

RKWorkplaceViewPane* RKWorkplaceView::activePane () const {
	RK_TRACE (APP);

	if (newpane) return newpane;

	for (int i = 0; i < panes.size (); ++i) {
		if (panes[i]->isActive ()) return panes[i];
	}

	// Esp. when switching between console and script window, consider the previous window active
	RKWorkplaceViewPane *pane = findWindow (RKWorkplace::mainWorkplace ()->getHistory ()->previousDocumentWindow ());
	if (pane) return pane;

	// As a last resort, return top-left pane
	RK_ASSERT (!panes.isEmpty ());
	return panes.first ();
}

void RKWorkplaceView::initActions (KActionCollection *ac) {
	RK_TRACE (APP);

	action_page_left = ac->addAction("left_window", this, &RKWorkplaceView::pageLeft);
	action_page_left->setText (i18n ("Window Left"));
	ac->setDefaultShortcuts (action_page_left, {
		Qt::ControlModifier | Qt::Key_Less,
		Qt::ControlModifier | Qt::Key_Comma
	});

	action_page_right = ac->addAction("right_window", this, &RKWorkplaceView::pageRight);
	action_page_right->setText (i18n ("Window Right"));
	ac->setDefaultShortcuts (action_page_right, {
		Qt::ControlModifier | Qt::Key_Greater,
		Qt::ControlModifier | Qt::Key_Period
	});

	// NOTE: Icons, shortcuts, action names for split view actions as in kate
	action_split_vert = ac->addAction (QStringLiteral ("view_split_vert"));
	action_split_vert->setIcon (QIcon::fromTheme(QStringLiteral ("view-split-left-right")));
	action_split_vert->setText (i18n("Split Ve&rtical"));
	ac->setDefaultShortcut (action_split_vert, Qt::CTRL | Qt::SHIFT | Qt::Key_L);
	connect (action_split_vert, &QAction::triggered, this, &RKWorkplaceView::splitViewVert);
	action_split_vert->setWhatsThis (i18n ("Split the currently active view into two views, vertically."));

	action_split_horiz = ac->addAction (QStringLiteral ("view_split_horiz"));
	action_split_horiz->setIcon (QIcon::fromTheme(QStringLiteral ("view-split-top-bottom")));
	action_split_horiz->setText (i18n ("Split &Horizontal"));
	ac->setDefaultShortcut (action_split_horiz, Qt::CTRL | Qt::SHIFT | Qt::Key_T);
	connect (action_split_horiz, &QAction::triggered, this, &RKWorkplaceView::splitViewHoriz);
	action_split_horiz->setWhatsThis (i18n ("Split the currently active view into two views, horizontally."));

	panes.first ()->initActions ();
	updateActions ();
}

void RKWorkplaceView::updateActions () {
	RK_TRACE (APP);

	bool several_pages = panes.count () > 1 || (panes.count () > 0 && panes.first ()->count () > 1);
	action_page_left->setEnabled (several_pages);
	action_page_right->setEnabled (several_pages);
}

void RKWorkplaceView::pageLeft () {
	RK_TRACE (APP);

	RKWorkplaceViewPane *current = activePane ();
	int index = current->currentIndex ();
	if (index > 0) {
		current->setCurrentIndex (index - 1);
	} else {
		int pindex = panes.indexOf (current);
		if (pindex > 0) --pindex;
		else pindex = panes.size () - 1;
		if (panes[pindex]->count () < 1) {
			RK_ASSERT (false); // action should have been disabled on an empty workplaceview
			return;
		}
		// NOTE: setCurrentIndex() is not enough, here, as the index may be current, already, while the pane is still inactive.
		static_cast<RKMDIWindow*> (panes[pindex]->widget (panes[pindex]->count () - 1))->activate ();
	}
}

void RKWorkplaceView::pageRight () {
	RK_TRACE (APP);

	RKWorkplaceViewPane *current = activePane ();
	int index = current->currentIndex ();
	if (index < current->count () - 1) {
		current->setCurrentIndex (index + 1);
	} else {
		int pindex = panes.indexOf (current);
		if (pindex < panes.count () - 1) ++pindex;
		else pindex = 0;
		if (panes[pindex]->count () < 1) {
			RK_ASSERT (false); // action should have been disabled on an empty workplaceview
			return;
		}
		// NOTE: setCurrentIndex() is not enough, here, as the index may be current, already, while the pane is still inactive.
		static_cast<RKMDIWindow*> (panes[pindex]->widget (0))->activate ();
	}
}

void RKWorkplaceView::splitViewHoriz () {
	RK_TRACE (APP);
	splitView (Qt::Vertical);
}

void RKWorkplaceView::splitViewVert () {
	RK_TRACE (APP);
	splitView (Qt::Horizontal);
}

// NOTE: Some of this function taken from kate's kateviewmanager.cpp
void RKWorkplaceView::splitView (Qt::Orientation orientation, const QString &description, const QString &base) {
	RK_TRACE (APP);

	RKWorkplaceViewPane* pane = activePane ();

	QString _description = description;
	if (_description.isEmpty ()) {
		RKMDIWindow *active = dynamic_cast<RKMDIWindow *> (pane->currentWidget ());
		if (!active) {
			RKWorkplace::mainWorkplace ()->openHelpWindow (QUrl ("rkward://page/rkward_split_views"));
			RK_ASSERT (count () == 0);
			active = activePage ();
			RK_ASSERT (active);
		}
		_description = RKWorkplace::mainWorkplace ()->makeItemDescription (active);
	}

	QSplitter *splitter = qobject_cast<QSplitter *> (pane->parentWidget ());
    if (!splitter) {
		RK_ASSERT (splitter);
		return;
	}

	setUpdatesEnabled (false);
	QList<int> sizes = splitter->sizes ();

	// index where to insert new splitter/viewspace
	const int index = splitter->indexOf (pane);
	const int lindex = panes.indexOf (pane);

	newpane = createPane ();
	panes.insert (lindex + 1, newpane);

	// If there is only one child (left) in the current splitter, we can just set the orientation as needed.
	if (splitter->count () == 1) {
		splitter->setOrientation (orientation);
	}
	if (splitter->orientation () == orientation) {  // not the same as above: Also, if an existing larger splitter is suitably oriented, already
		// First calculate the size of the new and the old elements.
		// This is not pixel perfect, but reasonably close.
		for (int i = sizes.count () - 1; i >= 0; --i) {
			sizes[i] = sizes[i] * sizes.count () / (sizes.count () + 1);
		}
		sizes.insert (index + 1, (orientation == Qt::Horizontal ? splitter->width () : splitter->height ()) / (splitter->count () + 1) - splitter->handleWidth ());

		splitter->insertWidget (index + 1, newpane);
	} else {
		QSplitter *newsplitter = createSplitter (orientation);
		newsplitter->addWidget (pane);
		newsplitter->addWidget (newpane);
		splitter->insertWidget (index, newsplitter);
		QList<int> subsizes = newsplitter->sizes ();
		subsizes[0] = subsizes[1] = (subsizes[0] + subsizes[1]) / 2;
		newsplitter->setSizes (subsizes);
	}
	newpane->show ();
	// "copy" the "split" window to the new pane.
	// TODO: lines below will only work for the main window, for the time being. We need to rethink this, if we want to enable window splitting for detached windows, too.
	if (!RKWorkplace::mainWorkplace ()->restoreDocumentWindow (_description, base)) {
		RKWorkplace::mainWorkplace ()->openHelpWindow (QUrl ("rkward://page/rkward_split_views"));
	}
	newpane = nullptr;

	splitter->setSizes (sizes);
	setUpdatesEnabled (true);
}

void RKWorkplaceView::addWindow (RKMDIWindow *widget) {
	RK_TRACE (APP);

	int id = -1;

	QIcon icon = widget->windowIcon ();
	if (icon.isNull ()) icon = widget->topLevelWidget ()->windowIcon ();
	if (icon.isNull ()) RK_ASSERT (false);

	RKWorkplaceViewPane *pane = activePane ();
	RK_ASSERT (pane);
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
	return nullptr;
}

bool RKWorkplaceView::hasWindow (RKMDIWindow *widget) const {
	return (findWindow (widget) != nullptr);
}

bool RKWorkplaceView::windowInActivePane (RKMDIWindow *widget) const {
	return (findWindow (widget) == activePane ());
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
	if (panes.count () < 2) return;  // keep at least one pane around for layout purposes

	QSplitter* split = static_cast<QSplitter*> (pane->parentWidget ());
	pane->hide();
	pane->setParent(nullptr); // TODO: needed?
	pane->deleteLater();
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
	RK_ASSERT (pane);
	return (dynamic_cast<RKMDIWindow *> (pane->currentWidget ()));
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

void RKWorkplaceView::setCaption(const QString &caption) {
	RK_TRACE(APP);

	QWidget::setWindowTitle(caption);
	Q_EMIT captionChanged(caption);
}

void RKWorkplaceView::restoreLayout(const QString& desc) {
	RK_TRACE (APP);

	bool old_updates_enabled = updatesEnabled ();
	setUpdatesEnabled (false);

	QList<RKMDIWindow*> windows_to_readd;
	// It is probably not a good idea to restore the layout while merging workplaces, i.e. without closing existing windows, first.
	// However, we'll do our best to cope... For this, we clear out all splitters, first.
	for (int i = 0; i < panes.count (); ++i) {
		while (panes[i]->count ()) {
			RKMDIWindow *win = static_cast<RKMDIWindow*> (panes[i]->widget (0));
			panes[i]->removeTab (0);
			windows_to_readd.append (win);
			win->hide();
			win->setParent(nullptr);
		}
	}
	while (count ()) {
		delete (widget (0));
	}
	panes.clear ();

	QList<QSplitter*> parents;
	parents.append (this);
	QStringList dl = desc.split ('-');
	if (dl.value (0) == QStringLiteral ("col")) setOrientation (Qt::Vertical);
	else setOrientation (Qt::Horizontal);

	for (int i = 1; i < dl.size (); ++i) {
		if (parents.isEmpty ()) {
			RK_DEBUG (APP, DL_ERROR, "Bad specification while restoring workplace view layout");
			break;
		}
		if (dl[i] == QStringLiteral ("p")) {
			RKWorkplaceViewPane *pane = createPane ();
			pane = createPane ();
			panes.append (pane);
			parents.last ()->addWidget (pane);
		} else if (dl[i] == QStringLiteral ("row")) {
			QSplitter* newsplit = createSplitter (Qt::Horizontal);
			parents.last ()->addWidget (newsplit);
			parents.append (newsplit);
		} else if (dl[i] == QStringLiteral ("col")) {
			QSplitter* newsplit = createSplitter (Qt::Vertical);
			parents.last ()->addWidget (newsplit);
			parents.append (newsplit);
		} else {
			RK_ASSERT (dl[i].startsWith (QStringLiteral ("end")));
			const QStringList s = dl[i].mid (4).split (',');
			QList<int> sizes = parents.last ()->sizes ();  // simply to have an initialized list of the correct length
			for (int i = 0; i < sizes.count (); ++i) {
				int size = qMax (50, s.value (i).toInt ());  // avoid collapsed views in case of errors
				sizes[i] = size;
			}
			parents.last ()->setSizes (sizes);
			parents.pop_back ();
		}
	}

	if (panes.isEmpty ()) {
		panes.append (createPane ());
		addWidget (panes[0]);
	}

	newpane = panes.value (0);

	for (int i = 0; i < windows_to_readd.count (); ++i) {
		addWindow (windows_to_readd[i]);
	}

	setUpdatesEnabled (old_updates_enabled);
}

void RKWorkplaceView::nextPane () {
	RK_TRACE (APP);

	RKWorkplaceViewPane *pane = activePane ();
	int index = panes.indexOf (pane);
	newpane = panes.value (index + 1);

	RK_DEBUG (APP, DL_DEBUG, "Activating pane %p after pane %p (index %d / %d)", newpane, pane, index + 1, index);
}

void RKWorkplaceView::purgeEmptyPanes () {
	RK_TRACE (APP);

	newpane = nullptr; // just in case of broken specifications during workplace restoration
	for (int i = 0; i < panes.count (); ++i) {
		if (panes.count() > 1 && panes[i]->count() < 1) {
			purgePane (panes[i]);
			--i;
		}
	}
}

QString listLayout (const QSplitter *parent) {
	RK_TRACE (APP);

	QString ret = (parent->orientation () == Qt::Horizontal ? QStringLiteral ("row") : QStringLiteral ("col"));

	for (int i = 0; i < parent->count (); ++i) {
		QWidget* w = parent->widget (i);
		RKWorkplaceViewPane *pane = qobject_cast<RKWorkplaceViewPane*> (w);
		if (pane) {
			ret.append (QStringLiteral ("-p"));
		} else {
			QSplitter* sub = qobject_cast<QSplitter*> (w);
			ret.append ('-' + listLayout (sub));
		}
	}

	ret.append (QStringLiteral ("-end"));
	const QList<int> sizes = parent->sizes ();
	for (int i = 0; i < sizes.count (); ++i) {
		ret.append (',' + QString::number (sizes[i]));
	}

	return ret;
}

void listContents (const QSplitter *parent, QStringList *ret) {
	RK_TRACE (APP);

	for (int i = 0; i < parent->count (); ++i) {
		QWidget* w = parent->widget (i);
		RKWorkplaceViewPane *pane = qobject_cast<RKWorkplaceViewPane*> (w);
		if (pane) {
			for (int j = 0; j < pane->count (); ++j) {
				QString desc = RKWorkplace::mainWorkplace ()->makeItemDescription (static_cast<RKMDIWindow*> (pane->widget (j)));
				if (!desc.isEmpty ()) ret->append (desc);
			}
			ret->append (QStringLiteral ("pane_end::::"));
		} else {
			QSplitter* sub = qobject_cast<QSplitter*> (w);
			listContents (sub, ret);
		}
	}
}

QString RKWorkplaceView::listLayout () const {
	return (::listLayout (this));
}

QStringList RKWorkplaceView::listContents () const {
	RK_TRACE (APP);

	QStringList ret;
	::listContents (this, &ret);
	return ret;
}
