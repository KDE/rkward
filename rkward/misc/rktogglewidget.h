/***************************************************************************
                          rktogglewidget  -  description
                             -------------------
    begin                : Fri Aug 20 2004
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
#ifndef RKTOGGLEWIDGET_H
#define RKTOGGLEWIDGET_H

#include <qwidget.h>

class QCloseEvent;

/**
Basically just a QWidget which sends a signal when it gets closed. By default this signal is connected to the slotToggleWindowClosed of RKWardApp, so as to keep track of which togglable windows are open and which are closed.

@author Thomas Friedrichsmeier
*/
class RKToggleWidget : public QWidget
{
Q_OBJECT
public:
    RKToggleWidget (QWidget *parent = 0);

    ~RKToggleWidget();
signals:
	void closed ();
protected:
	void closeEvent (QCloseEvent *e);
};

#endif
