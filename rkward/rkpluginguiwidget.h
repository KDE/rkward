/***************************************************************************
                          rkpluginguiwidget  -  description
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
#ifndef RKPLUGINGUIWIDGET_H
#define RKPLUGINGUIWIDGET_H

#include <qwidget.h>

class RKPlugin;

/**
The GUI/Widget constructed by a Plugin.
The purpose of using this class instead of plain QWidget is to notify the RKPlugin when the window has been closed.

@author Thomas Friedrichsmeier
*/
class RKPluginGUIWidget : public QWidget {
Q_OBJECT
public:
    RKPluginGUIWidget(RKPlugin *parent);

    ~RKPluginGUIWidget();
protected:
	void closeEvent (QCloseEvent *e);
private:
	RKPlugin *plugin;
};

#endif
