/***************************************************************************
                          rkfindbar  -  description
                             -------------------
    begin                : Tue Feb 24 2015
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

#include "rkfindbar.h"

#include <QHBoxLayout>
#include <QToolButton>
#include <QCheckBox>
#include <QLineEdit>

#include <khistorycombobox.h>
#include <KLocalizedString>
#include <QIcon>

#include "rkcommonfunctions.h"

#include "../debug.h"

RKFindBar::RKFindBar (QWidget* parent, bool custom) : QWidget (parent) {
	RK_TRACE (APP);

	mlayout = new QHBoxLayout (this);
	mlayout->setContentsMargins (0, 0, 0, 0);
	QToolButton* close_button = new QToolButton (this);
	close_button->setIcon (QIcon::fromTheme("dialog-close"));
	close_button->setAutoRaise (true);   // makes it flat
	connect (close_button, &QToolButton::clicked, this, &RKFindBar::hide);
	mlayout->addWidget (close_button);

	QHBoxLayout* slayout = new QHBoxLayout ();
	mlayout->addLayout (slayout, 1);
	slayout->setContentsMargins (0, 0, 0, 0);
	slayout->setSpacing (0);
	term_edit = new KHistoryComboBox (this);
	term_edit->setMaximumWidth (fontMetrics ().width ("This is quite a long search term by any standard, indeed"));
	term_edit->setMinimumWidth (fontMetrics ().width ("A short search term"));
	connect (term_edit, &KHistoryComboBox::editTextChanged, this, &RKFindBar::searchChanged);
	connect (term_edit, static_cast<void (KHistoryComboBox::*)(const QString&)>(&KHistoryComboBox::returnPressed), this, &RKFindBar::forward);
	regular_palette = term_edit->palette ();
	nomatch_palette = regular_palette;
	nomatch_palette.setColor (QPalette::Text, QColor (255, 0, 0));
	slayout->addWidget (term_edit, 1);

	QToolButton* backward_button = new QToolButton (this);
	backward_button->setArrowType (Qt::UpArrow);
	backward_button->setContentsMargins (0, 0, 0, 0);
	RKCommonFunctions::setTips (i18n ("Search backwards (previous occurrence of search term)"), backward_button);
	connect (backward_button, &QToolButton::clicked, this, &RKFindBar::backward);
	slayout->addWidget (backward_button);
	QToolButton* forward_button = new QToolButton (this);
	forward_button->setArrowType (Qt::DownArrow);
	forward_button->setContentsMargins (0, 0, 0, 0);
	RKCommonFunctions::setTips (i18n ("Search forward (next occurrence of search term)"), forward_button);
	connect (forward_button, &QToolButton::clicked, this, &RKFindBar::forward);
	slayout->addWidget (forward_button);

	mlayout->addSpacing (15);

	if (!custom) {
		setPrimaryOptions (QList<QWidget*> () << getOption (MatchCase) << getOption (FindAsYouType) << getOption (HighlightAll));
	}
}

RKFindBar::~RKFindBar () {
	RK_TRACE (APP);
}

void RKFindBar::setPrimaryOptions (const QList<QWidget*>& options) {
	RK_TRACE (APP);

	for (int i = 0; i < options.size (); ++i) {
		mlayout->addWidget (options[i]);
	}
	mlayout->addStretch ();
}

QCheckBox* RKFindBar::getOption (const RKFindBar::FindOptions option) {
	RK_TRACE (APP);

	if (!default_actions.contains (option)) {
		QCheckBox* action;
		if (option == MatchCase) {
			action = new QCheckBox (i18n ("Match case"), this);
		} else if (option == FindAsYouType) {
			action = new QCheckBox (i18n ("Find as you type"), this);
			action->setChecked (true);
		} else if (option == HighlightAll) {
			action = new QCheckBox (i18n ("Highlight all matches"), this);
		} else {
			RK_ASSERT (false);
		}
		connect (action, &QCheckBox::stateChanged, this, &RKFindBar::searchChanged);
		default_actions.insert (option, action);
	}

	return (default_actions[option]);
}

bool RKFindBar::isOptionSet (const RKFindBar::FindOptions option) const {
	if (!default_actions.contains (option)) return false;
	return default_actions[option]->isChecked ();
}

void RKFindBar::searchChanged () {
	RK_TRACE (APP);
	term_edit->lineEdit ()->setPalette (regular_palette);
	term_edit->lineEdit ()->setFocus ();
	if (default_actions.contains (FindAsYouType) && default_actions[FindAsYouType]->isChecked ()) forward ();
}

void RKFindBar::forward () {
	RK_TRACE (APP);
	doSearch (false);
}

void RKFindBar::backward () {
	RK_TRACE (APP);
	doSearch (true);
}

void RKFindBar::doSearch (bool backward) {
	RK_TRACE (APP);
	show ();
	bool found = false;
	QString term = term_edit->currentText ();
	findRequest (term, backward, this, &found);
	if (!(found || term.isEmpty ())) term_edit->lineEdit ()->setPalette (nomatch_palette);
}

void RKFindBar::activate () {
	RK_TRACE (APP);

	show ();
	term_edit->lineEdit ()->selectAll ();
	term_edit->lineEdit ()->setFocus ();
}

void RKFindBar::activateWithFindAsYouType () {
	RK_TRACE (APP);

	getOption (FindAsYouType)->setChecked (true);
	activate ();
}


