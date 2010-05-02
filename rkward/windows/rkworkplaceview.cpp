/***************************************************************************
                          rkworkplaceview  -  description
                             -------------------
    begin                : Tue Sep 26 2006
    copyright            : (C) 2006, 2007, 2009, 2010 by Thomas Friedrichsmeier
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
#include <kicon.h>
#include <kdeversion.h>
#include <kacceleratormanager.h>
#include <kmenu.h>

#include <qapplication.h>
#include <qevent.h>
#include <qlayout.h>
#include <QToolButton>

#include "rkmdiwindow.h"
#include "rkworkplace.h"
#include "../misc/rkstandardicons.h"

#include "../debug.h"


RKWorkplaceView::RKWorkplaceView (QWidget *parent) : KTabWidget (parent) {
	RK_TRACE (APP);

	// close button(s)
	QToolButton* close_button = new QToolButton (this);
	close_button->setIcon (KIcon ("tab-close"));
	connect (close_button, SIGNAL (clicked()), this, SLOT (closeCurrentPage()));
	close_button->adjustSize ();
	setCornerWidget (close_button, Qt::TopRightCorner);

#if KDE_IS_VERSION(4,1,0)
#	if QT_VERSION >= 0x040500
	setTabsClosable (true);
	connect (this, SIGNAL (tabCloseRequested(int)), this, SLOT (closePage(int)));
#	else
	setCloseButtonEnabled (true);
	connect (this, SIGNAL (closeRequest(QWidget*)), this, SLOT (closePage(QWidget*)));
#	endif
#endif

#if QT_VERSION >= 0x040500
	setMovable (true);
#else
	setTabReorderingEnabled (true);	// the KDE function is deprecated sind Qt 4.5 / KDE 4.4
#endif

	tabBar ()->setContextMenuPolicy (Qt::CustomContextMenu);
	connect (tabBar (), SIGNAL (customContextMenuRequested(const QPoint&)), this, SLOT (showContextMenu(const QPoint&)));

	KAcceleratorManager::setNoAccel (tabBar ());	// TODO: This is a WORKAROUND for a bug in kdelibs where tabs named "a0.txt", "a1.txt", etc. will steal the Alt+0/1... shortcuts
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

void RKWorkplaceView::addWindow (RKMDIWindow *widget) {
	RK_TRACE (APP);

	int id = -1;
	setUpdatesEnabled (false);

	QIcon icon = widget->windowIcon ();
	if (icon.isNull ()) icon = widget->topLevelWidget ()->windowIcon ();
	if (icon.isNull ()) RK_ASSERT (false);

	id = addTab (widget, icon, widget->shortCaption ());

	connect (widget, SIGNAL (captionChanged (RKMDIWindow *)), this, SLOT (childCaptionChanged (RKMDIWindow *)));
	widget->show ();

	if (count () > 1) setTabBarHidden (false);

	setCurrentIndex (id);		// activate the new tab

	setUpdatesEnabled (true);
}

bool RKWorkplaceView::hasWindow (RKMDIWindow *widget) {
	return (indexOf (widget) != -1);
}

void RKWorkplaceView::removeWindow (RKMDIWindow *widget, bool destroyed) {
	RK_TRACE (APP);

	setUpdatesEnabled (false);

	int id = indexOf (widget);		// which page is it?
	RK_DO (if (id == -1) qDebug ("did not find page in RKWorkplaceView::removeWindow"), APP, DL_WARNING);
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

	KMenu* m = new KMenu (this);
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


#include "rkworkplaceview.moc"
