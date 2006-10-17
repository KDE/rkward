/***************************************************************************
                          rkworkplaceview  -  description
                             -------------------
    begin                : Tue Sep 26 2006
    copyright            : (C) 2006 by Thomas Friedrichsmeier
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

#include <qwidgetstack.h>
#include <qapplication.h>
#include <qevent.h>
#include <qlayout.h>
#include <qiconset.h>

#include "rkmdiwindow.h"

#include "../debug.h"

RKWorkplaceView::RKWorkplaceView (QWidget *parent) : QWidget (parent) {
	RK_TRACE (APP);

	QVBoxLayout *vbox = new QVBoxLayout (this);
	tabs = new KTabBar (this);
	tabs->setHoverCloseButton (true);
	tabs->setFocusPolicy (QWidget::NoFocus);
	tabs->hide ();
	connect (tabs, SIGNAL (selected (int)), this, SLOT (setPage (int)));
	connect (tabs, SIGNAL (closeRequest (int)), this, SLOT (closePage (int)));
	vbox->addWidget (tabs);

	widgets = new QWidgetStack (this);
	vbox->addWidget (widgets);
}

RKWorkplaceView::~RKWorkplaceView () {
	RK_TRACE (APP);
}

void RKWorkplaceView::addPage (RKMDIWindow *widget) {
	RK_TRACE (APP);

	widgets->addWidget (widget);
	int id = tabs->addTab (new QTab (*(widget->icon ()), widget->shortCaption ()));
	pages.insert (id, widget);
	connect (widget, SIGNAL (captionChanged (RKMDIWindow *)), this, SLOT (childCaptionChanged (RKMDIWindow *)));
	widget->show ();

	if (!tabs->isShown ()) {
		if (tabs->count () > 1) tabs->show ();
	}

	setPage (id);		// active new window
}

void RKWorkplaceView::removePage (RKMDIWindow *widget, bool destroyed) {
	RK_TRACE (APP);

	int id = idOfWidget (widget);		// which page is it?
	RK_DO (if (id == -1) qDebug ("did not find page in RKWorkplaceView::removePage"), APP, DL_WARNING);
	if (!destroyed) disconnect (widget, SIGNAL (captionChanged (RKMDIWindow *)), this, SLOT (childCaptionChanged (RKMDIWindow *)));

	int oldindex = tabs->indexOf (tabs->currentTab ());	// which page will have to be activated later?
	int oldcount = tabs->count ();
	QTab *new_tab = tabs->tabAt (oldindex);
	if (widget == activePage ()) {
		if (oldindex >= 1) {
			new_tab = tabs->tabAt (oldindex - 1);
		} else if (oldindex < (oldcount - 1)) {
			new_tab = tabs->tabAt (oldindex + 1);
		} else {
			new_tab = 0;
		}
	}

	widgets->removeWidget (widget);			// remove
	tabs->removeTab (tabs->tab (id));
	pages.remove (id);

	if (oldcount <= 2) tabs->hide ();		// activate next page
	if (new_tab == 0) {
		RK_ASSERT (oldcount == 1);
		setCaption (QString ());
		emit (pageChanged (0));
	} else {
		//tabs->setCurrentTab (new_tab); 	// somehome this version is NOT safe! (tabbar fails to emit signal?)
		setPage (new_tab->identifier ());
	}
}

void RKWorkplaceView::setActivePage (RKMDIWindow *widget) {
	RK_TRACE (APP);

	int id = idOfWidget (widget);
	RK_DO (if (id == -1) qDebug ("did not find page in RKWorkplaceView::setActivePage"), APP, DL_WARNING);

	tabs->setCurrentTab (id);
}

RKMDIWindow *RKWorkplaceView::activePage () {
	RK_TRACE (APP);
	RK_DO (qDebug ("active page %d: %d, visible: %d", tabs->currentTab (), (int) pages[tabs->currentTab ()], (int) widgets->visibleWidget ()), APP, DL_DEBUG);

	if (tabs->currentTab () == -1) return 0;
	// The assert below can in fact fail temporarily, as the widgetstack (widgets) does not update immediately after widgets->raiseWidget ().
	//RK_ASSERT (pages[tabs->currentTab ()] == widgets->visibleWidget ());
	return (pages[tabs->currentTab ()]);
}

QString RKWorkplaceView::activeCaption () {
	RK_TRACE (APP);

	RKMDIWindow *window = activePage ();
	if (!window) return QString ();
	return window->shortCaption ();
}

void RKWorkplaceView::closePage (int index) {
	RK_TRACE (APP);
	int page = tabs->tabAt (index)->identifier ();
	RK_ASSERT (pages.find (page) != pages.end ());

	RKMDIWindow *window = pages[page];
	window->close (true);
}

void RKWorkplaceView::setPage (int page) {
	RK_TRACE (APP);
	RK_ASSERT (pages.find (page) != pages.end ());

	if (tabs->currentTab () != page) {
		tabs->setCurrentTab (page);
		return;		// will get here again via signal from tabs
	}

	RK_DO (qDebug ("setting page %d: %d", page, (int) pages[page]), APP, DL_DEBUG);
	RKMDIWindow *window = pages[page];
	widgets->raiseWidget (window);

	window->setFocus ();

	emit (pageChanged (window));
	setCaption (window->shortCaption ());
}

void RKWorkplaceView::childCaptionChanged (RKMDIWindow *widget) {
	RK_TRACE (APP);

	int id = idOfWidget (widget);
	QTab *tab = tabs->tab (id);
	RK_ASSERT (tab);
	tab->setText (widget->shortCaption ());
	if (id == tabs->currentTab ()) setCaption (widget->shortCaption ());
}

int RKWorkplaceView::idOfWidget (RKMDIWindow *widget) {
	RK_TRACE (APP);

	for (PageMap::const_iterator it = pages.constBegin (); it != pages.constEnd (); ++it) {
		if (it.data () == widget) return it.key ();
	}

	return -1;
}

void RKWorkplaceView::setCaption (const QString &caption) {
	RK_TRACE (APP);

	QWidget::setCaption (caption);
	emit (captionChanged (caption));
}

#include "rkworkplaceview.moc"
