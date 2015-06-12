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
#include <QPushButton>

class RKTabSlideBar;
class RKTabSlide : public QWidget {

private:
	RKTabSlideBar *tab_bar;
	QSplitter *splitter;
	QWidget *content;
};

class RKTabButton;
class RKTabSlideBar : public QScrollArea {
	Q_OBJECT
public:
	RKTabSlideBar (QWidget* parent);

	void insertTab (int index, const QString &short_label, const QString &long_label, const QString &tip);
	void changeTab (int index, const QString &short_label, const QString &long_label, const QString &tip);
	void removeTab (int index);
signals:
	void deleteClicked (int index);
	void insertClicked (int index);
	void activated (int index);
private:
friend class RKTabButton;
	void deleteClicked (RKTabButton*);
	void insertClicked (RKTabButton*);
	void editClicked (RKTabButton*);
	int currentindex;
	QList<RKTabButton*> tabs;
	QWidget* vbox;
	QVBoxLayout *vlayout;
};

class RKTabButton : public QPushButton {
	Q_OBJECT
private:
friend class RKTabSlideBar;
	RKTabButton (QWidget *parent, RKTabSlideBar *bar);

	void change (const QString &short_label, const QString &long_label);
	void setWide (bool make_wide);
	enum InternalButtons {
		Delete,
		Insert,
		Edit
	};
	void setInternalButtons (int buttons);
protected:
	void paintEvent (QPaintEvent *event);
private:
	QString short_label;
	QString long_label;
private slots:
	void buttonClicked (); // To relay clicked()-signal without additional mapper
	int index;
};

#endif


