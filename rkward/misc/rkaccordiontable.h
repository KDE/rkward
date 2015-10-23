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

#ifndef RKTABSLIDE_H
#define RKTABSLIDE_H

#include <QWidget>
#include <QScrollArea>
#include <QTreeView>

class QSplitter;
class RKTabSlideBar;
class RKTabSlide : public QWidget {
	Q_OBJECT
public:
	RKTabSlide (QWidget *parent);

	RKTabSlideBar *tabBar () const { return tab_bar; };
	QWidget *contentArea () const { return content; };
public slots:
	void activate (int index);
private:
	RKTabSlideBar *tab_bar;
	QSplitter *splitter;
	QWidget *content;
	int last_content_width;
};

class RKTabSlideBar : public QTreeView {  // basing on QTreeView, rather than QTableView, as the former is already designed for drawing row by row, rather than cell by cell
	Q_OBJECT
public:
	RKTabSlideBar (QWidget* parent);

	enum InternalButtons {
		Delete,
		Insert,
		Edit
	};
	void setInternalButtons (int buttons);

	void setWide (bool wide_format);
signals:
	void deleteClicked (int index);
	void insertClicked (int index);
	void activated (int index);
protected:
	void drawRow (QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const; // re-implemented from QTreeView
	int sizeHintForRow (int) const { return tab_height; }; // re-implemented from QAbstractItemView: Rows have uniform size. TODO: Guess we also need a QItemDelegate implementing sizeHint().
private:
	int currentindex;
	bool is_wide;
	int buttons;
	int tab_height;
	int tab_vspace;
	int tab_hspace;
};

#endif
