/***************************************************************************
                          rkdebugmessagewindow  -  description
                             -------------------
    begin                : Sat Dec 01 2012
    copyright            : (C) 2012 by Thomas Friedrichsmeier
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

#ifndef RKDEBUGMESSAGEWINDOW_H
#define RKDEBUGMESSAGEWINDOW_H

#include "rkmdiwindow.h"

class RKDebugMessageWindowWidget;
class QTreeWidget;

/** Tool window for displaying RKWard debug messages. Mainly targetted at plugin
 * developers. */
class RKDebugMessageWindow : public RKMDIWindow {
public:
	RKDebugMessageWindow (QWidget *parent, bool tool_window, const char *name=0);
	~RKDebugMessageWindow ();

/** reimplemented to create the real widget only when the viewer is shown */
	void showEvent (QShowEvent *e);
/** reimplemented to discard the real widget only when the viewer is hidden */
	void hideEvent (QHideEvent *e);
	static RKDebugMessageWindow *instance () { return _instance; };
	static void newMessage (const int flags, const int level, const QString &message);
private:
	void createWidget ();
	void discardWidget ();
	RKDebugMessageWindowWidget *real_widget;
	bool first;
	QWidget *layout_widget;
friend class RKWardMainWindow;
	static RKDebugMessageWindow *_instance;
};

/** The internal widget used in RKDebugMessageWindow
*/
class RKDebugMessageWindowWidget : public QWidget {
public:
	RKDebugMessageWindowWidget (QWidget *parent);
	~RKDebugMessageWindowWidget ();

	void newMessage (const int flags, const int level, const QString &message);
private:
	QTreeWidget *message_viewer;
};

#endif
