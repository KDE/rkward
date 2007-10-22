/***************************************************************************
                          rcontrolwindow  -  description
                             -------------------
    begin                : Wed Oct 12 2005
    copyright            : (C) 2005, 2007 by Thomas Friedrichsmeier
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

#ifndef RCONTROLWINDOW_H
#define RCONTROLWINDOW_H

#include <kparts/part.h>

#include <qlabel.h>
#include <QTreeView>

#include "rkmdiwindow.h"

class QPushButton;
class RCommand;
class RCommandChain;
class RChainOrCommand;
class RControlWindowListViewItem;
class RControlWindowPart;

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
	RControlWindow (QWidget *parent, bool tool_window, const char *name=0);
/** destructor */
	~RControlWindow ();

/** reimplemented to start listening to the RCommandStackModel when showing. */
	void showEvent (QShowEvent *e);
/** when hidden, disconnect from the RCommandStackModel to save ressources */
	void hideEvent (QHideEvent *e);
/** Static reference to the control window */
	static RControlWindow* getControl () { return control_window; };
public slots:
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

/**
	\brief Part interface to RControlWindow

Part interface to RControlWindow

@author Thomas Friedrichsmeier
*/
class RControlWindowPart : public KParts::Part {
	Q_OBJECT
friend class RControlWindow;
protected:
/** constructor. */
	RControlWindowPart (RControlWindow *widget);
/** destructor */
	~RControlWindowPart ();
};

#endif
