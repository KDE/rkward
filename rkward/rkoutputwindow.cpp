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
#include <qpopupmenu.h>
#include <qfile.h>

#include <klocale.h>
#include <kmenubar.h>

#include "settings/rksettingsmodulelogfiles.h"
#include "settings/rksettings.h"

#include "debug.h"

RKOutputWindow::RKOutputWindow (QWidget *parent) : RKToggleWidget (parent) {
	RK_TRACE (OUTPUT);
	QGridLayout *grid = new QGridLayout (this, 2, 1);
	browser = new QTextBrowser (this);
	grid->addWidget (browser, 1, 0);
	
	setCaption (i18n ("Output Window"));

// create menu-bar
	KMenuBar *menu = new KMenuBar (this);
	QPopupMenu *output_menu = new QPopupMenu (this);
	output_menu->setItemEnabled (output_menu->insertItem (i18n ("&Print Output"), 0, 0), false);
	output_menu->setItemEnabled (output_menu->insertItem (i18n ("Export Output"), 0, 0), false);
	output_menu->insertItem (i18n ("&Clear Output"), this, SLOT (clearOutput ()));
	output_menu->insertSeparator ();
	output_menu->insertItem (i18n ("Configure"), this, SLOT (configureOutput ()));
	menu->insertItem (i18n ("Output"), output_menu);
	
	QPopupMenu *help_menu = new QPopupMenu (this);
	help_menu->setItemEnabled (help_menu->insertItem (i18n ("Sorry, no help available so far"), 0, 0), false);
	menu->insertItem (i18n ("Help"), help_menu);

	grid->addWidget (menu, 0, 0);
}


RKOutputWindow::~RKOutputWindow () {
	RK_TRACE (OUTPUT);
}

void RKOutputWindow::checkNewInput () {
	RK_TRACE (OUTPUT);
	browser->setSource (RKSettingsModuleLogfiles::filesPath() + "/rk_out.html");
	browser->reload ();
	browser->scrollToBottom ();
}

void RKOutputWindow::configureOutput () {
	RK_TRACE (OUTPUT);
	RKSettings::configureSettings (RKSettings::Output, this);
}

void RKOutputWindow::clearOutput () {
	RK_TRACE (OUTPUT);
	QFile out_file (RKSettingsModuleLogfiles::filesPath () + "/rk_out.html");
	out_file.remove ();
	checkNewInput ();
}

#include "rkoutputwindow.moc"
