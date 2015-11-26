/***************************************************************************
                          rkdynamicsearchline  -  description
                             -------------------
    begin                : Mon Nov 16 2015
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

#ifndef RKDYNAMICSEARCHLINE_H
#define RKDYNAMICSEARCHLINE_H

#include <QLineEdit>

#include <QTimer>

class QSortFilterProxyModel;

/** This class is mostly like KFilterProxySearchLine, except allowing us to filter using regexps.
 *  Also some internal differences, due to the fact that we don't have to hide implementation details as in a framework lib. */
class RKDynamicSearchLine : public QLineEdit {
	Q_OBJECT
public:
	RKDynamicSearchLine (QWidget *parent);
	virtual ~RKDynamicSearchLine ();

/** If a model is set, will call setFilterRegExp() when the search string is changed. */
	void setModelToFilter (QSortFilterProxyModel* _model) { model = _model; };
	QString regexpTip () const;
signals:
	void searchChanged (const QRegExp& search);
private slots:
	void textChanged ();
	void delayedSearch ();
private:
	QTimer timer;
	QSortFilterProxyModel *model;
};

#endif
