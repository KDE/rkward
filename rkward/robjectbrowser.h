/***************************************************************************
                          robjectbrowser  -  description
                             -------------------
    begin                : Thu Aug 19 2004
    copyright            : (C) 2004 by Thomas Friedrichsmeier
    email                : tfry@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef ROBJECTBROWSER_H
#define ROBJECTBROWSER_H

#include <qwidget.h>

class QListView;
class QListViewItem;
class QPushButton;
class RObject;

/**
This widget provides a browsable list of all objects in the R workspace

@author Thomas Friedrichsmeier
*/
class RObjectBrowser : public QWidget
{
Q_OBJECT
public:
    RObjectBrowser ();

    ~RObjectBrowser ();
public slots:
	void updateButtonClicked ();
	void updateComplete (bool changed);
private:
	friend class RKwardApp;
	void initialize ();

	QPushButton *update_button;
	QListView *list_view;
	
	void addObject (QListViewItem *parent, RObject *object);
};

#endif
