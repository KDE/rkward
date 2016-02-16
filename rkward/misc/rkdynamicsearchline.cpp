/***************************************************************************
                          rkdynamicsearchline  -  description
                             -------------------
    begin                : Mon Nov 16 2015
    copyright            : (C) 2015-2016 by Thomas Friedrichsmeier
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

#include "rkdynamicsearchline.h"

#include <klocale.h>

#include <QSortFilterProxyModel>
#include <QAction>

#include "rkstandardicons.h"

#include "../debug.h"

RKDynamicSearchLine::RKDynamicSearchLine (QWidget *parent) : QLineEdit (parent) {
	RK_TRACE (MISC);

	model = 0;
	setClearButtonEnabled (true);
	setPlaceholderText (i18n ("Search"));
	timer.setSingleShot (true);
	connect (&timer, &QTimer::timeout, this, &RKDynamicSearchLine::delayedSearch);
	connect (this, &QLineEdit::textChanged, this, &RKDynamicSearchLine::textChanged);
	working_indicator = new QAction (this);
	working_indicator->setIcon (RKStandardIcons::getIcon (RKStandardIcons::StatusWaitingUpdating));
}

RKDynamicSearchLine::~RKDynamicSearchLine () {
	RK_TRACE (MISC);
}

void RKDynamicSearchLine::textChanged () {
	RK_TRACE (MISC);

	if (!timer.isActive ()) {
		addAction (working_indicator, QLineEdit::TrailingPosition);
	}
	timer.start (300);
}

QString RKDynamicSearchLine::regexpTip () const {
	return (i18n ("<p><b>Note:</b> This search line accepts so-called regular expressions. To limit the search to matches at the start of the string, start the filter with '^', e.g. ('^rk.'). To limit searches to the end of the string, append '$' at the end of the filter. To match arbitrary text in the middle of the search term, insert '.*'.</p>"));
}

void RKDynamicSearchLine::delayedSearch () {
	RK_TRACE (MISC);

	QString term = text ();
	bool allnum = true;
	for (int i = 0; i < term.size (); ++i) {
		if (term.at (i).isLetterOrNumber ()) {
			allnum = false;
			break;
		}
	}

	QRegExp filter (term, Qt::CaseInsensitive, allnum ? QRegExp::FixedString : QRegExp::RegExp2);
	if (model) model->setFilterRegExp (filter);
	removeAction (working_indicator);
	emit (searchChanged (filter));
}
