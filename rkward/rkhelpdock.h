/***************************************************************************
                          rkward.h  -  description
                             -------------------
    begin                : Tue Oct 29 20:06:08 CET 2002 
    copyright            : (C) 2002 by Thomas Friedrichsmeier 
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
 
#ifndef RKHELPDOCK_H
#define RKHELPDOCK_H

#include <qwidget.h>

/**
@author Thomas Friedrichsmeier
*/
class RKHelpDock : public QWidget
{
Q_OBJECT
public:
    RKHelpDock(QWidget *parent = 0, const char *name = 0);

    ~RKHelpDock();
    
private:
    QBoxLayout* pLayout;
    //KHelpDlg dlg;

};

#endif
