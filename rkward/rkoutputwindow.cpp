/***************************************************************************
                          rkoutputwindow  -  description
                             -------------------
    begin                : Tue Jul 27 2004
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
#include "rkoutputwindow.h"

#include <qlayout.h>
#include <qtextbrowser.h>

#include <klocale.h>

RKOutputWindow::RKOutputWindow (QWidget *parent, const char *name) : QWidget (parent, name) {
	QGridLayout *grid = new QGridLayout (this, 1, 1, 1, 6);
	browser = new QTextBrowser (this);
	grid->addWidget (browser, 0, 0);
	
	setCaption (i18n ("Output Window"));
}


RKOutputWindow::~RKOutputWindow () {
}

void RKOutputWindow::checkNewInput () {
	qDebug ("checkNewInput");
	browser->setSource ("/home/thomas/develop/rkward/rkward/rk_out.html");
	browser->reload ();
	browser->scrollToBottom ();
}

#include "rkoutputwindow.moc"
