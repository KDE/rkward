/*
rkdebugmessagewindow - This file is part of the RKWard project. Created: Sat Dec 01 2012
SPDX-FileCopyrightText: 2012 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef RKDEBUGMESSAGEWINDOW_H
#define RKDEBUGMESSAGEWINDOW_H

#include "rkmdiwindow.h"

class RKDebugMessageWindowWidget;
class QTextEdit;

/** Tool window for displaying RKWard debug messages. Mainly targeted at plugin
 * developers. */
class RKDebugMessageWindow : public RKMDIWindow {
public:
	RKDebugMessageWindow(QWidget *parent, bool tool_window, const char *name=nullptr);
	~RKDebugMessageWindow();

/** reimplemented to create the real widget only when the viewer is shown */
	void showEvent (QShowEvent *e) override;
/** reimplemented to discard the real widget only when the viewer is hidden */
	void hideEvent (QHideEvent *e) override;
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
	explicit RKDebugMessageWindowWidget (QWidget *parent);
	~RKDebugMessageWindowWidget ();

	void newMessage (const int flags, const int level, const QString &message);
private:
	QTextEdit *message_viewer;
};

#endif
