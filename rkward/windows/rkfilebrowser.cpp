/***************************************************************************
                          rkfilebrowser  -  description
                             -------------------
    begin                : Thu Apr 26 2007
    copyright            : (C) 2007 by Thomas Friedrichsmeier
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

#include "rkfilebrowser.h"

#include <kdiroperator.h>

#include <qdir.h>
#include <qlayout.h>
#include <qvbox.h>

#include "../misc/rkdummypart.h"

#include "../debug.h"

RKFileBrowser::RKFileBrowser (QWidget *parent, bool tool_window, char *name) : RKMDIWindow (parent, FileBrowserWindow, tool_window, name) {
	RK_TRACE (APP);

	real_widget = 0;

	QVBoxLayout *layout = new QVBoxLayout (this);
	wrapper = new QVBox (this);
	wrapper->setFocusPolicy (QWidget::StrongFocus);
	layout->addWidget (wrapper);

	RKDummyPart *part = new RKDummyPart (this, wrapper);
	setPart (part);
	initializeActivationSignals ();
}

RKFileBrowser::~RKFileBrowser () {
	RK_TRACE (APP);

	hide ();
}

void RKFileBrowser::hide () {
	RK_TRACE (APP);

	if (real_widget) {
		real_widget->close (false);
	}
}

void RKFileBrowser::show () {
	RK_TRACE (APP);

	if (!real_widget) {
		RK_DO (qDebug ("creating file browser"), APP, DL_INFO);

		real_widget = new KDirOperator (QDir::currentDirPath (), wrapper);
		real_widget->setView(KFile::Default);
		real_widget->setPreviewWidget (0);
		wrapper->setFocusProxy (real_widget);
	}

	real_widget->show ();
	RKMDIWindow::show ();
}

void RKFileBrowser::currentWDChanged () {
	RK_TRACE (APP);
}

#include "rkfilebrowser.moc"
