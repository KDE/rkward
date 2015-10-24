/***************************************************************************
                          rkaccordiontable  -  description
                             -------------------
    begin                : Fri Oct 24 2015
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

#ifndef RKACCORDIONTABLE_H
#define RKACCORDIONTABLE_H

#include <QWidget>
#include <QTableView>
#include <QAbstractProxyModel>

class QAbstractItemModel;

class RKAccordionTable : public QTableView {
	Q_OBJECT
public:
	RKAccordionTable (QWidget *parent);
	~RKAccordionTable ();

	QWidget *defaultWidget () const { return default_widget; };

	void setModel (QAbstractItemModel *model);
protected:
	void currentChanged (const QModelIndex& current, const QModelIndex& previous);   // reimplemented to adjust selection / activate correct row
	void resizeEvent (QResizeEvent* event);                                          // reimplemented to make the current content widget stretch / shrink
	void rowsAboutToBeRemoved (const QModelIndex& parent, int start, int end);       // reimplemented to switch to a different row, if current row is about to be removed
private:
	QWidget *default_widget;
	QAbstractProxyModel *pmodel;
};

#endif
