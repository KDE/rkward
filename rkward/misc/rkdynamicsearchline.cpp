/*
rkdynamicsearchline - This file is part of RKWard (https://rkward.kde.org). Created: Mon Nov 16 2015
SPDX-FileCopyrightText: 2015-2016 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "rkdynamicsearchline.h"

#include <KLocalizedString>

#include <QSortFilterProxyModel>
#include <QAction>

#include "rkstandardicons.h"

#include "../debug.h"

RKDynamicSearchLine::RKDynamicSearchLine (QWidget *parent) : QLineEdit (parent) {
	RK_TRACE (MISC);

	model = nullptr;
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

	QRegularExpression filter (allnum ? QRegularExpression::escape(term) : term, QRegularExpression::CaseInsensitiveOption);
	if (model) model->setFilterRegularExpression (filter);
	removeAction (working_indicator);
	Q_EMIT searchChanged(filter);
}
