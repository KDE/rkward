/*
rkdynamicsearchline - This file is part of the RKWard project. Created: Mon Nov 16 2015
SPDX-FileCopyrightText: 2015-2016 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef RKDYNAMICSEARCHLINE_H
#define RKDYNAMICSEARCHLINE_H

#include <QLineEdit>
#include <QRegularExpression>

#include <QTimer>

class QSortFilterProxyModel;
class QAction;

/** This class is mostly like KFilterProxySearchLine, except allowing us to filter using regexps.
 *  Also some internal differences, due to the fact that we don't have to hide implementation details as in a framework lib. */
class RKDynamicSearchLine : public QLineEdit {
	Q_OBJECT
public:
	explicit RKDynamicSearchLine (QWidget *parent);
	virtual ~RKDynamicSearchLine ();

/** If a model is set, will call setFilterRegExp() when the search string is changed. */
	void setModelToFilter (QSortFilterProxyModel* _model) { model = _model; };
	QString regexpTip () const;
Q_SIGNALS:
	void searchChanged (const QRegularExpression& search);
private Q_SLOTS:
	void textChanged ();
	void delayedSearch ();
private:
	QTimer timer;
	QAction *working_indicator;
	QSortFilterProxyModel *model;
};

#endif
