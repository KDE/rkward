/*
rkcallstackviewer - This file is part of the RKWard project. Created: Wed Oct 19 2011
SPDX-FileCopyrightText: 2011 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

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
	RKCallstackViewer(QWidget *parent, bool tool_window, const char *name=nullptr);
	~RKCallstackViewer();

/** reimplemented to create the real widget only when the viewer is shown for the first time */
	void showEvent (QShowEvent *e) override;
	static RKCallstackViewer *instance () { return _instance; };
public Q_SLOTS:
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
	explicit RKCallstackViewerWidget (QWidget *parent);
	~RKCallstackViewerWidget ();

	void updateState ();
private Q_SLOTS:
	void frameChanged (int frame_number);
private:
	QListWidget *frame_selector;
	QLabel *frame_info;
	RKCommandEditorWindow *frame_source;
};

#endif
