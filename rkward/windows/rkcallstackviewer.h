/***************************************************************************
                          rkcallstackviewer  -  description
                             -------------------
    begin                : Wed Oct 19 2011
    copyright            : (C) 2011 by Thomas Friedrichsmeier
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

#ifndef RKCALLSTACKVIEWER_H
#define RKCALLSTACKVIEWER_H

#include "rkmdiwindow.h"

class RKCallstackViewerWidget;
class RKCommandEditorWindow;
class QListWidget;
class QLabel;

/** The call stack (tool) window. In order to save some startup time, the widget is not really created until it is first shown. Hence, this is mostly just a wrapper around RKCallstackViewerWidget */
class RKCallstackViewer : public RKMDIWindow {
	Q_OBJECT
public:
	RKCallstackViewer (QWidget *parent, bool tool_window, const char *name=0);
	~RKCallstackViewer ();

/** reimplemented to create the real widget only when the viewer is shown for the first time */
	void showEvent (QShowEvent *e);
	static RKCallstackViewer *instance () { return _instance; };
public slots:
	void newDebugState ();
private:
	void createRealWidget ();
	RKCallstackViewerWidget *real_widget;
	QWidget *layout_widget;
friend class RKWardMainWindow;
	static RKCallstackViewer *_instance;
};

/** The internal widget used in RKCallstackViewer 
*/
class RKCallstackViewerWidget : public QWidget {
	Q_OBJECT
public:
	RKCallstackViewerWidget (QWidget *parent);
	~RKCallstackViewerWidget ();

	void updateState ();
private slots:
	void frameChanged (int frame_number);
private:
	QListWidget *frame_selector;
	QLabel *frame_info;
	RKCommandEditorWindow *frame_source;
};

#endif
