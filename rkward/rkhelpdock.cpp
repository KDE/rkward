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

#include <qlayout.h>

#include "khelpdlg.h"

#include "rkhelpdock.h"

RKHelpDock::RKHelpDock(QWidget *parent, const char *name)
 : QWidget(parent, name)
{
	/*dlg = new KHelpDlg();
	pLayout = new QHBoxLayout( this, 0, -1, "layout");
	pLayout->addWidget(dlg);*/
}


RKHelpDock::~RKHelpDock()
{
}


#include "rkhelpdock.moc"
