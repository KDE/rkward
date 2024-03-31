/*
rcontrolwindow - This file is part of the RKWard project. Created: Wed Oct 12 2005
SPDX-FileCopyrightText: 2005-2009 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef RCONTROLWINDOW_H
#define RCONTROLWINDOW_H

#include <qlabel.h>
#include <QTreeView>

#include "rkmdiwindow.h"

class QPushButton;
class RCommand;
class RCommandChain;
class RChainOrCommand;
class RControlWindowListViewItem;

/**
	\brief Interface to control R command execution

This class provides a GUI interface to inspect, and manipulate the current RCommandStack, and to Pause/Resume the R engine.

@author Thomas Friedrichsmeier
*/
class RControlWindow : public RKMDIWindow {
	Q_OBJECT
public:
/** constructor.
@param parent parent QWidget, usually RKGlobals::rkApp () or similar */
	RControlWindow (QWidget *parent, bool tool_window, const char *name=nullptr);
/** destructor */
	~RControlWindow ();

/** reimplemented to start listening to the RCommandStackModel when showing. */
	void showEvent (QShowEvent *e) override;
/** when hidden, disconnect from the RCommandStackModel to save resources */
	void hideEvent (QHideEvent *e) override;
/** Static reference to the control window */
	static RControlWindow* getControl () { return control_window; };
public Q_SLOTS:
/** cancel button was clicked. Cancel selected commands (unless they are RCommand::Sync). */
	void cancelButtonClicked ();
/** pause button was clicked. Pause/Resume processing of the stack */
	void pauseButtonClicked ();
/** configure button was clicked. Invoke settings dialog */
	void configureButtonClicked ();
private:
	QTreeView *commands_view;
	QPushButton *cancel_button;
	QPushButton *pause_button;

	bool paused;
friend class RKWardMainWindow;
	static RControlWindow *control_window;
};

#endif
