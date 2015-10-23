/***************************************************************************
                          rktabslide  -  description
                             -------------------
    begin                : Fri Jun 22 2015
    copyright            : (C) 2015 by Thomas Friedrichsmeier
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

#include "rktabslide.h"

#include <QSplitter>
#include <QVBoxLayout>
#include <QPainter>
#include <kvbox.h>

#include "../debug.h"

RKTabSlide::RKTabSlide (QWidget* parent) : QWidget (parent) {
	RK_TRACE (MISC);

	new QVBoxLayout (this);
	splitter = new QSplitter (this);
	tab_bar = new RKTabSlideBar (0);
	splitter->addWidget (tab_bar);
	splitter->setCollapsible (0, false);
	content = new KVBox;
	splitter->addWidget (content);
	layout ()->addWidget (splitter);

	last_content_width = -1;

	connect (tab_bar, SIGNAL (activated(int)), this, SLOT (activate(int)));
}

void RKTabSlide::activate (int index) {
	RK_TRACE (MISC);

	if (index < 0) {
		int s = splitter->sizes ().value (1, 0);
		if (s > 0) last_content_width = s;
		splitter->setSizes (QList<int> () << width () << 0);
		tab_bar->setWide (true);
	} else {
		tab_bar->setWide (false);
		int s = last_content_width;
		if (s <= 0) {
			s = content->minimumWidth ();
		}
		splitter->setSizes (QList<int> () << width () - s << s);
	}
}

RKTabSlideBar::RKTabSlideBar (QWidget* parent) : QTreeView (parent) {
	RK_TRACE (MISC);

	buttons = 0;
	setWide (false);

	QStyleOptionTabV3 toption;
	toption.shape = QTabBar::RoundedWest;
	tab_vspace = style()->pixelMetric (QStyle::PM_TabBarTabVSpace, &toption);
	tab_hspace = style()->pixelMetric (QStyle::PM_TabBarTabVSpace, &toption);
	tab_height = fontMetrics ().height () + tab_vspace;

	setItemsExpandable (false);
	setRootIsDecorated (false);
	setSelectionBehavior (QAbstractItemView::SelectRows);
	setSelectionMode (QAbstractItemView::SingleSelection);
}
#error Gaaaaah. Perhaps base on QListView + QItemDelegate w/o clipping, instead?
void RKTabSlideBar::drawRow (QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const {
//	RK_TRACE (MISC);

	QStyleOptionTabV3 toption;
	toption.initFrom (this);
	toption.shape = QTabBar::RoundedWest;
	toption.rect = option.rect;
	toption.rect.moveTop (option.rect.y () + dirtyRegionOffset ().y () + tab_vspace);
	toption.rect.moveLeft (option.rect.x () + dirtyRegionOffset ().x () + tab_hspace);
	toption.text = index.data ().toString ();

	int current_row = currentIndex ().row ();
	if (current_row > 0) {
		int paint_row = index.row ();
		if (current_row == paint_row) toption.state |= QStyle::State_Selected;
		else if (current_row == (paint_row - 1)) toption.selectedPosition = QStyleOptionTabV3::PreviousIsSelected;
		else if (current_row == (paint_row + 1)) toption.selectedPosition = QStyleOptionTabV3::NextIsSelected;
	}

	style ()->drawControl (QStyle::CE_TabBarTabShape, &toption, painter);
	toption.shape = QTabBar::RoundedNorth;
	style ()->drawControl (QStyle::CE_TabBarTabLabel, &toption, painter);
}

void RKTabSlideBar::setWide (bool wide_format) {
	RK_TRACE (MISC);

	is_wide = wide_format;
	setHorizontalScrollBarPolicy (is_wide ? Qt::ScrollBarAsNeeded : Qt::ScrollBarAlwaysOff);
	// TODO: redraw
}


// KF5 TODO: remove:
#include "rktabslide.moc"
