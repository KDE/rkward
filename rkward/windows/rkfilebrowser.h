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

#ifndef RKFILEBROWSER_H
#define RKFILEBROWSER_H

#include "rkmdiwindow.h"

class KDirOperator;
class QVBox;

class RKFileBrowser : public RKMDIWindow {
	Q_OBJECT
public:
	RKFileBrowser (QWidget *parent, bool tool_window, char *name=0);
	~RKFileBrowser ();

/** reimplemented to create the real file browser widget only when the file browser is shown for the first time */
	void show ();
/** reimplemented to call close on the browser widget */
	void hide ();
public slots:
	void currentWDChanged ();
private:
	KDirOperator *real_widget;
	QVBox *wrapper;
};

#endif
