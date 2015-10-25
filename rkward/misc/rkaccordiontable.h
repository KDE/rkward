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
#include <QTreeView>

class QAbstractItemModel;
class QAbstractProxyModel;

class RKAccordionTable : public QTreeView {
	Q_OBJECT
public:
	RKAccordionTable (QWidget *parent);
	~RKAccordionTable ();

	QWidget *defaultWidget () const { return default_widget; };

	void setModel (QAbstractItemModel *model);
public slots:
	void rowExpanded (QModelIndex row);
protected:
	void currentChanged (const QModelIndex& current, const QModelIndex& previous);   // reimplemented to adjust selection / activate correct row
	void resizeEvent (QResizeEvent* event);                                          // reimplemented to make the current content widget stretch / shrink
private:
	QWidget *default_widget;
	QAbstractProxyModel *pmodel;
};

#endif
